#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"
#include "jasper/jas_debug.h"

#include "jpc_bs.h"

static jpc_bitstream_t *jpc_bitstream_alloc(void);

jpc_bitstream_t *jpc_bitstream_sopen(jas_stream_t *stream, char *mode)
{
	jpc_bitstream_t *b;

	if (!(b = jpc_bitstream_alloc())) {
		return 0;
	}

	b->flags_ = JPC_BITSTREAM_NOCLOSE;

	b->stream_ = stream;
	b->openmode_ = (mode[0] == 'w') ? JPC_BITSTREAM_WRITE : JPC_BITSTREAM_READ;

	b->cnt_ = (b->openmode_ == JPC_BITSTREAM_READ) ? 0 : 8;
	b->buf_ = 0;

	return b;
}

int jpc_bitstream_close(jpc_bitstream_t *b)
{
	int r = 0;

	if (jpc_bitstream_align(b)) {
		r = -1;
	}

	if (!(b->flags_ & JPC_BITSTREAM_NOCLOSE) && b->stream_) {
		if (jas_stream_close(b->stream_)) {
			r = -1;
		}
		b->stream_ = 0;
	}

	jas_free(b);
	return r;
}

static jpc_bitstream_t *jpc_bitstream_alloc()
{
	jpc_bitstream_t *b;

	if (!(b = jas_malloc(sizeof(jpc_bitstream_t)))) {
		return 0;
	}
	b->stream_ = 0;
	b->cnt_ = 0;
	b->flags_ = 0;
	b->openmode_ = 0;

	return b;
}

int jpc_bitstream_getbit_func(jpc_bitstream_t *b)
{
	int r;
	JAS_DBGLOG(1000, ("jpc_bitstream_getbit_func(%p)\n", b));
	r = jpc_bitstream_getbit_macro(b);
	JAS_DBGLOG(1000, ("jpc_bitstream_getbit_func -> %d\n", r));
	return r;
}

int jpc_bitstream_putbit_func(jpc_bitstream_t *b, int v)
{
	int r;
	JAS_DBGLOG(1000, ("jpc_bitstream_putbit_func(%p, %d)\n", b, v));
	r = jpc_bitstream_putbit_macro(b, v);
	JAS_DBGLOG(1000, ("jpc_bitstream_putbit_func() -> %d\n", r));
	return r;
}

long jpc_bitstream_getbits(jpc_bitstream_t *b, int n)
{
	long v;
	int u;

	assert(n >= 0 && n < 32);

	v = 0;
	while (--n >= 0) {
		if ((u = jpc_bitstream_getbit(b)) < 0) {
			return -1;
		}
		v = (v << 1) | u;
	}
	return v;
}

int jpc_bitstream_putbits(jpc_bitstream_t *b, int n, long v)
{
	int m;

	assert(n >= 0 && n < 32);
	assert(!(v & (~JAS_ONES(n))));

	m = n - 1;
	while (--n >= 0) {
		if (jpc_bitstream_putbit(b, (v >> m) & 1) == EOF) {
			return EOF;
		}
		v <<= 1;
	}
	return 0;
}

int jpc_bitstream_fillbuf(jpc_bitstream_t *b)
{
	int c;
	assert(b->openmode_ & JPC_BITSTREAM_READ);
	assert(b->cnt_ <= 0);

	if (b->flags_ & JPC_BITSTREAM_ERR) {
		b->cnt_ = 0;
		return -1;
	}

	if (b->flags_ & JPC_BITSTREAM_EOF) {
		b->buf_ = 0x7f;
		b->cnt_ = 7;
		return 1;
	}

	b->buf_ = (b->buf_ << 8) & 0xffff;
	if ((c = jas_stream_getc((b)->stream_)) == EOF) {
		b->flags_ |= JPC_BITSTREAM_EOF;
		return 1;
	}
	b->cnt_ = (b->buf_ == 0xff00) ? 6 : 7;
	b->buf_ |= c & ((1 << (b->cnt_ + 1)) - 1);
	return (b->buf_ >> b->cnt_) & 1;
}

int jpc_bitstream_needalign(jpc_bitstream_t *b)
{
	if (b->openmode_ & JPC_BITSTREAM_READ) {
		if ((b->cnt_ < 8 && b->cnt_ > 0) || ((b->buf_ >> 8) & 0xff) == 0xff) {
			return 1;
		}
	} else if (b->openmode_ & JPC_BITSTREAM_WRITE) {
		if ((b->cnt_ < 8 && b->cnt_ >= 0) || ((b->buf_ >> 8) & 0xff) == 0xff) {
			return 1;
		}
	} else {
		assert(0);
		return -1;
	}
	return 0;
}

int jpc_bitstream_pending(jpc_bitstream_t *b)
{
	if (b->openmode_ & JPC_BITSTREAM_WRITE) {
#if 1
		if (b->cnt_ < 8) {
			return 1;
		}
#else
		if (b->cnt_ < 8) {
			if (((b->buf_ >> 8) & 0xff) == 0xff) {
				return 2;
			}
			return 1;
		}
#endif
		return 0;
	} else {
		return -1;
	}
}

int jpc_bitstream_align(jpc_bitstream_t *b)
{
	int r;
	if (b->openmode_ & JPC_BITSTREAM_READ) {
		r = jpc_bitstream_inalign(b, 0, 0);
	} else if (b->openmode_ & JPC_BITSTREAM_WRITE) {
		r = jpc_bitstream_outalign(b, 0);
	} else {
		abort();
	}
	return r;
}

int jpc_bitstream_inalign(jpc_bitstream_t *b, int fillmask, int filldata)
{
	int n;
	int v;
	int u;
	int numfill;
	int m;

	numfill = 7;
	m = 0;
	v = 0;
	if (b->cnt_ > 0) {
		n = b->cnt_;
	} else if (!b->cnt_) {
		n = ((b->buf_ & 0xff) == 0xff) ? 7 : 0;
	} else {
		n = 0;
	}
	if (n > 0) {
		if ((u = jpc_bitstream_getbits(b, n)) < 0) {
			return -1;
		}
		m += n;
		v = (v << n) | u;
	}
	if ((b->buf_ & 0xff) == 0xff) {
		if ((u = jpc_bitstream_getbits(b, 7)) < 0) {
			return -1;
		}
		v = (v << 7) | u;
		m += 7;
	}
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

int jpc_bitstream_outalign(jpc_bitstream_t *b, int filldata)
{
	int n;
	int v;

	assert(b->openmode_ & JPC_BITSTREAM_WRITE);

	assert(!(filldata & (~0x3f)));

	if (!b->cnt_) {
		if ((b->buf_ & 0xff) == 0xff) {
			n = 7;
			v = filldata;
		} else {
			n = 0;
			v = 0;
		}
	} else if (b->cnt_ > 0 && b->cnt_ < 8) {
		n = b->cnt_;
		v = filldata >> (7 - n);
	} else {
		n = 0;
		v = 0;
		return 0;
	}

	if (n > 0) {
		if (jpc_bitstream_putbits(b, n, v)) {
			return -1;
		}
	}
	if (b->cnt_ < 8) {
		assert(b->cnt_ >= 0 && b->cnt_ < 8);
		assert((b->buf_ & 0xff) != 0xff);
		if (jas_stream_putc(b->stream_, b->buf_ & 0xff) == EOF) {
			return -1;
		}
		b->cnt_ = 8;
		b->buf_ = (b->buf_ << 8) & 0xffff;
	}

	return 0;
}