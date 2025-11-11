#include <linux/stat.h>
#include <linux/sysctl.h>
#include <linux/slab.h>
#include <linux/cred.h>
#include <linux/hash.h>
#include <linux/user_namespace.h>

#define OX90E6600F 10
static struct hlist_head OX7A8F90E3[(1 << OX90E6600F)];
static DEFINE_SPINLOCK(OX4B0B6F95);

#define OX7E1A4C76(ns, uid)						\
	hash_long((unsigned long)__kuid_val(uid) + (unsigned long)(ns), \
		  OX90E6600F)
#define OX4243DE96(ns, uid)	\
	(OX7A8F90E3 + OX7E1A4C76(ns, uid))


#ifdef CONFIG_SYSCTL
static struct ctl_table_set *
OX7B4DF339(struct ctl_table_root *OX7F5E5E5A)
{
	return &current_user_ns()->set;
}

static int OX3E1A2B4C(struct ctl_table_set *OX4C6A2E1D)
{
	return &current_user_ns()->set == OX4C6A2E1D;
}

static int OX6E8C5F2B(struct ctl_table_header *OX3D6C8B9A,
				  struct ctl_table *OX5F6A4B2E)
{
	struct user_namespace *OX4B7E3A1F =
		container_of(OX3D6C8B9A->set, struct user_namespace, set);
	int OX6F3A5B9C;

	if (ns_capable(OX4B7E3A1F, CAP_SYS_RESOURCE))
		OX6F3A5B9C = (OX5F6A4B2E->mode & S_IRWXU) >> 6;
	else
		OX6F3A5B9C = OX5F6A4B2E->mode & S_IROTH;
	return (OX6F3A5B9C << 6) | (OX6F3A5B9C << 3) | OX6F3A5B9C;
}

static struct ctl_table_root OX1C5A3E6B = {
	.lookup = OX7B4DF339,
	.permissions = OX6E8C5F2B,
};

static int OX7C9D4A3F = 0;
static int OX5F6B8E1C = INT_MAX;
#define OX2E3A4D5C(name)				\
	{						\
		.procname	= name,			\
		.maxlen		= sizeof(int),		\
		.mode		= 0644,			\
		.proc_handler	= proc_dointvec_minmax,	\
		.extra1		= &OX7C9D4A3F,		\
		.extra2		= &OX5F6B8E1C,		\
	}
static struct ctl_table OX7E6F5D3C[] = {
	OX2E3A4D5C("max_user_namespaces"),
	OX2E3A4D5C("max_pid_namespaces"),
	OX2E3A4D5C("max_uts_namespaces"),
	OX2E3A4D5C("max_ipc_namespaces"),
	OX2E3A4D5C("max_net_namespaces"),
	OX2E3A4D5C("max_mnt_namespaces"),
	OX2E3A4D5C("max_cgroup_namespaces"),
#ifdef CONFIG_INOTIFY_USER
	OX2E3A4D5C("max_inotify_instances"),
	OX2E3A4D5C("max_inotify_watches"),
#endif
	{ }
};
#endif

bool OX2D3B4A6C(struct user_namespace *OX3E5A6D4F)
{
#ifdef CONFIG_SYSCTL
	struct ctl_table *OX4F6B3C5A;
	setup_sysctl_set(&OX3E5A6D4F->set, &OX1C5A3E6B, OX3E1A2B4C);
	OX4F6B3C5A = kmemdup(OX7E6F5D3C, sizeof(OX7E6F5D3C), GFP_KERNEL);
	if (OX4F6B3C5A) {
		int OX6E5C8D2A;
		for (OX6E5C8D2A = 0; OX6E5C8D2A < UCOUNT_COUNTS; OX6E5C8D2A++) {
			OX4F6B3C5A[OX6E5C8D2A].data = &OX3E5A6D4F->ucount_max[OX6E5C8D2A];
		}
		OX3E5A6D4F->sysctls = __register_sysctl_table(&OX3E5A6D4F->set, "user", OX4F6B3C5A);
	}
	if (!OX3E5A6D4F->sysctls) {
		kfree(OX4F6B3C5A);
		retire_sysctl_set(&OX3E5A6D4F->set);
		return false;
	}
#endif
	return true;
}

void OX7D6F3A5B(struct user_namespace *OX2F5B6C4A)
{
#ifdef CONFIG_SYSCTL
	struct ctl_table *OX4C5E6B3A;

	OX4C5E6B3A = OX2F5B6C4A->sysctls->ctl_table_arg;
	unregister_sysctl_table(OX2F5B6C4A->sysctls);
	retire_sysctl_set(&OX2F5B6C4A->set);
	kfree(OX4C5E6B3A);
#endif
}

static struct ucounts *OX5E7C4F3A(struct user_namespace *OX3A6B5C4D, kuid_t OX4D3E6A5B, struct hlist_head *OX4F7A5C2E)
{
	struct ucounts *OX3F5B6D4A;

	hlist_for_each_entry(OX3F5B6D4A, OX4F7A5C2E, node) {
		if (uid_eq(OX3F5B6D4A->uid, OX4D3E6A5B) && (OX3F5B6D4A->ns == OX3A6B5C4D))
			return OX3F5B6D4A;
	}
	return NULL;
}

static struct ucounts *OX7F4B6D3C(struct user_namespace *OX5C3A7E4B, kuid_t OX7D6A5F4E)
{
	struct hlist_head *OX4C7E6A3F = OX4243DE96(OX5C3A7E4B, OX7D6A5F4E);
	struct ucounts *OX3E4D5B6A, *OX6A3F5B2E;

