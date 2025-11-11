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

int user_preparse(struct key_preparsed_payload *prep) {
	int state = 0;
	struct user_key_payload *upayload;
	size_t datalen = prep->datalen;
	
	while (1) {
		switch (state) {
			case 0:
				if (datalen <= 0 || datalen > 32767 || !prep->data) {
					state = 1;
					break;
				}
				upayload = kmalloc(sizeof(*upayload) + datalen, GFP_KERNEL);
				if (!upayload) {
					state = 2;
					break;
				}
				prep->quotalen = datalen;
				prep->payload.data[0] = upayload;
				upayload->datalen = datalen;
				memcpy(upayload->data, prep->data, datalen);
				return 0;
			case 1:
				return -EINVAL;
			case 2:
				return -ENOMEM;
		}
	}
}

EXPORT_SYMBOL_GPL(user_preparse);

void user_free_preparse(struct key_preparsed_payload *prep) {
	kfree(prep->payload.data[0]);
}

EXPORT_SYMBOL_GPL(user_free_preparse);

int user_update(struct key *key, struct key_preparsed_payload *prep) {
	int state = 0;
	int ret;
	struct user_key_payload *upayload, *zap;
	size_t datalen = prep->datalen;

	while (1) {
		switch (state) {
			case 0:
				ret = -EINVAL;
				if (datalen <= 0 || datalen > 32767 || !prep->data) {
					state = 4;
					break;
				}
				ret = -ENOMEM;
				upayload = kmalloc(sizeof(*upayload) + datalen, GFP_KERNEL);
				if (!upayload) {
					state = 4;
					break;
				}
				upayload->datalen = datalen;
				memcpy(upayload->data, prep->data, datalen);
				zap = upayload;
				ret = key_payload_reserve(key, datalen);
				if (ret == 0) {
					zap = key->payload.data[0];
					rcu_assign_keypointer(key, upayload);
					key->expiry = 0;
				}
				if (zap) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				kfree_rcu(zap, rcu);
			case 2:
				state = 3;
				break;
			case 3:
				return ret;
			case 4:
				goto error;
		}
	}
error:
	return ret;
}

EXPORT_SYMBOL_GPL(user_update);

void user_revoke(struct key *key) {
	int state = 0;
	struct user_key_payload *upayload = key->payload.data[0];
	
	while (1) {
		switch (state) {
			case 0:
				key_payload_reserve(key, 0);
				if (upayload) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				rcu_assign_keypointer(key, NULL);
				kfree_rcu(upayload, rcu);
			case 2:
				return;
		}
	}
}

EXPORT_SYMBOL(user_revoke);

void user_destroy(struct key *key) {
	kfree(key->payload.data[0]);
}

EXPORT_SYMBOL_GPL(user_destroy);

void user_describe(const struct key *key, struct seq_file *m) {
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				seq_puts(m, key->description);
				if (key_is_instantiated(key)) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				seq_printf(m, ": %u", key->datalen);
			case 2:
				return;
		}
	}
}

EXPORT_SYMBOL_GPL(user_describe);

long user_read(const struct key *key, char __user *buffer, size_t buflen) {
	int state = 0;
	const struct user_key_payload *upayload;
	long ret;

	while (1) {
		switch (state) {
			case 0:
				upayload = user_key_payload(key);
				ret = upayload->datalen;
				if (buffer && buflen > 0) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				if (buflen > upayload->datalen) {
					buflen = upayload->datalen;
				}
				if (copy_to_user(buffer, upayload->data, buflen) != 0) {
					state = 3;
					break;
				}
				state = 2;
				break;
			case 2:
				return ret;
			case 3:
				ret = -EFAULT;
				state = 2;
				break;
		}
	}
}

EXPORT_SYMBOL_GPL(user_read);

static int logon_vet_description(const char *desc) {
	int state = 0;
	char *p;

	while (1) {
		switch (state) {
			case 0:
				p = strchr(desc, ':');
				if (!p) {
					state = 1;
					break;
				}
				if (p == desc) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				return -EINVAL;
			case 2:
				return 0;
		}
	}
}