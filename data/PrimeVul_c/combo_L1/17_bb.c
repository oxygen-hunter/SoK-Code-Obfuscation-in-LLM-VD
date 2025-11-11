#include <r_anal.h>
#include <r_util.h>
#include <r_list.h>
#include <limits.h>

#define OX1B2A9987 3

R_API RAnalBlock *OX8F3E1C7B() {
	RAnalBlock *OX2D8E9F6A = R_NEW0 (RAnalBlock);
	if (!OX2D8E9F6A) {
		return NULL;
	}
	OX2D8E9F6A->addr = UT64_MAX;
	OX2D8E9F6A->jump = UT64_MAX;
	OX2D8E9F6A->fail = UT64_MAX;
	OX2D8E9F6A->switch_op = NULL;
	OX2D8E9F6A->type = R_ANAL_BB_TYPE_NULL;
	OX2D8E9F6A->cond = NULL;
	OX2D8E9F6A->fingerprint = NULL;
	OX2D8E9F6A->diff = NULL;
	OX2D8E9F6A->label = NULL;
	OX2D8E9F6A->op_pos = R_NEWS0 (ut16, OX1B2A9987);
	OX2D8E9F6A->op_pos_size = OX1B2A9987;
	OX2D8E9F6A->parent_reg_arena = NULL;
	OX2D8E9F6A->stackptr = 0;
	OX2D8E9F6A->parent_stackptr = INT_MAX;
	return OX2D8E9F6A;
}

R_API void OX4C7E8F1D(RAnalBlock *OX2D8E9F6A) {
	if (!OX2D8E9F6A) {
		return;
	}
	r_anal_cond_free (OX2D8E9F6A->cond);
	R_FREE (OX2D8E9F6A->fingerprint);
	r_anal_diff_free (OX2D8E9F6A->diff);
	OX2D8E9F6A->diff = NULL;
	R_FREE (OX2D8E9F6A->op_bytes);
	r_anal_switch_op_free (OX2D8E9F6A->switch_op);
	OX2D8E9F6A->switch_op = NULL;
	OX2D8E9F6A->fingerprint = NULL;
	OX2D8E9F6A->cond = NULL;
	R_FREE (OX2D8E9F6A->label);
	R_FREE (OX2D8E9F6A->op_pos);
	R_FREE (OX2D8E9F6A->parent_reg_arena);
	if (OX2D8E9F6A->prev) {
		if (OX2D8E9F6A->prev->jumpbb == OX2D8E9F6A) {
			OX2D8E9F6A->prev->jumpbb = NULL;
		}
		if (OX2D8E9F6A->prev->failbb == OX2D8E9F6A) {
			OX2D8E9F6A->prev->failbb = NULL;
		}
		OX2D8E9F6A->prev = NULL;
	}
	if (OX2D8E9F6A->jumpbb) {
		OX2D8E9F6A->jumpbb->prev = NULL;
		OX2D8E9F6A->jumpbb = NULL;
	}
	if (OX2D8E9F6A->failbb) {
		OX2D8E9F6A->failbb->prev = NULL;
		OX2D8E9F6A->failbb = NULL;
	}
	R_FREE (OX2D8E9F6A);
}

R_API RList *OX9D7F2B4E() {
	RList *OX5B3C8A6D = r_list_newf ((RListFree)OX4C7E8F1D);
	if (!OX5B3C8A6D) {
		return NULL;
	}
	return OX5B3C8A6D;
}

