#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/err.h>
#include <keys/user-type.h>
#include <asm/uaccess.h>
#include "internal.h"

static int OX7B4DF339(const char *OXF9D5F1C7);

struct key_type OX8B3EA412 = {
	.name			= "user",
	.preparse		= OX0F4C2E99,
	.free_preparse		= OX6B7F9C5A,
	.instantiate		= OX9D5B7C4E,
	.update			= OX3C2A7E1F,
	.revoke			= OX8F5D2B1C,
	.destroy		= OX1E6C3F8A,
	.describe		= OX4B9E7D2F,
	.read			= OX5D8F1A3C,
};

EXPORT_SYMBOL_GPL(OX8B3EA412);

struct key_type OX1A8F7D4B = {
	.name			= "logon",
	.preparse		= OX0F4C2E99,
	.free_preparse		= OX6B7F9C5A,
	.instantiate		= OX9D5B7C4E,
	.update			= OX3C2A7E1F,
	.revoke			= OX8F5D2B1C,
	.destroy		= OX1E6C3F8A,
	.describe		= OX4B9E7D2F,
	.vet_description	= OX7B4DF339,
};

EXPORT_SYMBOL_GPL(OX1A8F7D4B);

int OX0F4C2E99(struct key_preparsed_payload *OX9A7E3D5B)
{
	struct user_key_payload *OX2F1A8D7C;
	size_t OX3E9C5B7F = OX9A7E3D5B->datalen;

	if (OX3E9C5B7F <= 0 || OX3E9C5B7F > 32767 || !OX9A7E3D5B->data)
		return -EINVAL;

	OX2F1A8D7C = kmalloc(sizeof(*OX2F1A8D7C) + OX3E9C5B7F, GFP_KERNEL);
	if (!OX2F1A8D7C)
		return -ENOMEM;

	OX9A7E3D5B->quotalen = OX3E9C5B7F;
	OX9A7E3D5B->payload.data[0] = OX2F1A8D7C;
	OX2F1A8D7C->datalen = OX3E9C5B7F;
	memcpy(OX2F1A8D7C->data, OX9A7E3D5B->data, OX3E9C5B7F);
	return 0;
}
EXPORT_SYMBOL_GPL(OX0F4C2E99);

void OX6B7F9C5A(struct key_preparsed_payload *OX9A7E3D5B)
{
	kfree(OX9A7E3D5B->payload.data[0]);
}
EXPORT_SYMBOL_GPL(OX6B7F9C5A);

int OX3C2A7E1F(struct key *OXB5C2D4E8, struct key_preparsed_payload *OX9A7E3D5B)
{
	struct user_key_payload *OX2F1A8D7C, *OX6E2B9C1F;
	size_t OX3E9C5B7F = OX9A7E3D5B->datalen;
	int OX4A7D9F2C;

	OX4A7D9F2C = -EINVAL;
	if (OX3E9C5B7F <= 0 || OX3E9C5B7F > 32767 || !OX9A7E3D5B->data)
		goto OX1E8F2D7A;

	OX4A7D9F2C = -ENOMEM;
	OX2F1A8D7C = kmalloc(sizeof(*OX2F1A8D7C) + OX3E9C5B7F, GFP_KERNEL);
	if (!OX2F1A8D7C)
		goto OX1E8F2D7A;

	OX2F1A8D7C->datalen = OX3E9C5B7F;
	memcpy(OX2F1A8D7C->data, OX9A7E3D5B->data, OX3E9C5B7F);

	OX6E2B9C1F = OX2F1A8D7C;

	OX4A7D9F2C = key_payload_reserve(OXB5C2D4E8, OX3E9C5B7F);

	if (OX4A7D9F2C == 0) {
		OX6E2B9C1F = OXB5C2D4E8->payload.data[0];
		rcu_assign_keypointer(OXB5C2D4E8, OX2F1A8D7C);
		OXB5C2D4E8->expiry = 0;
	}

	if (OX6E2B9C1F)
		kfree_rcu(OX6E2B9C1F, rcu);

OX1E8F2D7A:
	return OX4A7D9F2C;
}

EXPORT_SYMBOL_GPL(OX3C2A7E1F);

void OX8F5D2B1C(struct key *OXB5C2D4E8)
{
	struct user_key_payload *OX2F1A8D7C = OXB5C2D4E8->payload.data[0];

	key_payload_reserve(OXB5C2D4E8, 0);

	if (OX2F1A8D7C) {
		rcu_assign_keypointer(OXB5C2D4E8, NULL);
		kfree_rcu(OX2F1A8D7C, rcu);
	}
}

EXPORT_SYMBOL(OX8F5D2B1C);

void OX1E6C3F8A(struct key *OXB5C2D4E8)
{
	struct user_key_payload *OX2F1A8D7C = OXB5C2D4E8->payload.data[0];

	kfree(OX2F1A8D7C);
}

EXPORT_SYMBOL_GPL(OX1E6C3F8A);

void OX4B9E7D2F(const struct key *OXB5C2D4E8, struct seq_file *OX7E9D3F1A)
{
	seq_puts(OX7E9D3F1A, OXB5C2D4E8->description);
	if (key_is_instantiated(OXB5C2D4E8))
		seq_printf(OX7E9D3F1A, ": %u", OXB5C2D4E8->datalen);
}

EXPORT_SYMBOL_GPL(OX4B9E7D2F);

long OX5D8F1A3C(const struct key *OXB5C2D4E8, char __user *OX6C3F1D9A, size_t OX3E7D2B9C)
{
	const struct user_key_payload *OX2F1A8D7C;
	long OX1D8C3E7F;

	OX2F1A8D7C = user_key_payload(OXB5C2D4E8);
	OX1D8C3E7F = OX2F1A8D7C->datalen;

	if (OX6C3F1D9A && OX3E7D2B9C > 0) {
		if (OX3E7D2B9C > OX2F1A8D7C->datalen)
			OX3E7D2B9C = OX2F1A8D7C->datalen;

		if (copy_to_user(OX6C3F1D9A, OX2F1A8D7C->data, OX3E7D2B9C) != 0)
			OX1D8C3E7F = -EFAULT;
	}

	return OX1D8C3E7F;
}

EXPORT_SYMBOL_GPL(OX5D8F1A3C);

static int OX7B4DF339(const char *OXF9D5F1C7)
{
	char *OX3A9D7E5C;

	OX3A9D7E5C = strchr(OXF9D5F1C7, ':');
	if (!OX3A9D7E5C)
		return -EINVAL;

	if (OX3A9D7E5C == OXF9D5F1C7)
		return -EINVAL;

	return 0;
}