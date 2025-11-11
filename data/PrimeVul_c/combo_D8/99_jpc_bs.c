#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"
#include "jasper/jas_debug.h"

#include "jpc_bs.h"

static jpc_bitstream_t *jpc_bitstream_alloc_func();

jpc_bitstream_t *jpc_bitstream_sopen(jas_stream_t *stream, char *mode)
{
	jpc_bitstream_t *jpc_data;
	if (!(jpc_data = jpc_bitstream_alloc_func())) {
		return 0;
	}
	jpc_data->flags_ = JPC_BITSTREAM_NOCLOSE;
	jpc_data->stream_ = stream;
	jpc_data->openmode_ = (mode[0] == 'w') ? JPC_BITSTREAM_WRITE : JPC_BITSTREAM_READ;
	jpc_data->cnt_ = (jpc_data->openmode_ == JPC_BITSTREAM_READ) ? 0 : 8;
	jpc_data->buf_ = 0;
	return jpc_data;
}

int jpc_bitstream_close(jpc_bitstream_t *jpc_data)
{
	int result = 0;
	if (jpc_bitstream_align(jpc_data)) {
		result = -1;
	}
	if (!(jpc_data->flags_ & JPC_BITSTREAM_NOCLOSE) && jpc_data->stream_) {
		if (jas_stream_close(jpc_data->stream_)) {
			result = -1;
		}
		jpc_data->stream_ = 0;
	}
	jas_free(jpc_data);
	return result;
}

static jpc_bitstream_t *jpc_bitstream_alloc_func()
{
	jpc_bitstream_t *jpc_data;
	if (!(jpc_data = jas_malloc(sizeof(jpc_bitstream_t)))) {
		return 0;
	}
	jpc_data->stream_ = 0;
	jpc_data->cnt_ = 0;
	jpc_data->flags_ = 0;
	jpc_data->openmode_ = 0;
	return jpc_data;
}

int jpc_bitstream_getbit_func(jpc_bitstream_t *jpc_data)
{
	int result;
	JAS_DBGLOG(1000, ("jpc_bitstream_getbit_func(%p)\n", jpc_data));
	result = jpc_bitstream_getbit_macro(jpc_data);
	JAS_DBGLOG(1000, ("jpc_bitstream_getbit_func -> %d\n", result));
	return result;
}

int jpc_bitstream_putbit_func(jpc_bitstream_t *jpc_data, int b)
{
	int result;
	JAS_DBGLOG(1000, ("jpc_bitstream_putbit_func(%p, %d)\n", jpc_data, b));
	result = jpc_bitstream_putbit_macro(jpc_data, b);
	JAS_DBGLOG(1000, ("jpc_bitstream_putbit_func() -> %d\n", result));
	return result;
}

long jpc_bitstream_getbits(jpc_bitstream_t *jpc_data, int n)
{
	long v;
	int u;
	assert(n >= 0 && n < 32);
	v = 0;
	while (--n >= 0) {
		if ((u = jpc_bitstream_getbit(jpc_data)) < 0) {
			return -1;
		}
		v = (v << 1) | u;
	}
	return v;
}

int jpc_bitstream_putbits(jpc_bitstream_t *jpc_data, int n, long v)
{
	int m;
	assert(n >= 0 && n < 32);
	assert(!(v & (~JAS_ONES(n))));
	m = n - 1;
	while (--n >= 0) {
		if (jpc_bitstream_putbit(jpc_data, (v >> m) & 1) == EOF) {
			return EOF;
		}
		v <<= 1;
	}
	return 0;
}

int jpc_bitstream_fillbuf(jpc_bitstream_t *jpc_data)
{
	int c;
	assert(jpc_data->openmode_ & JPC_BITSTREAM_READ);
	assert(jpc_data->cnt_ <= 0);
	if (jpc_data->flags_ & JPC_BITSTREAM_ERR) {
		jpc_data->cnt_ = 0;
		return -1;
	}
	if (jpc_data->flags_ & JPC_BITSTREAM_EOF) {
		jpc_data->buf_ = 0x7f;
		jpc_data->cnt_ = 7;
		return 1;
	}
	jpc_data->buf_ = (jpc_data->buf_ << 8) & 0xffff;
	if ((c = jas_stream_getc((jpc_data)->stream_)) == EOF) {
		jpc_data->flags_ |= JPC_BITSTREAM_EOF;
		return 1;
	}
	jpc_data->cnt_ = (jpc_data->buf_ == 0xff00) ? 6 : 7;
	jpc_data->buf_ |= c & ((1 << (jpc_data->cnt_ + 1)) - 1);
	return (jpc_data->buf_ >> jpc_data->cnt_) & 1;
}