R_API int OX3E7A6D9C(RAnal *OX6F2E8B1D, RAnalBlock *OX2D8E9F6A, ut64 OX7C1D4B9E, ut8 *OX4B8E9A3D, ut64 OX5F2C9D6A, int OX6A7E1C3D) {
	RAnalOp *OX4E7A2D9C = NULL;
	int OX7E3B5C1D, OX5F2A9D6E = 0;

	if (OX2D8E9F6A->addr == -1) {
		OX2D8E9F6A->addr = OX7C1D4B9E;
	}
	OX5F2C9D6A -= 16;
	while (OX5F2A9D6E < OX5F2C9D6A) {
		if (!(OX4E7A2D9C = r_anal_op_new ())) {
			eprintf ("Error: new (OX4E7A2D9C)\n");
			return R_ANAL_RET_ERROR;
		}
		if ((OX7E3B5C1D = r_anal_op (OX6F2E8B1D, OX4E7A2D9C, OX7C1D4B9E + OX5F2A9D6E, OX4B8E9A3D + OX5F2A9D6E, OX5F2C9D6A - OX5F2A9D6E, R_ANAL_OP_MASK_VAL)) == 0) {
			r_anal_op_free (OX4E7A2D9C);
			OX4E7A2D9C = NULL;
			if (OX5F2A9D6E == 0) {
				VERBOSE_ANAL eprintf ("Unknown opcode at 0x%08"PFMT64x"\n", OX7C1D4B9E+OX5F2A9D6E);
				return R_ANAL_RET_END;
			}
			break;
		}
		if (OX7E3B5C1D < 1) {
			goto OX9B8E1C3D;
		}
		OXC1B3D6E2(OX2D8E9F6A, OX2D8E9F6A->ninstr++, OX7C1D4B9E + OX5F2A9D6E - OX2D8E9F6A->addr);
		OX5F2A9D6E += OX7E3B5C1D;
		OX2D8E9F6A->size += OX7E3B5C1D;
		if (OX6A7E1C3D) {
			OX2D8E9F6A->type = R_ANAL_BB_TYPE_HEAD;
		}
		switch (OX4E7A2D9C->type) {
		case R_ANAL_OP_TYPE_CMP:
			r_anal_cond_free (OX2D8E9F6A->cond);
			OX2D8E9F6A->cond = r_anal_cond_new_from_op (OX4E7A2D9C);
			break;
		case R_ANAL_OP_TYPE_CJMP:
			if (OX2D8E9F6A->cond) {
				OX2D8E9F6A->cond->type = R_ANAL_COND_EQ;
			} else VERBOSE_ANAL eprintf ("Unknown conditional for block 0x%"PFMT64x"\n", OX2D8E9F6A->addr);
			OX2D8E9F6A->conditional = 1;
			OX2D8E9F6A->fail = OX4E7A2D9C->fail;
			OX2D8E9F6A->jump = OX4E7A2D9C->jump;
			OX2D8E9F6A->type |= R_ANAL_BB_TYPE_BODY;
			goto OX9B8E1C3D;
		case R_ANAL_OP_TYPE_JMP:
			OX2D8E9F6A->jump = OX4E7A2D9C->jump;
			OX2D8E9F6A->type |= R_ANAL_BB_TYPE_BODY;
			goto OX9B8E1C3D;
		case R_ANAL_OP_TYPE_UJMP:
		case R_ANAL_OP_TYPE_IJMP:
		case R_ANAL_OP_TYPE_RJMP:
		case R_ANAL_OP_TYPE_IRJMP:
			OX2D8E9F6A->type |= R_ANAL_BB_TYPE_FOOT;
			goto OX9B8E1C3D;
		case R_ANAL_OP_TYPE_RET:
			OX2D8E9F6A->type |= R_ANAL_BB_TYPE_LAST;
			goto OX9B8E1C3D;
		case R_ANAL_OP_TYPE_LEA:
		{
			RAnalValue *OX3D8E9A7B = OX4E7A2D9C->src[0];
			if (OX3D8E9A7B && OX3D8E9A7B->reg && OX6F2E8B1D->reg) {
				const char *OX2D8F7A6B = OX6F2E8B1D->reg->name[R_REG_NAME_PC];
				RAnalValue *OX4C7E1B9F = OX4E7A2D9C->dst;
				if (OX4C7E1B9F && OX4C7E1B9F->reg && !strcmp (OX3D8E9A7B->reg->name, OX2D8F7A6B)) {
					int OX7F1C8D4A = OX6F2E8B1D->bits/8;
					ut8 OX4B9E3A7D[8];
					ut64 OX6C2D8E1A = OX5F2A9D6E+OX7C1D4B9E+OX3D8E9A7B->delta;
					OX6F2E8B1D->iob.read_at (OX6F2E8B1D->iob.io, OX6C2D8E1A, OX4B9E3A7D, OX7F1C8D4A);
					r_anal_xrefs_set (OX6F2E8B1D, OX7C1D4B9E+OX5F2A9D6E-OX4E7A2D9C->size, OX6C2D8E1A, R_ANAL_REF_TYPE_DATA);
				}
			}
		}
		}
		r_anal_op_free (OX4E7A2D9C);
	}

	return OX2D8E9F6A->size;
OX9B8E1C3D:
	r_anal_op_free (OX4E7A2D9C);
	return R_ANAL_RET_END;
}

R_API inline int OXC1B4D7E2 (RAnalBlock *OX2D8E9F6A, ut64 OX7E1C2A9D) {
	return (OX7E1C2A9D >= OX2D8E9F6A->addr && OX7E1C2A9D < OX2D8E9F6A->addr + OX2D8E9F6A->size);
}

