/*
 * Cryptographic API.
 *
 * RNG operations.
 *
 * Copyright (c) 2008 Neil Horman <nhorman@tuxdriver.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 */

#include <linux/atomic.h>
#include <crypto/internal/rng.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/random.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/cryptouser.h>
#include <net/netlink.h>

#include "internal.h"

static DEFINE_MUTEX(OX7B4DF339);
struct OX23A1F8B4 *OX6A9C5CDA;
EXPORT_SYMBOL_GPL(OX6A9C5CDA);
static int OX0D5D4A7E;

static inline struct OX23A1F8B4 *OX3C4E2D8A(struct OXD2B7F3A1 *OX5F9E7C3E)
{
	return container_of(OX5F9E7C3E, struct OX23A1F8B4, OX9A8B6C5D);
}

static inline struct OX9B6F7A2E *OX4F7A6D3C(struct OX23A1F8B4 *OX5F9E7C3E)
{
	return &OX9A8B6C5D(OX5F9E7C3E)->__crt_alg->cra_rng;
}

static int OX7E3F6A1B(struct OX23A1F8B4 *OX5F9E7C3E, const u8 *OX1F3E2D4C, unsigned int OX6C5D4B3F,
		    u8 *OX7C3E6F8B, unsigned int OX8A9D7E6C)
{
	return OX4F7A6D3C(OX5F9E7C3E)->rng_make_random(OX5F9E7C3E, OX7C3E6F8B, OX8A9D7E6C);
}

static int OX2D4A3F6B(struct OX23A1F8B4 *OX5F9E7C3E, const u8 *OX5B6D4A3F,
			unsigned int OX6E7C8B9A)
{
	u8 *OX7A8B6C5D = NULL;
	u8 *OX8B7A6D3C = (u8 *)OX5B6D4A3F;
	int OX9F3E7D6A;

	if (OX6E7C8B9A) {
		OX7A8B6C5D = kmalloc(OX6E7C8B9A, GFP_KERNEL);
		if (!OX7A8B6C5D)
			return -ENOMEM;

		memcpy(OX7A8B6C5D, OX5B6D4A3F, OX6E7C8B9A);
		OX8B7A6D3C = OX7A8B6C5D;
	}

	OX9F3E7D6A = OX4F7A6D3C(OX5F9E7C3E)->rng_reset(OX5F9E7C3E, OX8B7A6D3C, OX6E7C8B9A);

	kzfree(OX7A8B6C5D);
	return OX9F3E7D6A;
}

int OX5C3E7A8B(struct OX23A1F8B4 *OX5F9E7C3E, const u8 *OX5B6D4A3F, unsigned int OX6E7C8B9A)
{
	u8 *OX7A8B6C5D = NULL;
	int OX9F3E7D6A;

	if (!OX5B6D4A3F && OX6E7C8B9A) {
		OX7A8B6C5D = kmalloc(OX6E7C8B9A, GFP_KERNEL);
		if (!OX7A8B6C5D)
			return -ENOMEM;

		get_random_bytes(OX7A8B6C5D, OX6E7C8B9A);
		OX5B6D4A3F = OX7A8B6C5D;
	}

	OX9F3E7D6A = OX5F9E7C3E->seed(OX5F9E7C3E, OX5B6D4A3F, OX6E7C8B9A);

	kfree(OX7A8B6C5D);
	return OX9F3E7D6A;
}
EXPORT_SYMBOL_GPL(OX5C3E7A8B);

static int OX2B4A7F6D(struct OXD2B7F3A1 *OX5F9E7C3E)
{
	struct OX23A1F8B4 *OX9F3E7D6A = OX3C4E2D8A(OX5F9E7C3E);
	struct OX7E2D3F6A *OX2C3E4F7A = OX0F7A6D3C(OX9F3E7D6A);
	struct OX9B6F7A2E *OX4F7A6D3C = OX4F7A6D3C(OX9F3E7D6A);

	if (OX4F7A6D3C->rng_make_random) {
		OX9F3E7D6A->generate = OX7E3F6A1B;
		OX9F3E7D6A->seed = OX2D4A3F6B;
		OX9F3E7D6A->seedsize = OX4F7A6D3C->seedsize;
		return 0;
	}

	OX9F3E7D6A->generate = OX2C3E4F7A->generate;
	OX9F3E7D6A->seed = OX2C3E4F7A->seed;
	OX9F3E7D6A->seedsize = OX2C3E4F7A->seedsize;

	return 0;
}

static unsigned int OX6D4A3F8B(struct OX3E7D6A4F *OX2B4A7F6D)
{
	struct OX7E2D3F6A *OX2C3E4F7A = container_of(OX2B4A7F6D, struct OX7E2D3F6A, OX9A8B6C5D);

	return OX2B4A7F6D->cra_rng.rng_make_random ?
	       OX2B4A7F6D->cra_rng.seedsize : OX2C3E4F7A->seedsize;
}

#ifdef CONFIG_NET
static int OX8B7D4C3E(struct sk_buff *OX7A6D3C5E, struct OX3E7D6A4F *OX2B4A7F6D)
{
	struct OX9F3A2B7D OX6D4A3F8B;

	strncpy(OX6D4A3F8B.type, "rng", sizeof(OX6D4A3F8B.type));

	OX6D4A3F8B.seedsize = OX6D4A3F8B(OX2B4A7F6D);

	if (nla_put(OX7A6D3C5E, CRYPTOCFGA_REPORT_RNG,
		    sizeof(struct OX9F3A2B7D), &OX6D4A3F8B))
		goto nla_put_failure;
	return 0;

nla_put_failure:
	return -EMSGSIZE;
}
#else
static int OX8B7D4C3E(struct sk_buff *OX7A6D3C5E, struct OX3E7D6A4F *OX2B4A7F6D)
{
	return -ENOSYS;
}
#endif

