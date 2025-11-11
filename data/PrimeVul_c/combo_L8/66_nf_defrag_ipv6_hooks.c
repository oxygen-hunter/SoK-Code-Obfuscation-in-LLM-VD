#include <Python.h>
#include <dlfcn.h>
#include <stdio.h>
#include <linux/types.h>
#include <linux/ipv6.h>
#include <linux/in6.h>
#include <linux/netfilter.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <linux/sysctl.h>
#include <net/ipv6.h>
#include <net/inet_frag.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter_bridge.h>
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/ipv6/nf_conntrack_ipv6.h>
#endif
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/ipv6/nf_defrag_ipv6.h>

void load_python_code(const char* module_name, const char* function_name) {
    Py_Initialize();
    PyObject *pName = PyUnicode_DecodeFSDefault(module_name);
    PyObject *pModule = PyImport_Import(pName);
    if (pModule != NULL) {
        PyObject *pFunc = PyObject_GetAttrString(pModule, function_name);
        if (PyCallable_Check(pFunc)) {
            PyObject_CallObject(pFunc, NULL);
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    }
    Py_DECREF(pName);
    Py_Finalize();
}

static enum ip6_defrag_users nf_ct6_defrag_user(unsigned int hooknum, struct sk_buff *skb) {
    u16 z = NF_CT_DEFAULT_ZONE_ID;
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
    if (skb->nfct) {
        enum ip_conntrack_info ctinfo;
        const struct nf_conn *ct = nf_ct_get(skb, &ctinfo);

        z = nf_ct_zone_id(nf_ct_zone(ct), CTINFO2DIR(ctinfo));
    }
#endif
    if (nf_bridge_in_prerouting(skb))
        return IP6_DEFRAG_CONNTRACK_BRIDGE_IN + z;

    if (hooknum == NF_INET_PRE_ROUTING)
        return IP6_DEFRAG_CONNTRACK_IN + z;
    else
        return IP6_DEFRAG_CONNTRACK_OUT + z;
}

static unsigned int ipv6_defrag(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    int err;
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
    if (skb->nfct && !nf_ct_is_template((struct nf_conn *)skb->nfct))
        return NF_ACCEPT;
#endif

    err = nf_ct_frag6_gather(state->net, skb, nf_ct6_defrag_user(state->hook, skb));
    if (err == -EINPROGRESS)
        return NF_STOLEN;

    return NF_ACCEPT;
}

static struct nf_hook_ops ipv6_defrag_ops[] = {
    {
        .hook       = ipv6_defrag,
        .pf         = NFPROTO_IPV6,
        .hooknum    = NF_INET_PRE_ROUTING,
        .priority   = NF_IP6_PRI_CONNTRACK_DEFRAG,
    },
    {
        .hook       = ipv6_defrag,
        .pf         = NFPROTO_IPV6,
        .hooknum    = NF_INET_LOCAL_OUT,
        .priority   = NF_IP6_PRI_CONNTRACK_DEFRAG,
    },
};

static int __init nf_defrag_init(void) {
    int ret = 0;
    ret = nf_ct_frag6_init();
    if (ret < 0) {
        pr_err("nf_defrag_ipv6: can't initialize frag6.\n");
        return ret;
    }
    ret = nf_register_hooks(ipv6_defrag_ops, ARRAY_SIZE(ipv6_defrag_ops));
    if (ret < 0) {
        pr_err("nf_defrag_ipv6: can't register hooks\n");
        goto cleanup_frag6;
    }
    load_python_code("module_name", "function_name");
    return ret;

cleanup_frag6:
    nf_ct_frag6_cleanup();
    return ret;
}

static void __exit nf_defrag_fini(void) {
    nf_unregister_hooks(ipv6_defrag_ops, ARRAY_SIZE(ipv6_defrag_ops));
    nf_ct_frag6_cleanup();
}

void nf_defrag_ipv6_enable(void) {
}
EXPORT_SYMBOL_GPL(nf_defrag_ipv6_enable);

module_init(nf_defrag_init);
module_exit(nf_defrag_fini);

MODULE_LICENSE("GPL");