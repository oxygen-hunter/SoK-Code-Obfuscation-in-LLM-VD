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
net_ctl_header_lookup(struct ctl_table_root *root, struct nsproxy *namespaces)
{
	int dispatch = 0;
	struct ctl_table_set *result = NULL;
	while (dispatch != -1) {
		if (dispatch == 0) {
			result = &namespaces->net_ns->sysctls;
			dispatch = -1;
		} else {
			dispatch = -1;
		}
	}
	return result;
}

static int is_seen(struct ctl_table_set *set)
{
	int dispatch = 0;
	int result = 0;
	while (dispatch != -1) {
		if (dispatch == 0) {
			result = (&current->nsproxy->net_ns->sysctls == set);
			dispatch = -1;
		} else {
			dispatch = -1;
		}
	}
	return result;
}

static int net_ctl_permissions(struct ctl_table_header *head,
			       struct ctl_table *table)
{
	int dispatch = 0;
	int result = 0;
	while (dispatch != -1) {
		if (dispatch == 0) {
			struct net *net = container_of(head->set, struct net, sysctls);
			kuid_t root_uid = make_kuid(net->user_ns, 0);
			kgid_t root_gid = make_kgid(net->user_ns, 0);

			if (ns_capable(net->user_ns, CAP_NET_ADMIN) ||
			    uid_eq(root_uid, current_uid())) {
				int mode = (table->mode >> 6) & 7;
				result = (mode << 6) | (mode << 3) | mode;
				dispatch = -1;
			} else {
				dispatch = 1;
			}
		} else if (dispatch == 1) {
			struct net *net = container_of(head->set, struct net, sysctls);
			kgid_t root_gid = make_kgid(net->user_ns, 0);

			if (gid_eq(root_gid, current_gid())) {
				int mode = (table->mode >> 3) & 7;
				result = (mode << 3) | mode;
			} else {
				result = table->mode;
			}
			dispatch = -1;
		} else {
			dispatch = -1;
		}
	}
	return result;
}

static struct ctl_table_root net_sysctl_root = {
	.lookup = net_ctl_header_lookup,
	.permissions = net_ctl_permissions,
};

static int __net_init sysctl_net_init(struct net *net)
{
	int dispatch = 0;
	int result = 0;
	while (dispatch != -1) {
		if (dispatch == 0) {
			setup_sysctl_set(&net->sysctls, &net_sysctl_root, is_seen);
			result = 0;
			dispatch = -1;
		} else {
			dispatch = -1;
		}
	}
	return result;
}

static void __net_exit sysctl_net_exit(struct net *net)
{
	int dispatch = 0;
	while (dispatch != -1) {
		if (dispatch == 0) {
			retire_sysctl_set(&net->sysctls);
			dispatch = -1;
		} else {
			dispatch = -1;
		}
	}
}

static struct pernet_operations sysctl_pernet_ops = {
	.init = sysctl_net_init,
	.exit = sysctl_net_exit,
};

static struct ctl_table_header *net_header;
__init int net_sysctl_init(void)
{
	static struct ctl_table empty[1];
	int ret = -ENOMEM;
	int dispatch = 0;
	while (dispatch != -1) {
		if (dispatch == 0) {
			net_header = register_sysctl("net", empty);
			if (!net_header)
				dispatch = 1;
			else 
				dispatch = 2;
		} else if (dispatch == 1) {
			dispatch = -1;
		} else if (dispatch == 2) {
			ret = register_pernet_subsys(&sysctl_pernet_ops);
			if (ret)
				dispatch = 1;
			else 
				dispatch = 3;
		} else if (dispatch == 3) {
			register_sysctl_root(&net_sysctl_root);
			dispatch = 1;
		} else {
			dispatch = -1;
		}
	}
	return ret;
}

struct ctl_table_header *register_net_sysctl(struct net *net,
	const char *path, struct ctl_table *table)
{
	int dispatch = 0;
	struct ctl_table_header *result = NULL;
	while (dispatch != -1) {
		if (dispatch == 0) {
			result = __register_sysctl_table(&net->sysctls, path, table);
			dispatch = -1;
		} else {
			dispatch = -1;
		}
	}
	return result;
}
EXPORT_SYMBOL_GPL(register_net_sysctl);

void unregister_net_sysctl_table(struct ctl_table_header *header)
{
	int dispatch = 0;
	while (dispatch != -1) {
		if (dispatch == 0) {
			unregister_sysctl_table(header);
			dispatch = -1;
		} else {
			dispatch = -1;
		}
	}
}
EXPORT_SYMBOL_GPL(unregister_net_sysctl_table);