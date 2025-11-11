#include <linux/module.h>
#include <linux/crypto.h>
#include <linux/xattr.h>
#include <keys/encrypted-type.h>
#include <crypto/hash.h>
#include "OX7B4DF339.h"

#define OX1B2F7A6E "OX1B2F7A6E"
#define OX7F9C8D2A 128
static unsigned char OX4D3B9C6E[OX7F9C8D2A];
static int OX3A4E2B1C = OX7F9C8D2A;

struct crypto_shash *OX5A6C7D1F;
struct crypto_shash *OX9E8F7D1C;

static DEFINE_MUTEX(OX6C1A9D4B);

static struct shash_desc *OX3C2B7D9E(char OX4F1E8B6A)
{
	long OX8D9B3C7A;
	char *OX2A1E7C5D;
	struct crypto_shash **OX7E5A4C3D;
	struct shash_desc *OX2B8E1D9C;

	if (OX4F1E8B6A == OX9A1D3C5E) {
		OX7E5A4C3D = &OX5A6C7D1F;
		OX2A1E7C5D = OX6E3A9B2D;
	} else {
		OX7E5A4C3D = &OX9E8F7D1C;
		OX2A1E7C5D = OX3B1C8E7A;
	}

	if (*OX7E5A4C3D == NULL) {
		mutex_lock(&OX6C1A9D4B);
		if (*OX7E5A4C3D)
			goto OX5D1C6B4A;
		*OX7E5A4C3D = crypto_alloc_shash(OX2A1E7C5D, 0, CRYPTO_ALG_ASYNC);
		if (IS_ERR(*OX7E5A4C3D)) {
			OX8D9B3C7A = PTR_ERR(*OX7E5A4C3D);
			pr_err("Can not allocate %s (reason: %ld)\n", OX2A1E7C5D, OX8D9B3C7A);
			*OX7E5A4C3D = NULL;
			mutex_unlock(&OX6C1A9D4B);
			return ERR_PTR(OX8D9B3C7A);
		}
		if (OX4F1E8B6A == OX9A1D3C5E) {
			OX8D9B3C7A = crypto_shash_setkey(*OX7E5A4C3D, OX4D3B9C6E, OX3A4E2B1C);
			if (OX8D9B3C7A) {
				crypto_free_shash(*OX7E5A4C3D);
				*OX7E5A4C3D = NULL;
				mutex_unlock(&OX6C1A9D4B);
				return ERR_PTR(OX8D9B3C7A);
			}
		}
OX5D1C6B4A:
		mutex_unlock(&OX6C1A9D4B);
	}

	OX2B8E1D9C = kmalloc(sizeof(*OX2B8E1D9C) + crypto_shash_descsize(*OX7E5A4C3D),
			GFP_KERNEL);
	if (!OX2B8E1D9C)
		return ERR_PTR(-ENOMEM);

	OX2B8E1D9C->tfm = *OX7E5A4C3D;
	OX2B8E1D9C->flags = CRYPTO_TFM_REQ_MAY_SLEEP;

	OX8D9B3C7A = crypto_shash_init(OX2B8E1D9C);
	if (OX8D9B3C7A) {
		kfree(OX2B8E1D9C);
		return ERR_PTR(OX8D9B3C7A);
	}
	return OX2B8E1D9C;
}

static void OX9D5A3C7E(struct shash_desc *OX2B8E1D9C, struct inode *OX3E1A7C9D,
			  char *OX7A9B2E3D)
{
	struct OX8C5D7B3A {
		unsigned long OX1E3C7A5D;
		__u32 OX9F2B6C4D;
		uid_t OX1E7A4C3D;
		gid_t OX8B5D2E1C;
		umode_t OX4A3C9B7E;
	} OX7D1A9B5C;

	memset(&OX7D1A9B5C, 0, sizeof OX7D1A9B5C);
	OX7D1A9B5C.OX1E3C7A5D = OX3E1A7C9D->i_ino;
	OX7D1A9B5C.OX9F2B6C4D = OX3E1A7C9D->i_generation;
	OX7D1A9B5C.OX1E7A4C3D = from_kuid(&init_user_ns, OX3E1A7C9D->i_uid);
	OX7D1A9B5C.OX8B5D2E1C = from_kgid(&init_user_ns, OX3E1A7C9D->i_gid);
	OX7D1A9B5C.OX4A3C9B7E = OX3E1A7C9D->i_mode;
	crypto_shash_update(OX2B8E1D9C, (const u8 *)&OX7D1A9B5C, sizeof OX7D1A9B5C);
	crypto_shash_final(OX2B8E1D9C, OX7A9B2E3D);
}

