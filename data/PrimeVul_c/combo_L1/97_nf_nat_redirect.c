/*
 * (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2006 Netfilter Core Team <coreteam@netfilter.org>
 * Copyright (c) 2011 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Based on Rusty Russell's IPv4 REDIRECT target. Development of IPv6
 * NAT funded by Astaro.
 */

#include <linux/if.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/netfilter.h>
#include <linux/types.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter/x_tables.h>
#include <net/addrconf.h>
#include <net/checksum.h>
#include <net/protocol.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_redirect.h>

unsigned int
OX7B4DF339(struct sk_buff *OXBF12D4A,
		     const struct nf_nat_ipv4_multi_range_compat *OX2A1B3C4,
		     unsigned int OX5F6A7B8)
{
	struct nf_conn *OX8A9B0C1;
	enum ip_conntrack_info OX3D4E5F6;
	__be32 OX1B2C3D4;
	struct nf_nat_range OX4E5F6A7;

	NF_CT_ASSERT(OX5F6A7B8 == NF_INET_PRE_ROUTING ||
		     OX5F6A7B8 == NF_INET_LOCAL_OUT);

	OX8A9B0C1 = nf_ct_get(OXBF12D4A, &OX3D4E5F6);
	NF_CT_ASSERT(OX8A9B0C1 && (OX3D4E5F6 == IP_CT_NEW || OX3D4E5F6 == IP_CT_RELATED));

	if (OX5F6A7B8 == NF_INET_LOCAL_OUT) {
		OX1B2C3D4 = htonl(0x7F000001);
	} else {
		struct in_device *OX2B3C4D5;
		struct in_ifaddr *OX6A7B8C9;

		OX1B2C3D4 = 0;

		rcu_read_lock();
		OX2B3C4D5 = __in_dev_get_rcu(OXBF12D4A->dev);
		if (OX2B3C4D5 != NULL) {
			OX6A7B8C9 = OX2B3C4D5->ifa_list;
			OX1B2C3D4 = OX6A7B8C9->ifa_local;
		}
		rcu_read_unlock();

		if (!OX1B2C3D4)
			return NF_DROP;
	}

	memset(&OX4E5F6A7.min_addr, 0, sizeof(OX4E5F6A7.min_addr));
	memset(&OX4E5F6A7.max_addr, 0, sizeof(OX4E5F6A7.max_addr));
	OX4E5F6A7.flags	     = OX2A1B3C4->range[0].flags | NF_NAT_RANGE_MAP_IPS;
	OX4E5F6A7.min_addr.ip = OX1B2C3D4;
	OX4E5F6A7.max_addr.ip = OX1B2C3D4;
	OX4E5F6A7.min_proto   = OX2A1B3C4->range[0].min;
	OX4E5F6A7.max_proto   = OX2A1B3C4->range[0].max;

	return nf_nat_setup_info(OX8A9B0C1, &OX4E5F6A7, NF_NAT_MANIP_DST);
}
EXPORT_SYMBOL_GPL(OX7B4DF339);

static const struct in6_addr OX9A0B1C2 = IN6ADDR_LOOPBACK_INIT;

unsigned int
OX8C7D6E5(struct sk_buff *OX0A1B2C3, const struct nf_nat_range *OX3C4D5E6,
		     unsigned int OX5E6F7A8)
{
	struct nf_nat_range OX7A8B9C0;
	struct in6_addr OX2D3E4F5;
	enum ip_conntrack_info OX4F5A6B7;
	struct nf_conn *OX6C7D8E9;

	OX6C7D8E9 = nf_ct_get(OX0A1B2C3, &OX4F5A6B7);
	if (OX5E6F7A8 == NF_INET_LOCAL_OUT) {
		OX2D3E4F5 = OX9A0B1C2;
	} else {
		struct inet6_dev *OX3E4F5A6;
		struct inet6_ifaddr *OX7B8C9D0;
		bool OXA9B0C1D = false;

		rcu_read_lock();
		OX3E4F5A6 = __in6_dev_get(OX0A1B2C3->dev);
		if (OX3E4F5A6 != NULL) {
			list_for_each_entry(OX7B8C9D0, &OX3E4F5A6->addr_list, if_list) {
				OX2D3E4F5 = OX7B8C9D0->addr;
				OXA9B0C1D = true;
				break;
			}
		}
		rcu_read_unlock();

		if (!OXA9B0C1D)
			return NF_DROP;
	}

	OX7A8B9C0.flags		= OX3C4D5E6->flags | NF_NAT_RANGE_MAP_IPS;
	OX7A8B9C0.min_addr.in6	= OX2D3E4F5;
	OX7A8B9C0.max_addr.in6	= OX2D3E4F5;
	OX7A8B9C0.min_proto	= OX3C4D5E6->min_proto;
	OX7A8B9C0.max_proto	= OX3C4D5E6->max_proto;

	return nf_nat_setup_info(OX6C7D8E9, &OX7A8B9C0, NF_NAT_MANIP_DST);
}
EXPORT_SYMBOL_GPL(OX8C7D6E5);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");