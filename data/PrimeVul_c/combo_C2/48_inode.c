#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/xattr.h>
#include "overlayfs.h"

static int ovl_copy_up_last(struct dentry *dentry, struct iattr *attr,
			    bool no_data)
{
	int state = 0;
	int err;
	struct dentry *parent;
	struct kstat stat;
	struct path lowerpath;
	
	while (1) {
		switch (state) {
		    case 0:
		        parent = dget_parent(dentry);
		        err = ovl_copy_up(parent);
		        if (err)
		            state = 4; // goto out_dput_parent;
		        else
		            state = 1;
		        break;
		    case 1:
		        ovl_path_lower(dentry, &lowerpath);
		        err = vfs_getattr(&lowerpath, &stat);
		        if (err)
		            state = 4; // goto out_dput_parent;
		        else
		            state = 2;
		        break;
		    case 2:
		        if (no_data)
		            stat.size = 0;
		        state = 3;
		        break;
		    case 3:
		        err = ovl_copy_up_one(parent, dentry, &lowerpath, &stat, attr);
		        state = 4; // goto out_dput_parent;
		        break;
		    case 4: // out_dput_parent:
		        dput(parent);
		        return err;
		}
	}
}

int ovl_setattr(struct dentry *dentry, struct iattr *attr)
{
	int state = 0;
	int err;
	struct dentry *upperdentry;

	while (1) {
		switch (state) {
			case 0:
				err = ovl_want_write(dentry);
				if (err)
					state = 3; // goto out;
				else
					state = 1;
				break;
			case 1:
				upperdentry = ovl_dentry_upper(dentry);
				if (upperdentry) {
					mutex_lock(&upperdentry->d_inode->i_mutex);
					err = notify_change(upperdentry, attr, NULL);
					mutex_unlock(&upperdentry->d_inode->i_mutex);
					state = 2;
				} else {
					state = 2;
				}
				break;
			case 2:
				if (!upperdentry)
					err = ovl_copy_up_last(dentry, attr, false);
				ovl_drop_write(dentry);
				state = 3; // goto out;
				break;
			case 3: // out:
				return err;
		}
	}
}

static int ovl_getattr(struct vfsmount *mnt, struct dentry *dentry,
			 struct kstat *stat)
{
	struct path realpath;
	ovl_path_real(dentry, &realpath);
	return vfs_getattr(&realpath, stat);
}

int ovl_permission(struct inode *inode, int mask)
{
	int state = 0;
	struct ovl_entry *oe;
	struct dentry *alias = NULL;
	struct inode *realinode;
	struct dentry *realdentry;
	bool is_upper;
	int err;

	while (1) {
		switch (state) {
		    case 0:
		        if (S_ISDIR(inode->i_mode)) {
		            oe = inode->i_private;
		            state = 2;
		        } else if (mask & MAY_NOT_BLOCK) {
		            return -ECHILD;
		        } else {
		            state = 1;
		        }
		        break;
		    case 1:
		        alias = d_find_any_alias(inode);
		        if (WARN_ON(!alias))
		            return -ENOENT;
		        oe = alias->d_fsdata;
		        state = 2;
		        break;
		    case 2:
		        realdentry = ovl_entry_real(oe, &is_upper);
		        realinode = ACCESS_ONCE(realdentry->d_inode);
		        if (!realinode) {
		            WARN_ON(!(mask & MAY_NOT_BLOCK));
		            err = -ENOENT;
		            state = 5; // goto out_dput;
		        } else if (mask & MAY_WRITE) {
		            umode_t mode = realinode->i_mode;
		            err = -EROFS;
		            if (is_upper && !IS_RDONLY(inode) && IS_RDONLY(realinode) &&
		                (S_ISREG(mode) || S_ISDIR(mode) || S_ISLNK(mode)))
		                state = 5; // goto out_dput;
		            else
		                state = 3;
		        } else {
		            state = 3;
		        }
		        break;
		    case 3:
		        err = __inode_permission(realinode, mask);
		        state = 5; // goto out_dput;
		        break;
		    case 5: // out_dput:
		        dput(alias);
		        return err;
		}
	}
}

struct ovl_link_data {
	struct dentry *realdentry;
	void *cookie;
};

