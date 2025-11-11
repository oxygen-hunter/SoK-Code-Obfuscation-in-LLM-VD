#include <linux/random.h>
#include <linux/string.h>
#include <linux/fscrypto.h>

static int OX7B4DF339(struct inode *OX1A2B3C4D)
{
	if (!OX1A2B3C4D->i_sb->s_cop->get_context)
		return 0;
	return (OX1A2B3C4D->i_sb->s_cop->get_context(OX1A2B3C4D, NULL, 0L) > 0);
}

static int OX8C9D7E6F(struct inode *OX1A2B3C4D,
				const struct fscrypt_policy *OX5E6F7A8B)
{
	struct fscrypt_context OX9ABCDEF0;
	int OX12345678;

	if (!OX1A2B3C4D->i_sb->s_cop->get_context)
		return 0;

	OX12345678 = OX1A2B3C4D->i_sb->s_cop->get_context(OX1A2B3C4D, &OX9ABCDEF0, sizeof(OX9ABCDEF0));
	if (OX12345678 != sizeof(OX9ABCDEF0))
		return 0;

	return (memcmp(OX9ABCDEF0.master_key_descriptor, OX5E6F7A8B->master_key_descriptor,
			FS_KEY_DESCRIPTOR_SIZE) == 0 &&
			(OX9ABCDEF0.flags == OX5E6F7A8B->flags) &&
			(OX9ABCDEF0.contents_encryption_mode ==
			 OX5E6F7A8B->contents_encryption_mode) &&
			(OX9ABCDEF0.filenames_encryption_mode ==
			 OX5E6F7A8B->filenames_encryption_mode));
}

static int OX0FEDCBA9(struct inode *OX1A2B3C4D,
				const struct fscrypt_policy *OX5E6F7A8B)
{
	struct fscrypt_context OX9ABCDEF0;
	int OX12345678;

	if (!OX1A2B3C4D->i_sb->s_cop->set_context)
		return -EOPNOTSUPP;

	if (OX1A2B3C4D->i_sb->s_cop->prepare_context) {
		OX12345678 = OX1A2B3C4D->i_sb->s_cop->prepare_context(OX1A2B3C4D);
		if (OX12345678)
			return OX12345678;
	}

	OX9ABCDEF0.format = FS_ENCRYPTION_CONTEXT_FORMAT_V1;
	memcpy(OX9ABCDEF0.master_key_descriptor, OX5E6F7A8B->master_key_descriptor,
					FS_KEY_DESCRIPTOR_SIZE);

	if (!fscrypt_valid_contents_enc_mode(
				OX5E6F7A8B->contents_encryption_mode)) {
		printk(KERN_WARNING
		       "%s: Invalid contents encryption mode %d\n", __func__,
			OX5E6F7A8B->contents_encryption_mode);
		return -EINVAL;
	}

	if (!fscrypt_valid_filenames_enc_mode(
				OX5E6F7A8B->filenames_encryption_mode)) {
		printk(KERN_WARNING
			"%s: Invalid filenames encryption mode %d\n", __func__,
			OX5E6F7A8B->filenames_encryption_mode);
		return -EINVAL;
	}

	if (OX5E6F7A8B->flags & ~FS_POLICY_FLAGS_VALID)
		return -EINVAL;

	OX9ABCDEF0.contents_encryption_mode = OX5E6F7A8B->contents_encryption_mode;
	OX9ABCDEF0.filenames_encryption_mode = OX5E6F7A8B->filenames_encryption_mode;
	OX9ABCDEF0.flags = OX5E6F7A8B->flags;
	BUILD_BUG_ON(sizeof(OX9ABCDEF0.nonce) != FS_KEY_DERIVATION_NONCE_SIZE);
	get_random_bytes(OX9ABCDEF0.nonce, FS_KEY_DERIVATION_NONCE_SIZE);

	return OX1A2B3C4D->i_sb->s_cop->set_context(OX1A2B3C4D, &OX9ABCDEF0, sizeof(OX9ABCDEF0), NULL);
}

int OX2468ACE0(struct inode *OX1A2B3C4D,
				const struct fscrypt_policy *OX5E6F7A8B)
{
	if (OX5E6F7A8B->version != 0)
		return -EINVAL;

	if (!OX7B4DF339(OX1A2B3C4D)) {
		if (!OX1A2B3C4D->i_sb->s_cop->empty_dir)
			return -EOPNOTSUPP;
		if (!OX1A2B3C4D->i_sb->s_cop->empty_dir(OX1A2B3C4D))
			return -ENOTEMPTY;
		return OX0FEDCBA9(OX1A2B3C4D, OX5E6F7A8B);
	}

	if (OX8C9D7E6F(OX1A2B3C4D, OX5E6F7A8B))
		return 0;

	printk(KERN_WARNING "%s: Policy inconsistent with encryption context\n",
	       __func__);
	return -EINVAL;
}
EXPORT_SYMBOL(OX2468ACE0);

