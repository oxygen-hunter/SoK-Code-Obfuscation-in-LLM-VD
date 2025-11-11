#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "jasper/jas_seq.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"

typedef struct {
	int numcols_;
	int numrows_;
	jas_seqent_t *data_;
	jas_seqent_t **rows_;
	int datasize_;
	int maxrows_;
	int flags_;
	int xstart_;
	int ystart_;
	int xend_;
	int yend_;
} jas_matrix_t;

typedef jas_matrix_t jas_seq2d_t;

jas_matrix_t *jas_seq2d_create(int a, int b, int c, int d) {
	jas_matrix_t *e;
	assert(a <= c && b <= d);
	if (!(e = jas_matrix_create(d - b, c - a))) {
		return 0;
	}
	e->ystart_ = b; e->xstart_ = a; e->xend_ = c; e->yend_ = d;
	return e;
}

jas_matrix_t *jas_matrix_create(int a, int b) {
	jas_matrix_t *c; int i;
	if (a < 0 || b < 0) return 0;
	if (!(c = jas_malloc(sizeof(jas_matrix_t)))) return 0;
	c->flags_ = 0; c->numrows_ = a; c->numcols_ = b; c->rows_ = 0;
	c->maxrows_ = a; c->data_ = 0; c->datasize_ = a * b;
	if (c->maxrows_ > 0) {
		if (!(c->rows_ = jas_alloc2(c->maxrows_, sizeof(jas_seqent_t *)))) {
			jas_matrix_destroy(c); return 0;
		}
	}
	if (c->datasize_ > 0) {
		if (!(c->data_ = jas_alloc2(c->datasize_, sizeof(jas_seqent_t)))) {
			jas_matrix_destroy(c); return 0;
		}
	}
	for (i = 0; i < a; ++i) c->rows_[i] = &c->data_[i * c->numcols_];
	for (i = 0; i < c->datasize_; ++i) c->data_[i] = 0;
	c->xstart_ = 0; c->ystart_ = 0; c->xend_ = c->numcols_; c->yend_ = c->numrows_;
	return c;
}

void jas_matrix_destroy(jas_matrix_t *a) {
	if (a->data_) {
		assert(!(a->flags_ & JAS_MATRIX_REF));
		jas_free(a->data_); a->data_ = 0;
	}
	if (a->rows_) {
		jas_free(a->rows_); a->rows_ = 0;
	}
	jas_free(a);
}

jas_seq2d_t *jas_seq2d_copy(jas_seq2d_t *a) {
	jas_matrix_t *b; int i; int j;
	b = jas_seq2d_create(jas_seq2d_xstart(a), jas_seq2d_ystart(a), jas_seq2d_xend(a), jas_seq2d_yend(a));
	assert(b);
	for (i = 0; i < a->numrows_; ++i) {
		for (j = 0; j < a->numcols_; ++j) {
			*jas_matrix_getref(b, i, j) = jas_matrix_get(a, i, j);
		}
	}
	return b;
}

jas_matrix_t *jas_matrix_copy(jas_matrix_t *a) {
	jas_matrix_t *b; int i; int j;
	b = jas_matrix_create(a->numrows_, a->numcols_);
	for (i = 0; i < a->numrows_; ++i) {
		for (j = 0; j < a->numcols_; ++j) {
			*jas_matrix_getref(b, i, j) = jas_matrix_get(a, i, j);
		}
	}
	return b;
}

void jas_seq2d_bindsub(jas_matrix_t *a, jas_matrix_t *b, int c, int d, int e, int f) {
	jas_matrix_bindsub(a, b, d - b->ystart_, c - b->xstart_, f - b->ystart_ - 1, e - b->xstart_ - 1);
}

void jas_matrix_bindsub(jas_matrix_t *a, jas_matrix_t *b, int c, int d, int e, int f) {
	int i;
	if (a->data_) {
		if (!(a->flags_ & JAS_MATRIX_REF)) {
			jas_free(a->data_);
		}
		a->data_ = 0; a->datasize_ = 0;
	}
	if (a->rows_) {
		jas_free(a->rows_);
		a->rows_ = 0;
	}
	a->flags_ |= JAS_MATRIX_REF;
	a->numrows_ = e - c + 1;
	a->numcols_ = f - d + 1;
	a->maxrows_ = a->numrows_;
	if (!(a->rows_ = jas_alloc2(a->maxrows_, sizeof(jas_seqent_t *)))) {
		abort();
	}
	for (i = 0; i < a->numrows_; ++i) {
		a->rows_[i] = b->rows_[c + i] + d;
	}
	a->xstart_ = b->xstart_ + d;
	a->ystart_ = b->ystart_ + c;
	a->xend_ = a->xstart_ + a->numcols_;
	a->yend_ = a->ystart_ + a->numrows_;
}

int jas_matrix_cmp(jas_matrix_t *a, jas_matrix_t *b) {
	int i; int j;
	if (a->numrows_ != b->numrows_ || a->numcols_ != b->numcols_) {
		return 1;
	}
	for (i = 0; i < a->numrows_; ++i) {
		for (j = 0; j < a->numcols_; ++j) {
			if (jas_matrix_get(a, i, j) != jas_matrix_get(b, i, j)) {
				return 1;
			}
		}
	}
	return 0;
}

