#include <linux/export.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/asn1_decoder.h>
#include <linux/asn1_ber_bytecode.h>

static const unsigned char asn1_op_lengths[ASN1_OP__NR] = {
	[ASN1_OP_MATCH]				= 1 + 1,
	[ASN1_OP_MATCH_OR_SKIP]			= 1 + 1,
	[ASN1_OP_MATCH_ACT]			= 1 + 1     + 1,
	[ASN1_OP_MATCH_ACT_OR_SKIP]		= 1 + 1     + 1,
	[ASN1_OP_MATCH_JUMP]			= 1 + 1 + 1,
	[ASN1_OP_MATCH_JUMP_OR_SKIP]		= 1 + 1 + 1,
	[ASN1_OP_MATCH_ANY]			= 1,
	[ASN1_OP_MATCH_ANY_ACT]			= 1         + 1,
	[ASN1_OP_COND_MATCH_OR_SKIP]		= 1 + 1,
	[ASN1_OP_COND_MATCH_ACT_OR_SKIP]	= 1 + 1     + 1,
	[ASN1_OP_COND_MATCH_JUMP_OR_SKIP]	= 1 + 1 + 1,
	[ASN1_OP_COND_MATCH_ANY]		= 1,
	[ASN1_OP_COND_MATCH_ANY_ACT]		= 1         + 1,
	[ASN1_OP_COND_FAIL]			= 1,
	[ASN1_OP_COMPLETE]			= 1,
	[ASN1_OP_ACT]				= 1         + 1,
	[ASN1_OP_MAYBE_ACT]			= 1         + 1,
	[ASN1_OP_RETURN]			= 1,
	[ASN1_OP_END_SEQ]			= 1,
	[ASN1_OP_END_SEQ_OF]			= 1     + 1,
	[ASN1_OP_END_SET]			= 1,
	[ASN1_OP_END_SET_OF]			= 1     + 1,
	[ASN1_OP_END_SEQ_ACT]			= 1         + 1,
	[ASN1_OP_END_SEQ_OF_ACT]		= 1     + 1 + 1,
	[ASN1_OP_END_SET_ACT]			= 1         + 1,
	[ASN1_OP_END_SET_OF_ACT]		= 1     + 1 + 1,
};

static int asn1_find_indefinite_length(const unsigned char *data, size_t datalen,
				       size_t *_dp, size_t *_len,
				       const char **_errmsg)
{
	unsigned char tag, tmp;
	size_t dp = *_dp, len, n;
	int indef_level = 1;
	int control = 0;

	while (1) {
		switch (control) {
		case 0:
			if (unlikely(datalen - dp < 2)) {
				if (datalen == dp)
					control = 1;
				else
					control = 6;
				break;
			}
			tag = data[dp++];
			if (tag == 0) {
				if (data[dp++] != 0)
					control = 5;
				else if (--indef_level <= 0) {
					*_len = dp - *_dp;
					*_dp = dp;
					return 0;
				}
				control = 0;
			} else {
				if (unlikely((tag & 0x1f) == ASN1_LONG_TAG)) {
					control = 2;
				} else {
					control = 3;
				}
			}
			break;
		case 1:
			*_errmsg = "Missing EOC in indefinite len cons";
			control = 7;
			break;
		case 2:
			do {
				if (unlikely(datalen - dp < 2))
					control = 6;
				else
					tmp = data[dp++];
			} while (tmp & 0x80);
			control = 3;
			break;
		case 3:
			len = data[dp++];
			if (len <= 0x7f) {
				dp += len;
				control = 0;
			} else if (unlikely(len == ASN1_INDEFINITE_LENGTH)) {
				if (unlikely((tag & ASN1_CONS_BIT) == ASN1_PRIM << 5))
					control = 4;
				else {
					indef_level++;
					control = 0;
				}
			} else {
				n = len - 0x80;
				if (unlikely(n > sizeof(size_t) - 1))
					control = 8;
				else if (unlikely(n > datalen - dp))
					control = 6;
				else {
					for (len = 0; n > 0; n--) {
						len <<= 8;
						len |= data[dp++];
					}
					dp += len;
					control = 0;
				}
			}
			break;
		case 4:
			*_errmsg = "Indefinite len primitive not permitted";
			control = 7;
			break;
		case 5:
			*_errmsg = "Invalid length EOC";
			control = 7;
			break;
		case 6:
			*_errmsg = "Data overrun error";
			control = 7;
			break;
		case 7:
			*_dp = dp;
			return -1;
		case 8:
			*_errmsg = "Unsupported length";
			control = 7;
			break;
		}
	}
}

