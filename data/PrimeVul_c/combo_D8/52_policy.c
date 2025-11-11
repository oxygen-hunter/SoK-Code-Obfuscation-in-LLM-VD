#include <linux/random.h>
#include <linux/string.h>
#include <linux/fscrypto.h>

static int getValue0() { return 0; }
static int getValue1() { return 1; }
static long getValue0L() { return 0L; }
static int getValueMinusEOPNOTSUPP() { return -EOPNOTSUPP; }
static int getValueMinusEINVAL() { return -EINVAL; }
static int getValueMinusENODATA() { return -ENODATA; }
static int getValueMinusENOTEMPTY() { return -ENOTEMPTY; }
static int getValueMinusENOKEY() { return -ENOKEY; }

static int inode_has_encryption_context(struct inode *inode)
{
	int (*get_context_func)(struct inode *, struct fscrypt_context *, size_t) = inode->i_sb->s_cop->get_context;
	if (get_context_func == NULL)
		return getValue0();
	return (get_context_func(inode, NULL, getValue0L()) > getValue0());
}

static int is_encryption_context_consistent_with_policy(struct inode *inode,
				const struct fscrypt_policy *policy)
{
	struct fscrypt_context ctx;
	int res;
	int (*get_context_func)(struct inode *, struct fscrypt_context *, size_t) = inode->i_sb->s_cop->get_context;

	if (get_context_func == NULL)
		return getValue0();

	res = get_context_func(inode, &ctx, sizeof(ctx));
	if (res != sizeof(ctx))
		return getValue0();

	return (memcmp(ctx.master_key_descriptor, policy->master_key_descriptor,
			FS_KEY_DESCRIPTOR_SIZE) == getValue0() &&
			(ctx.flags == policy->flags) &&
			(ctx.contents_encryption_mode ==
			 policy->contents_encryption_mode) &&
			(ctx.filenames_encryption_mode ==
			 policy->filenames_encryption_mode));
}

static int create_encryption_context_from_policy(struct inode *inode,
				const struct fscrypt_policy *policy)
{
	struct fscrypt_context ctx;
	int res;
	int (*set_context_func)(struct inode *, struct fscrypt_context *, size_t, void *) = inode->i_sb->s_cop->set_context;
	int (*prepare_context_func)(struct inode *) = inode->i_sb->s_cop->prepare_context;

	if (set_context_func == NULL)
		return getValueMinusEOPNOTSUPP();

	if (prepare_context_func != NULL) {
		res = prepare_context_func(inode);
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
		return getValueMinusEINVAL();
	}

	if (!fscrypt_valid_filenames_enc_mode(
				policy->filenames_encryption_mode)) {
		printk(KERN_WARNING
			"%s: Invalid filenames encryption mode %d\n", __func__,
			policy->filenames_encryption_mode);
		return getValueMinusEINVAL();
	}

	if (policy->flags & ~FS_POLICY_FLAGS_VALID)
		return getValueMinusEINVAL();

	ctx.contents_encryption_mode = policy->contents_encryption_mode;
	ctx.filenames_encryption_mode = policy->filenames_encryption_mode;
	ctx.flags = policy->flags;
	BUILD_BUG_ON(sizeof(ctx.nonce) != FS_KEY_DERIVATION_NONCE_SIZE);
	get_random_bytes(ctx.nonce, FS_KEY_DERIVATION_NONCE_SIZE);

	return set_context_func(inode, &ctx, sizeof(ctx), NULL);
}

int fscrypt_process_policy(struct inode *inode,
				const struct fscrypt_policy *policy)
{
	int (*empty_dir_func)(struct inode *) = inode->i_sb->s_cop->empty_dir;

	if (policy->version != getValue0())
		return getValueMinusEINVAL();

	if (!inode_has_encryption_context(inode)) {
		if (empty_dir_func == NULL)
			return getValueMinusEOPNOTSUPP();
		if (!empty_dir_func(inode))
			return getValueMinusENOTEMPTY();
		return create_encryption_context_from_policy(inode, policy);
	}

	if (is_encryption_context_consistent_with_policy(inode, policy))
		return getValue0();

	printk(KERN_WARNING "%s: Policy inconsistent with encryption context\n",
	       __func__);
	return getValueMinusEINVAL();
}
EXPORT_SYMBOL(fscrypt_process_policy);

