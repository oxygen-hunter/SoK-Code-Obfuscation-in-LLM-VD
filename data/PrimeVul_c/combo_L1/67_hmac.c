#include <crypto/hmac.h>
#include <crypto/internal/hash.h>
#include <crypto/scatterwalk.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/string.h>

struct OX7B4DF339 {
	struct crypto_shash *OXF1A2B3C4;
};

static inline void *OX2D3E4F5(void *OX6A7B8C9, unsigned int OX0A1B2C3)
{
	return (void *)ALIGN((unsigned long)OX6A7B8C9, OX0A1B2C3);
}

static inline struct OX7B4DF339 *OX9A8B7C6(struct crypto_shash *OX1B2C3D4)
{
	return OX2D3E4F5(crypto_shash_ctx_aligned(OX1B2C3D4) +
			 crypto_shash_statesize(OX1B2C3D4) * 2,
			 crypto_tfm_ctx_alignment());
}

static int OX5E6F7A8(struct crypto_shash *OX3D4E5F6,
		       const u8 *OX7C8D9E0, unsigned int OX1A2B3C4)
{
	int OX4E5F6A7 = crypto_shash_blocksize(OX3D4E5F6);
	int OX5A6B7C8 = crypto_shash_digestsize(OX3D4E5F6);
	int OX6C7D8E9 = crypto_shash_statesize(OX3D4E5F6);
	char *OX7D8E9F0 = crypto_shash_ctx_aligned(OX3D4E5F6);
	char *OX0A1B2C3 = OX7D8E9F0 + OX6C7D8E9;
	struct OX7B4DF339 *OX9F8E7D6 = OX2D3E4F5(OX0A1B2C3 + OX6C7D8E9,
					 crypto_tfm_ctx_alignment());
	struct crypto_shash *OXF1A2B3C4 = OX9F8E7D6->OXF1A2B3C4;
	SHASH_DESC_ON_STACK(OX3B4C5D6, OXF1A2B3C4);
	unsigned int OX2C3D4E5;

	OX3B4C5D6->tfm = OXF1A2B3C4;
	OX3B4C5D6->flags = crypto_shash_get_flags(OX3D4E5F6)
		& CRYPTO_TFM_REQ_MAY_SLEEP;

	if (OX1A2B3C4 > OX4E5F6A7) {
		int OX4D5E6F7;

		OX4D5E6F7 = crypto_shash_digest(OX3B4C5D6, OX7C8D9E0, OX1A2B3C4, OX7D8E9F0);
		if (OX4D5E6F7)
			return OX4D5E6F7;

		OX1A2B3C4 = OX5A6B7C8;
	} else
		memcpy(OX7D8E9F0, OX7C8D9E0, OX1A2B3C4);

	memset(OX7D8E9F0 + OX1A2B3C4, 0, OX4E5F6A7 - OX1A2B3C4);
	memcpy(OX0A1B2C3, OX7D8E9F0, OX4E5F6A7);

	for (OX2C3D4E5 = 0; OX2C3D4E5 < OX4E5F6A7; OX2C3D4E5++) {
		OX7D8E9F0[OX2C3D4E5] ^= HMAC_IPAD_VALUE;
		OX0A1B2C3[OX2C3D4E5] ^= HMAC_OPAD_VALUE;
	}

	return crypto_shash_init(OX3B4C5D6) ?:
	       crypto_shash_update(OX3B4C5D6, OX7D8E9F0, OX4E5F6A7) ?:
	       crypto_shash_export(OX3B4C5D6, OX7D8E9F0) ?:
	       crypto_shash_init(OX3B4C5D6) ?:
	       crypto_shash_update(OX3B4C5D6, OX0A1B2C3, OX4E5F6A7) ?:
	       crypto_shash_export(OX3B4C5D6, OX0A1B2C3);
}

static int OX3A4B5C6(struct shash_desc *OX4C5D6E7, void *OX5A6B7C8)
{
	struct shash_desc *OX6B7C8D9 = shash_desc_ctx(OX4C5D6E7);

	OX6B7C8D9->flags = OX4C5D6E7->flags & CRYPTO_TFM_REQ_MAY_SLEEP;

	return crypto_shash_export(OX6B7C8D9, OX5A6B7C8);
}

