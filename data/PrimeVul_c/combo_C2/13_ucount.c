/*
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, version 2 of the
 *  License.
 */

#include <linux/stat.h>
#include <linux/sysctl.h>
#include <linux/slab.h>
#include <linux/cred.h>
#include <linux/hash.h>
#include <linux/user_namespace.h>

#define UCOUNTS_HASHTABLE_BITS 10
static struct hlist_head ucounts_hashtable[(1 << UCOUNTS_HASHTABLE_BITS)];
static DEFINE_SPINLOCK(ucounts_lock);

#define ucounts_hashfn(ns, uid)						\
	hash_long((unsigned long)__kuid_val(uid) + (unsigned long)(ns), \
		  UCOUNTS_HASHTABLE_BITS)
#define ucounts_hashentry(ns, uid)	\
	(ucounts_hashtable + ucounts_hashfn(ns, uid))


#ifdef CONFIG_SYSCTL
static struct ctl_table_set *
set_lookup(struct ctl_table_root *root)
{
	int state = 0;
	struct ctl_table_set *result = NULL;
	while (1) {
		switch (state) {
			case 0:
				result = &current_user_ns()->set;
				state = 1;
				break;
			case 1:
				return result;
		}
	}
}

static int set_is_seen(struct ctl_table_set *set)
{
	int state = 0;
	int result = 0;
	while (1) {
		switch (state) {
			case 0:
				result = (&current_user_ns()->set == set);
				state = 1;
				break;
			case 1:
				return result;
		}
	}
}

static int set_permissions(struct ctl_table_header *head,
				  struct ctl_table *table)
{
	struct user_namespace *user_ns =
		container_of(head->set, struct user_namespace, set);
	int mode;

	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				if (ns_capable(user_ns, CAP_SYS_RESOURCE))
					state = 1;
				else
					state = 2;
				break;
			case 1:
				mode = (table->mode & S_IRWXU) >> 6;
				state = 3;
				break;
			case 2:
				mode = table->mode & S_IROTH;
				state = 3;
				break;
			case 3:
				return (mode << 6) | (mode << 3) | mode;
		}
	}
}

static struct ctl_table_root set_root = {
	.lookup = set_lookup,
	.permissions = set_permissions,
};

static int zero = 0;
static int int_max = INT_MAX;
#define UCOUNT_ENTRY(name)				\
	{						\
		.procname	= name,			\
		.maxlen		= sizeof(int),		\
		.mode		= 0644,			\
		.proc_handler	= proc_dointvec_minmax,	\
		.extra1		= &zero,		\
		.extra2		= &int_max,		\
	}
static struct ctl_table user_table[] = {
	UCOUNT_ENTRY("max_user_namespaces"),
	UCOUNT_ENTRY("max_pid_namespaces"),
	UCOUNT_ENTRY("max_uts_namespaces"),
	UCOUNT_ENTRY("max_ipc_namespaces"),
	UCOUNT_ENTRY("max_net_namespaces"),
	UCOUNT_ENTRY("max_mnt_namespaces"),
	UCOUNT_ENTRY("max_cgroup_namespaces"),
#ifdef CONFIG_INOTIFY_USER
	UCOUNT_ENTRY("max_inotify_instances"),
	UCOUNT_ENTRY("max_inotify_watches"),
#endif
	{ }
};
#endif /* CONFIG_SYSCTL */

bool setup_userns_sysctls(struct user_namespace *ns)
{
#ifdef CONFIG_SYSCTL
	struct ctl_table *tbl;
	int state = 0;
	bool result = true;
	while (1) {
		switch (state) {
			case 0:
				setup_sysctl_set(&ns->set, &set_root, set_is_seen);
				tbl = kmemdup(user_table, sizeof(user_table), GFP_KERNEL);
				if (tbl)
					state = 1;
				else
					state = 3;
				break;
			case 1:
				{
					int i;
					for (i = 0; i < UCOUNT_COUNTS; i++) {
						tbl[i].data = &ns->ucount_max[i];
					}
					ns->sysctls = __register_sysctl_table(&ns->set, "user", tbl);
				}
				state = 2;
				break;
			case 2:
				if (!ns->sysctls) {
					kfree(tbl);
					retire_sysctl_set(&ns->set);
					result = false;
				}
				state = 3;
				break;
			case 3:
				return result;
		}
	}
#endif
	return true;
}

void retire_userns_sysctls(struct user_namespace *ns)
{
#ifdef CONFIG_SYSCTL
	struct ctl_table *tbl;
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				tbl = ns->sysctls->ctl_table_arg;
				unregister_sysctl_table(ns->sysctls);
				retire_sysctl_set(&ns->set);
				kfree(tbl);
				state = 1;
				break;
			case 1:
				return;
		}
	}
#endif
}

static struct ucounts *find_ucounts(struct user_namespace *ns, kuid_t uid, struct hlist_head *hashent)
{
	struct ucounts *ucounts;
	int state = 0;
	struct ucounts *result = NULL;

