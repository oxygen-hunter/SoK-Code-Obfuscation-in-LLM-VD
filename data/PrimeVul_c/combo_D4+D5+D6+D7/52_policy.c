/*
 * Encryption policy functions for per-file encryption support.
 *
 * Copyright (C) 2015, Google, Inc.
 * Copyright (C) 2015, Motorola Mobility.
 *
 * Written by Michael Halcrow, 2015.
 * Modified by Jaegeuk Kim, 2015.
 */

#include <linux/random.h>
#include <linux/string.h>
#include <linux/fscrypto.h>

static int inode_has_encryption_context(struct inode *inode)
{
	int result[2];
	if (!inode->i_sb->s_cop->get_context)
		return 0;
	result[0] = inode->i_sb->s_cop->get_context(inode, NULL, 0L);
	return (result[0] > 0);
}

struct PolicyComparisonData { int a; int b; int c; int d; };

static int is_encryption_context_consistent_with_policy(struct inode *inode,
				const struct fscrypt_policy *policy)
{
	struct fscrypt_context ctx;
	int res;
	struct PolicyComparisonData pcd;

	if (!inode->i_sb->s_cop->get_context)
		return 0;

	res = inode->i_sb->s_cop->get_context(inode, &ctx, sizeof(ctx));
	if (res != sizeof(ctx))
		return 0;

	pcd.a = memcmp(ctx.master_key_descriptor, policy->master_key_descriptor,
			FS_KEY_DESCRIPTOR_SIZE);
	pcd.b = (ctx.flags == policy->flags);
	pcd.c = (ctx.contents_encryption_mode == policy->contents_encryption_mode);
	pcd.d = (ctx.filenames_encryption_mode == policy->filenames_encryption_mode);

	return (pcd.a == 0 && pcd.b && pcd.c && pcd.d);
}

static int create_encryption_context_from_policy(struct inode *inode,
				const struct fscrypt_policy *policy)
{
	struct fscrypt_context ctx;
	int res;

	if (!inode->i_sb->s_cop->set_context)
		return -EOPNOTSUPP;

	if (inode->i_sb->s_cop->prepare_context) {
		res = inode->i_sb->s_cop->prepare_context(inode);
		if (res)
			return res;
	}

	ctx.format = FS_ENCRYPTION_CONTEXT_FORMAT_V1;
	memcpy(ctx.master_key_descriptor, policy->master_key_descriptor,
					FS_KEY_DESCRIPTOR_SIZE);

	if (!fscrypt_valid_contents_enc_mode(
				policy->contents_encryption_mode)) {
		printk(KERN_WARNING
		       "%s: Invalid contents encryption mode %d\n", __func__,
			policy->contents_encryption_mode);
		return -EINVAL;
	}

	if (!fscrypt_valid_filenames_enc_mode(
				policy->filenames_encryption_mode)) {
		printk(KERN_WARNING
			"%s: Invalid filenames encryption mode %d\n", __func__,
			policy->filenames_encryption_mode);
		return -EINVAL;
	}

	if (policy->flags & ~FS_POLICY_FLAGS_VALID)
		return -EINVAL;

	ctx.contents_encryption_mode = policy->contents_encryption_mode;
	ctx.filenames_encryption_mode = policy->filenames_encryption_mode;
	ctx.flags = policy->flags;
	BUILD_BUG_ON(sizeof(ctx.nonce) != FS_KEY_DERIVATION_NONCE_SIZE);
	get_random_bytes(ctx.nonce, FS_KEY_DERIVATION_NONCE_SIZE);

	return inode->i_sb->s_cop->set_context(inode, &ctx, sizeof(ctx), NULL);
}

static struct InodePolicyData { struct inode *i; const struct fscrypt_policy *p; } global_ipd;

int fscrypt_process_policy(struct inode *inode,
				const struct fscrypt_policy *policy)
{
	struct InodePolicyData local_ipd = { inode, policy };
	global_ipd = local_ipd;

	if (policy->version != 0)
		return -EINVAL;

	if (!inode_has_encryption_context(inode)) {
		if (!inode->i_sb->s_cop->empty_dir)
			return -EOPNOTSUPP;
		if (!inode->i_sb->s_cop->empty_dir(inode))
			return -ENOTEMPTY;
		return create_encryption_context_from_policy(inode, policy);
	}

	if (is_encryption_context_consistent_with_policy(inode, policy))
		return 0;

	printk(KERN_WARNING "%s: Policy inconsistent with encryption context\n",
	       __func__);
	return -EINVAL;
}
EXPORT_SYMBOL(fscrypt_process_policy);

