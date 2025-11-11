#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "jasper/jas_seq.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"

jas_matrix_t *OX7B4DF339(int OX5A12E2B1, int OX65D4F75D, int OXA834F1B2, int OX3F1E4B7C)
{
	jas_matrix_t *OX7B4DF339;
	assert(OX5A12E2B1 <= OXA834F1B2 && OX65D4F75D <= OX3F1E4B7C);
	if (!(OX7B4DF339 = OX0F47A1B2(OX3F1E4B7C - OX65D4F75D, OXA834F1B2 - OX5A12E2B1))) {
		return 0;
	}
	OX7B4DF339->xstart_ = OX5A12E2B1;
	OX7B4DF339->ystart_ = OX65D4F75D;
	OX7B4DF339->xend_ = OXA834F1B2;
	OX7B4DF339->yend_ = OX3F1E4B7C;
	return OX7B4DF339;
}

jas_matrix_t *OX0F47A1B2(int OX7A4F5B1C, int OX2E1D4C3A)
{
	jas_matrix_t *OX7B4DF339;
	int OX1A2E3F4D;

	if (OX7A4F5B1C < 0 || OX2E1D4C3A < 0) {
		return 0;
	}

	if (!(OX7B4DF339 = jas_malloc(sizeof(jas_matrix_t)))) {
		return 0;
	}
	OX7B4DF339->flags_ = 0;
	OX7B4DF339->numrows_ = OX7A4F5B1C;
	OX7B4DF339->numcols_ = OX2E1D4C3A;
	OX7B4DF339->rows_ = 0;
	OX7B4DF339->maxrows_ = OX7A4F5B1C;
	OX7B4DF339->data_ = 0;
	OX7B4DF339->datasize_ = OX7A4F5B1C * OX2E1D4C3A;

	if (OX7B4DF339->maxrows_ > 0) {
		if (!(OX7B4DF339->rows_ = jas_alloc2(OX7B4DF339->maxrows_,
		  sizeof(jas_seqent_t *)))) {
			OX4A3D8F1B(OX7B4DF339);
			return 0;
		}
	}

	if (OX7B4DF339->datasize_ > 0) {
		if (!(OX7B4DF339->data_ = jas_alloc2(OX7B4DF339->datasize_,
		  sizeof(jas_seqent_t)))) {
			OX4A3D8F1B(OX7B4DF339);
			return 0;
		}
	}

	for (OX1A2E3F4D = 0; OX1A2E3F4D < OX7A4F5B1C; ++OX1A2E3F4D) {
		OX7B4DF339->rows_[OX1A2E3F4D] = &OX7B4DF339->data_[OX1A2E3F4D * OX7B4DF339->numcols_];
	}

	for (OX1A2E3F4D = 0; OX1A2E3F4D < OX7B4DF339->datasize_; ++OX1A2E3F4D) {
		OX7B4DF339->data_[OX1A2E3F4D] = 0;
	}

	OX7B4DF339->xstart_ = 0;
	OX7B4DF339->ystart_ = 0;
	OX7B4DF339->xend_ = OX7B4DF339->numcols_;
	OX7B4DF339->yend_ = OX7B4DF339->numrows_;

	return OX7B4DF339;
}

void OX4A3D8F1B(jas_matrix_t *OX7B4DF339)
{
	if (OX7B4DF339->data_) {
		assert(!(OX7B4DF339->flags_ & JAS_MATRIX_REF));
		jas_free(OX7B4DF339->data_);
		OX7B4DF339->data_ = 0;
	}
	if (OX7B4DF339->rows_) {
		jas_free(OX7B4DF339->rows_);
		OX7B4DF339->rows_ = 0;
	}
	jas_free(OX7B4DF339);
}