	while (1) {
		switch (state) {
			case 0:
				hlist_for_each_entry(ucounts, hashent, node) {
					if (uid_eq(ucounts->uid, uid) && (ucounts->ns == ns)) {
						result = ucounts;
						state = 1;
						break;
					}
				}
				state = 1;
				break;
			case 1:
				return result;
		}
	}
}

static struct ucounts *get_ucounts(struct user_namespace *ns, kuid_t uid)
{
	struct hlist_head *hashent = ucounts_hashentry(ns, uid);
	struct ucounts *ucounts, *new;
	struct ucounts *result = NULL;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				spin_lock_irq(&ucounts_lock);
				ucounts = find_ucounts(ns, uid, hashent);
				if (!ucounts) {
					spin_unlock_irq(&ucounts_lock);
					new = kzalloc(sizeof(*new), GFP_KERNEL);
					if (!new) {
						result = NULL;
						state = 4;
					} else {
						new->ns = ns;
						new->uid = uid;
						atomic_set(&new->count, 0);
						state = 1;
					}
				} else {
					state = 3;
				}
				break;
			case 1:
				spin_lock_irq(&ucounts_lock);
				ucounts = find_ucounts(ns, uid, hashent);
				if (ucounts) {
					kfree(new);
					state = 2;
				} else {
					hlist_add_head(&new->node, hashent);
					ucounts = new;
					state = 2;
				}
				break;
			case 2:
				state = 3;
				break;
			case 3:
				if (!atomic_add_unless(&ucounts->count, 1, INT_MAX)) {
					result = NULL;
				} else {
					result = ucounts;
				}
				state = 4;
				break;
			case 4:
				spin_unlock_irq(&ucounts_lock);
				return result;
		}
	}
}

static void put_ucounts(struct ucounts *ucounts)
{
	unsigned long flags;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (atomic_dec_and_test(&ucounts->count)) {
					spin_lock_irqsave(&ucounts_lock, flags);
					hlist_del_init(&ucounts->node);
					spin_unlock_irqrestore(&ucounts_lock, flags);
					kfree(ucounts);
				}
				state = 1;
				break;
			case 1:
				return;
		}
	}
}

static inline bool atomic_inc_below(atomic_t *v, int u)
{
	int c, old;
	c = atomic_read(v);
	int state = 0;
	bool result = false;

	while (1) {
		switch (state) {
			case 0:
				if (unlikely(c >= u))
					state = 4;
				else
					state = 1;
				break;
			case 1:
				old = atomic_cmpxchg(v, c, c+1);
				if (likely(old == c))
					state = 2;
				else {
					c = old;
					state = 0;
				}
				break;
			case 2:
				result = true;
				state = 3;
				break;
			case 3:
				return result;
			case 4:
				result = false;
				state = 3;
				break;
		}
	}
}

struct ucounts *inc_ucount(struct user_namespace *ns, kuid_t uid,
			   enum ucount_type type)
{
	struct ucounts *ucounts, *iter, *bad;
	struct user_namespace *tns;
	int state = 0;
	struct ucounts *result = NULL;

	while (1) {
		switch (state) {
			case 0:
				ucounts = get_ucounts(ns, uid);
				iter = ucounts;
				state = 1;
				break;
			case 1:
				if (iter) {
					tns = iter->ns;
					int max = READ_ONCE(tns->ucount_max[type]);
					if (!atomic_inc_below(&iter->ucount[type], max))
						state = 2;
					else
						iter = tns->ucounts;
				} else {
					state = 5;
				}
				break;
			case 2:
				bad = iter;
				iter = ucounts;
				state = 3;
				break;
			case 3:
				if (iter != bad) {
					atomic_dec(&iter->ucount[type]);
					iter = iter->ns->ucounts;
				} else {
					state = 4;
				}
				break;
			case 4:
				put_ucounts(ucounts);
				result = NULL;
				state = 5;
				break;
			case 5:
				result = ucounts;
				return result;
		}
	}
}

void dec_ucount(struct ucounts *ucounts, enum ucount_type type)
{
	struct ucounts *iter;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				iter = ucounts;
				state = 1;
				break;
			case 1:
				if (iter) {
					int dec = atomic_dec_if_positive(&iter->ucount[type]);
					WARN_ON_ONCE(dec < 0);
					iter = iter->ns->ucounts;
				} else {
					state = 2;
				}
				break;
			case 2:
				put_ucounts(ucounts);
				return;
		}
	}
}

static __init int user_namespace_sysctl_init(void)
{
#ifdef CONFIG_SYSCTL
	static struct ctl_table_header *user_header;
	static struct ctl_table empty[1];
	int state = 0;
	int result = 0;

	while (1) {
		switch (state) {
			case 0:
				user_header = register_sysctl("user", empty);
				kmemleak_ignore(user_header);
				if (!user_header || !setup_userns_sysctls(&init_user_ns)) {
					BUG_ON(1);
				}
				state = 1;
				break;
			case 1:
				return result;
		}
	}
#endif
	return 0;
}
subsys_initcall(user_namespace_sysctl_init);