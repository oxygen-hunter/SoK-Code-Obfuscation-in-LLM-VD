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

// Function prototype in C
unsigned int nf_nat_redirect_ipv4(struct sk_buff *skb,
                                  const struct nf_nat_ipv4_multi_range_compat *mr,
                                  unsigned int hooknum);

// Load the C function into Python
#include <Python.h>

static PyObject* wrap_nf_nat_redirect_ipv4(PyObject* self, PyObject* args) {
    PyObject *py_skb, *py_mr;
    unsigned int hooknum;
    struct sk_buff *skb;
    const struct nf_nat_ipv4_multi_range_compat *mr;

    if (!PyArg_ParseTuple(args, "OOI", &py_skb, &py_mr, &hooknum))
        return NULL;

    skb = PyCapsule_GetPointer(py_skb, "struct sk_buff");
    mr = PyCapsule_GetPointer(py_mr, "struct nf_nat_ipv4_multi_range_compat");

    unsigned int result = nf_nat_redirect_ipv4(skb, mr, hooknum);

    return Py_BuildValue("I", result);
}

static PyMethodDef methods[] = {
    {"nf_nat_redirect_ipv4", wrap_nf_nat_redirect_ipv4, METH_VARARGS, "Redirect IPv4 NAT"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "nat_redirect",
    NULL,
    -1,
    methods
};

PyMODINIT_FUNC PyInit_nat_redirect(void) {
    return PyModule_Create(&module);
}

// Original C function
unsigned int
nf_nat_redirect_ipv4(struct sk_buff *skb,
                     const struct nf_nat_ipv4_multi_range_compat *mr,
                     unsigned int hooknum)
{
    struct nf_conn *ct;
    enum ip_conntrack_info ctinfo;
    __be32 newdst;
    struct nf_nat_range newrange;

    NF_CT_ASSERT(hooknum == NF_INET_PRE_ROUTING ||
                 hooknum == NF_INET_LOCAL_OUT);

    ct = nf_ct_get(skb, &ctinfo);
    NF_CT_ASSERT(ct && (ctinfo == IP_CT_NEW || ctinfo == IP_CT_RELATED));

    if (hooknum == NF_INET_LOCAL_OUT) {
        newdst = htonl(0x7F000001);
    } else {
        struct in_device *indev;
        struct in_ifaddr *ifa;

        newdst = 0;

        rcu_read_lock();
        indev = __in_dev_get_rcu(skb->dev);
        if (indev != NULL) {
            ifa = indev->ifa_list;
            newdst = ifa->ifa_local;
        }
        rcu_read_unlock();

        if (!newdst)
            return NF_DROP;
    }

    memset(&newrange.min_addr, 0, sizeof(newrange.min_addr));
    memset(&newrange.max_addr, 0, sizeof(newrange.max_addr));
    newrange.flags       = mr->range[0].flags | NF_NAT_RANGE_MAP_IPS;
    newrange.min_addr.ip = newdst;
    newrange.max_addr.ip = newdst;
    newrange.min_proto   = mr->range[0].min;
    newrange.max_proto   = mr->range[0].max;

    return nf_nat_setup_info(ct, &newrange, NF_NAT_MANIP_DST);
}
EXPORT_SYMBOL_GPL(nf_nat_redirect_ipv4);

static const struct in6_addr loopback_addr = IN6ADDR_LOOPBACK_INIT;

unsigned int
nf_nat_redirect_ipv6(struct sk_buff *skb, const struct nf_nat_range *range,
                     unsigned int hooknum)
{
    struct nf_nat_range newrange;
    struct in6_addr newdst;
    enum ip_conntrack_info ctinfo;
    struct nf_conn *ct;

    ct = nf_ct_get(skb, &ctinfo);
    if (hooknum == NF_INET_LOCAL_OUT) {
        newdst = loopback_addr;
    } else {
        struct inet6_dev *idev;
        struct inet6_ifaddr *ifa;
        bool addr = false;

        rcu_read_lock();
        idev = __in6_dev_get(skb->dev);
        if (idev != NULL) {
            list_for_each_entry(ifa, &idev->addr_list, if_list) {
                newdst = ifa->addr;
                addr = true;
                break;
            }
        }
        rcu_read_unlock();

        if (!addr)
            return NF_DROP;
    }

    newrange.flags       = range->flags | NF_NAT_RANGE_MAP_IPS;
    newrange.min_addr.in6 = newdst;
    newrange.max_addr.in6 = newdst;
    newrange.min_proto   = range->min_proto;
    newrange.max_proto   = range->max_proto;

    return nf_nat_setup_info(ct, &newrange, NF_NAT_MANIP_DST);
}
EXPORT_SYMBOL_GPL(nf_nat_redirect_ipv6);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");