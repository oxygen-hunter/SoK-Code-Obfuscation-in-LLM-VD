#include <linux/types.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack_l4proto.h>

static unsigned int nf_ct_generic_timeout __read_mostly = 600*HZ;

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

#define STACK_SIZE 256
#define MEMORY_SIZE 256

typedef struct {
    int stack[STACK_SIZE];
    int memory[MEMORY_SIZE];
    int sp;
    int pc;
    int running;
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_execute(VM *vm, Instruction *program) {
    while (vm->running) {
        switch (program[vm->pc++]) {
            case PUSH: vm_push(vm, program[vm->pc++]); break;
            case POP: vm_pop(vm); break;
            case ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP: vm->pc = program[vm->pc]; break;
            case JZ: {
                int addr = program[vm->pc++];
                if (vm_pop(vm) == 0) vm->pc = addr;
                break;
            }
            case LOAD: {
                int addr = program[vm->pc++];
                vm_push(vm, vm->memory[addr]);
                break;
            }
            case STORE: {
                int addr = program[vm->pc++];
                vm->memory[addr] = vm_pop(vm);
                break;
            }
            case CALL: {
                int addr = program[vm->pc++];
                vm_push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case RET: vm->pc = vm_pop(vm); break;
            case HALT: vm->running = 0; break;
        }
    }
}

static inline struct nf_generic_net *generic_pernet(struct net *net) {
    return &net->ct.nf_ct_proto.generic;
}

static bool generic_pkt_to_tuple(const struct sk_buff *skb, unsigned int dataoff, struct nf_conntrack_tuple *tuple) {
    tuple->src.u.all = 0;
    tuple->dst.u.all = 0;

    return true;
}

static bool generic_invert_tuple(struct nf_conntrack_tuple *tuple, const struct nf_conntrack_tuple *orig) {
    tuple->src.u.all = 0;
    tuple->dst.u.all = 0;

    return true;
}

static int generic_print_tuple(struct seq_file *s, const struct nf_conntrack_tuple *tuple) {
    return 0;
}

static unsigned int *generic_get_timeouts(struct net *net) {
    return &(generic_pernet(net)->timeout);
}

static int generic_packet(struct nf_conn *ct, const struct sk_buff *skb, unsigned int dataoff, enum ip_conntrack_info ctinfo, u_int8_t pf, unsigned int hooknum, unsigned int *timeout) {
    VM vm;
    vm_init(&vm);
    Instruction program[] = {
        PUSH, *timeout,
        CALL, 8, // Call refresh
        PUSH, NF_ACCEPT,
        HALT,
        // refresh function at address 8
        PUSH, (int)ct,
        PUSH, (int)ctinfo,
        PUSH, (int)skb,
        STORE, 0, // Simulate refresh_acct
        RET
    };
    vm_execute(&vm, program);
    return vm_pop(&vm);
}

static bool generic_new(struct nf_conn *ct, const struct sk_buff *skb, unsigned int dataoff, unsigned int *timeouts) {
    return true;
}

#if IS_ENABLED(CONFIG_NF_CT_NETLINK_TIMEOUT)

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_cttimeout.h>

static int generic_timeout_nlattr_to_obj(struct nlattr *tb[], struct net *net, void *data) {
    unsigned int *timeout = data;
    struct nf_generic_net *gn = generic_pernet(net);

    if (tb[CTA_TIMEOUT_GENERIC_TIMEOUT]) {
        *timeout = ntohl(nla_get_be32(tb[CTA_TIMEOUT_GENERIC_TIMEOUT])) * HZ;
    } else {
        *timeout = gn->timeout;
    }

    return 0;
}

static int generic_timeout_obj_to_nlattr(struct sk_buff *skb, const void *data) {
    const unsigned int *timeout = data;

    if (nla_put_be32(skb, CTA_TIMEOUT_GENERIC_TIMEOUT, htonl(*timeout / HZ)))
        return -ENOSPC;

    return 0;
}

static const struct nla_policy generic_timeout_nla_policy[CTA_TIMEOUT_GENERIC_MAX+1] = {
    [CTA_TIMEOUT_GENERIC_TIMEOUT] = { .type = NLA_U32 },
};

#endif /* CONFIG_NF_CT_NETLINK_TIMEOUT */

#ifdef CONFIG_SYSCTL
static struct ctl_table generic_sysctl_table[] = {
    {
        .procname = "nf_conntrack_generic_timeout",
        .maxlen = sizeof(unsigned int),
        .mode = 0644,
        .proc_handler = proc_dointvec_jiffies,
    },
    { }
};
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
static struct ctl_table generic_compat_sysctl_table[] = {
    {
        .procname = "ip_conntrack_generic_timeout",
        .maxlen = sizeof(unsigned int),
        .mode = 0644,
        .proc_handler = proc_dointvec_jiffies,
    },
    { }
};
#endif /* CONFIG_NF_CONNTRACK_PROC_COMPAT */
#endif /* CONFIG_SYSCTL */

static int generic_kmemdup_sysctl_table(struct nf_proto_net *pn, struct nf_generic_net *gn) {
#ifdef CONFIG_SYSCTL
    pn->ctl_table = kmemdup(generic_sysctl_table, sizeof(generic_sysctl_table), GFP_KERNEL);
    if (!pn->ctl_table)
        return -ENOMEM;

    pn->ctl_table[0].data = &gn->timeout;
#endif
    return 0;
}

static int generic_kmemdup_compat_sysctl_table(struct nf_proto_net *pn, struct nf_generic_net *gn) {
#ifdef CONFIG_SYSCTL
#ifdef CONFIG_NF_CONNTRACK_PROC_COMPAT
    pn->ctl_compat_table = kmemdup(generic_compat_sysctl_table, sizeof(generic_compat_sysctl_table), GFP_KERNEL);
    if (!pn->ctl_compat_table)
        return -ENOMEM;

    pn->ctl_compat_table[0].data = &gn->timeout;
#endif
#endif
    return 0;
}

static int generic_init_net(struct net *net, u_int16_t proto) {
    int ret;
    struct nf_generic_net *gn = generic_pernet(net);
    struct nf_proto_net *pn = &gn->pn;

    gn->timeout = nf_ct_generic_timeout;

    ret = generic_kmemdup_compat_sysctl_table(pn, gn);
    if (ret < 0)
        return ret;

    ret = generic_kmemdup_sysctl_table(pn, gn);
    if (ret < 0)
        nf_ct_kfree_compat_sysctl_table(pn);

    return ret;
}

static struct nf_proto_net *generic_get_net_proto(struct net *net) {
    return &net->ct.nf_ct_proto.generic.pn;
}

struct nf_conntrack_l4proto nf_conntrack_l4proto_generic __read_mostly = {
    .l3proto = PF_UNSPEC,
    .l4proto = 255,
    .name = "unknown",
    .pkt_to_tuple = generic_pkt_to_tuple,
    .invert_tuple = generic_invert_tuple,
    .print_tuple = generic_print_tuple,
    .packet = generic_packet,
    .get_timeouts = generic_get_timeouts,
    .new = generic_new,
#if IS_ENABLED(CONFIG_NF_CT_NETLINK_TIMEOUT)
    .ctnl_timeout = {
        .nlattr_to_obj = generic_timeout_nlattr_to_obj,
        .obj_to_nlattr = generic_timeout_obj_to_nlattr,
        .nlattr_max = CTA_TIMEOUT_GENERIC_MAX,
        .obj_size = sizeof(unsigned int),
        .nla_policy = generic_timeout_nla_policy,
    },
#endif /* CONFIG_NF_CT_NETLINK_TIMEOUT */
    .init_net = generic_init_net,
    .get_net_proto = generic_get_net_proto,
};