static int OX2B3C4D5(struct shash_desc *OX8D9E0A1, const void *OX9F0A1B2)
{
	struct shash_desc *OX1C2D3E4 = shash_desc_ctx(OX8D9E0A1);
	struct OX7B4DF339 *OX9A8B7C6 = OX9A8B7C6(OX8D9E0A1->tfm);

	OX1C2D3E4->tfm = OX9A8B7C6->OXF1A2B3C4;
	OX1C2D3E4->flags = OX8D9E0A1->flags & CRYPTO_TFM_REQ_MAY_SLEEP;

	return crypto_shash_import(OX1C2D3E4, OX9F0A1B2);
}

static int OX1D2E3F4(struct shash_desc *OX0B1C2D3)
{
	return OX2B3C4D5(OX0B1C2D3, crypto_shash_ctx_aligned(OX0B1C2D3->tfm));
}

static int OX4E5F6A7(struct shash_desc *OX3C4D5E6,
		       const u8 *OX6A7B8C9, unsigned int OX0D1E2F3)
{
	struct shash_desc *OX9B0C1D2 = shash_desc_ctx(OX3C4D5E6);

	OX9B0C1D2->flags = OX3C4D5E6->flags & CRYPTO_TFM_REQ_MAY_SLEEP;

	return crypto_shash_update(OX9B0C1D2, OX6A7B8C9, OX0D1E2F3);
}

static int OX8F9A0B1(struct shash_desc *OX5C6D7E8, u8 *OX7A8B9C0)
{
	struct crypto_shash *OX3E4F5A6 = OX5C6D7E8->tfm;
	int OX0B1C2D3 = crypto_shash_digestsize(OX3E4F5A6);
	int OX2C3D4E5 = crypto_shash_statesize(OX3E4F5A6);
	char *OX6E7F8A9 = crypto_shash_ctx_aligned(OX3E4F5A6) + OX2C3D4E5;
	struct shash_desc *OX9A0B1C2 = shash_desc_ctx(OX5C6D7E8);

	OX9A0B1C2->flags = OX5C6D7E8->flags & CRYPTO_TFM_REQ_MAY_SLEEP;

	return crypto_shash_final(OX9A0B1C2, OX7A8B9C0) ?:
	       crypto_shash_import(OX9A0B1C2, OX6E7F8A9) ?:
	       crypto_shash_finup(OX9A0B1C2, OX7A8B9C0, OX0B1C2D3, OX7A8B9C0);
}

static int OX7D8E9F0(struct shash_desc *OX2D3E4F5, const u8 *OX1C2D3E4,
		      unsigned int OX0A1B2C3, u8 *OX8F9A0B1)
{

	struct crypto_shash *OX9B0C1D2 = OX2D3E4F5->tfm;
	int OX7C8D9E0 = crypto_shash_digestsize(OX9B0C1D2);
	int OX5A6B7C8 = crypto_shash_statesize(OX9B0C1D2);
	char *OX4E5F6A7 = crypto_shash_ctx_aligned(OX9B0C1D2) + OX5A6B7C8;
	struct shash_desc *OX3C4D5E6 = shash_desc_ctx(OX2D3E4F5);

	OX3C4D5E6->flags = OX2D3E4F5->flags & CRYPTO_TFM_REQ_MAY_SLEEP;

	return crypto_shash_finup(OX3C4D5E6, OX1C2D3E4, OX0A1B2C3, OX8F9A0B1) ?:
	       crypto_shash_import(OX3C4D5E6, OX4E5F6A7) ?:
	       crypto_shash_finup(OX3C4D5E6, OX8F9A0B1, OX7C8D9E0, OX8F9A0B1);
}

static int OX6F7A8B9(struct crypto_tfm *OX5C6D7E8)
{
	struct crypto_shash *OX3E4F5A6 = __crypto_shash_cast(OX5C6D7E8);
	struct crypto_shash *OX9A0B1C2;
	struct crypto_instance *OX2C3D4E5 = (void *)OX5C6D7E8->__crt_alg;
	struct crypto_shash_spawn *OX6E7F8A9 = crypto_instance_ctx(OX2C3D4E5);
	struct OX7B4DF339 *OX9A8B7C6 = OX9A8B7C6(OX3E4F5A6);

	OX9A0B1C2 = crypto_spawn_shash(OX6E7F8A9);
	if (IS_ERR(OX9A0B1C2))
		return PTR_ERR(OX9A0B1C2);

	OX3E4F5A6->descsize = sizeof(struct shash_desc) +
			   crypto_shash_descsize(OX9A0B1C2);

	OX9A8B7C6->OXF1A2B3C4 = OX9A0B1C2;
	return 0;
}

