#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <net/checksum.h>
#include <net/udp.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <linux/netfilter/nf_conntrack_snmp.h>
#include "nf_nat_snmp_basic.asn1.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("James Morris <jmorris@intercode.com.au>");
MODULE_DESCRIPTION("Basic SNMP Application Layer Gateway");
MODULE_ALIAS("ip_nat_snmp_basic");
MODULE_ALIAS_NFCT_HELPER("snmp_trap");

#define SNMP_PORT 161
#define SNMP_TRAP_PORT 162

static DEFINE_SPINLOCK(snmp_lock);

struct snmp_ctx {
	unsigned char *begin;
	__sum16 *check;
	__be32 from;
	__be32 to;
};

enum vm_instructions {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

struct vm {
    int stack[256];
    int sp;
    int pc;
    int reg[8];
    int (*program[256])(struct vm*);
};

static int vm_push(struct vm *vm) { vm->stack[vm->sp++] = vm->reg[vm->pc++]; return 0; }
static int vm_pop(struct vm *vm) { vm->reg[vm->pc++] = vm->stack[--vm->sp]; return 0; }
static int vm_add(struct vm *vm) { vm->stack[vm->sp-2] += vm->stack[vm->sp-1]; vm->sp--; return 0; }
static int vm_sub(struct vm *vm) { vm->stack[vm->sp-2] -= vm->stack[vm->sp-1]; vm->sp--; return 0; }
static int vm_jmp(struct vm *vm) { vm->pc = vm->reg[vm->pc]; return 0; }
static int vm_jz(struct vm *vm) { if (!vm->stack[vm->sp-1]) vm->pc = vm->reg[vm->pc]; vm->sp--; return 0; }
static int vm_load(struct vm *vm) { vm->stack[vm->sp++] = vm->reg[vm->pc++]; return 0; }
static int vm_store(struct vm *vm) { vm->reg[vm->pc++] = vm->stack[--vm->sp]; return 0; }
static int vm_call(struct vm *vm) { int addr = vm->reg[vm->pc++]; vm->stack[vm->sp++] = vm->pc; vm->pc = addr; return 0; }
static int vm_ret(struct vm *vm) { vm->pc = vm->stack[--vm->sp]; return 0; }

static void fast_csum(struct snmp_ctx *ctx, unsigned char offset)
{
    struct vm vm;
    vm.pc = 0;
    vm.sp = 0;
    vm.program[PUSH] = vm_push;
    vm.program[POP] = vm_pop;
    vm.program[ADD] = vm_add;
    vm.program[SUB] = vm_sub;
    vm.program[JMP] = vm_jmp;
    vm.program[JZ] = vm_jz;
    vm.program[LOAD] = vm_load;
    vm.program[STORE] = vm_store;
    vm.program[CALL] = vm_call;
    vm.program[RET] = vm_ret;

    vm.reg[0] = (int)(offset & 1);
    vm.reg[1] = 12;
    vm.reg[2] = 8;

    vm.program[0] = vm_load; // Load offset
    vm.program[1] = vm_jz;   // Jump if zero
    vm.program[2] = vm_load; // Load from
    vm.program[3] = vm_load; // Load to
    vm.program[4] = vm_sub;  // Subtract
    vm.program[5] = vm_store;// Store result
    vm.program[6] = vm_ret;  // Return

    vm.program[7] = vm_load; // Load from
    vm.program[8] = vm_load; // Load to
    vm.program[9] = vm_add;  // Add
    vm.program[10] = vm_store;// Store result
    vm.program[11] = vm_ret; // Return

    while (vm.program[vm.pc]) {
        vm.program[vm.pc](&vm);
    }

    *ctx->check = csum_fold(csum_partial(ctx->begin, vm.reg[1], ~csum_unfold(*ctx->check)));
}

int snmp_version(void *context, size_t hdrlen, unsigned char tag,
		 const void *data, size_t datalen)
{
	struct vm vm;
	vm.pc = 0;
	vm.sp = 0;
	vm.program[PUSH] = vm_push;
	vm.program[POP] = vm_pop;
	vm.program[ADD] = vm_add;
	vm.program[SUB] = vm_sub;
	vm.program[JMP] = vm_jmp;
	vm.program[JZ] = vm_jz;
	vm.program[LOAD] = vm_load;
	vm.program[STORE] = vm_store;
	vm.program[CALL] = vm_call;
	vm.program[RET] = vm_ret;

	vm.reg[0] = *(unsigned char *)data;
	vm.reg[1] = 1;

	vm.program[0] = vm_load; // Load data
	vm.program[1] = vm_sub;  // Subtract 1
	vm.program[2] = vm_jz;   // Jump if zero
	vm.program[3] = vm_push; // Push error
	vm.program[4] = vm_ret;  // Return

	vm.program[5] = vm_push; // Push success
	vm.program[6] = vm_ret;  // Return

	while (vm.program[vm.pc]) {
        vm.program[vm.pc](&vm);
    }

	return vm.stack[--vm.sp];
}

int snmp_helper(void *context, size_t hdrlen, unsigned char tag,
		const void *data, size_t datalen)
{
	struct snmp_ctx *ctx = (struct snmp_ctx *)context;
	__be32 *pdata = (__be32 *)data;

	struct vm vm;
	vm.pc = 0;
	vm.sp = 0;
	vm.program[PUSH] = vm_push;
	vm.program[POP] = vm_pop;
	vm.program[ADD] = vm_add;
	vm.program[SUB] = vm_sub;
	vm.program[JMP] = vm_jmp;
	vm.program[JZ] = vm_jz;
	vm.program[LOAD] = vm_load;
	vm.program[STORE] = vm_store;
	vm.program[CALL] = vm_call;
	vm.program[RET] = vm_ret;

	vm.reg[0] = (int)(*pdata == ctx->from);

	vm.program[0] = vm_load; // Load check
	vm.program[1] = vm_jz;   // Jump if zero
	vm.program[2] = vm_push; // Push logging function
	vm.program[3] = vm_call; // Call logging
	vm.program[4] = vm_push; // Push fast_csum function
	vm.program[5] = vm_call; // Call fast_csum
	vm.program[6] = vm_pop;  // Pop result
	vm.program[7] = vm_ret;  // Return

	vm.program[8] = vm_push; // Push success
	vm.program[9] = vm_ret;  // Return

	while (vm.program[vm.pc]) {
        vm.program[vm.pc](&vm);
    }

	if (*pdata == ctx->from) {
		pr_debug("%s: %pI4 to %pI4\n", __func__,
			 (void *)&ctx->from, (void *)&ctx->to);

		if (*ctx->check)
			fast_csum(ctx, (unsigned char *)data - ctx->begin);
		*pdata = ctx->to;
	}

	return 1;
}

static int snmp_translate(struct nf_conn *ct, int dir, struct sk_buff *skb)
{
	struct iphdr *iph = ip_hdr(skb);
	struct udphdr *udph = (struct udphdr *)((__be32 *)iph + iph->ihl);
	u16 datalen = ntohs(udph->len) - sizeof(struct udphdr);
	char *data = (unsigned char *)udph + sizeof(struct udphdr);
	struct snmp_ctx ctx;
	int ret;

	if (dir == IP_CT_DIR_ORIGINAL) {
		ctx.from = ct->tuplehash[dir].tuple.src.u3.ip;
		ctx.to = ct->tuplehash[!dir].tuple.dst.u3.ip;
	} else {
		ctx.from = ct->tuplehash[!dir].tuple.src.u3.ip;
		ctx.to = ct->tuplehash[dir].tuple.dst.u3.ip;
	}

	if (ctx.from == ctx.to)
		return NF_ACCEPT;

	ctx.begin = (unsigned char *)udph + sizeof(struct udphdr);
	ctx.check = &udph->check;
	ret = asn1_ber_decoder(&nf_nat_snmp_basic_decoder, &ctx, data, datalen);
	if (ret < 0) {
		nf_ct_helper_log(skb, ct, "parser failed\n");
		return NF_DROP;
	}

	return NF_ACCEPT;
}

static int help(struct sk_buff *skb, unsigned int protoff,
		struct nf_conn *ct,
		enum ip_conntrack_info ctinfo)
{
	int dir = CTINFO2DIR(ctinfo);
	unsigned int ret;
	const struct iphdr *iph = ip_hdr(skb);
	const struct udphdr *udph = (struct udphdr *)((__be32 *)iph + iph->ihl);