void jas_matrix_divpow2(jas_matrix_t *a, int b) {
	int i; int j; jas_seqent_t *c; int d; jas_seqent_t *e;
	if (jas_matrix_numrows(a) > 0 && jas_matrix_numcols(a) > 0) {
		assert(a->rows_);
		d = jas_matrix_rowstep(a);
		for (i = a->numrows_, c = a->rows_[0]; i > 0; --i, c += d) {
			for (j = a->numcols_, e = c; j > 0; --j, ++e) {
				*e = (*e >= 0) ? ((*e) >> b) : (-((-(*e)) >> b));
			}
		}
	}
}

void jas_matrix_clip(jas_matrix_t *a, jas_seqent_t b, jas_seqent_t c) {
	int i; int j; jas_seqent_t d; jas_seqent_t *e; jas_seqent_t *f; int g;
	if (jas_matrix_numrows(a) > 0 && jas_matrix_numcols(a) > 0) {
		assert(a->rows_);
		g = jas_matrix_rowstep(a);
		for (i = a->numrows_, e = a->rows_[0]; i > 0; --i, e += g) {
			f = e;
			for (j = a->numcols_, f = e; j > 0; --j, ++f) {
				d = *f;
				if (d < b) {
					*f = b;
				} else if (d > c) {
					*f = c;
				}
			}
		}
	}
}

void jas_matrix_asr(jas_matrix_t *a, int b) {
	int i; int j; jas_seqent_t *c; int d; jas_seqent_t *e;
	assert(b >= 0);
	if (jas_matrix_numrows(a) > 0 && jas_matrix_numcols(a) > 0) {
		assert(a->rows_);
		d = jas_matrix_rowstep(a);
		for (i = a->numrows_, c = a->rows_[0]; i > 0; --i, c += d) {
			for (j = a->numcols_, e = c; j > 0; --j, ++e) {
				*e = jas_seqent_asr(*e, b);
			}
		}
	}
}

void jas_matrix_asl(jas_matrix_t *a, int b) {
	int i; int j; jas_seqent_t *c; int d; jas_seqent_t *e;
	if (jas_matrix_numrows(a) > 0 && jas_matrix_numcols(a) > 0) {
		assert(a->rows_);
		d = jas_matrix_rowstep(a);
		for (i = a->numrows_, c = a->rows_[0]; i > 0; --i, c += d) {
			for (j = a->numcols_, e = c; j > 0; --j, ++e) {
				*e = jas_seqent_asl(*e, b);
			}
		}
	}
}

int jas_matrix_resize(jas_matrix_t *a, int b, int c) {
	int d; int i;
	d = b * c;
	if (d > a->datasize_ || b > a->maxrows_) return -1;
	a->numrows_ = b; a->numcols_ = c;
	for (i = 0; i < b; ++i) {
		a->rows_[i] = &a->data_[c * i];
	}
	return 0;
}

void jas_matrix_setall(jas_matrix_t *a, jas_seqent_t b) {
	int i; int j; jas_seqent_t *c; int d; jas_seqent_t *e;
	if (jas_matrix_numrows(a) > 0 && jas_matrix_numcols(a) > 0) {
		assert(a->rows_);
		d = jas_matrix_rowstep(a);
		for (i = a->numrows_, c = a->rows_[0]; i > 0; --i, c += d) {
			for (j = a->numcols_, e = c; j > 0; --j, ++e) {
				*e = b;
			}
		}
	}
}

jas_matrix_t *jas_seq2d_input(FILE *a) {
	jas_matrix_t *b; int i; int j; long c; int d; int e; int f; int g;
	if (fscanf(a, "%d %d", &f, &g) != 2) return 0;
	if (fscanf(a, "%d %d", &e, &d) != 2) return 0;
	if (!(b = jas_seq2d_create(f, g, f + e, g + d))) return 0;
	if (jas_matrix_numrows(b) != d || jas_matrix_numcols(b) != e) {
		abort();
	}
	for (i = 0; i < jas_matrix_numrows(b); i++) {
		for (j = 0; j < jas_matrix_numcols(b); j++) {
			if (fscanf(a, "%ld", &c) != 1) {
				jas_matrix_destroy(b); return 0;
			}
			jas_matrix_set(b, i, j, JAS_CAST(jas_seqent_t, c));
		}
	}
	return b;
}

int jas_seq2d_output(jas_matrix_t *a, FILE *b) {
#define MAXLINELEN	80
	int i; int j; jas_seqent_t c; char d[MAXLINELEN + 1]; char e[MAXLINELEN + 1]; int f;
	fprintf(b, "%d %d\n", jas_seq2d_xstart(a), jas_seq2d_ystart(a));
	fprintf(b, "%d %d\n", jas_matrix_numcols(a), jas_matrix_numrows(a));
	d[0] = '\0';
	for (i = 0; i < jas_matrix_numrows(a); ++i) {
		for (j = 0; j < jas_matrix_numcols(a); ++j) {
			c = jas_matrix_get(a, i, j);
			sprintf(e, "%s%4ld", (strlen(d) > 0) ? " " : "", JAS_CAST(long, c));
			f = strlen(d);
			if (f + strlen(e) > MAXLINELEN) {
				fputs(d, b); fputs("\n", b); d[0] = '\0';
			}
			strcat(d, e);
			if (j == jas_matrix_numcols(a) - 1) {
				fputs(d, b); fputs("\n", b); d[0] = '\0';
			}
		}
	}
	fputs(d, b);
	return 0;
}