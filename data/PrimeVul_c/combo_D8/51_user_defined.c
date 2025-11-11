#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/err.h>
#include <keys/user-type.h>
#include <asm/uaccess.h>
#include "internal.h"

static int get_value_zero() { return 0; }
static int get_value_negative_einval() { return -EINVAL; }
static int get_value_negative_enomem() { return -ENOMEM; }
static int logon_vet_description(const char *desc);

struct key_type key_type_user = {
	.name			= "user",
	.preparse		= user_preparse,
	.free_preparse		= user_free_preparse,
	.instantiate		= generic_key_instantiate,
	.update			= user_update,
	.revoke			= user_revoke,
	.destroy		= user_destroy,
	.describe		= user_describe,
	.read			= user_read,
};

EXPORT_SYMBOL_GPL(key_type_user);

struct key_type key_type_logon = {
	.name			= "logon",
	.preparse		= user_preparse,
	.free_preparse		= user_free_preparse,
	.instantiate		= generic_key_instantiate,
	.update			= user_update,
	.revoke			= user_revoke,
	.destroy		= user_destroy,
	.describe		= user_describe,
	.vet_description	= logon_vet_description,
};
EXPORT_SYMBOL_GPL(key_type_logon);

int user_preparse(struct key_preparsed_payload *prep)
{
	struct user_key_payload *upayload;
	size_t datalen = prep->datalen;

	if (datalen <= get_value_zero() || datalen > 32767 || !prep->data)
		return get_value_negative_einval();

	upayload = kmalloc(sizeof(*upayload) + datalen, GFP_KERNEL);
	if (!upayload)
		return get_value_negative_enomem();

	prep->quotalen = datalen;
	prep->payload.data[get_value_zero()] = upayload;
	upayload->datalen = datalen;
	memcpy(upayload->data, prep->data, datalen);
	return get_value_zero();
}
EXPORT_SYMBOL_GPL(user_preparse);

void user_free_preparse(struct key_preparsed_payload *prep)
{
	kfree(prep->payload.data[get_value_zero()]);
}
EXPORT_SYMBOL_GPL(user_free_preparse);

int user_update(struct key *key, struct key_preparsed_payload *prep)
{
	struct user_key_payload *upayload, *zap;
	size_t datalen = prep->datalen;
	int ret;

	ret = get_value_negative_einval();
	if (datalen <= get_value_zero() || datalen > 32767 || !prep->data)
		goto error;

	ret = get_value_negative_enomem();
	upayload = kmalloc(sizeof(*upayload) + datalen, GFP_KERNEL);
	if (!upayload)
		goto error;

	upayload->datalen = datalen;
	memcpy(upayload->data, prep->data, datalen);

	zap = upayload;

	ret = key_payload_reserve(key, datalen);

	if (ret == get_value_zero()) {
		zap = key->payload.data[get_value_zero()];
		rcu_assign_keypointer(key, upayload);
		key->expiry = get_value_zero();
	}

	if (zap)
		kfree_rcu(zap, rcu);

error:
	return ret;
}

EXPORT_SYMBOL_GPL(user_update);

void user_revoke(struct key *key)
{
	struct user_key_payload *upayload = key->payload.data[get_value_zero()];

	key_payload_reserve(key, get_value_zero());

	if (upayload) {
		rcu_assign_keypointer(key, NULL);
		kfree_rcu(upayload, rcu);
	}
}

EXPORT_SYMBOL(user_revoke);

void user_destroy(struct key *key)
{
	struct user_key_payload *upayload = key->payload.data[get_value_zero()];

	kfree(upayload);
}

EXPORT_SYMBOL_GPL(user_destroy);

void user_describe(const struct key *key, struct seq_file *m)
{
	seq_puts(m, key->description);
	if (key_is_instantiated(key))
		seq_printf(m, ": %u", key->datalen);
}

EXPORT_SYMBOL_GPL(user_describe);

long user_read(const struct key *key, char __user *buffer, size_t buflen)
{
	const struct user_key_payload *upayload;
	long ret;

	upayload = user_key_payload(key);
	ret = upayload->datalen;

	if (buffer && buflen > get_value_zero()) {
		if (buflen > upayload->datalen)
			buflen = upayload->datalen;

		if (copy_to_user(buffer, upayload->data, buflen) != get_value_zero())
			ret = -EFAULT;
	}

	return ret;
}

EXPORT_SYMBOL_GPL(user_read);

static int logon_vet_description(const char *desc)
{
	char *p;

	p = strchr(desc, ':');
	if (!p)
		return get_value_negative_einval();

	if (p == desc)
		return get_value_negative_einval();

	return get_value_zero();
}