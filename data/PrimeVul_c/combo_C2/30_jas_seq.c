#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "jasper/jas_seq.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"

jas_matrix_t *jas_seq2d_create(int xstart, int ystart, int xend, int yend)
{
	jas_matrix_t *matrix;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				assert(xstart <= xend && ystart <= yend);
				state = 1;
				break;
			case 1:
				if (!(matrix = jas_matrix_create(yend - ystart, xend - xstart))) {
					state = 2;
				} else {
					state = 3;
				}
				break;
			case 2:
				return 0;
			case 3:
				matrix->xstart_ = xstart;
				matrix->ystart_ = ystart;
				matrix->xend_ = xend;
				matrix->yend_ = yend;
				return matrix;
		}
	}
}

jas_matrix_t *jas_matrix_create(int numrows, int numcols)
{
	jas_matrix_t *matrix;
	int i;
	int state = 0;

	while(1) {
		switch (state) {
			case 0:
				if (numrows < 0 || numcols < 0) {
					state = 1;
				} else {
					state = 2;
				}
				break;
			case 1:
				return 0;
			case 2:
				if (!(matrix = jas_malloc(sizeof(jas_matrix_t)))) {
					state = 1;
				} else {
					state = 3;
				}
				break;
			case 3:
				matrix->flags_ = 0;
				matrix->numrows_ = numrows;
				matrix->numcols_ = numcols;
				matrix->rows_ = 0;
				matrix->maxrows_ = numrows;
				matrix->data_ = 0;
				matrix->datasize_ = numrows * numcols;
				state = 4;
				break;
			case 4:
				if (matrix->maxrows_ > 0) {
					if (!(matrix->rows_ = jas_alloc2(matrix->maxrows_, sizeof(jas_seqent_t *)))) {
						state = 5;
					} else {
						state = 6;
					}
				} else {
					state = 6;
				}
				break;
			case 5:
				jas_matrix_destroy(matrix);
				return 0;
			case 6:
				if (matrix->datasize_ > 0) {
					if (!(matrix->data_ = jas_alloc2(matrix->datasize_, sizeof(jas_seqent_t)))) {
						state = 5;
					} else {
						state = 7;
					}
				} else {
					state = 7;
				}
				break;
			case 7:
				for (i = 0; i < numrows; ++i) {
					matrix->rows_[i] = &matrix->data_[i * matrix->numcols_];
				}
				state = 8;
				break;
			case 8:
				for (i = 0; i < matrix->datasize_; ++i) {
					matrix->data_[i] = 0;
				}
				state = 9;
				break;
			case 9:
				matrix->xstart_ = 0;
				matrix->ystart_ = 0;
				matrix->xend_ = matrix->numcols_;
				matrix->yend_ = matrix->numrows_;
				return matrix;
		}
	}
}

void jas_matrix_destroy(jas_matrix_t *matrix)
{
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (matrix->data_) {
					assert(!(matrix->flags_ & JAS_MATRIX_REF));
					jas_free(matrix->data_);
					matrix->data_ = 0;
				}
				state = 1;
				break;
			case 1:
				if (matrix->rows_) {
					jas_free(matrix->rows_);
					matrix->rows_ = 0;
				}
				state = 2;
				break;
			case 2:
				jas_free(matrix);
				return;
		}
	}
}

jas_seq2d_t *jas_seq2d_copy(jas_seq2d_t *x)
{
	jas_matrix_t *y;
	int i;
	int j;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				y = jas_seq2d_create(jas_seq2d_xstart(x), jas_seq2d_ystart(x), jas_seq2d_xend(x), jas_seq2d_yend(x));
				assert(y);
				state = 1;
				break;
			case 1:
				for (i = 0; i < x->numrows_; ++i) {
					for (j = 0; j < x->numcols_; ++j) {
						*jas_matrix_getref(y, i, j) = jas_matrix_get(x, i, j);
					}
				}
				return y;
		}
	}
}

jas_matrix_t *jas_matrix_copy(jas_matrix_t *x)
{
	jas_matrix_t *y;
	int i;
	int j;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				y = jas_matrix_create(x->numrows_, x->numcols_);
				state = 1;
				break;
			case 1:
				for (i = 0; i < x->numrows_; ++i) {
					for (j = 0; j < x->numcols_; ++j) {
						*jas_matrix_getref(y, i, j) = jas_matrix_get(x, i, j);
					}
				}
				return y;
		}
	}
}

void jas_seq2d_bindsub(jas_matrix_t *s, jas_matrix_t *s1, int xstart, int ystart, int xend, int yend)
{
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				jas_matrix_bindsub(s, s1, ystart - s1->ystart_, xstart - s1->xstart_, yend - s1->ystart_ - 1, xend - s1->xstart_ - 1);
				return;
		}
	}
}

