#include <linux/random.h>
#include <linux/string.h>
#include <linux/fscrypto.h>

static int inode_has_encryption_context(struct inode *inode)
{
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				if (!inode->i_sb->s_cop->get_context) {
					return 0;
				}
				state = 1;
				break;
			case 1:
				return (inode->i_sb->s_cop->get_context(inode, NULL, 0L) > 0);
		}
	}
}

static int is_encryption_context_consistent_with_policy(struct inode *inode, const struct fscrypt_policy *policy)
{
	struct fscrypt_context ctx;
	int res;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (!inode->i_sb->s_cop->get_context) {
					return 0;
				}
				state = 1;
				break;
			case 1:
				res = inode->i_sb->s_cop->get_context(inode, &ctx, sizeof(ctx));
				if (res != sizeof(ctx)) {
					return 0;
				}
				state = 2;
				break;
			case 2:
				return (memcmp(ctx.master_key_descriptor, policy->master_key_descriptor, FS_KEY_DESCRIPTOR_SIZE) == 0 && (ctx.flags == policy->flags) && (ctx.contents_encryption_mode == policy->contents_encryption_mode) && (ctx.filenames_encryption_mode == policy->filenames_encryption_mode));
		}
	}
}

static int create_encryption_context_from_policy(struct inode *inode, const struct fscrypt_policy *policy)
{
	struct fscrypt_context ctx;
	int res;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (!inode->i_sb->s_cop->set_context) {
					return -EOPNOTSUPP;
				}
				state = 1;
				break;
			case 1:
				if (inode->i_sb->s_cop->prepare_context) {
					res = inode->i_sb->s_cop->prepare_context(inode);
					if (res) {
						return res;
					}
				}
				state = 2;
				break;
			case 2:
				ctx.format = FS_ENCRYPTION_CONTEXT_FORMAT_V1;
				memcpy(ctx.master_key_descriptor, policy->master_key_descriptor, FS_KEY_DESCRIPTOR_SIZE);
				if (!fscrypt_valid_contents_enc_mode(policy->contents_encryption_mode)) {
					printk(KERN_WARNING "%s: Invalid contents encryption mode %d\n", __func__, policy->contents_encryption_mode);
					return -EINVAL;
				}
				state = 3;
				break;
			case 3:
				if (!fscrypt_valid_filenames_enc_mode(policy->filenames_encryption_mode)) {
					printk(KERN_WARNING "%s: Invalid filenames encryption mode %d\n", __func__, policy->filenames_encryption_mode);
					return -EINVAL;
				}
				state = 4;
				break;
			case 4:
				if (policy->flags & ~FS_POLICY_FLAGS_VALID) {
					return -EINVAL;
				}
				state = 5;
				break;
			case 5:
				ctx.contents_encryption_mode = policy->contents_encryption_mode;
				ctx.filenames_encryption_mode = policy->filenames_encryption_mode;
				ctx.flags = policy->flags;
				BUILD_BUG_ON(sizeof(ctx.nonce) != FS_KEY_DERIVATION_NONCE_SIZE);
				get_random_bytes(ctx.nonce, FS_KEY_DERIVATION_NONCE_SIZE);
				state = 6;
				break;
			case 6:
				return inode->i_sb->s_cop->set_context(inode, &ctx, sizeof(ctx), NULL);
		}
	}
}

int fscrypt_process_policy(struct inode *inode, const struct fscrypt_policy *policy)
{
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (policy->version != 0) {
					return -EINVAL;
				}
				state = 1;
				break;
			case 1:
				if (!inode_has_encryption_context(inode)) {
					if (!inode->i_sb->s_cop->empty_dir) {
						return -EOPNOTSUPP;
					}
					if (!inode->i_sb->s_cop->empty_dir(inode)) {
						return -ENOTEMPTY;
					}
					return create_encryption_context_from_policy(inode, policy);
				}
				state = 2;
				break;
			case 2:
				if (is_encryption_context_consistent_with_policy(inode, policy)) {
					return 0;
				}
				printk(KERN_WARNING "%s: Policy inconsistent with encryption context\n", __func__);
				return -EINVAL;
		}
	}
}
EXPORT_SYMBOL(fscrypt_process_policy);