	if (udph->source == htons(SNMP_PORT) && dir != IP_CT_DIR_REPLY)
		return NF_ACCEPT;
	if (udph->dest == htons(SNMP_TRAP_PORT) && dir != IP_CT_DIR_ORIGINAL)
		return NF_ACCEPT;

	if (!(ct->status & IPS_NAT_MASK))
		return NF_ACCEPT;

	if (ntohs(udph->len) != skb->len - (iph->ihl << 2)) {
		nf_ct_helper_log(skb, ct, "dropping malformed packet\n");
		return NF_DROP;
	}

	if (!skb_make_writable(skb, skb->len)) {
		nf_ct_helper_log(skb, ct, "cannot mangle packet");
		return NF_DROP;
	}

	spin_lock_bh(&snmp_lock);
	ret = snmp_translate(ct, dir, skb);
	spin_unlock_bh(&snmp_lock);
	return ret;
}

static const struct nf_conntrack_expect_policy snmp_exp_policy = {
	.max_expected	= 0,
	.timeout	= 180,
};

static struct nf_conntrack_helper snmp_trap_helper __read_mostly = {
	.me			= THIS_MODULE,
	.help			= help,
	.expect_policy		= &snmp_exp_policy,
	.name			= "snmp_trap",
	.tuple.src.l3num	= AF_INET,
	.tuple.src.u.udp.port	= cpu_to_be16(SNMP_TRAP_PORT),
	.tuple.dst.protonum	= IPPROTO_UDP,
};

static int __init nf_nat_snmp_basic_init(void)
{
	BUG_ON(nf_nat_snmp_hook != NULL);
	RCU_INIT_POINTER(nf_nat_snmp_hook, help);

	return nf_conntrack_helper_register(&snmp_trap_helper);
}

static void __exit nf_nat_snmp_basic_fini(void)
{
	RCU_INIT_POINTER(nf_nat_snmp_hook, NULL);
	synchronize_rcu();
	nf_conntrack_helper_unregister(&snmp_trap_helper);
}

module_init(nf_nat_snmp_basic_init);
module_exit(nf_nat_snmp_basic_fini);