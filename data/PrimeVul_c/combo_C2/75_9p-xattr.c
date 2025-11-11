#include "qemu/osdep.h"
#include "9p.h"
#include "fsdev/file-op-9p.h"
#include "9p-xattr.h"
#include "9p-util.h"
#include "9p-local.h"

static XattrOperations *get_xattr_operations(XattrOperations **h, const char *name)
{
    XattrOperations *xops;
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                xops = *(h)++;
                dispatch = 1;
                break;
            case 1:
                if (xops == NULL) {
                    return NULL;
                }
                dispatch = 2;
                break;
            case 2:
                if (!strncmp(name, xops->name, strlen(xops->name))) {
                    return xops;
                }
                dispatch = 0;
                break;
        }
    }
}

ssize_t v9fs_get_xattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size)
{
    XattrOperations *xops = get_xattr_operations(ctx->xops, name);
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                if (xops) {
                    dispatch = 1;
                    break;
                }
                dispatch = 2;
                break;
            case 1:
                return xops->getxattr(ctx, path, name, value, size);
            case 2:
                errno = EOPNOTSUPP;
                return -1;
        }
    }
}

ssize_t pt_listxattr(FsContext *ctx, const char *path, char *name, void *value, size_t size)
{
    int name_size = strlen(name) + 1;
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                if (!value) {
                    return name_size;
                }
                dispatch = 1;
                break;
            case 1:
                if (size < name_size) {
                    errno = ERANGE;
                    return -1;
                }
                dispatch = 2;
                break;
            case 2:
                memcpy(value, name, name_size);
                return name_size;
        }
    }
}

static ssize_t flistxattrat_nofollow(int dirfd, const char *filename, char *list, size_t size)
{
    char *proc_path = g_strdup_printf("/proc/self/fd/%d/%s", dirfd, filename);
    int ret = llistxattr(proc_path, list, size);
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                g_free(proc_path);
                return ret;
        }
    }
}

ssize_t v9fs_list_xattr(FsContext *ctx, const char *path, void *value, size_t vsize)
{
    ssize_t size = 0;
    void *ovalue = value;
    XattrOperations *xops;
    char *orig_value, *orig_value_start;
    ssize_t xattr_len, parsed_len = 0, attr_len;
    char *dirpath, *name;
    int dirfd;

    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                dirpath = g_path_get_dirname(path);
                dirfd = local_opendir_nofollow(ctx, dirpath);
                g_free(dirpath);
                if (dirfd == -1) {
                    return -1;
                }
                name = g_path_get_basename(path);
                xattr_len = flistxattrat_nofollow(dirfd, name, value, 0);
                if (xattr_len <= 0) {
                    g_free(name);
                    close_preserve_errno(dirfd);
                    return xattr_len;
                }
                dispatch = 1;
                break;
            case 1:
                orig_value = g_malloc(xattr_len);
                xattr_len = flistxattrat_nofollow(dirfd, name, orig_value, xattr_len);
                g_free(name);
                close_preserve_errno(dirfd);
                if (xattr_len < 0) {
                    return -1;
                }
                orig_value_start = orig_value;
                dispatch = 2;
                break;
            case 2:
                if (xattr_len <= parsed_len) {
                    goto err_out;
                }
                xops = get_xattr_operations(ctx->xops, orig_value);
                if (!xops) {
                    goto next_entry;
                }
                if (!value) {
                    size += xops->listxattr(ctx, path, orig_value, value, vsize);
                } else {
                    size = xops->listxattr(ctx, path, orig_value, value, vsize);
                    if (size < 0) {
                        goto err_out;
                    }
                    value += size;
                    vsize -= size;
                }
                dispatch = 3;
                break;
            case 3:
next_entry:
                attr_len = strlen(orig_value) + 1;
                parsed_len += attr_len;
                orig_value += attr_len;
                dispatch = 2;
                break;
            case 4:
err_out:
                g_free(orig_value_start);
                if (value) {
                    size = value - ovalue;
                }
                return size;
        }
    }
}

int v9fs_set_xattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags)
{
    XattrOperations *xops = get_xattr_operations(ctx->xops, name);
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                if (xops) {
                    dispatch = 1;
                    break;
                }
                dispatch = 2;
                break;
            case 1:
                return xops->setxattr(ctx, path, name, value, size, flags);
            case 2:
                errno = EOPNOTSUPP;
                return -1;
        }
    }
}