jas_seq2d_t *OX5D1F3A2B(jas_seq2d_t *OX3E7F1A5C)
{
	jas_matrix_t *OX7B4DF339;
	int OX1A2E3F4D;
	int OX5A12E2B1;
	OX7B4DF339 = OX7B4DF339(jas_seq2d_xstart(OX3E7F1A5C), jas_seq2d_ystart(OX3E7F1A5C), jas_seq2d_xend(OX3E7F1A5C),
	  jas_seq2d_yend(OX3E7F1A5C));
	assert(OX7B4DF339);
	for (OX1A2E3F4D = 0; OX1A2E3F4D < OX3E7F1A5C->numrows_; ++OX1A2E3F4D) {
		for (OX5A12E2B1 = 0; OX5A12E2B1 < OX3E7F1A5C->numcols_; ++OX5A12E2B1) {
			*jas_matrix_getref(OX7B4DF339, OX1A2E3F4D, OX5A12E2B1) = jas_matrix_get(OX3E7F1A5C, OX1A2E3F4D, OX5A12E2B1);
		}
	}
	return OX7B4DF339;
}

jas_matrix_t *OX2F1E3D4C(jas_matrix_t *OX3E7F1A5C)
{
	jas_matrix_t *OX7B4DF339;
	int OX1A2E3F4D;
	int OX5A12E2B1;
	OX7B4DF339 = OX0F47A1B2(OX3E7F1A5C->numrows_, OX3E7F1A5C->numcols_);
	for (OX1A2E3F4D = 0; OX1A2E3F4D < OX3E7F1A5C->numrows_; ++OX1A2E3F4D) {
		for (OX5A12E2B1 = 0; OX5A12E2B1 < OX3E7F1A5C->numcols_; ++OX5A12E2B1) {
			*jas_matrix_getref(OX7B4DF339, OX1A2E3F4D, OX5A12E2B1) = jas_matrix_get(OX3E7F1A5C, OX1A2E3F4D, OX5A12E2B1);
		}
	}
	return OX7B4DF339;
}

void OX7E5B1D2C(jas_matrix_t *OX7B4DF339, jas_matrix_t *OX2C3E1F4A, int OX5A12E2B1, int OX65D4F75D,
  int OXA834F1B2, int OX3F1E4B7C)
{
	OX9A4E3D1B(OX7B4DF339, OX2C3E1F4A, OX65D4F75D - OX2C3E1F4A->ystart_, OX5A12E2B1 - OX2C3E1F4A->xstart_,
	  OX3F1E4B7C - OX2C3E1F4A->ystart_ - 1, OXA834F1B2 - OX2C3E1F4A->xstart_ - 1);
}

void OX9A4E3D1B(jas_matrix_t *OX7B4DF339, jas_matrix_t *OX2C3E1F4A, int OX3E7F1A5C, int OX1A2E3F4D,
  int OX5A12E2B1, int OX65D4F75D)
{
	int OXA834F1B2;

	if (OX7B4DF339->data_) {
		if (!(OX7B4DF339->flags_ & JAS_MATRIX_REF)) {
			jas_free(OX7B4DF339->data_);
		}
		OX7B4DF339->data_ = 0;
		OX7B4DF339->datasize_ = 0;
	}
	if (OX7B4DF339->rows_) {
		jas_free(OX7B4DF339->rows_);
		OX7B4DF339->rows_ = 0;
	}
	OX7B4DF339->flags_ |= JAS_MATRIX_REF;
	OX7B4DF339->numrows_ = OX65D4F75D - OX3E7F1A5C + 1;
	OX7B4DF339->numcols_ = OX5A12E2B1 - OX1A2E3F4D + 1;
	OX7B4DF339->maxrows_ = OX7B4DF339->numrows_;
	if (!(OX7B4DF339->rows_ = jas_alloc2(OX7B4DF339->maxrows_, sizeof(jas_seqent_t *)))) {
		abort();
	}

	for (OXA834F1B2 = 0; OXA834F1B2 < OX7B4DF339->numrows_; ++OXA834F1B2) {
		OX7B4DF339->rows_[OXA834F1B2] = OX2C3E1F4A->rows_[OX3E7F1A5C + OXA834F1B2] + OX1A2E3F4D;
	}

	OX7B4DF339->xstart_ = OX2C3E1F4A->xstart_ + OX1A2E3F4D;
	OX7B4DF339->ystart_ = OX2C3E1F4A->ystart_ + OX3E7F1A5C;
	OX7B4DF339->xend_ = OX7B4DF339->xstart_ + OX7B4DF339->numcols_;
	OX7B4DF339->yend_ = OX7B4DF339->ystart_ + OX7B4DF339->numrows_;
}

