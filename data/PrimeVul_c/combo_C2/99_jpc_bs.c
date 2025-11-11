#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"
#include "jasper/jas_debug.h"

#include "jpc_bs.h"

static jpc_bitstream_t *jpc_bitstream_alloc(void);

jpc_bitstream_t *jpc_bitstream_sopen(jas_stream_t *stream, char *mode) {
    jpc_bitstream_t *bitstream;
    int state = 0;
    while (1) {
        switch (state) {
            case 0:
                if (!(bitstream = jpc_bitstream_alloc())) {
                    return 0;
                }
                state = 1;
                break;
            case 1:
                bitstream->flags_ = JPC_BITSTREAM_NOCLOSE;
                state = 2;
                break;
            case 2:
                bitstream->stream_ = stream;
                bitstream->openmode_ = (mode[0] == 'w') ? JPC_BITSTREAM_WRITE : JPC_BITSTREAM_READ;
                state = 3;
                break;
            case 3:
                bitstream->cnt_ = (bitstream->openmode_ == JPC_BITSTREAM_READ) ? 0 : 8;
                bitstream->buf_ = 0;
                return bitstream;
        }
    }
}

int jpc_bitstream_close(jpc_bitstream_t *bitstream) {
    int ret = 0, state = 0;
    while (1) {
        switch (state) {
            case 0:
                if (jpc_bitstream_align(bitstream)) {
                    ret = -1;
                }
                state = 1;
                break;
            case 1:
                if (!(bitstream->flags_ & JPC_BITSTREAM_NOCLOSE) && bitstream->stream_) {
                    if (jas_stream_close(bitstream->stream_)) {
                        ret = -1;
                    }
                    bitstream->stream_ = 0;
                }
                jas_free(bitstream);
                return ret;
        }
    }
}

static jpc_bitstream_t *jpc_bitstream_alloc() {
    jpc_bitstream_t *bitstream;
    int state = 0;
    while (1) {
        switch (state) {
            case 0:
                if (!(bitstream = jas_malloc(sizeof(jpc_bitstream_t)))) {
                    return 0;
                }
                state = 1;
                break;
            case 1:
                bitstream->stream_ = 0;
                bitstream->cnt_ = 0;
                bitstream->flags_ = 0;
                bitstream->openmode_ = 0;
                return bitstream;
        }
    }
}

int jpc_bitstream_getbit_func(jpc_bitstream_t *bitstream) {
    int ret, state = 0;
    while (1) {
        switch (state) {
            case 0:
                JAS_DBGLOG(1000, ("jpc_bitstream_getbit_func(%p)\n", bitstream));
                state = 1;
                break;
            case 1:
                ret = jpc_bitstream_getbit_macro(bitstream);
                state = 2;
                break;
            case 2:
                JAS_DBGLOG(1000, ("jpc_bitstream_getbit_func -> %d\n", ret));
                return ret;
        }
    }
}

int jpc_bitstream_putbit_func(jpc_bitstream_t *bitstream, int b) {
    int ret, state = 0;
    while (1) {
        switch (state) {
            case 0:
                JAS_DBGLOG(1000, ("jpc_bitstream_putbit_func(%p, %d)\n", bitstream, b));
                state = 1;
                break;
            case 1:
                ret = jpc_bitstream_putbit_macro(bitstream, b);
                state = 2;
                break;
            case 2:
                JAS_DBGLOG(1000, ("jpc_bitstream_putbit_func() -> %d\n", ret));
                return ret;
        }
    }
}

long jpc_bitstream_getbits(jpc_bitstream_t *bitstream, int n) {
    long v;
    int u, state = 0;
    while (1) {
        switch (state) {
            case 0:
                assert(n >= 0 && n < 32);
                v = 0;
                state = 1;
                break;
            case 1:
                if (--n >= 0) {
                    if ((u = jpc_bitstream_getbit(bitstream)) < 0) {
                        return -1;
                    }
                    v = (v << 1) | u;
                } else {
                    return v;
                }
                break;
        }
    }
}

int jpc_bitstream_putbits(jpc_bitstream_t *bitstream, int n, long v) {
    int m, state = 0;
    while (1) {
        switch (state) {
            case 0:
                assert(n >= 0 && n < 32);
                assert(!(v & (~JAS_ONES(n))));
                m = n - 1;
                state = 1;
                break;
            case 1:
                if (--n >= 0) {
                    if (jpc_bitstream_putbit(bitstream, (v >> m) & 1) == EOF) {
                        return EOF;
                    }
                    v <<= 1;
                } else {
                    return 0;
                }
                break;
        }
    }
}

int jpc_bitstream_fillbuf(jpc_bitstream_t *bitstream) {
    int c, state = 0;
    while (1) {
        switch (state) {
            case 0:
                assert(bitstream->openmode_ & JPC_BITSTREAM_READ);
                assert(bitstream->cnt_ <= 0);
                state = 1;
                break;
            case 1:
                if (bitstream->flags_ & JPC_BITSTREAM_ERR) {
                    bitstream->cnt_ = 0;
                    return -1;
                }
                state = 2;
                break;
            case 2:
                if (bitstream->flags_ & JPC_BITSTREAM_EOF) {
                    bitstream->buf_ = 0x7f;
                    bitstream->cnt_ = 7;
                    return 1;
                }
                state = 3;
                break;
            case 3:
                bitstream->buf_ = (bitstream->buf_ << 8) & 0xffff;
                if ((c = jas_stream_getc((bitstream)->stream_)) == EOF) {
                    bitstream->flags_ |= JPC_BITSTREAM_EOF;
                    return 1;
                }
                bitstream->cnt_ = (bitstream->buf_ == 0xff00) ? 6 : 7;
                bitstream->buf_ |= c & ((1 << (bitstream->cnt_ + 1)) - 1);
                return (bitstream->buf_ >> bitstream->cnt_) & 1;
        }
    }
}

