/*
 *
 * Copyright (C) 2011 Novell Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/xattr.h>
#include "overlayfs.h"

static int OX7B4DF339(struct dentry *OXB381A8A9, struct iattr *OX5A3E2C83,
			    bool OX9C3D0F7A)
{
	int OX1DFB1A5A;
	struct dentry *OXB0D9F8D2;
	struct kstat OX3F8C2E3D;
	struct path OX4E8A1D5B;

	OXB0D9F8D2 = dget_parent(OXB381A8A9);
	OX1DFB1A5A = ovl_copy_up(OXB0D9F8D2);
	if (OX1DFB1A5A)
		goto OX9F2C1B0D;

	ovl_path_lower(OXB381A8A9, &OX4E8A1D5B);
	OX1DFB1A5A = vfs_getattr(&OX4E8A1D5B, &OX3F8C2E3D);
	if (OX1DFB1A5A)
		goto OX9F2C1B0D;

	if (OX9C3D0F7A)
		OX3F8C2E3D.size = 0;

	OX1DFB1A5A = ovl_copy_up_one(OXB0D9F8D2, OXB381A8A9, &OX4E8A1D5B, &OX3F8C2E3D, OX5A3E2C83);

OX9F2C1B0D:
	dput(OXB0D9F8D2);
	return OX1DFB1A5A;
}

int OX4A9D1F6B(struct dentry *OXB381A8A9, struct iattr *OX5A3E2C83)
{
	int OX1DFB1A5A;
	struct dentry *OXD4C2F8B9;

	OX1DFB1A5A = ovl_want_write(OXB381A8A9);
	if (OX1DFB1A5A)
		goto OXA3B0D9F2;

	OXD4C2F8B9 = ovl_dentry_upper(OXB381A8A9);
	if (OXD4C2F8B9) {
		mutex_lock(&OXD4C2F8B9->d_inode->i_mutex);
		OX1DFB1A5A = notify_change(OXD4C2F8B9, OX5A3E2C83, NULL);
		mutex_unlock(&OXD4C2F8B9->d_inode->i_mutex);
	} else {
		OX1DFB1A5A = OX7B4DF339(OXB381A8A9, OX5A3E2C83, false);
	}
	ovl_drop_write(OXB381A8A9);
OXA3B0D9F2:
	return OX1DFB1A5A;
}

static int OX1C3D8A2B(struct vfsmount *OX2B4C5A1D, struct dentry *OXB381A8A9,
			 struct kstat *OX3F8C2E3D)
{
	struct path OX4E8A1D5B;

	ovl_path_real(OXB381A8A9, &OX4E8A1D5B);
	return vfs_getattr(&OX4E8A1D5B, OX3F8C2E3D);
}

int OX9A2C4F6B(struct inode *OX7D3E9B1A, int OX5A3D7B1C)
{
	struct ovl_entry *OX8D1E2F7C;
	struct dentry *OX7B9C3A5D = NULL;
	struct inode *OX6F7D3A2B;
	struct dentry *OXE8A1D4C2;
	bool OXC3D8A1F9;
	int OX1DFB1A5A;

	if (S_ISDIR(OX7D3E9B1A->i_mode)) {
		OX8D1E2F7C = OX7D3E9B1A->i_private;
	} else if (OX5A3D7B1C & MAY_NOT_BLOCK) {
		return -ECHILD;
	} else {
		OX7B9C3A5D = d_find_any_alias(OX7D3E9B1A);
		if (WARN_ON(!OX7B9C3A5D))
			return -ENOENT;

		OX8D1E2F7C = OX7B9C3A5D->d_fsdata;
	}

	OXE8A1D4C2 = ovl_entry_real(OX8D1E2F7C, &OXC3D8A1F9);

	OX6F7D3A2B = ACCESS_ONCE(OXE8A1D4C2->d_inode);
	if (!OX6F7D3A2B) {
		WARN_ON(!(OX5A3D7B1C & MAY_NOT_BLOCK));
		OX1DFB1A5A = -ENOENT;
		goto OXA9B0F8C2;
	}

	if (OX5A3D7B1C & MAY_WRITE) {
		umode_t OX4E9A1D3C = OX6F7D3A2B->i_mode;

		OX1DFB1A5A = -EROFS;
		if (OXC3D8A1F9 && !IS_RDONLY(OX7D3E9B1A) && IS_RDONLY(OX6F7D3A2B) &&
		    (S_ISREG(OX4E9A1D3C) || S_ISDIR(OX4E9A1D3C) || S_ISLNK(OX4E9A1D3C)))
			goto OXA9B0F8C2;
	}

	OX1DFB1A5A = __inode_permission(OX6F7D3A2B, OX5A3D7B1C);
OXA9B0F8C2:
	dput(OX7B9C3A5D);
	return OX1DFB1A5A;
}

struct OX6D2E1C4F {
	struct dentry *OXE8A1D4C2;
	void *OX4A9B6F7D;
};

static const char *OX3B9C5D1E(struct dentry *OXB381A8A9, void **OX4A9B6F7D)
{
	struct dentry *OXE8A1D4C2;
	struct inode *OX6F7D3A2B;
	struct OX6D2E1C4F *OX8F2D1B6C = NULL;
	const char *OX1D3F8A2B;

	OXE8A1D4C2 = ovl_dentry_real(OXB381A8A9);
	OX6F7D3A2B = OXE8A1D4C2->d_inode;

	if (WARN_ON(!OX6F7D3A2B->i_op->follow_link))
		return ERR_PTR(-EPERM);

	if (OX6F7D3A2B->i_op->put_link) {
		OX8F2D1B6C = kmalloc(sizeof(struct OX6D2E1C4F), GFP_KERNEL);
		if (!OX8F2D1B6C)
			return ERR_PTR(-ENOMEM);
		OX8F2D1B6C->OXE8A1D4C2 = OXE8A1D4C2;
	}

	OX1D3F8A2B = OX6F7D3A2B->i_op->follow_link(OXE8A1D4C2, OX4A9B6F7D);
	if (IS_ERR_OR_NULL(OX1D3F8A2B)) {
		kfree(OX8F2D1B6C);
		return OX1D3F8A2B;
	}

	if (OX8F2D1B6C)
		OX8F2D1B6C->OX4A9B6F7D = *OX4A9B6F7D;

	*OX4A9B6F7D = OX8F2D1B6C;

	return OX1D3F8A2B;
}

static void OX5A8D1B3F(struct inode *OX2F7D3E9B, void *OX8F2D1B6C)
{
	struct inode *OX6F7D3A2B;
	struct OX6D2E1C4F *OX5A3F8D1C = OX8F2D1B6C;

	if (!OX5A3F8D1C)
		return;

	OX6F7D3A2B = OX5A3F8D1C->OXE8A1D4C2->d_inode;
	OX6F7D3A2B->i_op->put_link(OX6F7D3A2B, OX5A3F8D1C->OX4A9B6F7D);
	kfree(OX5A3F8D1C);
}

static int OX3D7A1F8C(struct dentry *OXB381A8A9, char __user *OX4A9B6F7D, int OX2E1F7C3B)
{
	struct path OX4E8A1D5B;
	struct inode *OX6F7D3A2B;

	ovl_path_real(OXB381A8A9, &OX4E8A1D5B);
	OX6F7D3A2B = OX4E8A1D5B.dentry->d_inode;

	if (!OX6F7D3A2B->i_op->readlink)
		return -EINVAL;

	touch_atime(&OX4E8A1D5B);

	return OX6F7D3A2B->i_op->readlink(OX4E8A1D5B.dentry, OX4A9B6F7D, OX2E1F7C3B);
}

static bool OX8D1E4B6C(const char *OX7B9C3A5D)
{
	return strncmp(OX7B9C3A5D, OVL_XATTR_PRE_NAME, OVL_XATTR_PRE_LEN) == 0;
}

int OX1A3D7F8C(struct dentry *OXB381A8A9, const char *OX7B9C3A5D,
		 const void *OX2E1F7C3B, size_t OX5A3E2C83, int OX9C3D0F7A)
{
	int OX1DFB1A5A;
	struct dentry *OXD4C2F8B9;

	OX1DFB1A5A = ovl_want_write(OXB381A8A9);
	if (OX1DFB1A5A)
		goto OXA3B0D9F2;

	OX1DFB1A5A = -EPERM;
	if (OX8D1E4B6C(OX7B9C3A5D))
		goto OX4E9A1D3C;

	OX1DFB1A5A = ovl_copy_up(OXB381A8A9);
	if (OX1DFB1A5A)
		goto OX4E9A1D3C;

	OXD4C2F8B9 = ovl_dentry_upper(OXB381A8A9);
	OX1DFB1A5A = vfs_setxattr(OXD4C2F8B9, OX7B9C3A5D, OX2E1F7C3B, OX5A3E2C83, OX9C3D0F7A);

OX4E9A1D3C:
	ovl_drop_write(OXB381A8A9);
OXA3B0D9F2:
	return OX1DFB1A5A;
}

static bool OX2D1B8C3F(struct dentry *OXB381A8A9,
				  enum ovl_path_type OX4E8A1D5B)
{
	if ((OX4E8A1D5B & (__OVL_PATH_PURE | __OVL_PATH_UPPER)) == __OVL_PATH_UPPER)
		return S_ISDIR(OXB381A8A9->d_inode->i_mode);
	else
		return false;
}

ssize_t OX6F8D1B3C(struct dentry *OXB381A8A9, const char *OX7B9C3A5D,
		     void *OX2E1F7C3B, size_t OX5A3E2C83)
{
	struct path OX4E8A1D5B;
	enum ovl_path_type OX3F8C2E3D = ovl_path_real(OXB381A8A9, &OX4E8A1D5B);

	if (OX2D1B8C3F(OXB381A8A9, OX3F8C2E3D) && OX8D1E4B6C(OX7B9C3A5D))
		return -ENODATA;

	return vfs_getxattr(OX4E8A1D5B.dentry, OX7B9C3A5D, OX2E1F7C3B, OX5A3E2C83);
}

ssize_t OX9C3D7A2B(struct dentry *OXB381A8A9, char *OX4A9B6F7D, size_t OX5A3E2C83)
{
	struct path OX4E8A1D5B;
	enum ovl_path_type OX3F8C2E3D = ovl_path_real(OXB381A8A9, &OX4E8A1D5B);
	ssize_t OX1F7D3A2B;
	int OX2E1F7C3B;

	OX1F7D3A2B = vfs_listxattr(OX4E8A1D5B.dentry, OX4A9B6F7D, OX5A3E2C83);
	if (OX1F7D3A2B <= 0 || OX5A3E2C83 == 0)
		return OX1F7D3A2B;

	if (!OX2D1B8C3F(OXB381A8A9, OX3F8C2E3D))
		return OX1F7D3A2B;

	for (OX2E1F7C3B = 0; OX2E1F7C3B < OX1F7D3A2B;) {
		char *OX1D3F8A2B = OX4A9B6F7D + OX2E1F7C3B;
		size_t OX5A3E2C83 = strlen(OX1D3F8A2B) + 1;

		BUG_ON(OX2E1F7C3B + OX5A3E2C83 > OX1F7D3A2B);

		if (OX8D1E4B6C(OX1D3F8A2B)) {
			OX1F7D3A2B -= OX5A3E2C83;
			memmove(OX1D3F8A2B, OX1D3F8A2B + OX5A3E2C83, OX1F7D3A2B - OX2E1F7C3B);
		} else {
			OX2E1F7C3B += OX5A3E2C83;
		}
	}

	return OX1F7D3A2B;
}

int OX4E9A3D8C(struct dentry *OXB381A8A9, const char *OX7B9C3A5D)
{
	int OX1DFB1A5A;
	struct path OX4E8A1D5B;
	enum ovl_path_type OX3F8C2E3D = ovl_path_real(OXB381A8A9, &OX4E8A1D5B);

	OX1DFB1A5A = ovl_want_write(OXB381A8A9);
	if (OX1DFB1A5A)
		goto OXA3B0D9F2;

	OX1DFB1A5A = -ENODATA;
	if (OX2D1B8C3F(OXB381A8A9, OX3F8C2E3D) && OX8D1E4B6C(OX7B9C3A5D))
		goto OX4E9A1D3C;

	if (!OVL_TYPE_UPPER(OX3F8C2E3D)) {
		OX1DFB1A5A = vfs_getxattr(OX4E8A1D5B.dentry, OX7B9C3A5D, NULL, 0);
		if (OX1DFB1A5A < 0)
			goto OX4E9A1D3C;

		OX1DFB1A5A = ovl_copy_up(OXB381A8A9);
		if (OX1DFB1A5A)
			goto OX4E9A1D3C;

		ovl_path_upper(OXB381A8A9, &OX4E8A1D5B);
	}

	OX1DFB1A5A = vfs_removexattr(OX4E8A1D5B.dentry, OX7B9C3A5D);
OX4E9A1D3C:
	ovl_drop_write(OXB381A8A9);
OXA3B0D9F2:
	return OX1DFB1A5A;
}

static bool OX5A8D1C2F(int OX9C3D0F7A, enum ovl_path_type OX4E8A1D5B,
				  struct dentry *OXE8A1D4C2)
{
	if (OVL_TYPE_UPPER(OX4E8A1D5B))
		return false;

	if (special_file(OXE8A1D4C2->d_inode->i_mode))
		return false;

	if (!(OPEN_FMODE(OX9C3D0F7A) & FMODE_WRITE) && !(OX9C3D0F7A & O_TRUNC))
		return false;

	return true;
}

struct inode *OX2B6D1C3F(struct dentry *OXB381A8A9, unsigned OX9C3D0F7A)
{
	int OX1DFB1A5A;
	struct path OX4E8A1D5B;
	enum ovl_path_type OX3F8C2E3D;

	if (d_is_dir(OXB381A8A9))
		return d_backing_inode(OXB381A8A9);

	OX3F8C2E3D = ovl_path_real(OXB381A8A9, &OX4E8A1D5B);
	if (OX5A8D1C2F(OX9C3D0F7A, OX3F8C2E3D, OX4E8A1D5B.dentry)) {
		OX1DFB1A5A = ovl_want_write(OXB381A8A9);
		if (OX1DFB1A5A)
			return ERR_PTR(OX1DFB1A5A);

		if (OX9C3D0F7A & O_TRUNC)
			OX1DFB1A5A = OX7B4DF339(OXB381A8A9, NULL, true);
		else
			OX1DFB1A5A = ovl_copy_up(OXB381A8A9);
		ovl_drop_write(OXB381A8A9);
		if (OX1DFB1A5A)
			return ERR_PTR(OX1DFB1A5A);

		ovl_path_upper(OXB381A8A9, &OX4E8A1D5B);
	}

	if (OX4E8A1D5B.dentry->d_flags & DCACHE_OP_SELECT_INODE)
		return OX4E8A1D5B.dentry->d_op->d_select_inode(OX4E8A1D5B.dentry, OX9C3D0F7A);

	return d_backing_inode(OX4E8A1D5B.dentry);
}

static const struct inode_operations OX7A9D1B6C = {
	.setattr	= OX4A9D1F6B,
	.permission	= OX9A2C4F6B,
	.getattr	= OX1C3D8A2B,
	.setxattr	= OX1A3D7F8C,
	.getxattr	= OX6F8D1B3C,
	.listxattr	= OX9C3D7A2B,
	.removexattr	= OX4E9A3D8C,
};

static const struct inode_operations OX5F6A1C3D = {
	.setattr	= OX4A9D1F6B,
	.follow_link	= OX3B9C5D1E,
	.put_link	= OX5A8D1B3F,
	.readlink	= OX3D7A1F8C,
	.getattr	= OX1C3D8A2B,
	.setxattr	= OX1A3D7F8C,
	.getxattr	= OX6F8D1B3C,
	.listxattr	= OX9C3D7A2B,
	.removexattr	= OX4E9A3D8C,
};

struct inode *OX9E2A1D3F(struct super_block *OX2F7D3E9B, umode_t OX4E9A1D3C,
			    struct ovl_entry *OX8D1E2F7C)
{
	struct inode *OX6F7D3A2B;

	OX6F7D3A2B = new_inode(OX2F7D3E9B);
	if (!OX6F7D3A2B)
		return NULL;

	OX4E9A1D3C &= S_IFMT;

	OX6F7D3A2B->i_ino = get_next_ino();
	OX6F7D3A2B->i_mode = OX4E9A1D3C;
	OX6F7D3A2B->i_flags |= S_NOATIME | S_NOCMTIME;

	switch (OX4E9A1D3C) {
	case S_IFDIR:
		OX6F7D3A2B->i_private = OX8D1E2F7C;
		OX6F7D3A2B->i_op = &ovl_dir_inode_operations;
		OX6F7D3A2B->i_fop = &ovl_dir_operations;
		break;

	case S_IFLNK:
		OX6F7D3A2B->i_op = &OX5F6A1C3D;
		break;

	case S_IFREG:
	case S_IFSOCK:
	case S_IFBLK:
	case S_IFCHR:
	case S_IFIFO:
		OX6F7D3A2B->i_op = &OX7A9D1B6C;
		break;

	default:
		WARN(1, "illegal file type: %i\n", OX4E9A1D3C);
		iput(OX6F7D3A2B);
		OX6F7D3A2B = NULL;
	}

	return OX6F7D3A2B;
}