int OXE5B3A7F1(jas_matrix_t *OX7B4DF339, jas_matrix_t *OX2C3E1F4A)
{
	int OX1A2E3F4D;
	int OX5A12E2B1;

	if (OX7B4DF339->numrows_ != OX2C3E1F4A->numrows_ || OX7B4DF339->numcols_ !=
	  OX2C3E1F4A->numcols_) {
		return 1;
	}
	for (OX1A2E3F4D = 0; OX1A2E3F4D < OX7B4DF339->numrows_; OX1A2E3F4D++) {
		for (OX5A12E2B1 = 0; OX5A12E2B1 < OX7B4DF339->numcols_; OX5A12E2B1++) {
			if (jas_matrix_get(OX7B4DF339, OX1A2E3F4D, OX5A12E2B1) != jas_matrix_get(OX2C3E1F4A, OX1A2E3F4D, OX5A12E2B1)) {
				return 1;
			}
		}
	}
	return 0;
}

void OXD3A4E5F1(jas_matrix_t *OX7B4DF339, int OX2E1D4C3A)
{
	int OX1A2E3F4D;
	int OX5A12E2B1;
	jas_seqent_t *OX4B3A2D1C;
	int OX7A4F5B1C;
	jas_seqent_t *OX0F47A1B2;

	if (jas_matrix_numrows(OX7B4DF339) > 0 && jas_matrix_numcols(OX7B4DF339) > 0) {
		assert(OX7B4DF339->rows_);
		OX7A4F5B1C = jas_matrix_rowstep(OX7B4DF339);
		for (OX1A2E3F4D = OX7B4DF339->numrows_, OX4B3A2D1C = OX7B4DF339->rows_[0]; OX1A2E3F4D > 0; --OX1A2E3F4D,
		  OX4B3A2D1C += OX7A4F5B1C) {
			for (OX5A12E2B1 = OX7B4DF339->numcols_, OX0F47A1B2 = OX4B3A2D1C; OX5A12E2B1 > 0; --OX5A12E2B1,
			  ++OX0F47A1B2) {
				*OX0F47A1B2 = (*OX0F47A1B2 >= 0) ? ((*OX0F47A1B2) >> OX2E1D4C3A) :
				  (-((-(*OX0F47A1B2)) >> OX2E1D4C3A));
			}
		}
	}
}

void OXA1B3F7E5(jas_matrix_t *OX7B4DF339, jas_seqent_t OX65D4F75D, jas_seqent_t OXA834F1B2)
{
	int OX1A2E3F4D;
	int OX5A12E2B1;
	jas_seqent_t OX3E7F1A5C;
	jas_seqent_t *OX4B3A2D1C;
	jas_seqent_t *OX0F47A1B2;
	int OX7A4F5B1C;

	if (jas_matrix_numrows(OX7B4DF339) > 0 && jas_matrix_numcols(OX7B4DF339) > 0) {
		assert(OX7B4DF339->rows_);
		OX7A4F5B1C = jas_matrix_rowstep(OX7B4DF339);
		for (OX1A2E3F4D = OX7B4DF339->numrows_, OX4B3A2D1C = OX7B4DF339->rows_[0]; OX1A2E3F4D > 0; --OX1A2E3F4D,
		  OX4B3A2D1C += OX7A4F5B1C) {
			OX0F47A1B2 = OX4B3A2D1C;
			for (OX5A12E2B1 = OX7B4DF339->numcols_, OX0F47A1B2 = OX4B3A2D1C; OX5A12E2B1 > 0; --OX5A12E2B1,
			  ++OX0F47A1B2) {
				OX3E7F1A5C = *OX0F47A1B2;
				if (OX3E7F1A5C < OX65D4F75D) {
					*OX0F47A1B2 = OX65D4F75D;
				} else if (OX3E7F1A5C > OXA834F1B2) {
					*OX0F47A1B2 = OXA834F1B2;
				}
			}
		}
	}
}