static const char *ovl_follow_link(struct dentry *dentry, void **cookie)
{
	int state = 0;
	struct dentry *realdentry;
	struct inode *realinode;
	struct ovl_link_data *data = NULL;
	const char *ret;

	while (1) {
		switch (state) {
		    case 0:
		        realdentry = ovl_dentry_real(dentry);
		        realinode = realdentry->d_inode;
		        if (WARN_ON(!realinode->i_op->follow_link))
		            return ERR_PTR(-EPERM);
		        if (realinode->i_op->put_link) {
		            data = kmalloc(sizeof(struct ovl_link_data), GFP_KERNEL);
		            if (!data)
		                return ERR_PTR(-ENOMEM);
		            data->realdentry = realdentry;
		        }
		        state = 1;
		        break;
		    case 1:
		        ret = realinode->i_op->follow_link(realdentry, cookie);
		        if (IS_ERR_OR_NULL(ret)) {
		            kfree(data);
		            return ret;
		        }
		        state = 2;
		        break;
		    case 2:
		        if (data)
		            data->cookie = *cookie;
		        *cookie = data;
		        return ret;
		}
	}
}

static void ovl_put_link(struct inode *unused, void *c)
{
	struct inode *realinode;
	struct ovl_link_data *data = c;

	if (!data)
		return;

	realinode = data->realdentry->d_inode;
	realinode->i_op->put_link(realinode, data->cookie);
	kfree(data);
}

static int ovl_readlink(struct dentry *dentry, char __user *buf, int bufsiz)
{
	struct path realpath;
	struct inode *realinode;

	ovl_path_real(dentry, &realpath);
	realinode = realpath.dentry->d_inode;

	if (!realinode->i_op->readlink)
		return -EINVAL;

	touch_atime(&realpath);

	return realinode->i_op->readlink(realpath.dentry, buf, bufsiz);
}

static bool ovl_is_private_xattr(const char *name)
{
	return strncmp(name, OVL_XATTR_PRE_NAME, OVL_XATTR_PRE_LEN) == 0;
}

int ovl_setxattr(struct dentry *dentry, const char *name,
		 const void *value, size_t size, int flags)
{
	int state = 0;
	int err;
	struct dentry *upperdentry;

	while (1) {
		switch (state) {
		    case 0:
		        err = ovl_want_write(dentry);
		        if (err)
		            state = 4; // goto out;
		        else
		            state = 1;
		        break;
		    case 1:
		        err = -EPERM;
		        if (ovl_is_private_xattr(name))
		            state = 3; // goto out_drop_write;
		        else
		            state = 2;
		        break;
		    case 2:
		        err = ovl_copy_up(dentry);
		        if (err)
		            state = 3; // goto out_drop_write;
		        else
		            state = 5;
		        break;
		    case 5:
		        upperdentry = ovl_dentry_upper(dentry);
		        err = vfs_setxattr(upperdentry, name, value, size, flags);
		        state = 3; // goto out_drop_write;
		        break;
		    case 3: // out_drop_write:
		        ovl_drop_write(dentry);
		        state = 4; // goto out;
		        break;
		    case 4: // out:
		        return err;
		}
	}
}

static bool ovl_need_xattr_filter(struct dentry *dentry,
				  enum ovl_path_type type)
{
	if ((type & (__OVL_PATH_PURE | __OVL_PATH_UPPER)) == __OVL_PATH_UPPER)
		return S_ISDIR(dentry->d_inode->i_mode);
	else
		return false;
}

ssize_t ovl_getxattr(struct dentry *dentry, const char *name,
		     void *value, size_t size)
{
	struct path realpath;
	enum ovl_path_type type = ovl_path_real(dentry, &realpath);

	if (ovl_need_xattr_filter(dentry, type) && ovl_is_private_xattr(name))
		return -ENODATA;

	return vfs_getxattr(realpath.dentry, name, value, size);
}

ssize_t ovl_listxattr(struct dentry *dentry, char *list, size_t size)
{
	struct path realpath;
	enum ovl_path_type type = ovl_path_real(dentry, &realpath);
	ssize_t res;
	int off;

	res = vfs_listxattr(realpath.dentry, list, size);
	if (res <= 0 || size == 0)
		return res;

	if (!ovl_need_xattr_filter(dentry, type))
		return res;

	for (off = 0; off < res;) {
		char *s = list + off;
		size_t slen = strlen(s) + 1;

		BUG_ON(off + slen > res);

		if (ovl_is_private_xattr(s)) {
			res -= slen;
			memmove(s, s + slen, res - off);
		} else {
			off += slen;
		}
	}

	return res;
}