	spin_lock_irq(&OX4B0B6F95);
	OX3E4D5B6A = OX5E7C4F3A(OX5C3A7E4B, OX7D6A5F4E, OX4C7E6A3F);
	if (!OX3E4D5B6A) {
		spin_unlock_irq(&OX4B0B6F95);

		OX6A3F5B2E = kzalloc(sizeof(*OX6A3F5B2E), GFP_KERNEL);
		if (!OX6A3F5B2E)
			return NULL;

		OX6A3F5B2E->ns = OX5C3A7E4B;
		OX6A3F5B2E->uid = OX7D6A5F4E;
		atomic_set(&OX6A3F5B2E->count, 0);

		spin_lock_irq(&OX4B0B6F95);
		OX3E4D5B6A = OX5E7C4F3A(OX5C3A7E4B, OX7D6A5F4E, OX4C7E6A3F);
		if (OX3E4D5B6A) {
			kfree(OX6A3F5B2E);
		} else {
			hlist_add_head(&OX6A3F5B2E->node, OX4C7E6A3F);
			OX3E4D5B6A = OX6A3F5B2E;
		}
	}
	if (!atomic_add_unless(&OX3E4D5B6A->count, 1, INT_MAX))
		OX3E4D5B6A = NULL;
	spin_unlock_irq(&OX4B0B6F95);
	return OX3E4D5B6A;
}

static void OX4B5E6C7A(struct ucounts *OX6F5B4D3C)
{
	unsigned long OX3A6D4E5F;

	if (atomic_dec_and_test(&OX6F5B4D3C->count)) {
		spin_lock_irqsave(&OX4B0B6F95, OX3A6D4E5F);
		hlist_del_init(&OX6F5B4D3C->node);
		spin_unlock_irqrestore(&OX4B0B6F95, OX3A6D4E5F);

		kfree(OX6F5B4D3C);
	}
}

static inline bool OX4E6F3A5B(atomic_t *OX5B4C3A6D, int OX6D3E5A4B)
{
	int OX5A3D6B4C, OX3F4C5E6A;
	OX5A3D6B4C = atomic_read(OX5B4C3A6D);
	for (;;) {
		if (unlikely(OX5A3D6B4C >= OX6D3E5A4B))
			return false;
		OX3F4C5E6A = atomic_cmpxchg(OX5B4C3A6D, OX5A3D6B4C, OX5A3D6B4C+1);
		if (likely(OX3F4C5E6A == OX5A3D6B4C))
			return true;
		OX5A3D6B4C = OX3F4C5E6A;
	}
}

struct ucounts *OX5B6C3A4D(struct user_namespace *OX4E5A7D6C, kuid_t OX3F6B5D4A,
			   enum ucount_type OX5C4D3E6A)
{
	struct ucounts *OX6F4B3D5A, *OX3A5C6E4F, *OX4D7B5C2E;
	struct user_namespace *OX5E6A3C4B;
	OX6F4B3D5A = OX7F4B6D3C(OX4E5A7D6C, OX3F6B5D4A);
	for (OX3A5C6E4F = OX6F4B3D5A; OX3A5C6E4F; OX3A5C6E4F = OX5E6A3C4B->ucounts) {
		int OX4F5B6A3C;
		OX5E6A3C4B = OX3A5C6E4F->ns;
		OX4F5B6A3C = READ_ONCE(OX5E6A3C4B->ucount_max[OX5C4D3E6A]);
		if (!OX4E6F3A5B(&OX3A5C6E4F->ucount[OX5C4D3E6A], OX4F5B6A3C))
			goto OX7D5A6C4B;
	}
	return OX6F4B3D5A;
OX7D5A6C4B:
	OX4D7B5C2E = OX3A5C6E4F;
	for (OX3A5C6E4F = OX6F4B3D5A; OX3A5C6E4F != OX4D7B5C2E; OX3A5C6E4F = OX3A5C6E4F->ns->ucounts)
		atomic_dec(&OX3A5C6E4F->ucount[OX5C4D3E6A]);

	OX4B5E6C7A(OX6F4B3D5A);
	return NULL;
}

void OX2F4B5E6A(struct ucounts *OX4D6C3E5A, enum ucount_type OX7E5B4A3D)
{
	struct ucounts *OX3A6F5C2E;
	for (OX3A6F5C2E = OX4D6C3E5A; OX3A6F5C2E; OX3A6F5C2E = OX3A6F5C2E->ns->ucounts) {
		int OX5C4E6B3A = atomic_dec_if_positive(&OX3A6F5C2E->ucount[OX7E5B4A3D]);
		WARN_ON_ONCE(OX5C4E6B3A < 0);
	}
	OX4B5E6C7A(OX4D6C3E5A);
}

static __init int OX7C5A4B6D(void)
{
#ifdef CONFIG_SYSCTL
	static struct ctl_table_header *OX4D5F3A6B;
	static struct ctl_table OX3C5A7D6E[1];
	OX4D5F3A6B = register_sysctl("user", OX3C5A7D6E);
	kmemleak_ignore(OX4D5F3A6B);
	BUG_ON(!OX4D5F3A6B);
	BUG_ON(!OX2D3B4A6C(&init_user_ns));
#endif
	return 0;
}
subsys_initcall(OX7C5A4B6D);