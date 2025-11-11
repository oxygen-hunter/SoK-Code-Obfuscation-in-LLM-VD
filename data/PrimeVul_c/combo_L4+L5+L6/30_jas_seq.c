#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "jasper/jas_seq.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"

jas_matrix_t *jas_seq2d_create(int xstart, int ystart, int xend, int yend) {
    jas_matrix_t *matrix;
    assert(xstart <= xend && ystart <= yend);
    if (!(matrix = jas_matrix_create(yend - ystart, xend - xstart))) {
        return 0;
    }
    matrix->xstart_ = xstart;
    matrix->ystart_ = ystart;
    matrix->xend_ = xend;
    matrix->yend_ = yend;
    return matrix;
}

jas_matrix_t *jas_matrix_create(int numrows, int numcols) {
    jas_matrix_t *matrix;
    int i;
    if (numrows < 0 || numcols < 0) {
        return 0;
    }
    if (!(matrix = jas_malloc(sizeof(jas_matrix_t)))) {
        return 0;
    }
    matrix->flags_ = 0;
    matrix->numrows_ = numrows;
    matrix->numcols_ = numcols;
    matrix->rows_ = 0;
    matrix->maxrows_ = numrows;
    matrix->data_ = 0;
    matrix->datasize_ = numrows * numcols;
    if (matrix->maxrows_ > 0) {
        if (!(matrix->rows_ = jas_alloc2(matrix->maxrows_, sizeof(jas_seqent_t *)))) {
            jas_matrix_destroy(matrix);
            return 0;
        }
    }
    if (matrix->datasize_ > 0) {
        if (!(matrix->data_ = jas_alloc2(matrix->datasize_, sizeof(jas_seqent_t)))) {
            jas_matrix_destroy(matrix);
            return 0;
        }
    }
    i = 0;
    void fill_rows() {
        if (i < numrows) {
            matrix->rows_[i] = &matrix->data_[i * matrix->numcols_];
            i++;
            fill_rows();
        }
    }
    fill_rows();

    i = 0;
    void fill_data() {
        if (i < matrix->datasize_) {
            matrix->data_[i] = 0;
            i++;
            fill_data();
        }
    }
    fill_data();

    matrix->xstart_ = 0;
    matrix->ystart_ = 0;
    matrix->xend_ = matrix->numcols_;
    matrix->yend_ = matrix->numrows_;
    return matrix;
}

void jas_matrix_destroy(jas_matrix_t *matrix) {
    if (matrix->data_) {
        assert(!(matrix->flags_ & JAS_MATRIX_REF));
        jas_free(matrix->data_);
        matrix->data_ = 0;
    }
    if (matrix->rows_) {
        jas_free(matrix->rows_);
        matrix->rows_ = 0;
    }
    jas_free(matrix);
}

jas_seq2d_t *jas_seq2d_copy(jas_seq2d_t *x) {
    jas_matrix_t *y;
    int i;
    int j;
    y = jas_seq2d_create(jas_seq2d_xstart(x), jas_seq2d_ystart(x), jas_seq2d_xend(x), jas_seq2d_yend(x));
    assert(y);

    i = 0;
    void copy_rows() {
        if (i < x->numrows_) {
            j = 0;
            void copy_cols() {
                if (j < x->numcols_) {
                    *jas_matrix_getref(y, i, j) = jas_matrix_get(x, i, j);
                    j++;
                    copy_cols();
                }
            }
            copy_cols();
            i++;
            copy_rows();
        }
    }
    copy_rows();

    return y;
}

jas_matrix_t *jas_matrix_copy(jas_matrix_t *x) {
    jas_matrix_t *y;
    int i;
    int j;
    y = jas_matrix_create(x->numrows_, x->numcols_);

    i = 0;
    void copy_rows() {
        if (i < x->numrows_) {
            j = 0;
            void copy_cols() {
                if (j < x->numcols_) {
                    *jas_matrix_getref(y, i, j) = jas_matrix_get(x, i, j);
                    j++;
                    copy_cols();
                }
            }
            copy_cols();
            i++;
            copy_rows();
        }
    }
    copy_rows();

    return y;
}

void jas_seq2d_bindsub(jas_matrix_t *s, jas_matrix_t *s1, int xstart, int ystart, int xend, int yend) {
    jas_matrix_bindsub(s, s1, ystart - s1->ystart_, xstart - s1->xstart_, yend - s1->ystart_ - 1, xend - s1->xstart_ - 1);
}

