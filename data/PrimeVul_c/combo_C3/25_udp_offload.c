#include <linux/skbuff.h>
#include <net/protocol.h>
#include <net/ipv6.h>
#include <net/udp.h>
#include <net/ip6_checksum.h>
#include "ip6_offload.h"

enum { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT, CALL };

typedef struct VM {
	int stack[256];
	int sp;
	int pc;
	int memory[256];
} VM;

void vm_init(VM *vm) {
	vm->sp = -1;
	vm->pc = 0;
}

void vm_push(VM *vm, int value) {
	vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
	return vm->stack[vm->sp--];
}

void vm_run(VM *vm, int *code) {
	int running = 1;
	while (running) {
		switch (code[vm->pc++]) {
			case PUSH:
				vm_push(vm, code[vm->pc++]);
				break;
			case POP:
				vm_pop(vm);
				break;
			case ADD:
				vm_push(vm, vm_pop(vm) + vm_pop(vm));
				break;
			case SUB:
				vm_push(vm, vm_pop(vm) - vm_pop(vm));
				break;
			case JMP:
				vm->pc = code[vm->pc];
				break;
			case JZ:
				if (vm_pop(vm) == 0)
					vm->pc = code[vm->pc];
				else
					vm->pc++;
				break;
			case LOAD:
				vm_push(vm, vm->memory[code[vm->pc++]]);
				break;
			case STORE:
				vm->memory[code[vm->pc++]] = vm_pop(vm);
				break;
			case HALT:
				running = 0;
				break;
			case CALL:
				if (code[vm->pc++] == 0)
					udp6_ufo_send_check((struct sk_buff *)vm_pop(vm));
				else
					udp6_ufo_fragment((struct sk_buff *)vm_pop(vm), (netdev_features_t)vm_pop(vm));
				break;
		}
	}
}

static int udp6_ufo_send_check(struct sk_buff *skb) {
	int bytecode[] = {
		PUSH, (int)skb,
		CALL, 0,
		HALT
	};

	VM vm;
	vm_init(&vm);
	vm_run(&vm, bytecode);

	const struct ipv6hdr *ipv6h;
	struct udphdr *uh;

	if (!pskb_may_pull(skb, sizeof(*uh)))
		return -EINVAL;

	if (likely(!skb->encapsulation)) {
		ipv6h = ipv6_hdr(skb);
		uh = udp_hdr(skb);

		uh->check = ~csum_ipv6_magic(&ipv6h->saddr, &ipv6h->daddr, skb->len,
					     IPPROTO_UDP, 0);
		skb->csum_start = skb_transport_header(skb) - skb->head;
		skb->csum_offset = offsetof(struct udphdr, check);
		skb->ip_summed = CHECKSUM_PARTIAL;
	}

	return 0;
}

static struct sk_buff *udp6_ufo_fragment(struct sk_buff *skb, netdev_features_t features) {
	int bytecode[] = {
		PUSH, (int)skb,
		PUSH, (int)features,
		CALL, 1,
		HALT
	};

	VM vm;
	vm_init(&vm);
	vm_run(&vm, bytecode);

	struct sk_buff *segs = ERR_PTR(-EINVAL);
	unsigned int mss;
	unsigned int unfrag_ip6hlen, unfrag_len;
	struct frag_hdr *fptr;
	u8 *packet_start, *prevhdr;
	u8 nexthdr;
	u8 frag_hdr_sz = sizeof(struct frag_hdr);
	int offset;
	__wsum csum;
	int tnl_hlen;

	mss = skb_shinfo(skb)->gso_size;
	if (unlikely(skb->len <= mss))
		goto out;

	if (skb_gso_ok(skb, features | NETIF_F_GSO_ROBUST)) {
		int type = skb_shinfo(skb)->gso_type;

		if (unlikely(type & ~(SKB_GSO_UDP |
				      SKB_GSO_DODGY |
				      SKB_GSO_UDP_TUNNEL |
				      SKB_GSO_GRE |
				      SKB_GSO_IPIP |
				      SKB_GSO_SIT |
				      SKB_GSO_MPLS) ||
			     !(type & (SKB_GSO_UDP))))
			goto out;

		skb_shinfo(skb)->gso_segs = DIV_ROUND_UP(skb->len, mss);

		segs = NULL;
		goto out;
	}

	if (skb->encapsulation && skb_shinfo(skb)->gso_type & SKB_GSO_UDP_TUNNEL)
		segs = skb_udp_tunnel_segment(skb, features);
	else {
		offset = skb_checksum_start_offset(skb);
		csum = skb_checksum(skb, offset, skb->len - offset, 0);
		offset += skb->csum_offset;
		*(__sum16 *)(skb->data + offset) = csum_fold(csum);
		skb->ip_summed = CHECKSUM_NONE;

		tnl_hlen = skb_tnl_header_len(skb);
		if (skb_headroom(skb) < (tnl_hlen + frag_hdr_sz)) {
			if (gso_pskb_expand_head(skb, tnl_hlen + frag_hdr_sz))
				goto out;
		}

		unfrag_ip6hlen = ip6_find_1stfragopt(skb, &prevhdr);
		nexthdr = *prevhdr;
		*prevhdr = NEXTHDR_FRAGMENT;
		unfrag_len = (skb_network_header(skb) - skb_mac_header(skb)) +
			     unfrag_ip6hlen + tnl_hlen;
		packet_start = (u8 *) skb->head + SKB_GSO_CB(skb)->mac_offset;
		memmove(packet_start-frag_hdr_sz, packet_start, unfrag_len);

		SKB_GSO_CB(skb)->mac_offset -= frag_hdr_sz;
		skb->mac_header -= frag_hdr_sz;
		skb->network_header -= frag_hdr_sz;

		fptr = (struct frag_hdr *)(skb_network_header(skb) + unfrag_ip6hlen);
		fptr->nexthdr = nexthdr;
		fptr->reserved = 0;
		ipv6_select_ident(fptr, (struct rt6_info *)skb_dst(skb));

		segs = skb_segment(skb, features);
	}

out:
	return segs;
}

static const struct net_offload udpv6_offload = {
	.callbacks = {
		.gso_send_check =	udp6_ufo_send_check,
		.gso_segment	=	udp6_ufo_fragment,
	},
};

int __init udp_offload_init(void) {
	return inet6_add_offload(&udpv6_offload, IPPROTO_UDP);
}