void jas_matrix_bindsub(jas_matrix_t *mat0, jas_matrix_t *mat1, int r0, int c0, int r1, int c1)
{
	int i;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (mat0->data_) {
					if (!(mat0->flags_ & JAS_MATRIX_REF)) {
						jas_free(mat0->data_);
					}
					mat0->data_ = 0;
					mat0->datasize_ = 0;
				}
				state = 1;
				break;
			case 1:
				if (mat0->rows_) {
					jas_free(mat0->rows_);
					mat0->rows_ = 0;
				}
				state = 2;
				break;
			case 2:
				mat0->flags_ |= JAS_MATRIX_REF;
				mat0->numrows_ = r1 - r0 + 1;
				mat0->numcols_ = c1 - c0 + 1;
				mat0->maxrows_ = mat0->numrows_;
				state = 3;
				break;
			case 3:
				if (!(mat0->rows_ = jas_alloc2(mat0->maxrows_, sizeof(jas_seqent_t *)))) {
					abort();
				}
				state = 4;
				break;
			case 4:
				for (i = 0; i < mat0->numrows_; ++i) {
					mat0->rows_[i] = mat1->rows_[r0 + i] + c0;
				}
				state = 5;
				break;
			case 5:
				mat0->xstart_ = mat1->xstart_ + c0;
				mat0->ystart_ = mat1->ystart_ + r0;
				mat0->xend_ = mat0->xstart_ + mat0->numcols_;
				mat0->yend_ = mat0->ystart_ + mat0->numrows_;
				return;
		}
	}
}

int jas_matrix_cmp(jas_matrix_t *mat0, jas_matrix_t *mat1)
{
	int i;
	int j;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (mat0->numrows_ != mat1->numrows_ || mat0->numcols_ != mat1->numcols_) {
					state = 1;
				} else {
					state = 2;
				}
				break;
			case 1:
				return 1;
			case 2:
				for (i = 0; i < mat0->numrows_; i++) {
					for (j = 0; j < mat0->numcols_; j++) {
						if (jas_matrix_get(mat0, i, j) != jas_matrix_get(mat1, i, j)) {
							return 1;
						}
					}
				}
				return 0;
		}
	}
}

void jas_matrix_divpow2(jas_matrix_t *matrix, int n)
{
	int i;
	int j;
	jas_seqent_t *rowstart;
	int rowstep;
	jas_seqent_t *data;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
					assert(matrix->rows_);
					rowstep = jas_matrix_rowstep(matrix);
					state = 1;
				} else {
					state = 2;
				}
				break;
			case 1:
				for (i = matrix->numrows_, rowstart = matrix->rows_[0]; i > 0; --i, rowstart += rowstep) {
					for (j = matrix->numcols_, data = rowstart; j > 0; --j, ++data) {
						*data = (*data >= 0) ? ((*data) >> n) : (-((-(*data)) >> n));
					}
				}
				// Fallthrough
			case 2:
				return;
		}
	}
}

void jas_matrix_clip(jas_matrix_t *matrix, jas_seqent_t minval, jas_seqent_t maxval)
{
	int i;
	int j;
	jas_seqent_t v;
	jas_seqent_t *rowstart;
	jas_seqent_t *data;
	int rowstep;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
					assert(matrix->rows_);
					rowstep = jas_matrix_rowstep(matrix);
					state = 1;
				} else {
					state = 2;
				}
				break;
			case 1:
				for (i = matrix->numrows_, rowstart = matrix->rows_[0]; i > 0; --i, rowstart += rowstep) {
					data = rowstart;
					for (j = matrix->numcols_, data = rowstart; j > 0; --j, ++data) {
						v = *data;
						if (v < minval) {
							*data = minval;
						} else if (v > maxval) {
							*data = maxval;
						}
					}
				}
				// Fallthrough
			case 2:
				return;
		}
	}
}

void jas_matrix_asr(jas_matrix_t *matrix, int n)
{
	int i;
	int j;
	jas_seqent_t *rowstart;
	int rowstep;
	jas_seqent_t *data;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				assert(n >= 0);
				if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
					assert(matrix->rows_);
					rowstep = jas_matrix_rowstep(matrix);
					state = 1;
				} else {
					state = 2;
				}
				break;
			case 1:
				for (i = matrix->numrows_, rowstart = matrix->rows_[0]; i > 0; --i, rowstart += rowstep) {
					for (j = matrix->numcols_, data = rowstart; j > 0; --j, ++data) {
						*data = jas_seqent_asr(*data, n);
					}
				}
				// Fallthrough
			case 2:
				return;
		}
	}
}