int fscrypt_get_policy(struct inode *inode, struct fscrypt_policy *policy)
{
	struct fscrypt_context ctx;
	int res;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (!inode->i_sb->s_cop->get_context || !inode->i_sb->s_cop->is_encrypted(inode)) {
					return -ENODATA;
				}
				state = 1;
				break;
			case 1:
				res = inode->i_sb->s_cop->get_context(inode, &ctx, sizeof(ctx));
				if (res != sizeof(ctx)) {
					return -ENODATA;
				}
				state = 2;
				break;
			case 2:
				if (ctx.format != FS_ENCRYPTION_CONTEXT_FORMAT_V1) {
					return -EINVAL;
				}
				state = 3;
				break;
			case 3:
				policy->version = 0;
				policy->contents_encryption_mode = ctx.contents_encryption_mode;
				policy->filenames_encryption_mode = ctx.filenames_encryption_mode;
				policy->flags = ctx.flags;
				memcpy(&policy->master_key_descriptor, ctx.master_key_descriptor, FS_KEY_DESCRIPTOR_SIZE);
				return 0;
		}
	}
}
EXPORT_SYMBOL(fscrypt_get_policy);

int fscrypt_has_permitted_context(struct inode *parent, struct inode *child)
{
	struct fscrypt_info *parent_ci, *child_ci;
	int res;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if ((parent == NULL) || (child == NULL)) {
					printk(KERN_ERR "parent %p child %p\n", parent, child);
					BUG_ON(1);
				}
				state = 1;
				break;
			case 1:
				if (!parent->i_sb->s_cop->is_encrypted(parent)) {
					return 1;
				}
				state = 2;
				break;
			case 2:
				if (!parent->i_sb->s_cop->is_encrypted(child)) {
					return 0;
				}
				state = 3;
				break;
			case 3:
				res = fscrypt_get_encryption_info(parent);
				if (res) {
					return 0;
				}
				state = 4;
				break;
			case 4:
				res = fscrypt_get_encryption_info(child);
				if (res) {
					return 0;
				}
				state = 5;
				break;
			case 5:
				parent_ci = parent->i_crypt_info;
				child_ci = child->i_crypt_info;
				if (!parent_ci && !child_ci) {
					return 1;
				}
				if (!parent_ci || !child_ci) {
					return 0;
				}
				state = 6;
				break;
			case 6:
				return (memcmp(parent_ci->ci_master_key, child_ci->ci_master_key, FS_KEY_DESCRIPTOR_SIZE) == 0 && (parent_ci->ci_data_mode == child_ci->ci_data_mode) && (parent_ci->ci_filename_mode == child_ci->ci_filename_mode) && (parent_ci->ci_flags == child_ci->ci_flags));
		}
	}
}
EXPORT_SYMBOL(fscrypt_has_permitted_context);

int fscrypt_inherit_context(struct inode *parent, struct inode *child, void *fs_data, bool preload)
{
	struct fscrypt_context ctx;
	struct fscrypt_info *ci;
	int res;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (!parent->i_sb->s_cop->set_context) {
					return -EOPNOTSUPP;
				}
				state = 1;
				break;
			case 1:
				res = fscrypt_get_encryption_info(parent);
				if (res < 0) {
					return res;
				}
				state = 2;
				break;
			case 2:
				ci = parent->i_crypt_info;
				if (ci == NULL) {
					return -ENOKEY;
				}
				state = 3;
				break;
			case 3:
				ctx.format = FS_ENCRYPTION_CONTEXT_FORMAT_V1;
				if (fscrypt_dummy_context_enabled(parent)) {
					ctx.contents_encryption_mode = FS_ENCRYPTION_MODE_AES_256_XTS;
					ctx.filenames_encryption_mode = FS_ENCRYPTION_MODE_AES_256_CTS;
					ctx.flags = 0;
					memset(ctx.master_key_descriptor, 0x42, FS_KEY_DESCRIPTOR_SIZE);
					res = 0;
				} else {
					ctx.contents_encryption_mode = ci->ci_data_mode;
					ctx.filenames_encryption_mode = ci->ci_filename_mode;
					ctx.flags = ci->ci_flags;
					memcpy(ctx.master_key_descriptor, ci->ci_master_key, FS_KEY_DESCRIPTOR_SIZE);
				}
				state = 4;
				break;
			case 4:
				get_random_bytes(ctx.nonce, FS_KEY_DERIVATION_NONCE_SIZE);
				res = parent->i_sb->s_cop->set_context(child, &ctx, sizeof(ctx), fs_data);
				if (res) {
					return res;
				}
				return preload ? fscrypt_get_encryption_info(child) : 0;
		}
	}
}
EXPORT_SYMBOL(fscrypt_inherit_context);