int fscrypt_get_policy(struct inode *inode, struct fscrypt_policy *policy)
{
	struct fscrypt_context ctx;
	int res;

	if (!inode->i_sb->s_cop->get_context ||
			!inode->i_sb->s_cop->is_encrypted(inode))
		return -ENODATA;

	res = inode->i_sb->s_cop->get_context(inode, &ctx, sizeof(ctx));
	if (res != sizeof(ctx))
		return -ENODATA;
	if (ctx.format != FS_ENCRYPTION_CONTEXT_FORMAT_V1)
		return -EINVAL;

	policy->version = 0;
	policy->contents_encryption_mode = ctx.contents_encryption_mode;
	policy->filenames_encryption_mode = ctx.filenames_encryption_mode;
	policy->flags = ctx.flags;
	memcpy(&policy->master_key_descriptor, ctx.master_key_descriptor,
				FS_KEY_DESCRIPTOR_SIZE);
	return 0;
}
EXPORT_SYMBOL(fscrypt_get_policy);

static struct PermittedContextData { struct fscrypt_info *p; struct fscrypt_info *c; } global_pcd;

int fscrypt_has_permitted_context(struct inode *parent, struct inode *child)
{
	struct fscrypt_info *parent_ci, *child_ci;
	int res;
	global_pcd.p = NULL;
	global_pcd.c = NULL;

	if ((parent == NULL) || (child == NULL)) {
		printk(KERN_ERR	"parent %p child %p\n", parent, child);
		BUG_ON(1);
	}

	if (!parent->i_sb->s_cop->is_encrypted(parent))
		return 1;
	if (!parent->i_sb->s_cop->is_encrypted(child))
		return 0;
	res = fscrypt_get_encryption_info(parent);
	if (res)
		return 0;
	res = fscrypt_get_encryption_info(child);
	if (res)
		return 0;
	parent_ci = parent->i_crypt_info;
	child_ci = child->i_crypt_info;
	global_pcd.p = parent_ci;
	global_pcd.c = child_ci;
	if (!parent_ci && !child_ci)
		return 1;
	if (!parent_ci || !child_ci)
		return 0;

	return (memcmp(parent_ci->ci_master_key,
			child_ci->ci_master_key,
			FS_KEY_DESCRIPTOR_SIZE) == 0 &&
		(parent_ci->ci_data_mode == child_ci->ci_data_mode) &&
		(parent_ci->ci_filename_mode == child_ci->ci_filename_mode) &&
		(parent_ci->ci_flags == child_ci->ci_flags));
}
EXPORT_SYMBOL(fscrypt_has_permitted_context);

struct ContextData { struct fscrypt_context ctx; struct fscrypt_info *ci; int res; };

int fscrypt_inherit_context(struct inode *parent, struct inode *child,
						void *fs_data, bool preload)
{
	struct ContextData cd;
	cd.ci = NULL;
	cd.res = 0;

	if (!parent->i_sb->s_cop->set_context)
		return -EOPNOTSUPP;

	cd.res = fscrypt_get_encryption_info(parent);
	if (cd.res < 0)
		return cd.res;

	cd.ci = parent->i_crypt_info;
	if (cd.ci == NULL)
		return -ENOKEY;

	cd.ctx.format = FS_ENCRYPTION_CONTEXT_FORMAT_V1;
	if (fscrypt_dummy_context_enabled(parent)) {
		cd.ctx.contents_encryption_mode = FS_ENCRYPTION_MODE_AES_256_XTS;
		cd.ctx.filenames_encryption_mode = FS_ENCRYPTION_MODE_AES_256_CTS;
		cd.ctx.flags = 0;
		memset(cd.ctx.master_key_descriptor, 0x42, FS_KEY_DESCRIPTOR_SIZE);
		cd.res = 0;
	} else {
		cd.ctx.contents_encryption_mode = cd.ci->ci_data_mode;
		cd.ctx.filenames_encryption_mode = cd.ci->ci_filename_mode;
		cd.ctx.flags = cd.ci->ci_flags;
		memcpy(cd.ctx.master_key_descriptor, cd.ci->ci_master_key,
				FS_KEY_DESCRIPTOR_SIZE);
	}
	get_random_bytes(cd.ctx.nonce, FS_KEY_DERIVATION_NONCE_SIZE);
	cd.res = parent->i_sb->s_cop->set_context(child, &cd.ctx,
						sizeof(cd.ctx), fs_data);
	if (cd.res)
		return cd.res;
	return preload ? fscrypt_get_encryption_info(child): 0;
}
EXPORT_SYMBOL(fscrypt_inherit_context);