void jas_matrix_asl(jas_matrix_t *matrix, int n)
{
	int i;
	int j;
	jas_seqent_t *rowstart;
	int rowstep;
	jas_seqent_t *data;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
					assert(matrix->rows_);
					rowstep = jas_matrix_rowstep(matrix);
					state = 1;
				} else {
					state = 2;
				}
				break;
			case 1:
				for (i = matrix->numrows_, rowstart = matrix->rows_[0]; i > 0; --i, rowstart += rowstep) {
					for (j = matrix->numcols_, data = rowstart; j > 0; --j, ++data) {
						*data = jas_seqent_asl(*data, n);
					}
				}
				// Fallthrough
			case 2:
				return;
		}
	}
}

int jas_matrix_resize(jas_matrix_t *matrix, int numrows, int numcols)
{
	int size;
	int i;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				size = numrows * numcols;
				if (size > matrix->datasize_ || numrows > matrix->maxrows_) {
					state = 1;
				} else {
					state = 2;
				}
				break;
			case 1:
				return -1;
			case 2:
				matrix->numrows_ = numrows;
				matrix->numcols_ = numcols;
				state = 3;
				break;
			case 3:
				for (i = 0; i < numrows; ++i) {
					matrix->rows_[i] = &matrix->data_[numcols * i];
				}
				return 0;
		}
	}
}

void jas_matrix_setall(jas_matrix_t *matrix, jas_seqent_t val)
{
	int i;
	int j;
	jas_seqent_t *rowstart;
	int rowstep;
	jas_seqent_t *data;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
					assert(matrix->rows_);
					rowstep = jas_matrix_rowstep(matrix);
					state = 1;
				} else {
					state = 2;
				}
				break;
			case 1:
				for (i = matrix->numrows_, rowstart = matrix->rows_[0]; i > 0; --i, rowstart += rowstep) {
					for (j = matrix->numcols_, data = rowstart; j > 0; --j, ++data) {
						*data = val;
					}
				}
				// Fallthrough
			case 2:
				return;
		}
	}
}

jas_matrix_t *jas_seq2d_input(FILE *in)
{
	jas_matrix_t *matrix;
	int i;
	int j;
	long x;
	int numrows;
	int numcols;
	int xoff;
	int yoff;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (fscanf(in, "%d %d", &xoff, &yoff) != 2) {
					state = 1;
				} else {
					state = 2;
				}
				break;
			case 1:
				return 0;
			case 2:
				if (fscanf(in, "%d %d", &numcols, &numrows) != 2) {
					state = 1;
				} else {
					state = 3;
				}
				break;
			case 3:
				matrix = jas_seq2d_create(xoff, yoff, xoff + numcols, yoff + numrows);
				if (!matrix) {
					state = 1;
				} else {
					state = 4;
				}
				break;
			case 4:
				if (jas_matrix_numrows(matrix) != numrows || jas_matrix_numcols(matrix) != numcols) {
					abort();
				}
				state = 5;
				break;
			case 5:
				for (i = 0; i < jas_matrix_numrows(matrix); i++) {
					for (j = 0; j < jas_matrix_numcols(matrix); j++) {
						if (fscanf(in, "%ld", &x) != 1) {
							state = 6;
							break;
						}
						jas_matrix_set(matrix, i, j, JAS_CAST(jas_seqent_t, x));
					}
					if (state == 6) break;
				}
				if (state == 6) break;
				return matrix;
			case 6:
				jas_matrix_destroy(matrix);
				return 0;
		}
	}
}

int jas_seq2d_output(jas_matrix_t *matrix, FILE *out)
{
#define MAXLINELEN	80
	int i;
	int j;
	jas_seqent_t x;
	char buf[MAXLINELEN + 1];
	char sbuf[MAXLINELEN + 1];
	int n;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				fprintf(out, "%d %d\n", jas_seq2d_xstart(matrix), jas_seq2d_ystart(matrix));
				fprintf(out, "%d %d\n", jas_matrix_numcols(matrix), jas_matrix_numrows(matrix));
				buf[0] = '\0';
				state = 1;
				break;
			case 1:
				for (i = 0; i < jas_matrix_numrows(matrix); ++i) {
					for (j = 0; j < jas_matrix_numcols(matrix); ++j) {
						x = jas_matrix_get(matrix, i, j);
						sprintf(sbuf, "%s%4ld", (strlen(buf) > 0) ? " " : "", JAS_CAST(long, x));
						n = strlen(buf);
						if (n + strlen(sbuf) > MAXLINELEN) {
							fputs(buf, out);
							fputs("\n", out);
							buf[0] = '\0';
						}
						strcat(buf, sbuf);
						if (j == jas_matrix_numcols(matrix) - 1) {
							fputs(buf, out);
							fputs("\n", out);
							buf[0] = '\0';
						}
					}
				}
				fputs(buf, out);
				state = 2;
				break;
			case 2:
				return 0;
		}
	}
}