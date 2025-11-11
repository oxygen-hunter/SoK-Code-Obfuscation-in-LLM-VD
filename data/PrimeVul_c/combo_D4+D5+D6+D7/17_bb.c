#include <r_anal.h>
#include <r_util.h>
#include <r_list.h>
#include <limits.h>

#define DFLT_NINSTR 3

R_API RAnalBlock *r_anal_bb_new() {
	RAnalBlock *q = R_NEW0 (RAnalBlock);
	if (!q) {
		return NULL;
	}
	q->jump = q->fail = q->addr = UT64_MAX;
	q->switch_op = NULL;
	q->type = R_ANAL_BB_TYPE_NULL;
	q->cond = NULL;
	q->fingerprint = NULL;
	q->diff = NULL;
	q->label = NULL;
	q->op_pos = R_NEWS0 (ut16, DFLT_NINSTR);
	q->op_pos_size = DFLT_NINSTR;
	q->parent_reg_arena = NULL;
	q->stackptr = 0;
	q->parent_stackptr = INT_MAX;
	return q;
}

R_API void r_anal_bb_free(RAnalBlock *q) {
	if (!q) {
		return;
	}
	r_anal_cond_free (q->cond);
	R_FREE (q->fingerprint);
	r_anal_diff_free (q->diff);
	q->diff = NULL;
	R_FREE (q->op_bytes);
	r_anal_switch_op_free (q->switch_op);
	q->switch_op = NULL;
	q->fingerprint = NULL;
	q->cond = NULL;
	R_FREE (q->label);
	R_FREE (q->op_pos);
	R_FREE (q->parent_reg_arena);
	if (q->prev) {
		if (q->prev->jumpbb == q) {
			q->prev->jumpbb = NULL;
		}
		if (q->prev->failbb == q) {
			q->prev->failbb = NULL;
		}
		q->prev = NULL;
	}
	if (q->jumpbb) {
		q->jumpbb->prev = NULL;
		q->jumpbb = NULL;
	}
	if (q->failbb) {
		q->failbb->prev = NULL;
		q->failbb = NULL;
	}
	R_FREE (q);
}

R_API RList *r_anal_bb_list_new() {
	RList *a = r_list_newf ((RListFree)r_anal_bb_free);
	if (!a) {
		return NULL;
	}
	return a;
}

R_API int r_anal_bb(RAnal *anal, RAnalBlock *q, ut64 addr, ut8 *buf, ut64 len, int head) {
	RAnalOp *op = NULL;
	int oplen, idx = 0;

	if (q->addr == -1) {
		q->addr = addr;
	}
	len -= 16;
	while (idx < len) {
		if (!(op = r_anal_op_new ())) {
			eprintf ("Error: new (op)\n");
			return R_ANAL_RET_ERROR;
		}
		if ((oplen = r_anal_op (anal, op, addr + idx, buf + idx, len - idx, R_ANAL_OP_MASK_VAL)) == 0) {
			r_anal_op_free (op);
			op = NULL;
			if (idx == 0) {
				VERBOSE_ANAL eprintf ("Unknown opcode at 0x%08"PFMT64x"\n", addr+idx);
				return R_ANAL_RET_END;
			}
			break;
		}
		if (oplen < 1) {
			goto beach;
		}
		r_anal_bb_set_offset (q, q->ninstr++, addr + idx - q->addr);
		idx += oplen;
		q->size += oplen;
		if (head) {
			q->type = R_ANAL_BB_TYPE_HEAD;
		}
		switch (op->type) {
		case R_ANAL_OP_TYPE_CMP:
			r_anal_cond_free (q->cond);
			q->cond = r_anal_cond_new_from_op (op);
			break;
		case R_ANAL_OP_TYPE_CJMP:
			if (q->cond) {
				q->cond->type = R_ANAL_COND_EQ;
			} else VERBOSE_ANAL eprintf ("Unknown conditional for block 0x%"PFMT64x"\n", q->addr);
			q->conditional = 1;
			q->fail = op->fail;
			q->jump = op->jump;
			q->type |= R_ANAL_BB_TYPE_BODY;
			goto beach;
		case R_ANAL_OP_TYPE_JMP:
			q->jump = op->jump;
			q->type |= R_ANAL_BB_TYPE_BODY;
			goto beach;
		case R_ANAL_OP_TYPE_UJMP:
		case R_ANAL_OP_TYPE_IJMP:
		case R_ANAL_OP_TYPE_RJMP:
		case R_ANAL_OP_TYPE_IRJMP:
			q->type |= R_ANAL_BB_TYPE_FOOT;
			goto beach;
		case R_ANAL_OP_TYPE_RET:
			q->type |= R_ANAL_BB_TYPE_LAST;
			goto beach;
		case R_ANAL_OP_TYPE_LEA:
		{
			RAnalValue *src = op->src[0];
			if (src && src->reg && anal->reg) {
				const char *pc = anal->reg->name[R_REG_NAME_PC];
				RAnalValue *dst = op->dst;
				if (dst && dst->reg && !strcmp (src->reg->name, pc)) {
					int memref = anal->bits/8;
					ut8 b[8];
					ut64 ptr = idx+addr+src->delta;
					anal->iob.read_at (anal->iob.io, ptr, b, memref);
					r_anal_xrefs_set (anal, addr+idx-op->size, ptr, R_ANAL_REF_TYPE_DATA);
				}
			}
		}
		}
		r_anal_op_free (op);
	}

	return q->size;
beach:
	r_anal_op_free (op);
	return R_ANAL_RET_END;
}

