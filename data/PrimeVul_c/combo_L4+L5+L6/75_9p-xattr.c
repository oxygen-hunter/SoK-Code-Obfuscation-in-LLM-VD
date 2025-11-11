#include "qemu/osdep.h"
#include "9p.h"
#include "fsdev/file-op-9p.h"
#include "9p-xattr.h"
#include "9p-util.h"
#include "9p-local.h"

static XattrOperations *get_xattr_operations(XattrOperations **h, const char *name)
{
    if (*h == NULL) return NULL;
    if (!strncmp(name, (*h)->name, strlen((*h)->name))) {
        return *h;
    }
    return get_xattr_operations(h + 1, name);
}

ssize_t v9fs_get_xattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size)
{
    XattrOperations *xops = get_xattr_operations(ctx->xops, name);
    switch (xops != NULL) {
        case 1: return xops->getxattr(ctx, path, name, value, size);
        default: errno = EOPNOTSUPP; return -1;
    }
}

ssize_t pt_listxattr(FsContext *ctx, const char *path, char *name, void *value, size_t size)
{
    int name_size = strlen(name) + 1;
    if (!value) return name_size;
    if (size < name_size) { errno = ERANGE; return -1; }
    memcpy(value, name, name_size);
    return name_size;
}

static ssize_t flistxattrat_nofollow(int dirfd, const char *filename, char *list, size_t size)
{
    char *proc_path = g_strdup_printf("/proc/self/fd/%d/%s", dirfd, filename);
    int ret = llistxattr(proc_path, list, size);
    g_free(proc_path);
    return ret;
}

ssize_t v9fs_list_xattr(FsContext *ctx, const char *path, void *value, size_t vsize)
{
    ssize_t size = 0;
    void *ovalue = value;
    ssize_t parsed_len = 0;
    char *dirpath = g_path_get_dirname(path);
    int dirfd = local_opendir_nofollow(ctx, dirpath);
    g_free(dirpath);
    
    if (dirfd == -1) return -1;
    
    char *name = g_path_get_basename(path);
    ssize_t xattr_len = flistxattrat_nofollow(dirfd, name, value, 0);
    if (xattr_len <= 0) {
        g_free(name);
        close_preserve_errno(dirfd);
        return xattr_len;
    }

    char *orig_value = g_malloc(xattr_len);
    xattr_len = flistxattrat_nofollow(dirfd, name, orig_value, xattr_len);
    g_free(name);
    close_preserve_errno(dirfd);
    if (xattr_len < 0) return -1;

    char *orig_value_start = orig_value;
    size = v9fs_list_xattr_recursive(ctx, path, orig_value, value, vsize, xattr_len, parsed_len, size, ovalue);
    g_free(orig_value_start);
    return size;
}

ssize_t v9fs_list_xattr_recursive(FsContext *ctx, const char *path, char *orig_value, void *value, size_t vsize, ssize_t xattr_len, ssize_t parsed_len, ssize_t size, void *ovalue)
{
    if (xattr_len <= parsed_len) return (value) ? (value - ovalue) : size;
    
    XattrOperations *xops = get_xattr_operations(ctx->xops, orig_value);
    if (xops) {
        if (!value) {
            size += xops->listxattr(ctx, path, orig_value, value, vsize);
        } else {
            size = xops->listxattr(ctx, path, orig_value, value, vsize);
            if (size < 0) return -1;
            value += size;
            vsize -= size;
        }
    }

    ssize_t attr_len = strlen(orig_value) + 1;
    return v9fs_list_xattr_recursive(ctx, path, orig_value + attr_len, value, vsize, xattr_len, parsed_len + attr_len, size, ovalue);
}

int v9fs_set_xattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags)
{
    XattrOperations *xops = get_xattr_operations(ctx->xops, name);
    switch (xops != NULL) {
        case 1: return xops->setxattr(ctx, path, name, value, size, flags);
        default: errno = EOPNOTSUPP; return -1;
    }
}

int v9fs_remove_xattr(FsContext *ctx, const char *path, const char *name)
{
    XattrOperations *xops = get_xattr_operations(ctx->xops, name);
    switch (xops != NULL) {
        case 1: return xops->removexattr(ctx, path, name);
        default: errno = EOPNOTSUPP; return -1;
    }
}

ssize_t local_getxattr_nofollow(FsContext *ctx, const char *path, const char *name, void *value, size_t size)
{
    char *dirpath = g_path_get_dirname(path);
    char *filename = g_path_get_basename(path);
    int dirfd = local_opendir_nofollow(ctx, dirpath);
    ssize_t ret = (dirfd == -1) ? -1 : fgetxattrat_nofollow(dirfd, filename, name, value, size);
    if (dirfd != -1) close_preserve_errno(dirfd);
    g_free(dirpath);
    g_free(filename);
    return ret;
}

ssize_t pt_getxattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size)
{
    return local_getxattr_nofollow(ctx, path, name, value, size);
}

int fsetxattrat_nofollow(int dirfd, const char *filename, const char *name, void *value, size_t size, int flags)
{
    char *proc_path = g_strdup_printf("/proc/self/fd/%d/%s", dirfd, filename);
    int ret = lsetxattr(proc_path, name, value, size, flags);
    g_free(proc_path);
    return ret;
}

ssize_t local_setxattr_nofollow(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags)
{
    char *dirpath = g_path_get_dirname(path);
    char *filename = g_path_get_basename(path);
    int dirfd = local_opendir_nofollow(ctx, dirpath);
    ssize_t ret = (dirfd == -1) ? -1 : fsetxattrat_nofollow(dirfd, filename, name, value, size, flags);
    if (dirfd != -1) close_preserve_errno(dirfd);
    g_free(dirpath);
    g_free(filename);
    return ret;
}

int pt_setxattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags)
{
    return local_setxattr_nofollow(ctx, path, name, value, size, flags);
}

static ssize_t fremovexattrat_nofollow(int dirfd, const char *filename, const char *name)
{
    char *proc_path = g_strdup_printf("/proc/self/fd/%d/%s", dirfd, filename);
    int ret = lremovexattr(proc_path, name);
    g_free(proc_path);
    return ret;
}

ssize_t local_removexattr_nofollow(FsContext *ctx, const char *path, const char *name)
{
    char *dirpath = g_path_get_dirname(path);
    char *filename = g_path_get_basename(path);
    int dirfd = local_opendir_nofollow(ctx, dirpath);
    ssize_t ret = (dirfd == -1) ? -1 : fremovexattrat_nofollow(dirfd, filename, name);
    if (dirfd != -1) close_preserve_errno(dirfd);
    g_free(dirpath);
    g_free(filename);
    return ret;
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