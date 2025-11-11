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
OX7B4DF339(struct ctl_table_root *OX4F7E8C1C, struct nsproxy *OX8E3A7B1D)
{
	return &OX8E3A7B1D->net_ns->sysctls;
}

static int OX2CAA1F6A(struct ctl_table_set *OX3E8F1B29)
{
	return &current->nsproxy->net_ns->sysctls == OX3E8F1B29;
}

static int OXB1D1C3E3(struct ctl_table_header *OX9F4C3E2E,
			       struct ctl_table *OX1A7C2E4B)
{
	struct net *OX7A4B2C1D = container_of(OX9F4C3E2E->set, struct net, sysctls);
	kuid_t OX5C1D4B3E = make_kuid(OX7A4B2C1D->user_ns, 0);
	kgid_t OX4B2C1D7A = make_kgid(OX7A4B2C1D->user_ns, 0);

	if (ns_capable(OX7A4B2C1D->user_ns, CAP_NET_ADMIN) ||
	    uid_eq(OX5C1D4B3E, current_uid())) {
		int OX3E2E4B1A = (OX1A7C2E4B->mode >> 6) & 7;
		return (OX3E2E4B1A << 6) | (OX3E2E4B1A << 3) | OX3E2E4B1A;
	}
	if (gid_eq(OX4B2C1D7A, current_gid())) {
		int OX3E2E4B1A = (OX1A7C2E4B->mode >> 3) & 7;
		return (OX3E2E4B1A << 3) | OX3E2E4B1A;
	}
	return OX1A7C2E4B->mode;
}

static struct ctl_table_root OX4A2B3C1D = {
	.lookup = OX7B4DF339,
	.permissions = OXB1D1C3E3,
};

static int __net_init OX9B3D4F5C(struct net *OX7A4B2C1D)
{
	setup_sysctl_set(&OX7A4B2C1D->sysctls, &OX4A2B3C1D, OX2CAA1F6A);
	return 0;
}

static void __net_exit OX5C4D3B2A(struct net *OX7A4B2C1D)
{
	retire_sysctl_set(&OX7A4B2C1D->sysctls);
}

static struct pernet_operations OX2A3B4C5D = {
	.init = OX9B3D4F5C,
	.exit = OX5C4D3B2A,
};

static struct ctl_table_header *OX7F4E3D2C;
__init int OX1D2C3B4A(void)
{
	static struct ctl_table OX8A7B6C5D[1];
	int OX4F5E6D7C = -ENOMEM;
	OX7F4E3D2C = register_sysctl("net", OX8A7B6C5D);
	if (!OX7F4E3D2C)
		goto OXC2D3E4F;
	OX4F5E6D7C = register_pernet_subsys(&OX2A3B4C5D);
	if (OX4F5E6D7C)
		goto OXC2D3E4F;
	register_sysctl_root(&OX4A2B3C1D);
OXC2D3E4F:
	return OX4F5E6D7C;
}

struct ctl_table_header *OX9C8B7A6D(struct net *OX7A4B2C1D,
	const char *OX3B4A5C6D, struct ctl_table *OX1A2B3C4D)
{
	return __register_sysctl_table(&OX7A4B2C1D->sysctls, OX3B4A5C6D, OX1A2B3C4D);
}
EXPORT_SYMBOL_GPL(OX9C8B7A6D);

void OX8B7C6D5A(struct ctl_table_header *OX7F4E3D2C)
{
	unregister_sysctl_table(OX7F4E3D2C);
}
EXPORT_SYMBOL_GPL(OX8B7C6D5A);