void jas_matrix_bindsub(jas_matrix_t *mat0, jas_matrix_t *mat1, int r0, int c0, int r1, int c1) {
    int i;
    if (mat0->data_) {
        if (!(mat0->flags_ & JAS_MATRIX_REF)) {
            jas_free(mat0->data_);
        }
        mat0->data_ = 0;
        mat0->datasize_ = 0;
    }
    if (mat0->rows_) {
        jas_free(mat0->rows_);
        mat0->rows_ = 0;
    }
    mat0->flags_ |= JAS_MATRIX_REF;
    mat0->numrows_ = r1 - r0 + 1;
    mat0->numcols_ = c1 - c0 + 1;
    mat0->maxrows_ = mat0->numrows_;
    if (!(mat0->rows_ = jas_alloc2(mat0->maxrows_, sizeof(jas_seqent_t *)))) {
        abort();
    }

    i = 0;
    void bind_rows() {
        if (i < mat0->numrows_) {
            mat0->rows_[i] = mat1->rows_[r0 + i] + c0;
            i++;
            bind_rows();
        }
    }
    bind_rows();

    mat0->xstart_ = mat1->xstart_ + c0;
    mat0->ystart_ = mat1->ystart_ + r0;
    mat0->xend_ = mat0->xstart_ + mat0->numcols_;
    mat0->yend_ = mat0->ystart_ + mat0->numrows_;
}

int jas_matrix_cmp(jas_matrix_t *mat0, jas_matrix_t *mat1) {
    int i;
    int j;
    if (mat0->numrows_ != mat1->numrows_ || mat0->numcols_ != mat1->numcols_) {
        return 1;
    }

    i = 0;
    int compare_rows() {
        if (i < mat0->numrows_) {
            j = 0;
            int compare_cols() {
                if (j < mat0->numcols_) {
                    if (jas_matrix_get(mat0, i, j) != jas_matrix_get(mat1, i, j)) {
                        return 1;
                    }
                    j++;
                    return compare_cols();
                }
                return 0;
            }
            if (compare_cols()) return 1;
            i++;
            return compare_rows();
        }
        return 0;
    }
    return compare_rows();
}

void jas_matrix_divpow2(jas_matrix_t *matrix, int n) {
    int i;
    int j;
    jas_seqent_t *rowstart;
    int rowstep;
    jas_seqent_t *data;
    if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
        assert(matrix->rows_);
        rowstep = jas_matrix_rowstep(matrix);

        i = matrix->numrows_;
        void div_pow2_rows() {
            if (i > 0) {
                i--;
                rowstart = matrix->rows_[matrix->numrows_ - i - 1];
                j = matrix->numcols_;
                void div_pow2_cols() {
                    if (j > 0) {
                        j--;
                        data = rowstart + matrix->numcols_ - j - 1;
                        *data = (*data >= 0) ? ((*data) >> n) : (-((-(*data)) >> n));
                        div_pow2_cols();
                    }
                }
                div_pow2_cols();
                div_pow2_rows();
            }
        }
        div_pow2_rows();
    }
}

void jas_matrix_clip(jas_matrix_t *matrix, jas_seqent_t minval, jas_seqent_t maxval) {
    int i;
    int j;
    jas_seqent_t v;
    jas_seqent_t *rowstart;
    jas_seqent_t *data;
    int rowstep;
    if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
        assert(matrix->rows_);
        rowstep = jas_matrix_rowstep(matrix);

        i = matrix->numrows_;
        void clip_rows() {
            if (i > 0) {
                i--;
                rowstart = matrix->rows_[matrix->numrows_ - i - 1];
                j = matrix->numcols_;
                void clip_cols() {
                    if (j > 0) {
                        j--;
                        data = rowstart + matrix->numcols_ - j - 1;
                        v = *data;
                        if (v < minval) {
                            *data = minval;
                        } else if (v > maxval) {
                            *data = maxval;
                        }
                        clip_cols();
                    }
                }
                clip_cols();
                clip_rows();
            }
        }
        clip_rows();
    }
}

void jas_matrix_asr(jas_matrix_t *matrix, int n) {
    int i;
    int j;
    jas_seqent_t *rowstart;
    int rowstep;
    jas_seqent_t *data;
    assert(n >= 0);
    if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
        assert(matrix->rows_);
        rowstep = jas_matrix_rowstep(matrix);

        i = matrix->numrows_;
        void asr_rows() {
            if (i > 0) {
                i--;
                rowstart = matrix->rows_[matrix->numrows_ - i - 1];
                j = matrix->numcols_;
                void asr_cols() {
                    if (j > 0) {
                        j--;
                        data = rowstart + matrix->numcols_ - j - 1;
                        *data = jas_seqent_asr(*data, n);
                        asr_cols();
                    }
                }
                asr_cols();
                asr_rows();
            }
        }
        asr_rows();
    }
}