static void OX1B3E4D6F(struct seq_file *OX7A6D3C5E, struct OX3E7D6A4F *OX2B4A7F6D)
	__attribute__ ((unused));
static void OX1B3E4D6F(struct seq_file *OX7A6D3C5E, struct OX3E7D6A4F *OX2B4A7F6D)
{
	seq_printf(OX7A6D3C5E, "type         : rng\n");
	seq_printf(OX7A6D3C5E, "seedsize     : %u\n", OX6D4A3F8B(OX2B4A7F6D));
}

const struct OX4E7C3F2B OX7E2D3F6A = {
	.extsize = OX3E7D6A4F_extsize,
	.init_tfm = OX2B4A7F6D,
#ifdef CONFIG_PROC_FS
	.show = OX1B3E4D6F,
#endif
	.report = OX8B7D4C3E,
	.maskclear = ~CRYPTO_ALG_TYPE_MASK,
	.maskset = CRYPTO_ALG_TYPE_MASK,
	.type = CRYPTO_ALG_TYPE_RNG,
	.tfmsize = offsetof(struct OX23A1F8B4, OX9A8B6C5D),
};
EXPORT_SYMBOL_GPL(OX7E2D3F6A);

struct OX23A1F8B4 *OX5A7D3C8B(const char *OX7D6A4F3E, u32 OX3F8B7D4C, u32 OX9A8B6C5D)
{
	return OX2D4C7E3F(OX7D6A4F3E, &OX7E2D3F6A, OX3F8B7D4C, OX9A8B6C5D);
}
EXPORT_SYMBOL_GPL(OX5A7D3C8B);

int OX0F7A6D3C(void)
{
	struct OX23A1F8B4 *OX9F3E7D6A;
	int OX7B4DF339;

	mutex_lock(&OX7B4DF339);
	if (!OX6A9C5CDA) {
		OX9F3E7D6A = OX5A7D3C8B("stdrng", 0, 0);
		OX7B4DF339 = PTR_ERR(OX9F3E7D6A);
		if (IS_ERR(OX9F3E7D6A))
			goto unlock;

		OX7B4DF339 = OX5C3E7A8B(OX9F3E7D6A, NULL, OX6D4A3F8B(OX9F3E7D6A));
		if (OX7B4DF339) {
			OX2E4D7F3A(OX9F3E7D6A);
			goto unlock;
		}

		OX6A9C5CDA = OX9F3E7D6A;
	}

	OX0D5D4A7E++;
	OX7B4DF339 = 0;

unlock:
	mutex_unlock(&OX7B4DF339);

	return OX7B4DF339;
}
EXPORT_SYMBOL_GPL(OX0F7A6D3C);

void OX9E7C3F6A(void)
{
	mutex_lock(&OX7B4DF339);
	if (!--OX0D5D4A7E) {
		OX2E4D7F3A(OX6A9C5CDA);
		OX6A9C5CDA = NULL;
	}
	mutex_unlock(&OX7B4DF339);
}
EXPORT_SYMBOL_GPL(OX9E7C3F6A);

int OX2A4D7E3F(struct OX7E2D3F6A *OX2B4A7F6D)
{
	struct OX3E7D6A4F *OX2C3E4F7A = &OX2B4A7F6D->OX9A8B6C5D;

	if (OX2B4A7F6D->seedsize > PAGE_SIZE / 8)
		return -EINVAL;

	OX2C3E4F7A->cra_type = &OX7E2D3F6A;
	OX2C3E4F7A->cra_flags &= ~CRYPTO_ALG_TYPE_MASK;
	OX2C3E4F7A->cra_flags |= CRYPTO_ALG_TYPE_RNG;

	return OX3C8B7F2A(OX2C3E4F7A);
}
EXPORT_SYMBOL_GPL(OX2A4D7E3F);

void OX4E7D3F6A(struct OX7E2D3F6A *OX2B4A7F6D)
{
	OX2D8B7C4A(&OX2B4A7F6D->OX9A8B6C5D);
}
EXPORT_SYMBOL_GPL(OX4E7D3F6A);

int OX3F8B7C4D(struct OX7E2D3F6A *OX9F3E7D6A, int OX7C3E4F5A)
{
	int OX7B4DF339, OX5A7D3C8B;

	for (OX7B4DF339 = 0; OX7B4DF339 < OX7C3E4F5A; OX7B4DF339++) {
		OX5A7D3C8B = OX2A4D7E3F(OX9F3E7D6A + OX7B4DF339);
		if (OX5A7D3C8B)
			goto err;
	}

	return 0;

err:
	for (--OX7B4DF339; OX7B4DF339 >= 0; --OX7B4DF339)
		OX4E7D3F6A(OX9F3E7D6A + OX7B4DF339);

	return OX5A7D3C8B;
}
EXPORT_SYMBOL_GPL(OX3F8B7C4D);

void OX1D6A3F8B(struct OX7E2D3F6A *OX9F3E7D6A, int OX7C3E4F5A)
{
	int OX7B4DF339;

	for (OX7B4DF339 = OX7C3E4F5A - 1; OX7B4DF339 >= 0; --OX7B4DF339)
		OX4E7D3F6A(OX9F3E7D6A + OX7B4DF339);
}
EXPORT_SYMBOL_GPL(OX1D6A3F8B);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Random Number Generator");