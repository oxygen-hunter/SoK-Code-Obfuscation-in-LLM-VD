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
#define VM_STACK_SIZE 256
#define VM_MEMORY_SIZE 1024

static DEFINE_SPINLOCK(snmp_lock);

struct snmp_ctx {
	unsigned char *begin;
	__sum16 *check;
	__be32 from;
	__be32 to;
};

typedef enum {
	INSTR_PUSH = 1,
	INSTR_POP,
	INSTR_ADD,
	INSTR_SUB,
	INSTR_JMP,
	INSTR_JZ,
	INSTR_LOAD,
	INSTR_STORE,
	INSTR_CSUM
} VMInstruction;

typedef struct {
	int stack[VM_STACK_SIZE];
	int sp;
	unsigned char memory[VM_MEMORY_SIZE];
	int pc;
} VM;

void vm_init(VM *vm) {
	vm->sp = -1;
	vm->pc = 0;
}

void vm_push(VM *vm, int value) {
	if (vm->sp < VM_STACK_SIZE - 1) {
		vm->stack[++vm->sp] = value;
	}
}

int vm_pop(VM *vm) {
	if (vm->sp >= 0) {
		return vm->stack[vm->sp--];
	}
	return 0;
}

void vm_execute(VM *vm, unsigned char *program, int programSize) {
	while (vm->pc < programSize) {
		switch (program[vm->pc++]) {
			case INSTR_PUSH: {
				int value = (program[vm->pc++] << 8) | program[vm->pc++];
				vm_push(vm, value);
				break;
			}
			case INSTR_POP:
				vm_pop(vm);
				break;
			case INSTR_ADD: {
				int b = vm_pop(vm);
				int a = vm_pop(vm);
				vm_push(vm, a + b);
				break;
			}
			case INSTR_SUB: {
				int b = vm_pop(vm);
				int a = vm_pop(vm);
				vm_push(vm, a - b);
				break;
			}
			case INSTR_JMP:
				vm->pc = (program[vm->pc++] << 8) | program[vm->pc];
				break;
			case INSTR_JZ: {
				int addr = (program[vm->pc++] << 8) | program[vm->pc++];
				if (vm_pop(vm) == 0) {
					vm->pc = addr;
				}
				break;
			}
			case INSTR_LOAD: {
				int addr = vm_pop(vm);
				vm_push(vm, vm->memory[addr]);
				break;
			}
			case INSTR_STORE: {
				int addr = vm_pop(vm);
				int value = vm_pop(vm);
				vm->memory[addr] = value;
				break;
			}
			case INSTR_CSUM: {
				struct snmp_ctx *ctx = (struct snmp_ctx *)vm->memory;
				unsigned char offset = vm_pop(vm);
				unsigned char s[12] = {0,};
				int size;
				if (offset & 1) {
					memcpy(&s[1], &ctx->from, 4);
					memcpy(&s[7], &ctx->to, 4);
					s[0] = ~0;
					s[1] = ~s[1];
					s[2] = ~s[2];
					s[3] = ~s[3];
					s[4] = ~s[4];
					s[5] = ~0;
					size = 12;
				} else {
					memcpy(&s[0], &ctx->from, 4);
					memcpy(&s[4], &ctx->to, 4);
					s[0] = ~s[0];
					s[1] = ~s[1];
					s[2] = ~s[2];
					s[3] = ~s[3];
					size = 8;
				}
				*ctx->check = csum_fold(csum_partial(s, size, ~csum_unfold(*ctx->check)));
				break;
			}
		}
	}
}

int snmp_helper(void *context, size_t hdrlen, unsigned char tag, const void *data, size_t datalen) {
	struct snmp_ctx *ctx = (struct snmp_ctx *)context;
	__be32 *pdata = (__be32 *)data;

	if (*pdata == ctx->from) {
		pr_debug("%s: %pI4 to %pI4\n", __func__, (void *)&ctx->from, (void *)&ctx->to);

		if (*ctx->check) {
			VM vm;
			vm_init(&vm);
			vm.memory[0] = (unsigned char)data - ctx->begin;
			vm_push(&vm, 0); // offset
			unsigned char program[] = {INSTR_LOAD, INSTR_CSUM};
			vm_execute(&vm, program, sizeof(program));
		}
		*pdata = ctx->to;
	}

	return 1;
}

static int snmp_translate(struct nf_conn *ct, int dir, struct sk_buff *skb) {
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

static int help(struct sk_buff *skb, unsigned int protoff, struct nf_conn *ct, enum ip_conntrack_info ctinfo) {
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

static int __init nf_nat_snmp_basic_init(void) {
	BUG_ON(nf_nat_snmp_hook != NULL);
	RCU_INIT_POINTER(nf_nat_snmp_hook, help);

	return nf_conntrack_helper_register(&snmp_trap_helper);
}

static void __exit nf_nat_snmp_basic_fini(void) {
	RCU_INIT_POINTER(nf_nat_snmp_hook, NULL);
	synchronize_rcu();
	nf_conntrack_helper_unregister(&snmp_trap_helper);
}

module_init(nf_nat_snmp_basic_init);
module_exit(nf_nat_snmp_basic_fini);