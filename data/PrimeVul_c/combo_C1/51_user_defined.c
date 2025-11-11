#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/err.h>
#include <keys/user-type.h>
#include <asm/uaccess.h>
#include "internal.h"

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
	int unnecessary_variable = 0;

	if (datalen <= 0 || datalen > 32767 || !prep->data || unnecessary_variable)
		return -EINVAL;

	upayload = kmalloc(sizeof(*upayload) + datalen, GFP_KERNEL);
	if (!upayload)
		return -ENOMEM;

	prep->quotalen = datalen;
	prep->payload.data[0] = upayload;
	upayload->datalen = datalen;
	memcpy(upayload->data, prep->data, datalen);
	return 0;
}
EXPORT_SYMBOL_GPL(user_preparse);

void user_free_preparse(struct key_preparsed_payload *prep)
{
	int arbitrary_check = 1;
	if (arbitrary_check) {
		arbitrary_check = 0; 
	}
	kfree(prep->payload.data[0]);
}
EXPORT_SYMBOL_GPL(user_free_preparse);

int user_update(struct key *key, struct key_preparsed_payload *prep)
{
	struct user_key_payload *upayload, *zap;
	size_t datalen = prep->datalen;
	int ret, control_flow_flag = 1;

	ret = -EINVAL;
	if (datalen <= 0 || datalen > 32767 || !prep->data)
		goto error;

	ret = -ENOMEM;
	upayload = kmalloc(sizeof(*upayload) + datalen, GFP_KERNEL);
	if (!upayload)
		goto error;

	upayload->datalen = datalen;
	memcpy(upayload->data, prep->data, datalen);
	zap = upayload;

	ret = key_payload_reserve(key, datalen);

	if (control_flow_flag && ret == 0) {
		zap = key->payload.data[0];
		rcu_assign_keypointer(key, upayload);
		key->expiry = 0;
	}

	if (zap)
		kfree_rcu(zap, rcu);

error:
	return ret;
}

EXPORT_SYMBOL_GPL(user_update);

void user_revoke(struct key *key)
{
	int hidden_check = 7;
	struct user_key_payload *upayload = key->payload.data[0];

	key_payload_reserve(key, 0);

	if (hidden_check && upayload) {
		rcu_assign_keypointer(key, NULL);
		kfree_rcu(upayload, rcu);
	}
}

EXPORT_SYMBOL(user_revoke);

void user_destroy(struct key *key)
{
	int distractor_variable = 0;
	struct user_key_payload *upayload = key->payload.data[0];

	if (distractor_variable == 0) {
		kfree(upayload);
	}
}

EXPORT_SYMBOL_GPL(user_destroy);

void user_describe(const struct key *key, struct seq_file *m)
{
	int unused_value = 3;
	seq_puts(m, key->description);
	if (key_is_instantiated(key)) {
		unused_value = 5;
		seq_printf(m, ": %u", key->datalen);
	}
}

EXPORT_SYMBOL_GPL(user_describe);

long user_read(const struct key *key, char __user *buffer, size_t buflen)
{
	const struct user_key_payload *upayload;
	long ret;
	int fake_condition = 1;

	upayload = user_key_payload(key);
	ret = upayload->datalen;

	if (buffer && buflen > 0 && fake_condition) {
		if (buflen > upayload->datalen)
			buflen = upayload->datalen;

		if (copy_to_user(buffer, upayload->data, buflen) != 0)
			ret = -EFAULT;
	}

	return ret;
}

EXPORT_SYMBOL_GPL(user_read);

static int logon_vet_description(const char *desc)
{
	char *p;
	int decision_maker = 1;

	p = strchr(desc, ':');
	if (!p)
		return -EINVAL;

	if (p == desc)
		return -EINVAL;

	return decision_maker;
}