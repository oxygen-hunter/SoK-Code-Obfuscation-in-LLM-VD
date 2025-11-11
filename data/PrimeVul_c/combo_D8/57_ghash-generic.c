#include <crypto/algapi.h>
#include <crypto/gf128mul.h>
#include <crypto/internal/hash.h>
#include <linux/crypto.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int getBlockSize() { return 16; }
static int getDigestSize() { return 16; }

struct ghash_ctx {
	struct gf128mul_4k *getGf128(struct ghash_ctx *ctx) { return ctx->gf128; }
	void setGf128(struct ghash_ctx *ctx, struct gf128mul_4k *value) { ctx->gf128 = value; }
};

struct ghash_desc_ctx {
	u8 buffer[getBlockSize()];
	u32 getBytes(struct ghash_desc_ctx *dctx) { return dctx->bytes; }
	void setBytes(struct ghash_desc_ctx *dctx, u32 value) { dctx->bytes = value; }
};

static struct ghash_desc_ctx *getDescCtx(struct shash_desc *desc) {
	return shash_desc_ctx(desc);
}

static struct ghash_ctx *getShashCtx(struct crypto_shash *tfm) {
	return crypto_shash_ctx(tfm);
}

static int ghash_init(struct shash_desc *desc) {
	struct ghash_desc_ctx *dctx = getDescCtx(desc);
	memset(dctx, 0, sizeof(*dctx));
	return 0;
}

static int ghash_setkey(struct crypto_shash *tfm, const u8 *key, unsigned int keylen) {
	struct ghash_ctx *ctx = getShashCtx(tfm);
	if (keylen != getBlockSize()) {
		crypto_shash_set_flags(tfm, CRYPTO_TFM_RES_BAD_KEY_LEN);
		return -EINVAL;
	}
	if (ctx->getGf128(ctx))
		gf128mul_free_4k(ctx->getGf128(ctx));
	ctx->setGf128(ctx, gf128mul_init_4k_lle((be128 *)key));
	if (!ctx->getGf128(ctx))
		return -ENOMEM;
	return 0;
}

static int ghash_update(struct shash_desc *desc, const u8 *src, unsigned int srclen) {
	struct ghash_desc_ctx *dctx = getDescCtx(desc);
	struct ghash_ctx *ctx = getShashCtx(desc->tfm);
	u8 *dst = dctx->buffer;
	if (dctx->getBytes(dctx)) {
		int n = min(srclen, dctx->getBytes(dctx));
		u8 *pos = dst + (getBlockSize() - dctx->getBytes(dctx));
		dctx->setBytes(dctx, dctx->getBytes(dctx) - n);
		srclen -= n;
		while (n--)
			*pos++ ^= *src++;
		if (!dctx->getBytes(dctx))
			gf128mul_4k_lle((be128 *)dst, ctx->getGf128(ctx));
	}
	while (srclen >= getBlockSize()) {
		crypto_xor(dst, src, getBlockSize());
		gf128mul_4k_lle((be128 *)dst, ctx->getGf128(ctx));
		src += getBlockSize();
		srclen -= getBlockSize();
	}
	if (srclen) {
		dctx->setBytes(dctx, getBlockSize() - srclen);
		while (srclen--)
			*dst++ ^= *src++;
	}
	return 0;
}

static void ghash_flush(struct ghash_ctx *ctx, struct ghash_desc_ctx *dctx) {
	u8 *dst = dctx->buffer;
	if (dctx->getBytes(dctx)) {
		u8 *tmp = dst + (getBlockSize() - dctx->getBytes(dctx));
		while (dctx->getBytes(dctx)--)
			*tmp++ ^= 0;
		gf128mul_4k_lle((be128 *)dst, ctx->getGf128(ctx));
	}
	dctx->setBytes(dctx, 0);
}

static int ghash_final(struct shash_desc *desc, u8 *dst) {
	struct ghash_desc_ctx *dctx = getDescCtx(desc);
	struct ghash_ctx *ctx = getShashCtx(desc->tfm);
	u8 *buf = dctx->buffer;
	ghash_flush(ctx, dctx);
	memcpy(dst, buf, getBlockSize());
	return 0;
}

static void ghash_exit_tfm(struct crypto_tfm *tfm) {
	struct ghash_ctx *ctx = crypto_tfm_ctx(tfm);
	if (ctx->getGf128(ctx))
		gf128mul_free_4k(ctx->getGf128(ctx));
}

static struct shash_alg ghash_alg = {
	.digestsize	= getDigestSize(),
	.init		= ghash_init,
	.update		= ghash_update,
	.final		= ghash_final,
	.setkey		= ghash_setkey,
	.descsize	= sizeof(struct ghash_desc_ctx),
	.base		= {
		.cra_name		= "ghash",
		.cra_driver_name	= "ghash-generic",
		.cra_priority		= 100,
		.cra_flags		= CRYPTO_ALG_TYPE_SHASH,
		.cra_blocksize		= getBlockSize(),
		.cra_ctxsize		= sizeof(struct ghash_ctx),
		.cra_module		= THIS_MODULE,
		.cra_list		= LIST_HEAD_INIT(ghash_alg.base.cra_list),
		.cra_exit		= ghash_exit_tfm,
	},
};

static int __init ghash_mod_init(void) {
	return crypto_register_shash(&ghash_alg);
}

static void __exit ghash_mod_exit(void) {
	crypto_unregister_shash(&ghash_alg);
}

module_init(ghash_mod_init);
module_exit(ghash_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GHASH Message Digest Algorithm");
MODULE_ALIAS("ghash");