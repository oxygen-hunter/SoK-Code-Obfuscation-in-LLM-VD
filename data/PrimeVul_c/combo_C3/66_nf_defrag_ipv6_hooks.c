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

#define PUSH 0
#define POP 1
#define ADD 2
#define SUB 3
#define JMP 4
#define JZ 5
#define LOAD 6
#define STORE 7
#define CALL_DEF_USER 8
#define CALL_DEFRAG 9
#define RETURN 10

typedef struct {
    int stack[256];
    int sp;
    int pc;
} VM;

void execute(VM *vm, int *prog) {
    int running = 1;
    while (running) {
        switch (prog[vm->pc++]) {
            case PUSH: vm->stack[vm->sp++] = prog[vm->pc++]; break;
            case POP: vm->sp--; break;
            case ADD: vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1]; vm->sp--; break;
            case SUB: vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1]; vm->sp--; break;
            case JMP: vm->pc = prog[vm->pc]; break;
            case JZ: if (vm->stack[--vm->sp] == 0) vm->pc = prog[vm->pc]; else vm->pc++; break;
            case LOAD: vm->stack[vm->sp++] = *((int *)prog[vm->pc++]); break;
            case STORE: *((int *)prog[vm->pc++]) = vm->stack[--vm->sp]; break;
            case CALL_DEF_USER: vm->stack[vm->sp++] = nf_ct6_defrag_user(vm->stack[--vm->sp], (struct sk_buff *)vm->stack[--vm->sp]); break;
            case CALL_DEFRAG: vm->stack[vm->sp++] = ipv6_defrag((void *)prog[vm->pc++], (struct sk_buff *)vm->stack[--vm->sp], (const struct nf_hook_state *)vm->stack[--vm->sp]); break;
            case RETURN: running = 0; break;
        }
    }
}

static enum ip6_defrag_users nf_ct6_defrag_user(unsigned int hooknum, struct sk_buff *skb) {
    VM vm = { .sp = 0, .pc = 0 };
    int prog[] = {
        PUSH, NF_CT_DEFAULT_ZONE_ID,
        PUSH, (int)skb,
        LOAD, (int)&skb->nfct,
        JZ, 10,
        PUSH, (int)skb,
        CALL_DEF_USER,
        STORE, (int)&prog[1],
        PUSH, nf_bridge_in_prerouting((struct sk_buff *)prog[1]),
        JZ, 18,
        PUSH, IP6_DEFRAG_CONNTRACK_BRIDGE_IN,
        ADD,
        RETURN,
        PUSH, hooknum,
        PUSH, NF_INET_PRE_ROUTING,
        SUB,
        JZ, 25,
        PUSH, IP6_DEFRAG_CONNTRACK_IN,
        ADD,
        RETURN,
        PUSH, IP6_DEFRAG_CONNTRACK_OUT,
        ADD,
        RETURN
    };
    execute(&vm, prog);
    return prog[1];
}

static unsigned int ipv6_defrag(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    VM vm = { .sp = 0, .pc = 0 };
    int prog[] = {
        PUSH, (int)skb,
        LOAD, (int)&skb->nfct,
        JZ, 10,
        PUSH, (int)skb,
        CALL_DEFRAG,
        RETURN,
        PUSH, state->hook,
        PUSH, (int)skb,
        CALL_DEF_USER,
        STORE, (int)&prog[1],
        PUSH, state->net,
        PUSH, (int)skb,
        PUSH, prog[1],
        CALL_DEFRAG,
        JZ, 24,
        PUSH, NF_STOLEN,
        RETURN,
        PUSH, NF_ACCEPT,
        RETURN
    };
    execute(&vm, prog);
    return prog[1];
}

static struct nf_hook_ops ipv6_defrag_ops[] = {
    {
        .hook = ipv6_defrag,
        .pf = NFPROTO_IPV6,
        .hooknum = NF_INET_PRE_ROUTING,
        .priority = NF_IP6_PRI_CONNTRACK_DEFRAG,
    },
    {
        .hook = ipv6_defrag,
        .pf = NFPROTO_IPV6,
        .hooknum = NF_INET_LOCAL_OUT,
        .priority = NF_IP6_PRI_CONNTRACK_DEFRAG,
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