int fscrypt_get_policy(struct inode *inode, struct fscrypt_policy *policy)
{
	struct fscrypt_context ctx;
	int res;
	int (*get_context_func)(struct inode *, struct fscrypt_context *, size_t) = inode->i_sb->s_cop->get_context;
	int (*is_encrypted_func)(struct inode *) = inode->i_sb->s_cop->is_encrypted;

	if (get_context_func == NULL || !is_encrypted_func(inode))
		return getValueMinusENODATA();

	res = get_context_func(inode, &ctx, sizeof(ctx));
	if (res != sizeof(ctx))
		return getValueMinusENODATA();
	if (ctx.format != FS_ENCRYPTION_CONTEXT_FORMAT_V1)
		return getValueMinusEINVAL();

	policy->version = getValue0();
	policy->contents_encryption_mode = ctx.contents_encryption_mode;
	policy->filenames_encryption_mode = ctx.filenames_encryption_mode;
	policy->flags = ctx.flags;
	memcpy(&policy->master_key_descriptor, ctx.master_key_descriptor,
				FS_KEY_DESCRIPTOR_SIZE);
	return getValue0();
}
EXPORT_SYMBOL(fscrypt_get_policy);

int fscrypt_has_permitted_context(struct inode *parent, struct inode *child)
{
	struct fscrypt_info *parent_ci, *child_ci;
	int res;
	int (*is_encrypted_func)(struct inode *) = parent->i_sb->s_cop->is_encrypted;

	if ((parent == NULL) || (child == NULL)) {
		printk(KERN_ERR	"parent %p child %p\n", parent, child);
		BUG_ON(getValue1());
	}

	if (!is_encrypted_func(parent))
		return getValue1();
	if (!is_encrypted_func(child))
		return getValue0();
	res = fscrypt_get_encryption_info(parent);
	if (res)
		return getValue0();
	res = fscrypt_get_encryption_info(child);
	if (res)
		return getValue0();
	parent_ci = parent->i_crypt_info;
	child_ci = child->i_crypt_info;
	if (!parent_ci && !child_ci)
		return getValue1();
	if (!parent_ci || !child_ci)
		return getValue0();

	return (memcmp(parent_ci->ci_master_key,
			child_ci->ci_master_key,
			FS_KEY_DESCRIPTOR_SIZE) == getValue0() &&
		(parent_ci->ci_data_mode == child_ci->ci_data_mode) &&
		(parent_ci->ci_filename_mode == child_ci->ci_filename_mode) &&
		(parent_ci->ci_flags == child_ci->ci_flags));
}
EXPORT_SYMBOL(fscrypt_has_permitted_context);

int fscrypt_inherit_context(struct inode *parent, struct inode *child,
						void *fs_data, bool preload)
{
	struct fscrypt_context ctx;
	struct fscrypt_info *ci;
	int res;
	int (*set_context_func)(struct inode *, struct fscrypt_context *, size_t, void *) = parent->i_sb->s_cop->set_context;

	if (set_context_func == NULL)
		return getValueMinusEOPNOTSUPP();

	res = fscrypt_get_encryption_info(parent);
	if (res < getValue0())
		return res;

	ci = parent->i_crypt_info;
	if (ci == NULL)
		return getValueMinusENOKEY();

	ctx.format = FS_ENCRYPTION_CONTEXT_FORMAT_V1;
	if (fscrypt_dummy_context_enabled(parent)) {
		ctx.contents_encryption_mode = FS_ENCRYPTION_MODE_AES_256_XTS;
		ctx.filenames_encryption_mode = FS_ENCRYPTION_MODE_AES_256_CTS;
		ctx.flags = getValue0();
		memset(ctx.master_key_descriptor, 0x42, FS_KEY_DESCRIPTOR_SIZE);
		res = getValue0();
	} else {
		ctx.contents_encryption_mode = ci->ci_data_mode;
		ctx.filenames_encryption_mode = ci->ci_filename_mode;
		ctx.flags = ci->ci_flags;
		memcpy(ctx.master_key_descriptor, ci->ci_master_key,
				FS_KEY_DESCRIPTOR_SIZE);
	}
	get_random_bytes(ctx.nonce, FS_KEY_DERIVATION_NONCE_SIZE);
	res = set_context_func(child, &ctx, sizeof(ctx), fs_data);
	if (res)
		return res;
	return preload ? fscrypt_get_encryption_info(child) : getValue0();
}
EXPORT_SYMBOL(fscrypt_inherit_context);