int v9fs_remove_xattr(FsContext *ctx, const char *path, const char *name)
{
    XattrOperations *xops = get_xattr_operations(ctx->xops, name);
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                if (xops) {
                    dispatch = 1;
                    break;
                }
                dispatch = 2;
                break;
            case 1:
                return xops->removexattr(ctx, path, name);
            case 2:
                errno = EOPNOTSUPP;
                return -1;
        }
    }
}

ssize_t local_getxattr_nofollow(FsContext *ctx, const char *path, const char *name, void *value, size_t size)
{
    char *dirpath = g_path_get_dirname(path);
    char *filename = g_path_get_basename(path);
    int dirfd;
    ssize_t ret = -1;
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                dirfd = local_opendir_nofollow(ctx, dirpath);
                if (dirfd == -1) {
                    dispatch = 1;
                    break;
                }
                dispatch = 2;
                break;
            case 1:
                goto out;
            case 2:
                ret = fgetxattrat_nofollow(dirfd, filename, name, value, size);
                close_preserve_errno(dirfd);
out:
                g_free(dirpath);
                g_free(filename);
                return ret;
        }
    }
}

ssize_t pt_getxattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size)
{
    return local_getxattr_nofollow(ctx, path, name, value, size);
}

int fsetxattrat_nofollow(int dirfd, const char *filename, const char *name, void *value, size_t size, int flags)
{
    char *proc_path = g_strdup_printf("/proc/self/fd/%d/%s", dirfd, filename);
    int ret = lsetxattr(proc_path, name, value, size, flags);
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                g_free(proc_path);
                return ret;
        }
    }
}

ssize_t local_setxattr_nofollow(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags)
{
    char *dirpath = g_path_get_dirname(path);
    char *filename = g_path_get_basename(path);
    int dirfd;
    ssize_t ret = -1;
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                dirfd = local_opendir_nofollow(ctx, dirpath);
                if (dirfd == -1) {
                    dispatch = 1;
                    break;
                }
                dispatch = 2;
                break;
            case 1:
                goto out;
            case 2:
                ret = fsetxattrat_nofollow(dirfd, filename, name, value, size, flags);
                close_preserve_errno(dirfd);
out:
                g_free(dirpath);
                g_free(filename);
                return ret;
        }
    }
}

int pt_setxattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags)
{
    return local_setxattr_nofollow(ctx, path, name, value, size, flags);
}

static ssize_t fremovexattrat_nofollow(int dirfd, const char *filename, const char *name)
{
    char *proc_path = g_strdup_printf("/proc/self/fd/%d/%s", dirfd, filename);
    int ret = lremovexattr(proc_path, name);
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                g_free(proc_path);
                return ret;
        }
    }
}

ssize_t local_removexattr_nofollow(FsContext *ctx, const char *path, const char *name)
{
    char *dirpath = g_path_get_dirname(path);
    char *filename = g_path_get_basename(path);
    int dirfd;
    ssize_t ret = -1;
    int dispatch = 0;
    for (;;) {
        switch (dispatch) {
            case 0:
                dirfd = local_opendir_nofollow(ctx, dirpath);
                if (dirfd == -1) {
                    dispatch = 1;
                    break;
                }
                dispatch = 2;
                break;
            case 1:
                goto out;
            case 2:
                ret = fremovexattrat_nofollow(dirfd, filename, name);
                close_preserve_errno(dirfd);
out:
                g_free(dirpath);
                g_free(filename);
                return ret;
        }
    }
}

int pt_removexattr(FsContext *ctx, const char *path, const char *name)
{
    return local_removexattr_nofollow(ctx, path, name);
}

ssize_t notsup_getxattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size)
{
    errno = ENOTSUP;
    return -1;
}

int notsup_setxattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags)
{
    errno = ENOTSUP;
    return -1;
}

ssize_t notsup_listxattr(FsContext *ctx, const char *path, char *name, void *value, size_t size)
{
    return 0;
}

int notsup_removexattr(FsContext *ctx, const char *path, const char *name)
{
    errno = ENOTSUP;
    return -1;
}

XattrOperations *mapped_xattr_ops[] = {
    &mapped_user_xattr,
    &mapped_pacl_xattr,
    &mapped_dacl_xattr,
    NULL,
};

XattrOperations *passthrough_xattr_ops[] = {
    &passthrough_user_xattr,
    &passthrough_acl_xattr,
    NULL,
};

XattrOperations *none_xattr_ops[] = {
    &passthrough_user_xattr,
    &none_acl_xattr,
    NULL,
};