int jpc_bitstream_needalign(jpc_bitstream_t *jpc_data)
{
	if (jpc_data->openmode_ & JPC_BITSTREAM_READ) {
		if ((jpc_data->cnt_ < 8 && jpc_data->cnt_ > 0) || ((jpc_data->buf_ >> 8) & 0xff) == 0xff) {
			return 1;
		}
	} else if (jpc_data->openmode_ & JPC_BITSTREAM_WRITE) {
		if ((jpc_data->cnt_ < 8 && jpc_data->cnt_ >= 0) || ((jpc_data->buf_ >> 8) & 0xff) == 0xff) {
			return 1;
		}
	} else {
		assert(0);
		return -1;
	}
	return 0;
}

int jpc_bitstream_pending(jpc_bitstream_t *jpc_data)
{
	if (jpc_data->openmode_ & JPC_BITSTREAM_WRITE) {
#if 1
		if (jpc_data->cnt_ < 8) {
			return 1;
		}
#else
		if (jpc_data->cnt_ < 8) {
			if (((jpc_data->buf_ >> 8) & 0xff) == 0xff) {
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

int jpc_bitstream_align(jpc_bitstream_t *jpc_data)
{
	int result;
	if (jpc_data->openmode_ & JPC_BITSTREAM_READ) {
		result = jpc_bitstream_inalign(jpc_data, 0, 0);
	} else if (jpc_data->openmode_ & JPC_BITSTREAM_WRITE) {
		result = jpc_bitstream_outalign(jpc_data, 0);
	} else {
		abort();
	}
	return result;
}

int jpc_bitstream_inalign(jpc_bitstream_t *jpc_data, int fillmask, int filldata)
{
	int n;
	int v;
	int u;
	int numfill;
	int m;
	numfill = 7;
	m = 0;
	v = 0;
	if (jpc_data->cnt_ > 0) {
		n = jpc_data->cnt_;
	} else if (!jpc_data->cnt_) {
		n = ((jpc_data->buf_ & 0xff) == 0xff) ? 7 : 0;
	} else {
		n = 0;
	}
	if (n > 0) {
		if ((u = jpc_bitstream_getbits(jpc_data, n)) < 0) {
			return -1;
		}
		m += n;
		v = (v << n) | u;
	}
	if ((jpc_data->buf_ & 0xff) == 0xff) {
		if ((u = jpc_bitstream_getbits(jpc_data, 7)) < 0) {
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

int jpc_bitstream_outalign(jpc_bitstream_t *jpc_data, int filldata)
{
	int n;
	int v;
	assert(jpc_data->openmode_ & JPC_BITSTREAM_WRITE);
	assert(!(filldata & (~0x3f)));
	if (!jpc_data->cnt_) {
		if ((jpc_data->buf_ & 0xff) == 0xff) {
			n = 7;
			v = filldata;
		} else {
			n = 0;
			v = 0;
		}
	} else if (jpc_data->cnt_ > 0 && jpc_data->cnt_ < 8) {
		n = jpc_data->cnt_;
		v = filldata >> (7 - n);
	} else {
		n = 0;
		v = 0;
		return 0;
	}
	if (n > 0) {
		if (jpc_bitstream_putbits(jpc_data, n, v)) {
			return -1;
		}
	}
	if (jpc_data->cnt_ < 8) {
		assert(jpc_data->cnt_ >= 0 && jpc_data->cnt_ < 8);
		assert((jpc_data->buf_ & 0xff) != 0xff);
		if (jas_stream_putc(jpc_data->stream_, jpc_data->buf_ & 0xff) == EOF) {
			return -1;
		}
		jpc_data->cnt_ = 8;
		jpc_data->buf_ = (jpc_data->buf_ << 8) & 0xffff;
	}
	return 0;
}