int asn1_ber_decoder(const struct asn1_decoder *decoder,
		     void *context,
		     const unsigned char *data,
		     size_t datalen)
{
	const unsigned char *machine = decoder->machine;
	const asn1_action_t *actions = decoder->actions;
	size_t machlen = decoder->machlen;
	enum asn1_opcode op;
	unsigned char tag = 0, csp = 0, jsp = 0, optag = 0, hdr = 0;
	const char *errmsg;
	size_t pc = 0, dp = 0, tdp = 0, len = 0;
	int ret;

	unsigned char flags = 0;
#define FLAG_INDEFINITE_LENGTH	0x01
#define FLAG_MATCHED		0x02
#define FLAG_LAST_MATCHED	0x04
#define FLAG_CONS		0x20

#define NR_CONS_STACK 10
	unsigned short cons_dp_stack[NR_CONS_STACK];
	unsigned short cons_datalen_stack[NR_CONS_STACK];
	unsigned char cons_hdrlen_stack[NR_CONS_STACK];
#define NR_JUMP_STACK 10
	unsigned char jump_stack[NR_JUMP_STACK];

	if (datalen > 65535)
		return -EMSGSIZE;

	int control = 0;

	while (1) {
		switch (control) {
		case 0:
			pr_debug("next_op: pc=\e[32m%zu\e[m/%zu dp=\e[33m%zu\e[m/%zu C=%d J=%d\n",
				pc, machlen, dp, datalen, csp, jsp);
			if (unlikely(pc >= machlen))
				control = 7;
			else {
				op = machine[pc];
				if (unlikely(pc + asn1_op_lengths[op] > machlen))
					control = 7;
				else if (op <= ASN1_OP__MATCHES_TAG) {
					unsigned char tmp;
					if ((op & ASN1_OP_MATCH__COND && flags & FLAG_MATCHED) || dp == datalen) {
						flags &= ~FLAG_LAST_MATCHED;
						pc += asn1_op_lengths[op];
						control = 0;
					} else {
						flags = 0;
						hdr = 2;
						if (unlikely(dp >= datalen - 1))
							control = 8;
						else {
							tag = data[dp++];
							if (unlikely((tag & 0x1f) == ASN1_LONG_TAG))
								control = 18;
							else if (op & ASN1_OP_MATCH__ANY) {
								pr_debug("- any %02x\n", tag);
								control = 4;
							} else {
								optag = machine[pc + 1];
								flags |= optag & FLAG_CONS;
								tmp = optag ^ tag;
								tmp &= ~(optag & ASN1_CONS_BIT);
								pr_debug("- match? %02x %02x %02x\n", tag, optag, tmp);

								if (tmp != 0) {
									if (op & ASN1_OP_MATCH__SKIP) {
										pc += asn1_op_lengths[op];
										dp--;
										control = 0;
									} else {
										control = 17;
									}
								} else {
									flags |= FLAG_MATCHED;
									control = 4;
								}
							}
						}
					}
				} else {
					control = 1;
				}
			}
			break;
		case 1:
			switch (op) {
			case ASN1_OP_MATCH_ANY_ACT:
			case ASN1_OP_COND_MATCH_ANY_ACT:
				ret = actions[machine[pc + 1]](context, hdr, tag, data + dp, len);
				if (ret < 0)
					return ret;
				control = 2;
				break;
			case ASN1_OP_MATCH_ACT:
			case ASN1_OP_MATCH_ACT_OR_SKIP:
			case ASN1_OP_COND_MATCH_ACT_OR_SKIP:
				ret = actions[machine[pc + 2]](context, hdr, tag, data + dp, len);
				if (ret < 0)
					return ret;
				control = 2;
				break;
			case ASN1_OP_MATCH:
			case ASN1_OP_MATCH_OR_SKIP:
			case ASN1_OP_MATCH_ANY:
			case ASN1_OP_COND_MATCH_OR_SKIP:
			case ASN1_OP_COND_MATCH_ANY:
				control = 2;
				break;
			case ASN1_OP_MATCH_JUMP:
			case ASN1_OP_MATCH_JUMP_OR_SKIP:
			case ASN1_OP_COND_MATCH_JUMP_OR_SKIP:
				pr_debug("- MATCH_JUMP\n");
				if (unlikely(jsp == NR_JUMP_STACK))
					control = 10;
				else {
					jump_stack[jsp++] = pc + asn1_op_lengths[op];
					pc = machine[pc + 2];
					control = 0;
				}
				break;
			case ASN1_OP_COND_FAIL:
				if (unlikely(!(flags & FLAG_MATCHED)))
					control = 17;
				else {
					pc += asn1_op_lengths[op];
					control = 0;
				}
				break;
			case ASN1_OP_COMPLETE:
				if (unlikely(jsp != 0 || csp != 0)) {
					pr_err("ASN.1 decoder error: Stacks not empty at completion (%u, %u)\n", jsp, csp);
					return -EBADMSG;
				}
				return 0;
			case ASN1_OP_END_SET:
			case ASN1_OP_END_SET_ACT:
				if (unlikely(!(flags & FLAG_MATCHED)))
					control = 17;
			case ASN1_OP_END_SEQ:
			case ASN1_OP_END_SET_OF:
			case ASN1_OP_END_SEQ_OF:
			case ASN1_OP_END_SEQ_ACT:
			case ASN1_OP_END_SET_OF_ACT:
			case ASN1_OP_END_SEQ_OF_ACT:
				if (unlikely(csp <= 0))
					control = 11;
				else {
					csp--;
					tdp = cons_dp_stack[csp];
					hdr = cons_hdrlen_stack[csp];
					len = datalen;
					datalen = cons_datalen_stack[csp];
					pr_debug("- end cons t=%zu dp=%zu l=%zu/%zu\n", tdp, dp, len, datalen);
					if (datalen == 0) {
						datalen = len;
						if (unlikely(datalen - dp < 2))
							control = 8;
						else if (data[dp++] != 0) {
							if (op & ASN1_OP_END__OF) {
								dp--;
								csp++;
								pc = machine[pc + 1];
								pr_debug("- continue\n");
								control = 0;
							} else {
								control = 13;
							}
						} else if (data[dp++] != 0) {
							control = 12;
						} else {
							len = dp - tdp - 2;
							control = 5;
						}
					} else {
						if (dp < len && (op & ASN1_OP_END__OF)) {
							datalen = len;
							csp++;
							pc = machine[pc + 1];
							pr_debug("- continue\n");
							control = 0;
						} else if (dp != len) {
							control = 14;
						} else {
							len -= tdp;
							pr_debug("- cons len l=%zu d=%zu\n", len, dp - tdp);
							control = 5;
						}
					}
				}
				break;
			case ASN1_OP_MAYBE_ACT:
				if (!(flags & FLAG_LAST_MATCHED)) {
					pc += asn1_op_lengths[op];
					control = 0;
				} else {
					control = 6;
				}
				break;
			case ASN1_OP_ACT:
				ret = actions[machine[pc + 1]](context, hdr, tag, data + tdp, len);
				if (ret < 0)
					return ret;
				pc += asn1_op_lengths[op];
				control = 0;
				break;
			case ASN1_OP_RETURN:
				if (unlikely(jsp <= 0))
					control = 9;
				else {
					pc = jump_stack[--jsp];
					flags |= FLAG_MATCHED | FLAG_LAST_MATCHED;
					control = 0;
				}
				break;
			default:
				control = 16;
				break;
			}
			break;
		case 2:
			if (!(flags & FLAG_CONS)) {
				if (flags & FLAG_INDEFINITE_LENGTH) {
					ret = asn1_find_indefinite_length(data, datalen, &dp, &len, &errmsg);
					if (ret < 0)
						control = 15;
				} else {
					dp += len;
				}
				pr_debug("- LEAF: %zu\n", len);
			}
			pc += asn1_op_lengths[op];
			control = 0;
			break;
		case 3:
			len = data[dp++];
			if (len > 0x7f) {
				if (unlikely(len == ASN1_INDEFINITE_LENGTH)) {
					if (unlikely(!(tag & ASN1_CONS_BIT)))
						control = 19;
					else {
						flags |= FLAG_INDEFINITE_LENGTH;
						if (unlikely(2 > datalen - dp))
							control = 8;
						else
							control = 1;
					}
				} else {
					int n = len - 0x80;
					if (unlikely(n > 2))
						control = 8;
					else if (unlikely(dp >= datalen - n))
						control = 8;
					else {
						hdr += n;
						for (len = 0; n > 0; n--) {
							len <<= 8;
							len |= data[dp++];
						}
						if (unlikely(len > datalen - dp))
							control = 8;
						else
							control = 1;
					}
				}
			} else {
				control = 1;
			}
			break;
		case 4:
			flags |= FLAG_MATCHED;
			control = 3;
			break;
		case 5:
			if (op & ASN1_OP_END__ACT) {
				unsigned char act;
				if (op & ASN1_OP_END__OF)
					act = machine[pc + 2];
				else
					act = machine[pc + 1];
				ret = actions[act](context, hdr, 0, data + tdp, len);
			}
			pc += asn1_op_lengths[op];
			control = 0;
			break;
		case 6:
			ret = actions[machine[pc + 1]](context, hdr, tag, data + tdp, len);
			if (ret < 0)
				return ret;
			pc += asn1_op_lengths[op];
			control = 0;
			break;
		case 7:
			errmsg = "Machine overrun error";
			control = 15;
			break;
		case 8:
			errmsg = "Data overrun error";
			control = 15;
			break;
		case 9:
			errmsg = "Jump stack underflow";
			control = 15;
			break;
		case 10:
			errmsg = "Jump stack overflow";
			control = 15;
			break;
		case 11:
			errmsg = "Cons stack underflow";
			control = 15;
			break;
		case 12:
			errmsg = "Invalid length EOC";
			control = 15;
			break;
		case 13:
			errmsg = "Missing EOC in indefinite len cons";
			control = 15;
			break;
		case 14:
			errmsg = "Cons length error";
			control = 15;
			break;
		case 15:
			pr_debug("\nASN1: %s [m=%zu d=%zu ot=%02x t=%02x l=%zu]\n", errmsg, pc, dp, optag, tag, len);
			return -EBADMSG;
		case 16:
			pr_err("ASN.1 decoder error: Found reserved opcode (%u) pc=%zu\n", op, pc);
			return -EBADMSG;
		case 17:
			errmsg = "Unexpected tag";
			control = 15;
			break;
		case 18:
			errmsg = "Long tag not supported";
			control = 15;
			break;
		case 19:
			errmsg = "Indefinite len primitive not permitted";
			control = 15;
			break;
		}
	}
}
EXPORT_SYMBOL_GPL(asn1_ber_decoder);