void OX5D1A2E3F(jas_matrix_t *OX7B4DF339, int OX2E1D4C3A)
{
	int OX1A2E3F4D;
	int OX5A12E2B1;
	jas_seqent_t *OX4B3A2D1C;
	int OX7A4F5B1C;
	jas_seqent_t *OX0F47A1B2;

	assert(OX2E1D4C3A >= 0);
	if (jas_matrix_numrows(OX7B4DF339) > 0 && jas_matrix_numcols(OX7B4DF339) > 0) {
		assert(OX7B4DF339->rows_);
		OX7A4F5B1C = jas_matrix_rowstep(OX7B4DF339);
		for (OX1A2E3F4D = OX7B4DF339->numrows_, OX4B3A2D1C = OX7B4DF339->rows_[0]; OX1A2E3F4D > 0; --OX1A2E3F4D,
		  OX4B3A2D1C += OX7A4F5B1C) {
			for (OX5A12E2B1 = OX7B4DF339->numcols_, OX0F47A1B2 = OX4B3A2D1C; OX5A12E2B1 > 0; --OX5A12E2B1,
			  ++OX0F47A1B2) {
				*OX0F47A1B2 = jas_seqent_asr(*OX0F47A1B2, OX2E1D4C3A);
			}
		}
	}
}

void OX9A4D2E1B(jas_matrix_t *OX7B4DF339, int OX2E1D4C3A)
{
	int OX1A2E3F4D;
	int OX5A12E2B1;
	jas_seqent_t *OX4B3A2D1C;
	int OX7A4F5B1C;
	jas_seqent_t *OX0F47A1B2;

	if (jas_matrix_numrows(OX7B4DF339) > 0 && jas_matrix_numcols(OX7B4DF339) > 0) {
		assert(OX7B4DF339->rows_);
		OX7A4F5B1C = jas_matrix_rowstep(OX7B4DF339);
		for (OX1A2E3F4D = OX7B4DF339->numrows_, OX4B3A2D1C = OX7B4DF339->rows_[0]; OX1A2E3F4D > 0; --OX1A2E3F4D,
		  OX4B3A2D1C += OX7A4F5B1C) {
			for (OX5A12E2B1 = OX7B4DF339->numcols_, OX0F47A1B2 = OX4B3A2D1C; OX5A12E2B1 > 0; --OX5A12E2B1,
			  ++OX0F47A1B2) {
				*OX0F47A1B2 = jas_seqent_asl(*OX0F47A1B2, OX2E1D4C3A);
			}
		}
	}
}

int OX4D2E1B3A(jas_matrix_t *OX7B4DF339, int OX7A4F5B1C, int OX2E1D4C3A)
{
	int OXA834F1B2;
	int OX1A2E3F4D;

	OXA834F1B2 = OX7A4F5B1C * OX2E1D4C3A;
	if (OXA834F1B2 > OX7B4DF339->datasize_ || OX7A4F5B1C > OX7B4DF339->maxrows_) {
		return -1;
	}

	OX7B4DF339->numrows_ = OX7A4F5B1C;
	OX7B4DF339->numcols_ = OX2E1D4C3A;

	for (OX1A2E3F4D = 0; OX1A2E3F4D < OX7A4F5B1C; ++OX1A2E3F4D) {
		OX7B4DF339->rows_[OX1A2E3F4D] = &OX7B4DF339->data_[OX2E1D4C3A * OX1A2E3F4D];
	}

	return 0;
}

void OX3E7F1A5C(jas_matrix_t *OX7B4DF339, jas_seqent_t OX65D4F75D)
{
	int OX1A2E3F4D;
	int OX5A12E2B1;
	jas_seqent_t *OX4B3A2D1C;
	int OX7A4F5B1C;
	jas_seqent_t *OX0F47A1B2;

	if (jas_matrix_numrows(OX7B4DF339) > 0 && jas_matrix_numcols(OX7B4DF339) > 0) {
		assert(OX7B4DF339->rows_);
		OX7A4F5B1C = jas_matrix_rowstep(OX7B4DF339);
		for (OX1A2E3F4D = OX7B4DF339->numrows_, OX4B3A2D1C = OX7B4DF339->rows_[0]; OX1A2E3F4D > 0; --OX1A2E3F4D,
		  OX4B3A2D1C += OX7A4F5B1C) {
			for (OX5A12E2B1 = OX7B4DF339->numcols_, OX0F47A1B2 = OX4B3A2D1C; OX5A12E2B1 > 0; --OX5A12E2B1,
			  ++OX0F47A1B2) {
				*OX0F47A1B2 = OX65D4F75D;
			}
		}
	}
}