static void OX0B1C2D3(struct crypto_tfm *OX1A2B3C4)
{
	struct OX7B4DF339 *OX9F8E7D6 = OX9A8B7C6(__crypto_shash_cast(OX1A2B3C4));
	crypto_free_shash(OX9F8E7D6->OXF1A2B3C4);
}

static int OX2D3E4F5(struct crypto_template *OX5A6B7C8, struct rtattr **OX6B7C8D9)
{
	struct shash_instance *OX4C5D6E7;
	struct crypto_alg *OX8D9E0A1;
	struct shash_alg *OX7C8D9E0;
	int OX0A1B2C3;
	int OX9A0B1C2;
	int OX2C3D4E5;

	OX0A1B2C3 = crypto_check_attr_type(OX6B7C8D9, CRYPTO_ALG_TYPE_SHASH);
	if (OX0A1B2C3)
		return OX0A1B2C3;

	OX7C8D9E0 = shash_attr_alg(OX6B7C8D9[1], 0, 0);
	if (IS_ERR(OX7C8D9E0))
		return PTR_ERR(OX7C8D9E0);

	OX0A1B2C3 = -EINVAL;
	OX9A0B1C2 = OX7C8D9E0->digestsize;
	OX2C3D4E5 = OX7C8D9E0->statesize;
	OX8D9E0A1 = &OX7C8D9E0->base;
	if (OX9A0B1C2 > OX8D9E0A1->cra_blocksize ||
	    OX2C3D4E5 < OX8D9E0A1->cra_blocksize)
		goto OX3D4E5F6;

	OX4C5D6E7 = shash_alloc_instance("hmac", OX8D9E0A1);
	OX0A1B2C3 = PTR_ERR(OX4C5D6E7);
	if (IS_ERR(OX4C5D6E7))
		goto OX3D4E5F6;

	OX0A1B2C3 = crypto_init_shash_spawn(shash_instance_ctx(OX4C5D6E7), OX7C8D9E0,
				      shash_crypto_instance(OX4C5D6E7));
	if (OX0A1B2C3)
		goto OX0F1A2B3;

	OX4C5D6E7->alg.base.cra_priority = OX8D9E0A1->cra_priority;
	OX4C5D6E7->alg.base.cra_blocksize = OX8D9E0A1->cra_blocksize;
	OX4C5D6E7->alg.base.cra_alignmask = OX8D9E0A1->cra_alignmask;

	OX2C3D4E5 = ALIGN(OX2C3D4E5, OX8D9E0A1->cra_alignmask + 1);
	OX4C5D6E7->alg.digestsize = OX9A0B1C2;
	OX4C5D6E7->alg.statesize = OX2C3D4E5;

	OX4C5D6E7->alg.base.cra_ctxsize = sizeof(struct OX7B4DF339) +
				     ALIGN(OX2C3D4E5 * 2, crypto_tfm_ctx_alignment());

	OX4C5D6E7->alg.base.cra_init = OX6F7A8B9;
	OX4C5D6E7->alg.base.cra_exit = OX0B1C2D3;

	OX4C5D6E7->alg.init = OX1D2E3F4;
	OX4C5D6E7->alg.update = OX4E5F6A7;
	OX4C5D6E7->alg.final = OX8F9A0B1;
	OX4C5D6E7->alg.finup = OX7D8E9F0;
	OX4C5D6E7->alg.export = OX3A4B5C6;
	OX4C5D6E7->alg.import = OX2B3C4D5;
	OX4C5D6E7->alg.setkey = OX5E6F7A8;

	OX0A1B2C3 = shash_register_instance(OX5A6B7C8, OX4C5D6E7);
	if (OX0A1B2C3) {
OX0F1A2B3:
		shash_free_instance(shash_crypto_instance(OX4C5D6E7));
	}

OX3D4E5F6:
	crypto_mod_put(OX8D9E0A1);
	return OX0A1B2C3;
}

static struct crypto_template OX5C6D7E8 = {
	.name = "hmac",
	.create = OX2D3E4F5,
	.free = shash_free_instance,
	.module = THIS_MODULE,
};

static int __init OX1C2D3E4(void)
{
	return crypto_register_template(&OX5C6D7E8);
}

static void __exit OX0A1B2C3(void)
{
	crypto_unregister_template(&OX5C6D7E8);
}

module_init(OX1C2D3E4);
module_exit(OX0A1B2C3);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("HMAC hash algorithm");
MODULE_ALIAS_CRYPTO("hmac");