int jpc_bitstream_needalign(jpc_bitstream_t *bitstream) {
    int state = 0;
    while (1) {
        switch (state) {
            case 0:
                if (bitstream->openmode_ & JPC_BITSTREAM_READ) {
                    state = 1;
                } else if (bitstream->openmode_ & JPC_BITSTREAM_WRITE) {
                    state = 2;
                } else {
                    assert(0);
                    return -1;
                }
                break;
            case 1:
                if ((bitstream->cnt_ < 8 && bitstream->cnt_ > 0) || ((bitstream->buf_ >> 8) & 0xff) == 0xff) {
                    return 1;
                }
                return 0;
            case 2:
                if ((bitstream->cnt_ < 8 && bitstream->cnt_ >= 0) || ((bitstream->buf_ >> 8) & 0xff) == 0xff) {
                    return 1;
                }
                return 0;
        }
    }
}

int jpc_bitstream_pending(jpc_bitstream_t *bitstream) {
    int state = 0;
    while (1) {
        switch (state) {
            case 0:
                if (bitstream->openmode_ & JPC_BITSTREAM_WRITE) {
                    state = 1;
                } else {
                    return -1;
                }
                break;
            case 1:
                if (bitstream->cnt_ < 8) {
                    return 1;
                }
                return 0;
        }
    }
}

int jpc_bitstream_align(jpc_bitstream_t *bitstream) {
    int ret, state = 0;
    while (1) {
        switch (state) {
            case 0:
                if (bitstream->openmode_ & JPC_BITSTREAM_READ) {
                    ret = jpc_bitstream_inalign(bitstream, 0, 0);
                    return ret;
                } else if (bitstream->openmode_ & JPC_BITSTREAM_WRITE) {
                    ret = jpc_bitstream_outalign(bitstream, 0);
                    return ret;
                } else {
                    abort();
                }
                break;
        }
    }
}

int jpc_bitstream_inalign(jpc_bitstream_t *bitstream, int fillmask, int filldata) {
    int n, v, u, numfill, m, state = 0;
    while (1) {
        switch (state) {
            case 0:
                numfill = 7;
                m = 0;
                v = 0;
                state = 1;
                break;
            case 1:
                if (bitstream->cnt_ > 0) {
                    n = bitstream->cnt_;
                } else if (!bitstream->cnt_) {
                    n = ((bitstream->buf_ & 0xff) == 0xff) ? 7 : 0;
                } else {
                    n = 0;
                }
                state = 2;
                break;
            case 2:
                if (n > 0) {
                    if ((u = jpc_bitstream_getbits(bitstream, n)) < 0) {
                        return -1;
                    }
                    m += n;
                    v = (v << n) | u;
                }
                state = 3;
                break;
            case 3:
                if ((bitstream->buf_ & 0xff) == 0xff) {
                    if ((u = jpc_bitstream_getbits(bitstream, 7)) < 0) {
                        return -1;
                    }
                    v = (v << 7) | u;
                    m += 7;
                }
                state = 4;
                break;
            case 4:
                if (m > numfill) {
                    v >>= m - numfill;
                } else {
                    filldata >>= numfill - m;
                    fillmask >>= numfill - m;
                }
                if (((~(v ^ filldata)) & fillmask) != fillmask) {
                    return 1;
                }
                return 0;
        }
    }
}

int jpc_bitstream_outalign(jpc_bitstream_t *bitstream, int filldata) {
    int n, v, state = 0;
    while (1) {
        switch (state) {
            case 0:
                assert(bitstream->openmode_ & JPC_BITSTREAM_WRITE);
                assert(!(filldata & (~0x3f)));
                state = 1;
                break;
            case 1:
                if (!bitstream->cnt_) {
                    if ((bitstream->buf_ & 0xff) == 0xff) {
                        n = 7;
                        v = filldata;
                    } else {
                        n = 0;
                        v = 0;
                    }
                } else if (bitstream->cnt_ > 0 && bitstream->cnt_ < 8) {
                    n = bitstream->cnt_;
                    v = filldata >> (7 - n);
                } else {
                    return 0;
                }
                state = 2;
                break;
            case 2:
                if (n > 0) {
                    if (jpc_bitstream_putbits(bitstream, n, v)) {
                        return -1;
                    }
                }
                if (bitstream->cnt_ < 8) {
                    assert(bitstream->cnt_ >= 0 && bitstream->cnt_ < 8);
                    assert((bitstream->buf_ & 0xff) != 0xff);
                    if (jas_stream_putc(bitstream->stream_, bitstream->buf_ & 0xff) == EOF) {
                        return -1;
                    }
                    bitstream->cnt_ = 8;
                    bitstream->buf_ = (bitstream->buf_ << 8) & 0xffff;
                }
                return 0;
        }
    }
}