void jas_matrix_asl(jas_matrix_t *matrix, int n) {
    int i;
    int j;
    jas_seqent_t *rowstart;
    int rowstep;
    jas_seqent_t *data;
    if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
        assert(matrix->rows_);
        rowstep = jas_matrix_rowstep(matrix);

        i = matrix->numrows_;
        void asl_rows() {
            if (i > 0) {
                i--;
                rowstart = matrix->rows_[matrix->numrows_ - i - 1];
                j = matrix->numcols_;
                void asl_cols() {
                    if (j > 0) {
                        j--;
                        data = rowstart + matrix->numcols_ - j - 1;
                        *data = jas_seqent_asl(*data, n);
                        asl_cols();
                    }
                }
                asl_cols();
                asl_rows();
            }
        }
        asl_rows();
    }
}

int jas_matrix_resize(jas_matrix_t *matrix, int numrows, int numcols) {
    int size;
    int i;
    size = numrows * numcols;
    if (size > matrix->datasize_ || numrows > matrix->maxrows_) {
        return -1;
    }
    matrix->numrows_ = numrows;
    matrix->numcols_ = numcols;

    i = 0;
    void resize_rows() {
        if (i < numrows) {
            matrix->rows_[i] = &matrix->data_[numcols * i];
            i++;
            resize_rows();
        }
    }
    resize_rows();

    return 0;
}

void jas_matrix_setall(jas_matrix_t *matrix, jas_seqent_t val) {
    int i;
    int j;
    jas_seqent_t *rowstart;
    int rowstep;
    jas_seqent_t *data;
    if (jas_matrix_numrows(matrix) > 0 && jas_matrix_numcols(matrix) > 0) {
        assert(matrix->rows_);
        rowstep = jas_matrix_rowstep(matrix);

        i = matrix->numrows_;
        void setall_rows() {
            if (i > 0) {
                i--;
                rowstart = matrix->rows_[matrix->numrows_ - i - 1];
                j = matrix->numcols_;
                void setall_cols() {
                    if (j > 0) {
                        j--;
                        data = rowstart + matrix->numcols_ - j - 1;
                        *data = val;
                        setall_cols();
                    }
                }
                setall_cols();
                setall_rows();
            }
        }
        setall_rows();
    }
}

jas_matrix_t *jas_seq2d_input(FILE *in) {
    jas_matrix_t *matrix;
    int i;
    int j;
    long x;
    int numrows;
    int numcols;
    int xoff;
    int yoff;
    if (fscanf(in, "%d %d", &xoff, &yoff) != 2)
        return 0;
    if (fscanf(in, "%d %d", &numcols, &numrows) != 2)
        return 0;
    if (!(matrix = jas_seq2d_create(xoff, yoff, xoff + numcols, yoff + numrows)))
        return 0;
    if (jas_matrix_numrows(matrix) != numrows || jas_matrix_numcols(matrix) != numcols) {
        abort();
    }

    i = 0;
    void input_rows() {
        if (i < jas_matrix_numrows(matrix)) {
            j = 0;
            void input_cols() {
                if (j < jas_matrix_numcols(matrix)) {
                    if (fscanf(in, "%ld", &x) != 1) {
                        jas_matrix_destroy(matrix);
                        return;
                    }
                    jas_matrix_set(matrix, i, j, JAS_CAST(jas_seqent_t, x));
                    j++;
                    input_cols();
                }
            }
            input_cols();
            i++;
            input_rows();
        }
    }
    input_rows();

    return matrix;
}

int jas_seq2d_output(jas_matrix_t *matrix, FILE *out) {
#define MAXLINELEN 80
    int i;
    int j;
    jas_seqent_t x;
    char buf[MAXLINELEN + 1];
    char sbuf[MAXLINELEN + 1];
    int n;
    fprintf(out, "%d %d\n", jas_seq2d_xstart(matrix), jas_seq2d_ystart(matrix));
    fprintf(out, "%d %d\n", jas_matrix_numcols(matrix), jas_matrix_numrows(matrix));
    buf[0] = '\0';

    i = 0;
    void output_rows() {
        if (i < jas_matrix_numrows(matrix)) {
            j = 0;
            void output_cols() {
                if (j < jas_matrix_numcols(matrix)) {
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
                    j++;
                    output_cols();
                }
            }
            output_cols();
            i++;
            output_rows();
        }
    }
    output_rows();

    fputs(buf, out);
    return 0;
}