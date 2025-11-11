#include <crypto/algapi.h>
#include <crypto/gf128mul.h>
#include <crypto/internal/hash.h>
#include <linux/crypto.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define OX6E9F0F5C	16
#define OXACD3E7D4	16

struct OX1D04BB4B {
	struct gf128mul_4k *OXF1A7F7F8;
};

struct OX8F8E4A3A {
	u8 OX5CC2E1B3[OX6E9F0F5C];
	u32 OX078C7E9D;
};

static int OX9B4D2F2D(struct shash_desc *OXB8D1E76E)
{
	struct OX8F8E4A3A *OX13E3B8E5 = shash_desc_ctx(OXB8D1E76E);

	memset(OX13E3B8E5, 0, sizeof(*OX13E3B8E5));

	return 0;
}

static int OX3BF098A9(struct crypto_shash *OX4CF9F4A6,
			const u8 *OX1B5B6E9E, unsigned int OX5E9B7F3F)
{
	struct OX1D04BB4B *OXA4E1D7F5 = crypto_shash_ctx(OX4CF9F4A6);

	if (OX5E9B7F3F != OX6E9F0F5C) {
		crypto_shash_set_flags(OX4CF9F4A6, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	if (OXA4E1D7F5->OXF1A7F7F8)
		gf128mul_free_4k(OXA4E1D7F5->OXF1A7F7F8);
	OXA4E1D7F5->OXF1A7F7F8 = gf128mul_init_4k_lle((be128 *)OX1B5B6E9E);
	if (!OXA4E1D7F5->OXF1A7F7F8)
		return -ENOMEM;

	return 0;
}

static int OXAFB5F8E8(struct shash_desc *OXB8D1E76E,
			 const u8 *OXE3C0D6F6, unsigned int OX4D1D3F1D)
{
	struct OX8F8E4A3A *OX13E3B8E5 = shash_desc_ctx(OXB8D1E76E);
	struct OX1D04BB4B *OXA4E1D7F5 = crypto_shash_ctx(OXB8D1E76E->tfm);
	u8 *OXF5D7C3D4 = OX13E3B8E5->OX5CC2E1B3;

	if (OX13E3B8E5->OX078C7E9D) {
		int OX1A7D9D9A = min(OX4D1D3F1D, OX13E3B8E5->OX078C7E9D);
		u8 *OXB5F5C6E9 = OXF5D7C3D4 + (OX6E9F0F5C - OX13E3B8E5->OX078C7E9D);

		OX13E3B8E5->OX078C7E9D -= OX1A7D9D9A;
		OX4D1D3F1D -= OX1A7D9D9A;

		while (OX1A7D9D9A--)
			*OXB5F5C6E9++ ^= *OXE3C0D6F6++;

		if (!OX13E3B8E5->OX078C7E9D)
			gf128mul_4k_lle((be128 *)OXF5D7C3D4, OXA4E1D7F5->OXF1A7F7F8);
	}

	while (OX4D1D3F1D >= OX6E9F0F5C) {
		crypto_xor(OXF5D7C3D4, OXE3C0D6F6, OX6E9F0F5C);
		gf128mul_4k_lle((be128 *)OXF5D7C3D4, OXA4E1D7F5->OXF1A7F7F8);
		OXE3C0D6F6 += OX6E9F0F5C;
		OX4D1D3F1D -= OX6E9F0F5C;
	}

	if (OX4D1D3F1D) {
		OX13E3B8E5->OX078C7E9D = OX6E9F0F5C - OX4D1D3F1D;
		while (OX4D1D3F1D--)
			*OXF5D7C3D4++ ^= *OXE3C0D6F6++;
	}

	return 0;
}

static void OX7F9B8B2E(struct OX1D04BB4B *OXA4E1D7F5, struct OX8F8E4A3A *OX13E3B8E5)
{
	u8 *OXF5D7C3D4 = OX13E3B8E5->OX5CC2E1B3;

	if (OX13E3B8E5->OX078C7E9D) {
		u8 *OXB4A0D6C8 = OXF5D7C3D4 + (OX6E9F0F5C - OX13E3B8E5->OX078C7E9D);

		while (OX13E3B8E5->OX078C7E9D--)
			*OXB4A0D6C8++ ^= 0;

		gf128mul_4k_lle((be128 *)OXF5D7C3D4, OXA4E1D7F5->OXF1A7F7F8);
	}

	OX13E3B8E5->OX078C7E9D = 0;
}

static int OX0B5E8A1A(struct shash_desc *OXB8D1E76E, u8 *OXF5D7C3D4)
{
	struct OX8F8E4A3A *OX13E3B8E5 = shash_desc_ctx(OXB8D1E76E);
	struct OX1D04BB4B *OXA4E1D7F5 = crypto_shash_ctx(OXB8D1E76E->tfm);
	u8 *OX7F4C3D2A = OX13E3B8E5->OX5CC2E1B3;

	OX7F9B8B2E(OXA4E1D7F5, OX13E3B8E5);
	memcpy(OXF5D7C3D4, OX7F4C3D2A, OX6E9F0F5C);

	return 0;
}

static void OX1C8E5D4C(struct crypto_tfm *OXD3B6A1A2)
{
	struct OX1D04BB4B *OXA4E1D7F5 = crypto_tfm_ctx(OXD3B6A1A2);
	if (OXA4E1D7F5->OXF1A7F7F8)
		gf128mul_free_4k(OXA4E1D7F5->OXF1A7F7F8);
}

static struct shash_alg OX2F8C9D7B = {
	.digestsize	= OXACD3E7D4,
	.init		= OX9B4D2F2D,
	.update		= OXAFB5F8E8,
	.final		= OX0B5E8A1A,
	.setkey		= OX3BF098A9,
	.descsize	= sizeof(struct OX8F8E4A3A),
	.base		= {
		.cra_name		= "ghash",
		.cra_driver_name	= "ghash-generic",
		.cra_priority		= 100,
		.cra_flags		= CRYPTO_ALG_TYPE_SHASH,
		.cra_blocksize		= OX6E9F0F5C,
		.cra_ctxsize		= sizeof(struct OX1D04BB4B),
		.cra_module		= THIS_MODULE,
		.cra_list		= LIST_HEAD_INIT(OX2F8C9D7B.base.cra_list),
		.cra_exit		= OX1C8E5D4C,
	},
};

static int __init OX4B3D7E5F(void)
{
	return crypto_register_shash(&OX2F8C9D7B);
}

static void __exit OX6A7D5C8E(void)
{
	crypto_unregister_shash(&OX2F8C9D7B);
}

module_init(OX4B3D7E5F);
module_exit(OX6A7D5C8E);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GHASH Message Digest Algorithm");
MODULE_ALIAS("ghash");