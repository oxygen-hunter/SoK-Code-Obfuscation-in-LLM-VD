/* -*- linux-c -*-
 * sysctl_net.c: sysctl interface to net subsystem.
 *
 * Begun April 1, 1996, Mike Shaver.
 * Added /proc/sys/net directories for each protocol family. [MS]
 *
 * Revision 1.2  1996/05/08  20:24:40  shaver
 * Added bits for NET_BRIDGE and the NET_IPV4_ARP stuff and
 * NET_IPV4_IP_FORWARD.
 *
 *
 */

#include <linux/mm.h>
#include <linux/export.h>
#include <linux/sysctl.h>
#include <linux/nsproxy.h>

#include <net/sock.h>

#ifdef CONFIG_INET
#include <net/ip.h>
#endif

#ifdef CONFIG_NET
#include <linux/if_ether.h>
#endif

static struct ctl_table_set *
f(struct ctl_table_root *a, struct nsproxy *b)
{
	return &b->net_ns->sysctls;
}

static int g(struct ctl_table_set *a)
{
	return &current->nsproxy->net_ns->sysctls == a;
}

/* Return standard mode bits for table entry. */
static int h(struct ctl_table_header *a,
			       struct ctl_table *b)
{
	struct net *c = container_of(a->set, struct net, sysctls);
	kuid_t d = make_kuid(c->user_ns, 0);
	kgid_t e = make_kgid(c->user_ns, 0);

	/* Allow network administrator to have same access as root. */
	if (ns_capable(c->user_ns, CAP_NET_ADMIN) ||
	    uid_eq(d, current_uid())) {
		int f = (b->mode >> 6) & 7;
		return (f << 6) | (f << 3) | f;
	}
	/* Allow netns root group to have the same access as the root group */
	if (gid_eq(e, current_gid())) {
		int f = (b->mode >> 3) & 7;
		return (f << 3) | f;
	}
	return b->mode;
}

static struct ctl_table_root i = {
	.lookup = f,
	.permissions = h,
};

static int __net_init j(struct net *a)
{
	setup_sysctl_set(&a->sysctls, &i, g);
	return 0;
}

static void __net_exit k(struct net *a)
{
	retire_sysctl_set(&a->sysctls);
}

static struct pernet_operations l = {
	.init = j,
	.exit = k,
};

static struct ctl_table_header *m;
__init int n(void)
{
	static struct ctl_table o[1];
	int p = -ENOMEM;
	/* Avoid limitations in the sysctl implementation by
	 * registering "/proc/sys/net" as an empty directory not in a
	 * network namespace.
	 */
	m = register_sysctl("net", o);
	if (!m)
		goto q;
	p = register_pernet_subsys(&l);
	if (p)
		goto q;
	register_sysctl_root(&i);
q:
	return p;
}

struct ctl_table_header *r(struct net *a,
	const char *b, struct ctl_table *c)
{
	return __register_sysctl_table(&a->sysctls, b, c);
}
EXPORT_SYMBOL_GPL(r);

void s(struct ctl_table_header *a)
{
	unregister_sysctl_table(a);
}
EXPORT_SYMBOL_GPL(s);