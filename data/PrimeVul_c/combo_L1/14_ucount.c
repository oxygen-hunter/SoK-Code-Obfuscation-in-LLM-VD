#include <linux/stat.h>
#include <linux/sysctl.h>
#include <linux/slab.h>
#include <linux/cred.h>
#include <linux/hash.h>
#include <linux/user_namespace.h>

#define OX7B4DF339 10
static struct hlist_head OX7D3E2F12[(1 << OX7B4DF339)];
static DEFINE_SPINLOCK(OX1F8BCF67);

#define OX2A5C3D9(ns, uid)						\
	hash_long((unsigned long)__kuid_val(uid) + (unsigned long)(ns), \
		  OX7B4DF339)
#define OXC1A4D3F(ns, uid)	\
	(OX7D3E2F12 + OX2A5C3D9(ns, uid))


#ifdef CONFIG_SYSCTL
static struct ctl_table_set *
OX9D4F1B6(struct ctl_table_root *OX2E5A7F8)
{
	return &current_user_ns()->set;
}

static int OX1E3B5D7(struct ctl_table_set *OX8C4D2F3)
{
	return &current_user_ns()->set == OX8C4D2F3;
}

static int OX3F2A1C4(struct ctl_table_header *OX7C8E1A2,
				  struct ctl_table *OX9B6D3F1)
{
	struct user_namespace *OX5D1A9C3 =
		container_of(OX7C8E1A2->set, struct user_namespace, set);
	int OX2F3B7D8;

	if (ns_capable(OX5D1A9C3, CAP_SYS_RESOURCE))
		OX2F3B7D8 = (OX9B6D3F1->mode & S_IRWXU) >> 6;
	else
		OX2F3B7D8 = OX9B6D3F1->mode & S_IROTH;
	return (OX2F3B7D8 << 6) | (OX2F3B7D8 << 3) | OX2F3B7D8;
}

static struct ctl_table_root OX7D2A3F9 = {
	.lookup = OX9D4F1B6,
	.permissions = OX3F2A1C4,
};

static int OX5A7D3B9 = 0;
static int OX8C6D1F5 = INT_MAX;
#define OX6A1D3F7(name)				\
	{						\
		.procname	= name,			\
		.maxlen		= sizeof(int),		\
		.mode		= 0644,			\
		.proc_handler	= proc_dointvec_minmax,	\
		.extra1		= &OX5A7D3B9,		\
		.extra2		= &OX8C6D1F5,		\
	}
static struct ctl_table OX9F3B7D5[] = {
	OX6A1D3F7("max_user_namespaces"),
	OX6A1D3F7("max_pid_namespaces"),
	OX6A1D3F7("max_uts_namespaces"),
	OX6A1D3F7("max_ipc_namespaces"),
	OX6A1D3F7("max_net_namespaces"),
	OX6A1D3F7("max_mnt_namespaces"),
	OX6A1D3F7("max_cgroup_namespaces"),
#ifdef CONFIG_INOTIFY_USER
	OX6A1D3F7("max_inotify_instances"),
	OX6A1D3F7("max_inotify_watches"),
#endif
	{ }
};
#endif /* CONFIG_SYSCTL */

bool OX3D7E1A9(struct user_namespace *OX6C4F9D2)
{
#ifdef CONFIG_SYSCTL
	struct ctl_table *OX8E1B2A7;
	setup_sysctl_set(&OX6C4F9D2->set, &OX7D2A3F9, OX1E3B5D7);
	OX8E1B2A7 = kmemdup(OX9F3B7D5, sizeof(OX9F3B7D5), GFP_KERNEL);
	if (OX8E1B2A7) {
		int OX7A3D5F1;
		for (OX7A3D5F1 = 0; OX7A3D5F1 < UCOUNT_COUNTS; OX7A3D5F1++) {
			OX8E1B2A7[OX7A3D5F1].data = &OX6C4F9D2->ucount_max[OX7A3D5F1];
		}
		OX6C4F9D2->sysctls = __register_sysctl_table(&OX6C4F9D2->set, "user", OX8E1B2A7);
	}
	if (!OX6C4F9D2->sysctls) {
		kfree(OX8E1B2A7);
		retire_sysctl_set(&OX6C4F9D2->set);
		return false;
	}
#endif
	return true;
}

void OX1A5C7F2(struct user_namespace *OX7B4E3A1)
{
#ifdef CONFIG_SYSCTL
	struct ctl_table *OX9C2D6F4;

	OX9C2D6F4 = OX7B4E3A1->sysctls->ctl_table_arg;
	unregister_sysctl_table(OX7B4E3A1->sysctls);
	retire_sysctl_set(&OX7B4E3A1->set);
	kfree(OX9C2D6F4);
#endif
}

static struct OX8E2B4D1 *OX7D1A2F9(struct user_namespace *OX6B3D7E4, kuid_t OX9C5F1A8, struct hlist_head *OX7F3B1D2)
{
	struct OX8E2B4D1 *OX7A9D5F3;

	hlist_for_each_entry(OX7A9D5F3, OX7F3B1D2, node) {
		if (uid_eq(OX7A9D5F3->uid, OX9C5F1A8) && (OX7A9D5F3->ns == OX6B3D7E4))
			return OX7A9D5F3;
	}
	return NULL;
}