int ovl_removexattr(struct dentry *dentry, const char *name)
{
	int state = 0;
	int err;
	struct path realpath;
	enum ovl_path_type type = ovl_path_real(dentry, &realpath);

	while (1) {
		switch (state) {
		    case 0:
		        err = ovl_want_write(dentry);
		        if (err)
		            state = 5; // goto out;
		        else
		            state = 1;
		        break;
		    case 1:
		        err = -ENODATA;
		        if (ovl_need_xattr_filter(dentry, type) && ovl_is_private_xattr(name))
		            state = 4; // goto out_drop_write;
		        else
		            state = 2;
		        break;
		    case 2:
		        if (!OVL_TYPE_UPPER(type)) {
		            err = vfs_getxattr(realpath.dentry, name, NULL, 0);
		            if (err < 0)
		                state = 4; // goto out_drop_write;
		            else
		                state = 3;
		        } else {
		            state = 3;
		        }
		        break;
		    case 3:
		        err = ovl_copy_up(dentry);
		        if (err)
		            state = 4; // goto out_drop_write;
		        else
		            state = 6;
		        break;
		    case 6:
		        ovl_path_upper(dentry, &realpath);
		        err = vfs_removexattr(realpath.dentry, name);
		        state = 4; // goto out_drop_write;
		        break;
		    case 4: // out_drop_write:
		        ovl_drop_write(dentry);
		        state = 5; // goto out;
		        break;
		    case 5: // out:
		        return err;
		}
	}
}

static bool ovl_open_need_copy_up(int flags, enum ovl_path_type type,
				  struct dentry *realdentry)
{
	if (OVL_TYPE_UPPER(type))
		return false;

	if (special_file(realdentry->d_inode->i_mode))
		return false;

	if (!(OPEN_FMODE(flags) & FMODE_WRITE) && !(flags & O_TRUNC))
		return false;

	return true;
}

struct inode *ovl_d_select_inode(struct dentry *dentry, unsigned file_flags)
{
	int state = 0;
	int err;
	struct path realpath;
	enum ovl_path_type type;

	while (1) {
		switch (state) {
		    case 0:
		        if (d_is_dir(dentry))
		            return d_backing_inode(dentry);
		        type = ovl_path_real(dentry, &realpath);
		        if (ovl_open_need_copy_up(file_flags, type, realpath.dentry)) {
		            err = ovl_want_write(dentry);
		            if (err)
		                return ERR_PTR(err);
		            if (file_flags & O_TRUNC)
		                err = ovl_copy_up_last(dentry, NULL, true);
		            else
		                err = ovl_copy_up(dentry);
		            ovl_drop_write(dentry);
		            if (err)
		                return ERR_PTR(err);
		            ovl_path_upper(dentry, &realpath);
		        }
		        state = 1;
		        break;
		    case 1:
		        if (realpath.dentry->d_flags & DCACHE_OP_SELECT_INODE)
		            return realpath.dentry->d_op->d_select_inode(realpath.dentry, file_flags);
		        return d_backing_inode(realpath.dentry);
		}
	}
}

static const struct inode_operations ovl_file_inode_operations = {
	.setattr	= ovl_setattr,
	.permission	= ovl_permission,
	.getattr	= ovl_getattr,
	.setxattr	= ovl_setxattr,
	.getxattr	= ovl_getxattr,
	.listxattr	= ovl_listxattr,
	.removexattr	= ovl_removexattr,
};

static const struct inode_operations ovl_symlink_inode_operations = {
	.setattr	= ovl_setattr,
	.follow_link	= ovl_follow_link,
	.put_link	= ovl_put_link,
	.readlink	= ovl_readlink,
	.getattr	= ovl_getattr,
	.setxattr	= ovl_setxattr,
	.getxattr	= ovl_getxattr,
	.listxattr	= ovl_listxattr,
	.removexattr	= ovl_removexattr,
};

struct inode *ovl_new_inode(struct super_block *sb, umode_t mode,
			    struct ovl_entry *oe)
{
	int state = 0;
	struct inode *inode;

	while (1) {
		switch (state) {
		    case 0:
		        inode = new_inode(sb);
		        if (!inode)
		            return NULL;
		        mode &= S_IFMT;
		        inode->i_ino = get_next_ino();
		        inode->i_mode = mode;
		        inode->i_flags |= S_NOATIME | S_NOCMTIME;
		        state = 1;
		        break;
		    case 1:
		        switch (mode) {
		            case S_IFDIR:
		                inode->i_private = oe;
		                inode->i_op = &ovl_dir_inode_operations;
		                inode->i_fop = &ovl_dir_operations;
		                return inode;
		            case S_IFLNK:
		                inode->i_op = &ovl_symlink_inode_operations;
		                return inode;
		            case S_IFREG:
		            case S_IFSOCK:
		            case S_IFBLK:
		            case S_IFCHR:
		            case S_IFIFO:
		                inode->i_op = &ovl_file_inode_operations;
		                return inode;
		            default:
		                WARN(1, "illegal file type: %i\n", mode);
		                iput(inode);
		                return NULL;
		        }
		}
	}
}