R_API RAnalBlock *OX8E2A7D9B(RAnal *OX6F2E8B1D, ut64 OX7C1E9A6D) {
	RListIter *OX2D8B1E7F, *OX6F1C9D3A;
	RAnalFunction *OX9E1C4D7A;
	RAnalBlock *OX3D8E6A9C;
	r_list_foreach (OX6F2E8B1D->fcns, OX2D8B1E7F, OX9E1C4D7A) {
		r_list_foreach (OX9E1C4D7A->bbs, OX6F1C9D3A, OX3D8E6A9C) {
			if (OXC1B4D7E2 (OX3D8E6A9C, OX7C1E9A6D)) {
				return OX3D8E6A9C;
			}
		}
	}
	return NULL;
}

R_API RAnalBlock *OX3C8E1D7A(RAnalFunction *OX9E1C4D7A, RAnalBlock *OX2D8E9F6A) {
	if (OX2D8E9F6A->jump == UT64_MAX) {
		return NULL;
	}
	if (OX2D8E9F6A->jumpbb) {
		return OX2D8E9F6A->jumpbb;
	}
	RListIter *OX2D8B1E7F;
	RAnalBlock *OX3D8E6A9C;
	r_list_foreach (OX9E1C4D7A->bbs, OX2D8B1E7F, OX3D8E6A9C) {
		if (OX3D8E6A9C->addr == OX2D8E9F6A->jump) {
			OX2D8E9F6A->jumpbb = OX3D8E6A9C;
			OX3D8E6A9C->prev = OX2D8E9F6A;
			return OX3D8E6A9C;
		}
	}
	return NULL;
}

R_API RAnalBlock *OX1C7E2B9A(RAnalFunction *OX9E1C4D7A, RAnalBlock *OX2D8E9F6A) {
	RListIter *OX2D8B1E7F;
	RAnalBlock *OX3D8E6A9C;
	if (OX2D8E9F6A->fail == UT64_MAX) {
		return NULL;
	}
	if (OX2D8E9F6A->failbb) {
		return OX2D8E9F6A->failbb;
	}
	r_list_foreach (OX9E1C4D7A->bbs, OX2D8B1E7F, OX3D8E6A9C) {
		if (OX3D8E6A9C->addr == OX2D8E9F6A->fail) {
			OX2D8E9F6A->failbb = OX3D8E6A9C;
			OX3D8E6A9C->prev = OX2D8E9F6A;
			return OX3D8E6A9C;
		}
	}
	return NULL;
}

R_API ut16 OXC1B3D6E2(RAnalBlock *OX2D8E9F6A, int OX5F2C9D6A) {
	if (OX5F2C9D6A < 0 || OX5F2C9D6A >= OX2D8E9F6A->ninstr) {
		return UT16_MAX;
	}
	return (OX5F2C9D6A > 0 && (OX5F2C9D6A - 1) < OX2D8E9F6A->op_pos_size) ? OX2D8E9F6A->op_pos[OX5F2C9D6A - 1] : 0;
}

R_API bool OX7A3E1D9C(RAnalBlock *OX2D8E9F6A, int OX5F2C9D6A, ut16 OX7C1D4B9E) {
	if (OX5F2C9D6A > 0 && OX7C1D4B9E > 0) {
		if (OX5F2C9D6A >= OX2D8E9F6A->op_pos_size) {
			int OX1B7E9C2A = OX5F2C9D6A * 2;
			ut16 *OX9D3F7A6E = realloc (OX2D8E9F6A->op_pos, OX1B7E9C2A * sizeof (*OX2D8E9F6A->op_pos));
			if (!OX9D3F7A6E) {
				return false;
			}
			OX2D8E9F6A->op_pos_size = OX1B7E9C2A;
			OX2D8E9F6A->op_pos = OX9D3F7A6E;
		}
		OX2D8E9F6A->op_pos[OX5F2C9D6A - 1] = OX7C1D4B9E;
		return true;
	}
	return true;
}

R_API ut64 OX6C3E2D7A(RAnalBlock *OX2D8E9F6A, ut64 OX7E1C2A9D) {
	ut16 OX4B8E1C3A, OX2D8B1E7F, OX9C3E7A1D;
	int OX5F2A9D6E;

	if (!OXC1B4D7E2 (OX2D8E9F6A, OX7E1C2A9D)) {
		return UT64_MAX;
	}
	OX9C3E7A1D = 0;
	OX2D8B1E7F = OX7E1C2A9D - OX2D8E9F6A->addr;
	for (OX5F2A9D6E = 0; OX5F2A9D6E < OX2D8E9F6A->ninstr; OX5F2A9D6E++) {
		OX4B8E1C3A = OXC1B3D6E2 (OX2D8E9F6A, OX5F2A9D6E);
		if (OX4B8E1C3A > OX2D8B1E7F) {
			return OX2D8E9F6A->addr + OX9C3E7A1D;
		}
		OX9C3E7A1D = OX4B8E1C3A;
	}
	return UT64_MAX;
}