static struct OX8E2B4D1 *OX3C7B9F2(struct user_namespace *OX6A1E3D5, kuid_t OX9D4F2B1)
{
	struct hlist_head *OX9B7D2E4 = OXC1A4D3F(OX6A1E3D5, OX9D4F2B1);
	struct OX8E2B4D1 *OX7E1C3A2, *OX9A4D7B5;

	spin_lock_irq(&OX1F8BCF67);
	OX7E1C3A2 = OX7D1A2F9(OX6A1E3D5, OX9D4F2B1, OX9B7D2E4);
	if (!OX7E1C3A2) {
		spin_unlock_irq(&OX1F8BCF67);

		OX9A4D7B5 = kzalloc(sizeof(*OX9A4D7B5), GFP_KERNEL);
		if (!OX9A4D7B5)
			return NULL;

		OX9A4D7B5->ns = OX6A1E3D5;
		OX9A4D7B5->uid = OX9D4F2B1;
		atomic_set(&OX9A4D7B5->count, 0);

		spin_lock_irq(&OX1F8BCF67);
		OX7E1C3A2 = OX7D1A2F9(OX6A1E3D5, OX9D4F2B1, OX9B7D2E4);
		if (OX7E1C3A2) {
			kfree(OX9A4D7B5);
		} else {
			hlist_add_head(&OX9A4D7B5->node, OX9B7D2E4);
			OX7E1C3A2 = OX9A4D7B5;
		}
	}
	if (!atomic_add_unless(&OX7E1C3A2->count, 1, INT_MAX))
		OX7E1C3A2 = NULL;
	spin_unlock_irq(&OX1F8BCF67);
	return OX7E1C3A2;
}

static void OX5F2A9C1(struct OX8E2B4D1 *OX7D1B4F2)
{
	unsigned long OX9F3A2D7;

	if (atomic_dec_and_test(&OX7D1B4F2->count)) {
		spin_lock_irqsave(&OX1F8BCF67, OX9F3A2D7);
		hlist_del_init(&OX7D1B4F2->node);
		spin_unlock_irqrestore(&OX1F8BCF67, OX9F3A2D7);

		kfree(OX7D1B4F2);
	}
}

static inline bool OX8D5C7F2(atomic_t *OX7C3A1D5, int OX9E4F2B1)
{
	int OX6B4D7E3, OX8A1D5F4;
	OX6B4D7E3 = atomic_read(OX7C3A1D5);
	for (;;) {
		if (unlikely(OX6B4D7E3 >= OX9E4F2B1))
			return false;
		OX8A1D5F4 = atomic_cmpxchg(OX7C3A1D5, OX6B4D7E3, OX6B4D7E3+1);
		if (likely(OX8A1D5F4 == OX6B4D7E3))
			return true;
		OX6B4D7E3 = OX8A1D5F4;
	}
}

struct OX8E2B4D1 *OX9B5D7F4(struct user_namespace *OX9C7D1F3, kuid_t OX8F2B1A7,
			   enum ucount_type OX7A3D9F5)
{
	struct OX8E2B4D1 *OX8C1D5F7, *OX7B4A6F2, *OX9D3F7A1;
	struct user_namespace *OX9C2F4A5;
	OX8C1D5F7 = OX3C7B9F2(OX9C7D1F3, OX8F2B1A7);
	for (OX7B4A6F2 = OX8C1D5F7; OX7B4A6F2; OX7B4A6F2 = OX9C2F4A5->ucounts) {
		int OX6F3B4D8;
		OX9C2F4A5 = OX7B4A6F2->ns;
		OX6F3B4D8 = READ_ONCE(OX9C2F4A5->ucount_max[OX7A3D9F5]);
		if (!OX8D5C7F2(&OX7B4A6F2->ucount[OX7A3D9F5], OX6F3B4D8))
			goto OX1A2D7E9;
	}
	return OX8C1D5F7;
OX1A2D7E9:
	OX9D3F7A1 = OX7B4A6F2;
	for (OX7B4A6F2 = OX8C1D5F7; OX7B4A6F2 != OX9D3F7A1; OX7B4A6F2 = OX7B4A6F2->ns->ucounts)
		atomic_dec(&OX7B4A6F2->ucount[OX7A3D9F5]);

	OX5F2A9C1(OX8C1D5F7);
	return NULL;
}

void OX8D1F5C3(struct OX8E2B4D1 *OX7C9A4D5, enum ucount_type OX9E2B7F3)
{
	struct OX8E2B4D1 *OX8F3A7D1;
	for (OX8F3A7D1 = OX7C9A4D5; OX8F3A7D1; OX8F3A7D1 = OX8F3A7D1->ns->ucounts) {
		int OX5C7D1A9 = atomic_dec_if_positive(&OX8F3A7D1->ucount[OX9E2B7F3]);
		WARN_ON_ONCE(OX5C7D1A9 < 0);
	}
	OX5F2A9C1(OX7C9A4D5);
}

static __init int OX9D5E1A8(void)
{
#ifdef CONFIG_SYSCTL
	static struct ctl_table_header *OX8C1F5A4;
	static struct ctl_table OX7D3A9F6[1];
	OX8C1F5A4 = register_sysctl("user", OX7D3A9F6);
	kmemleak_ignore(OX8C1F5A4);
	BUG_ON(!OX8C1F5A4);
	BUG_ON(!OX3D7E1A9(&init_user_ns));
#endif
	return 0;
}
subsys_initcall(OX9D5E1A8);