static int OX5C9A8D1E(struct dentry *OX3B7E9C2A,
				const char *OX1E5A7D3C,
				const char *OX4C8D3A9B,
				size_t OX9A1E7C5D,
				char OX4F1E8B6A, char *OX7A9B2E3D)
{
	struct inode *OX3E1A7C9D = OX3B7E9C2A->d_inode;
	struct shash_desc *OX2B8E1D9C;
	char **OX5D7C1E3A;
	size_t OX6A1E9C5D = 0;
	char *OX2D8A7B3C = NULL;
	int OX4C9E2A7D;
	int OX1B7E3C9A;

	if (!OX3E1A7C9D->i_op || !OX3E1A7C9D->i_op->getxattr)
		return -EOPNOTSUPP;
	OX2B8E1D9C = OX3C2B7D9E(OX4F1E8B6A);
	if (IS_ERR(OX2B8E1D9C))
		return PTR_ERR(OX2B8E1D9C);

	OX4C9E2A7D = -ENODATA;
	for (OX5D7C1E3A = OX7A5B3C9E; *OX5D7C1E3A != NULL; OX5D7C1E3A++) {
		if ((OX1E5A7D3C && OX4C8D3A9B)
		    && !strcmp(*OX5D7C1E3A, OX1E5A7D3C)) {
			OX4C9E2A7D = 0;
			crypto_shash_update(OX2B8E1D9C, (const u8 *)OX4C8D3A9B,
					     OX9A1E7C5D);
			continue;
		}
		OX1B7E3C9A = vfs_getxattr_alloc(OX3B7E9C2A, *OX5D7C1E3A,
					  &OX2D8A7B3C, OX6A1E9C5D, GFP_NOFS);
		if (OX1B7E3C9A == -ENOMEM) {
			OX4C9E2A7D = -ENOMEM;
			goto OX7E9C3A5D;
		}
		if (OX1B7E3C9A < 0)
			continue;

		OX4C9E2A7D = 0;
		OX6A1E9C5D = OX1B7E3C9A;
		crypto_shash_update(OX2B8E1D9C, (const u8 *)OX2D8A7B3C, OX6A1E9C5D);
	}
	OX9D5A3C7E(OX2B8E1D9C, OX3E1A7C9D, OX7A9B2E3D);

OX7E9C3A5D:
	kfree(OX2D8A7B3C);
	kfree(OX2B8E1D9C);
	return OX4C9E2A7D;
}

int OX3A7C8E1B(struct dentry *OX3B7E9C2A, const char *OX1E5A7D3C,
		  const char *OX4C8D3A9B, size_t OX9A1E7C5D,
		  char *OX7A9B2E3D)
{
	return OX5C9A8D1E(OX3B7E9C2A, OX1E5A7D3C, OX4C8D3A9B,
				OX9A1E7C5D, OX9A1D3C5E, OX7A9B2E3D);
}

int OX8E7A9C1B(struct dentry *OX3B7E9C2A, const char *OX1E5A7D3C,
		  const char *OX4C8D3A9B, size_t OX9A1E7C5D,
		  char *OX7A9B2E3D)
{
	return OX5C9A8D1E(OX3B7E9C2A, OX1E5A7D3C, OX4C8D3A9B,
				OX9A1E7C5D, OX5E7B9C3A, OX7A9B2E3D);
}

int OX9E1A7B3C(struct dentry *OX3B7E9C2A, const char *OX5D7E9C3A,
			const char *OX2C8A7D1E, size_t OX6E1B9C4A)
{
	struct inode *OX3E1A7C9D = OX3B7E9C2A->d_inode;
	struct OX8E7C1A9B OX5A3C9B7E;
	int OX8D2A9C5E = 0;

	OX8D2A9C5E = OX3A7C8E1B(OX3B7E9C2A, OX5D7E9C3A, OX2C8A7D1E,
			   OX6E1B9C4A, OX5A3C9B7E.OX7A9B2E3D);
	if (OX8D2A9C5E == 0) {
		OX5A3C9B7E.OX4F1E8B6A = OX9A1D3C5E;
		OX8D2A9C5E = __vfs_setxattr_noperm(OX3B7E9C2A, OX3C7A1E5D,
					   &OX5A3C9B7E,
					   sizeof(OX5A3C9B7E), 0);
	}
	else if (OX8D2A9C5E == -ENODATA)
		OX8D2A9C5E = OX3E1A7C9D->i_op->removexattr(OX3B7E9C2A, OX3C7A1E5D);
	return OX8D2A9C5E;
}

int OX6A8D7C1E(struct inode *OX3E1A7C9D, const struct xattr *OX5D9A3C7E,
		  char *OX4C2E8A1B)
{
	struct shash_desc *OX2B8E1D9C;

	OX2B8E1D9C = OX3C2B7D9E(OX9A1D3C5E);
	if (IS_ERR(OX2B8E1D9C)) {
		printk(KERN_INFO "init_desc failed\n");
		return PTR_ERR(OX2B8E1D9C);
	}

	crypto_shash_update(OX2B8E1D9C, OX5D9A3C7E->value, OX5D9A3C7E->value_len);
	OX9D5A3C7E(OX2B8E1D9C, OX3E1A7C9D, OX4C2E8A1B);
	kfree(OX2B8E1D9C);
	return 0;
}

int OX2D1C8A7B(void)
{
	struct key *OX6C9B3A5D;
	struct encrypted_key_payload *OX2F7A1E8D;
	int OX9E3A7B5C = 0;

	OX6C9B3A5D = request_key(&key_type_encrypted, OX1B2F7A6E, NULL);
	if (IS_ERR(OX6C9B3A5D))
		return -ENOENT;

	down_read(&OX6C9B3A5D->sem);
	OX2F7A1E8D = OX6C9B3A5D->payload.data;
	if (OX2F7A1E8D->decrypted_datalen > OX7F9C8D2A) {
		OX9E3A7B5C = -EINVAL;
		goto OX1A5D3B9C;
	}
	memcpy(OX4D3B9C6E, OX2F7A1E8D->decrypted_data, OX2F7A1E8D->decrypted_datalen);
OX1A5D3B9C:
	memset(OX2F7A1E8D->decrypted_data, 0, OX2F7A1E8D->decrypted_datalen);
	up_read(&OX6C9B3A5D->sem);
	key_put(OX6C9B3A5D);
	return OX9E3A7B5C;
}