int OX13579BDF(struct inode *OX1A2B3C4D, struct fscrypt_policy *OX5E6F7A8B)
{
	struct fscrypt_context OX9ABCDEF0;
	int OX12345678;

	if (!OX1A2B3C4D->i_sb->s_cop->get_context ||
			!OX1A2B3C4D->i_sb->s_cop->is_encrypted(OX1A2B3C4D))
		return -ENODATA;

	OX12345678 = OX1A2B3C4D->i_sb->s_cop->get_context(OX1A2B3C4D, &OX9ABCDEF0, sizeof(OX9ABCDEF0));
	if (OX12345678 != sizeof(OX9ABCDEF0))
		return -ENODATA;
	if (OX9ABCDEF0.format != FS_ENCRYPTION_CONTEXT_FORMAT_V1)
		return -EINVAL;

	OX5E6F7A8B->version = 0;
	OX5E6F7A8B->contents_encryption_mode = OX9ABCDEF0.contents_encryption_mode;
	OX5E6F7A8B->filenames_encryption_mode = OX9ABCDEF0.filenames_encryption_mode;
	OX5E6F7A8B->flags = OX9ABCDEF0.flags;
	memcpy(&OX5E6F7A8B->master_key_descriptor, OX9ABCDEF0.master_key_descriptor,
				FS_KEY_DESCRIPTOR_SIZE);
	return 0;
}
EXPORT_SYMBOL(OX13579BDF);

int OXACE1BDF2(struct inode *OX1A2B3C4D, struct inode *OX4D3C2B1A)
{
	struct fscrypt_info *OX9A8B7C6D, *OX6D7C8B9A;
	int OX12345678;

	if ((OX1A2B3C4D == NULL) || (OX4D3C2B1A == NULL)) {
		printk(KERN_ERR	"parent %p child %p\n", OX1A2B3C4D, OX4D3C2B1A);
		BUG_ON(1);
	}

	if (!OX1A2B3C4D->i_sb->s_cop->is_encrypted(OX1A2B3C4D))
		return 1;
	if (!OX1A2B3C4D->i_sb->s_cop->is_encrypted(OX4D3C2B1A))
		return 0;
	OX12345678 = fscrypt_get_encryption_info(OX1A2B3C4D);
	if (OX12345678)
		return 0;
	OX12345678 = fscrypt_get_encryption_info(OX4D3C2B1A);
	if (OX12345678)
		return 0;
	OX9A8B7C6D = OX1A2B3C4D->i_crypt_info;
	OX6D7C8B9A = OX4D3C2B1A->i_crypt_info;
	if (!OX9A8B7C6D && !OX6D7C8B9A)
		return 1;
	if (!OX9A8B7C6D || !OX6D7C8B9A)
		return 0;

	return (memcmp(OX9A8B7C6D->ci_master_key,
			OX6D7C8B9A->ci_master_key,
			FS_KEY_DESCRIPTOR_SIZE) == 0 &&
		(OX9A8B7C6D->ci_data_mode == OX6D7C8B9A->ci_data_mode) &&
		(OX9A8B7C6D->ci_filename_mode == OX6D7C8B9A->ci_filename_mode) &&
		(OX9A8B7C6D->ci_flags == OX6D7C8B9A->ci_flags));
}
EXPORT_SYMBOL(OXACE1BDF2);

int OX9B7D5F3A(struct inode *OX1A2B3C4D, struct inode *OX4D3C2B1A,
						void *OX2F4B6D8E, bool OX1E3F5A7B)
{
	struct fscrypt_context OX9ABCDEF0;
	struct fscrypt_info *OX9A8B7C6D;
	int OX12345678;

	if (!OX1A2B3C4D->i_sb->s_cop->set_context)
		return -EOPNOTSUPP;

	OX12345678 = fscrypt_get_encryption_info(OX1A2B3C4D);
	if (OX12345678 < 0)
		return OX12345678;

	OX9A8B7C6D = OX1A2B3C4D->i_crypt_info;
	if (OX9A8B7C6D == NULL)
		return -ENOKEY;

	OX9ABCDEF0.format = FS_ENCRYPTION_CONTEXT_FORMAT_V1;
	if (fscrypt_dummy_context_enabled(OX1A2B3C4D)) {
		OX9ABCDEF0.contents_encryption_mode = FS_ENCRYPTION_MODE_AES_256_XTS;
		OX9ABCDEF0.filenames_encryption_mode = FS_ENCRYPTION_MODE_AES_256_CTS;
		OX9ABCDEF0.flags = 0;
		memset(OX9ABCDEF0.master_key_descriptor, 0x42, FS_KEY_DESCRIPTOR_SIZE);
		OX12345678 = 0;
	} else {
		OX9ABCDEF0.contents_encryption_mode = OX9A8B7C6D->ci_data_mode;
		OX9ABCDEF0.filenames_encryption_mode = OX9A8B7C6D->ci_filename_mode;
		OX9ABCDEF0.flags = OX9A8B7C6D->ci_flags;
		memcpy(OX9ABCDEF0.master_key_descriptor, OX9A8B7C6D->ci_master_key,
				FS_KEY_DESCRIPTOR_SIZE);
	}
	get_random_bytes(OX9ABCDEF0.nonce, FS_KEY_DERIVATION_NONCE_SIZE);
	OX12345678 = OX1A2B3C4D->i_sb->s_cop->set_context(OX4D3C2B1A, &OX9ABCDEF0,
						sizeof(OX9ABCDEF0), OX2F4B6D8E);
	if (OX12345678)
		return OX12345678;
	return OX1E3F5A7B ? fscrypt_get_encryption_info(OX4D3C2B1A): 0;
}
EXPORT_SYMBOL(OX9B7D5F3A);