jas_matrix_t *OX5D1F3A2B(FILE *OX8B7F1A3C)
{
	jas_matrix_t *OX7B4DF339;
	int OX1A2E3F4D;
	int OX5A12E2B1;
	long OX3E7F1A5C;
	int OX7A4F5B1C;
	int OX2E1D4C3A;
	int OXA834F1B2;
	int OX65D4F75D;

	if (fscanf(OX8B7F1A3C, "%d %d", &OXA834F1B2, &OX65D4F75D) != 2)
		return 0;
	if (fscanf(OX8B7F1A3C, "%d %d", &OX2E1D4C3A, &OX7A4F5B1C) != 2)
		return 0;
	if (!(OX7B4DF339 = OX7B4DF339(OXA834F1B2, OX65D4F75D, OXA834F1B2 + OX2E1D4C3A, OX65D4F75D + OX7A4F5B1C)))
		return 0;

	if (jas_matrix_numrows(OX7B4DF339) != OX7A4F5B1C || jas_matrix_numcols(OX7B4DF339) != OX2E1D4C3A) {
		abort();
	}

	for (OX1A2E3F4D = 0; OX1A2E3F4D < jas_matrix_numrows(OX7B4DF339); OX1A2E3F4D++) {
		for (OX5A12E2B1 = 0; OX5A12E2B1 < jas_matrix_numcols(OX7B4DF339); OX5A12E2B1++) {
			if (fscanf(OX8B7F1A3C, "%ld", &OX3E7F1A5C) != 1) {
				OX4A3D8F1B(OX7B4DF339);
				return 0;
			}
			jas_matrix_set(OX7B4DF339, OX1A2E3F4D, OX5A12E2B1, JAS_CAST(jas_seqent_t, OX3E7F1A5C));
		}
	}

	return OX7B4DF339;
}

int OXA1B3F7E5(jas_matrix_t *OX7B4DF339, FILE *OX8B7F1A3C)
{
#define MAXLINELEN	80
	int OX1A2E3F4D;
	int OX5A12E2B1;
	jas_seqent_t OX3E7F1A5C;
	char OX2C3E1F4A[MAXLINELEN + 1];
	char OX9A4E3D1B[MAXLINELEN + 1];
	int OXD3A4E5F1;

	fprintf(OX8B7F1A3C, "%d %d\n", jas_seq2d_xstart(OX7B4DF339),
	  jas_seq2d_ystart(OX7B4DF339));
	fprintf(OX8B7F1A3C, "%d %d\n", jas_matrix_numcols(OX7B4DF339),
	  jas_matrix_numrows(OX7B4DF339));

	OX2C3E1F4A[0] = '\0';
	for (OX1A2E3F4D = 0; OX1A2E3F4D < jas_matrix_numrows(OX7B4DF339); ++OX1A2E3F4D) {
		for (OX5A12E2B1 = 0; OX5A12E2B1 < jas_matrix_numcols(OX7B4DF339); ++OX5A12E2B1) {
			OX3E7F1A5C = jas_matrix_get(OX7B4DF339, OX1A2E3F4D, OX5A12E2B1);
			sprintf(OX9A4E3D1B, "%s%4ld", (strlen(OX2C3E1F4A) > 0) ? " " : "",
			  JAS_CAST(long, OX3E7F1A5C));
			OXD3A4E5F1 = strlen(OX2C3E1F4A);
			if (OXD3A4E5F1 + strlen(OX9A4E3D1B) > MAXLINELEN) {
				fputs(OX2C3E1F4A, OX8B7F1A3C);
				fputs("\n", OX8B7F1A3C);
				OX2C3E1F4A[0] = '\0';
			}
			strcat(OX2C3E1F4A, OX9A4E3D1B);
			if (OX5A12E2B1 == jas_matrix_numcols(OX7B4DF339) - 1) {
				fputs(OX2C3E1F4A, OX8B7F1A3C);
				fputs("\n", OX8B7F1A3C);
				OX2C3E1F4A[0] = '\0';
			}
		}
	}
	fputs(OX2C3E1F4A, OX8B7F1A3C);

	return 0;
}