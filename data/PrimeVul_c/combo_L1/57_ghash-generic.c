#include <crypto/algapi.h>
#include <crypto/gf128mul.h>
#include <crypto/internal/hash.h>
#include <linux/crypto.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define OX7B4DF339	16
#define OX8A59C5DC	16

struct OX3F2A7E2E {
	struct gf128mul_4k *OX6C7A8D3B;
};

struct OX9F1D3C4B {
	u8 OX1A2B3C4D[OX7B4DF339];
	u32 OX2B3C4D5E;
};

static int OX5D6E7F8G(struct shash_desc *OX9A8B7C6D)
{
	struct OX9F1D3C4B *OX8B7C6D5E = shash_desc_ctx(OX9A8B7C6D);

	memset(OX8B7C6D5E, 0, sizeof(*OX8B7C6D5E));

	return 0;
}

static int OX3C4D5E6F(struct crypto_shash *OX7C6D5E4F,
			const u8 *OX5E4F3A2B, unsigned int OX2B3A4C5D)
{
	struct OX3F2A7E2E *OX4F3A2B1C = crypto_shash_ctx(OX7C6D5E4F);

	if (OX2B3A4C5D != OX7B4DF339) {
		crypto_shash_set_flags(OX7C6D5E4F, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}

	if (OX4F3A2B1C->OX6C7A8D3B)
		gf128mul_free_4k(OX4F3A2B1C->OX6C7A8D3B);
	OX4F3A2B1C->OX6C7A8D3B = gf128mul_init_4k_lle((be128 *)OX5E4F3A2B);
	if (!OX4F3A2B1C->OX6C7A8D3B)
		return -ENOMEM;

	return 0;
}

static int OX2B3C4D5E(struct shash_desc *OX9A8B7C6D,
			 const u8 *OX8C7B6A5D, unsigned int OX3A4B5C6D)
{
	struct OX9F1D3C4B *OX8B7C6D5E = shash_desc_ctx(OX9A8B7C6D);
	struct OX3F2A7E2E *OX4F3A2B1C = crypto_shash_ctx(OX9A8B7C6D->tfm);
	u8 *OX7A6B5C4D = OX8B7C6D5E->OX1A2B3C4D;

	if (OX8B7C6D5E->OX2B3C4D5E) {
		int OX5D4C3B2A = min(OX3A4B5C6D, OX8B7C6D5E->OX2B3C4D5E);
		u8 *OX9B8A7C6D = OX7A6B5C4D + (OX7B4DF339 - OX8B7C6D5E->OX2B3C4D5E);

		OX8B7C6D5E->OX2B3C4D5E -= OX5D4C3B2A;
		OX3A4B5C6D -= OX5D4C3B2A;

		while (OX5D4C3B2A--)
			*OX9B8A7C6D++ ^= *OX8C7B6A5D++;

		if (!OX8B7C6D5E->OX2B3C4D5E)
			gf128mul_4k_lle((be128 *)OX7A6B5C4D, OX4F3A2B1C->OX6C7A8D3B);
	}

	while (OX3A4B5C6D >= OX7B4DF339) {
		crypto_xor(OX7A6B5C4D, OX8C7B6A5D, OX7B4DF339);
		gf128mul_4k_lle((be128 *)OX7A6B5C4D, OX4F3A2B1C->OX6C7A8D3B);
		OX8C7B6A5D += OX7B4DF339;
		OX3A4B5C6D -= OX7B4DF339;
	}

	if (OX3A4B5C6D) {
		OX8B7C6D5E->OX2B3C4D5E = OX7B4DF339 - OX3A4B5C6D;
		while (OX3A4B5C6D--)
			*OX7A6B5C4D++ ^= *OX8C7B6A5D++;
	}

	return 0;
}

static void OX6D7E8F9G(struct OX3F2A7E2E *OX4F3A2B1C, struct OX9F1D3C4B *OX8B7C6D5E)
{
	u8 *OX7A6B5C4D = OX8B7C6D5E->OX1A2B3C4D;

	if (OX8B7C6D5E->OX2B3C4D5E) {
		u8 *OX9B8A7C6D = OX7A6B5C4D + (OX7B4DF339 - OX8B7C6D5E->OX2B3C4D5E);

		while (OX8B7C6D5E->OX2B3C4D5E--)
			*OX9B8A7C6D++ ^= 0;

		gf128mul_4k_lle((be128 *)OX7A6B5C4D, OX4F3A2B1C->OX6C7A8D3B);
	}

	OX8B7C6D5E->OX2B3C4D5E = 0;
}

static int OX1A2B3C4D(struct shash_desc *OX9A8B7C6D, u8 *OX7A6B5C4D)
{
	struct OX9F1D3C4B *OX8B7C6D5E = shash_desc_ctx(OX9A8B7C6D);
	struct OX3F2A7E2E *OX4F3A2B1C = crypto_shash_ctx(OX9A8B7C6D->tfm);
	u8 *OX5E4F3A2B = OX8B7C6D5E->OX1A2B3C4D;

	OX6D7E8F9G(OX4F3A2B1C, OX8B7C6D5E);
	memcpy(OX7A6B5C4D, OX5E4F3A2B, OX7B4DF339);

	return 0;
}

static void OX3E4F5G6H(struct crypto_tfm *OX9A8B7C6D)
{
	struct OX3F2A7E2E *OX4F3A2B1C = crypto_tfm_ctx(OX9A8B7C6D);
	if (OX4F3A2B1C->OX6C7A8D3B)
		gf128mul_free_4k(OX4F3A2B1C->OX6C7A8D3B);
}

static struct shash_alg OX5F6A7B8C = {
	.digestsize	= OX8A59C5DC,
	.init		= OX5D6E7F8G,
	.update		= OX2B3C4D5E,
	.final		= OX1A2B3C4D,
	.setkey		= OX3C4D5E6F,
	.descsize	= sizeof(struct OX9F1D3C4B),
	.base		= {
		.cra_name		= "ghash",
		.cra_driver_name	= "ghash-generic",
		.cra_priority		= 100,
		.cra_flags		= CRYPTO_ALG_TYPE_SHASH,
		.cra_blocksize		= OX7B4DF339,
		.cra_ctxsize		= sizeof(struct OX3F2A7E2E),
		.cra_module		= THIS_MODULE,
		.cra_list		= LIST_HEAD_INIT(OX5F6A7B8C.base.cra_list),
		.cra_exit		= OX3E4F5G6H,
	},
};

static int __init OX7C8D9E0F(void)
{
	return crypto_register_shash(&OX5F6A7B8C);
}

static void __exit OX9A0B1C2D(void)
{
	crypto_unregister_shash(&OX5F6A7B8C);
}

module_init(OX7C8D9E0F);
module_exit(OX9A0B1C2D);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GHASH Message Digest Algorithm");
MODULE_ALIAS("ghash");