R_API inline int r_anal_bb_is_in_offset (RAnalBlock *q, ut64 off) {
	ut64 temp_off = off;
	return (temp_off >= q->addr && temp_off < q->addr + q->size);
}

R_API RAnalBlock *r_anal_bb_from_offset(RAnal *anal, ut64 off) {
	RListIter *iter, *iter2;
	RAnalFunction *fcn;
	RAnalBlock *q;
	r_list_foreach (anal->fcns, iter, fcn) {
		r_list_foreach (fcn->bbs, iter2, q) {
			if (r_anal_bb_is_in_offset (q, off)) {
				return q;
			}
		}
	}
	return NULL;
}

R_API RAnalBlock *r_anal_bb_get_jumpbb(RAnalFunction *fcn, RAnalBlock *q) {
	if (q->jump == UT64_MAX) {
		return NULL;
	}
	if (q->jumpbb) {
		return q->jumpbb;
	}
	RListIter *iter;
	RAnalBlock *b;
	r_list_foreach (fcn->bbs, iter, b) {
		if (b->addr == q->jump) {
			q->jumpbb = b;
			b->prev = q;
			return b;
		}
	}
	return NULL;
}

R_API RAnalBlock *r_anal_bb_get_failbb(RAnalFunction *fcn, RAnalBlock *q) {
	RListIter *iter;
	RAnalBlock *b;
	if (q->fail == UT64_MAX) {
		return NULL;
	}
	if (q->failbb) {
		return q->failbb;
	}
	r_list_foreach (fcn->bbs, iter, b) {
		if (b->addr == q->fail) {
			q->failbb = b;
			b->prev = q;
			return b;
		}
	}
	return NULL;
}

R_API ut16 r_anal_bb_offset_inst(RAnalBlock *q, int i) {
	if (i < 0 || i >= q->ninstr) {
		return UT16_MAX;
	}
	return (i > 0 && (i - 1) < q->op_pos_size) ? q->op_pos[i - 1] : 0;
}

R_API bool r_anal_bb_set_offset(RAnalBlock *q, int i, ut16 v) {
	if (i > 0 && v > 0) {
		if (i >= q->op_pos_size) {
			int new_pos_size = i * 2;
			ut16 *tmp_op_pos = realloc (q->op_pos, new_pos_size * sizeof (*q->op_pos));
			if (!tmp_op_pos) {
				return false;
			}
			q->op_pos_size = new_pos_size;
			q->op_pos = tmp_op_pos;
		}
		q->op_pos[i - 1] = v;
		return true;
	}
	return true;
}

R_API ut64 r_anal_bb_opaddr_at(RAnalBlock *q, ut64 off) {
	ut16 delta, delta_off, last_delta;
	int i;

	if (!r_anal_bb_is_in_offset (q, off)) {
		return UT64_MAX;
	}
	last_delta = 0;
	delta_off = off - q->addr;
	for (i = 0; i < q->ninstr; i++) {
		delta = r_anal_bb_offset_inst (q, i);
		if (delta > delta_off) {
			return q->addr + last_delta;
		}
		last_delta = delta;
	}
	return UT64_MAX;
}