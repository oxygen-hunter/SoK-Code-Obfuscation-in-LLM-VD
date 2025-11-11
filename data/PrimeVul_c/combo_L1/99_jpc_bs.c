#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"
#include "jasper/jas_debug.h"

#include "jpc_bs.h"

static OX7B4DF339 *OX808E6B2D(void);

OX7B4DF339 *OXFB5E0D89(jas_stream_t *OX9EBAF3BE, char *OXED4487FB)
{
	OX7B4DF339 *OXD4A1D0C4;

	if (!(OXD4A1D0C4 = OX808E6B2D())) {
		return 0;
	}

	OXD4A1D0C4->flags_ = JPC_BITSTREAM_NOCLOSE;

	OXD4A1D0C4->stream_ = OX9EBAF3BE;
	OXD4A1D0C4->openmode_ = (OXED4487FB[0] == 'w') ? JPC_BITSTREAM_WRITE :
	  JPC_BITSTREAM_READ;

	OXD4A1D0C4->cnt_ = (OXD4A1D0C4->openmode_ == JPC_BITSTREAM_READ) ? 0 : 8;
	OXD4A1D0C4->buf_ = 0;

	return OXD4A1D0C4;
}

int OXCC52A5E4(OX7B4DF339 *OXD4A1D0C4)
{
	int OX48AEF5C7 = 0;

	if (OX1EDE1866(OXD4A1D0C4)) {
		OX48AEF5C7 = -1;
	}

	if (!(OXD4A1D0C4->flags_ & JPC_BITSTREAM_NOCLOSE) && OXD4A1D0C4->stream_) {
		if (jas_stream_close(OXD4A1D0C4->stream_)) {
			OX48AEF5C7 = -1;
		}
		OXD4A1D0C4->stream_ = 0;
	}

	jas_free(OXD4A1D0C4);
	return OX48AEF5C7;
}

static OX7B4DF339 *OX808E6B2D()
{
	OX7B4DF339 *OXD4A1D0C4;

	if (!(OXD4A1D0C4 = jas_malloc(sizeof(OX7B4DF339)))) {
		return 0;
	}
	OXD4A1D0C4->stream_ = 0;
	OXD4A1D0C4->cnt_ = 0;
	OXD4A1D0C4->flags_ = 0;
	OXD4A1D0C4->openmode_ = 0;

	return OXD4A1D0C4;
}

int OX4F9FDC2A(OX7B4DF339 *OXD4A1D0C4)
{
	int OX48AEF5C7;
	JAS_DBGLOG(1000, ("OX4F9FDC2A(%p)\n", OXD4A1D0C4));
	OX48AEF5C7 = OXF6C3D7C7(OXD4A1D0C4);
	JAS_DBGLOG(1000, ("OX4F9FDC2A -> %d\n", OX48AEF5C7));
	return OX48AEF5C7;
}

int OX40BFD83D(OX7B4DF339 *OXD4A1D0C4, int OX24E06D4A)
{
	int OX48AEF5C7;
	JAS_DBGLOG(1000, ("OX40BFD83D(%p, %d)\n", OXD4A1D0C4, OX24E06D4A));
	OX48AEF5C7 = OX0A9D3A39(OXD4A1D0C4, OX24E06D4A);
	JAS_DBGLOG(1000, ("OX40BFD83D() -> %d\n", OX48AEF5C7));
	return OX48AEF5C7;
}

long OX4AE42F7F(OX7B4DF339 *OXD4A1D0C4, int OX99F7B1F2)
{
	long OX9B9C5F1A;
	int OX2E2D2C8B;

	assert(OX99F7B1F2 >= 0 && OX99F7B1F2 < 32);

	OX9B9C5F1A = 0;
	while (--OX99F7B1F2 >= 0) {
		if ((OX2E2D2C8B = jpc_bitstream_getbit(OXD4A1D0C4)) < 0) {
			return -1;
		}
		OX9B9C5F1A = (OX9B9C5F1A << 1) | OX2E2D2C8B;
	}
	return OX9B9C5F1A;
}

int OX9B96F6E6(OX7B4DF339 *OXD4A1D0C4, int OX99F7B1F2, long OX9B9C5F1A)
{
	int OX2E2D2C8B;

	assert(OX99F7B1F2 >= 0 && OX99F7B1F2 < 32);
	assert(!(OX9B9C5F1A & (~JAS_ONES(OX99F7B1F2))));

	OX2E2D2C8B = OX99F7B1F2 - 1;
	while (--OX99F7B1F2 >= 0) {
		if (jpc_bitstream_putbit(OXD4A1D0C4, (OX9B9C5F1A >> OX2E2D2C8B) & 1) == EOF) {
			return EOF;
		}
		OX9B9C5F1A <<= 1;
	}
	return 0;
}

int OXFD0DF7BB(OX7B4DF339 *OXD4A1D0C4)
{
	int OX2E2D2C8B;
	assert(OXD4A1D0C4->openmode_ & JPC_BITSTREAM_READ);
	assert(OXD4A1D0C4->cnt_ <= 0);

	if (OXD4A1D0C4->flags_ & JPC_BITSTREAM_ERR) {
		OXD4A1D0C4->cnt_ = 0;
		return -1;
	}

	if (OXD4A1D0C4->flags_ & JPC_BITSTREAM_EOF) {
		OXD4A1D0C4->buf_ = 0x7f;
		OXD4A1D0C4->cnt_ = 7;
		return 1;
	}

	OXD4A1D0C4->buf_ = (OXD4A1D0C4->buf_ << 8) & 0xffff;
	if ((OX2E2D2C8B = jas_stream_getc((OXD4A1D0C4)->stream_)) == EOF) {
		OXD4A1D0C4->flags_ |= JPC_BITSTREAM_EOF;
		return 1;
	}
	OXD4A1D0C4->cnt_ = (OXD4A1D0C4->buf_ == 0xff00) ? 6 : 7;
	OXD4A1D0C4->buf_ |= OX2E2D2C8B & ((1 << (OXD4A1D0C4->cnt_ + 1)) - 1);
	return (OXD4A1D0C4->buf_ >> OXD4A1D0C4->cnt_) & 1;
}

int OXF1B93E8D(OX7B4DF339 *OXD4A1D0C4)
{
	if (OXD4A1D0C4->openmode_ & JPC_BITSTREAM_READ) {
		if ((OXD4A1D0C4->cnt_ < 8 && OXD4A1D0C4->cnt_ > 0) ||
		  ((OXD4A1D0C4->buf_ >> 8) & 0xff) == 0xff) {
			return 1;
		}
	} else if (OXD4A1D0C4->openmode_ & JPC_BITSTREAM_WRITE) {
		if ((OXD4A1D0C4->cnt_ < 8 && OXD4A1D0C4->cnt_ >= 0) ||
		  ((OXD4A1D0C4->buf_ >> 8) & 0xff) == 0xff) {
			return 1;
		}
	} else {
		assert(0);
		return -1;
	}
	return 0;
}

int OX4F84E5F1(OX7B4DF339 *OXD4A1D0C4)
{
	if (OXD4A1D0C4->openmode_ & JPC_BITSTREAM_WRITE) {
#if 1
		if (OXD4A1D0C4->cnt_ < 8) {
			return 1;
		}
#else
		if (OXD4A1D0C4->cnt_ < 8) {
			if (((OXD4A1D0C4->buf_ >> 8) & 0xff) == 0xff) {
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

int OX1EDE1866(OX7B4DF339 *OXD4A1D0C4)
{
	int OX48AEF5C7;
	if (OXD4A1D0C4->openmode_ & JPC_BITSTREAM_READ) {
		OX48AEF5C7 = OX8F2B7D78(OXD4A1D0C4, 0, 0);
	} else if (OXD4A1D0C4->openmode_ & JPC_BITSTREAM_WRITE) {
		OX48AEF5C7 = OX2FBF4CAC(OXD4A1D0C4, 0);
	} else {
		abort();
	}
	return OX48AEF5C7;
}

int OX8F2B7D78(OX7B4DF339 *OXD4A1D0C4, int OXEFD5BA8F,
  int OX6A7B8C4B)
{
	int OX99F7B1F2;
	int OX9B9C5F1A;
	int OX2E2D2C8B;
	int OX5C3DFE2F;

	OX5C3DFE2F = 7;
	OX9B9C5F1A = 0;
	OX9B9C5F1A = 0;
	if (OXD4A1D0C4->cnt_ > 0) {
		OX99F7B1F2 = OXD4A1D0C4->cnt_;
	} else if (!OXD4A1D0C4->cnt_) {
		OX99F7B1F2 = ((OXD4A1D0C4->buf_ & 0xff) == 0xff) ? 7 : 0;
	} else {
		OX99F7B1F2 = 0;
	}
	if (OX99F7B1F2 > 0) {
		if ((OX2E2D2C8B = OX4AE42F7F(OXD4A1D0C4, OX99F7B1F2)) < 0) {
			return -1;
		}
		OX9B9C5F1A += OX99F7B1F2;
		OX9B9C5F1A = (OX9B9C5F1A << OX99F7B1F2) | OX2E2D2C8B;
	}
	if ((OXD4A1D0C4->buf_ & 0xff) == 0xff) {
		if ((OX2E2D2C8B = OX4AE42F7F(OXD4A1D0C4, 7)) < 0) {
			return -1;
		}
		OX9B9C5F1A = (OX9B9C5F1A << 7) | OX2E2D2C8B;
		OX9B9C5F1A += 7;
	}
	if (OX9B9C5F1A > OX5C3DFE2F) {
		OX9B9C5F1A >>= OX9B9C5F1A - OX5C3DFE2F;
	} else {
		OX6A7B8C4B >>= OX5C3DFE2F - OX9B9C5F1A;
		OXEFD5BA8F >>= OX5C3DFE2F - OX9B9C5F1A;
	}
	if (((~(OX9B9C5F1A ^ OX6A7B8C4B)) & OXEFD5BA8F) != OXEFD5BA8F) {
		return 1;
	}

	return 0;
}

int OX2FBF4CAC(OX7B4DF339 *OXD4A1D0C4, int OX6A7B8C4B)
{
	int OX99F7B1F2;
	int OX9B9C5F1A;

	assert(OXD4A1D0C4->openmode_ & JPC_BITSTREAM_WRITE);

	assert(!(OX6A7B8C4B & (~0x3f)));

	if (!OXD4A1D0C4->cnt_) {
		if ((OXD4A1D0C4->buf_ & 0xff) == 0xff) {
			OX99F7B1F2 = 7;
			OX9B9C5F1A = OX6A7B8C4B;
		} else {
			OX99F7B1F2 = 0;
			OX9B9C5F1A = 0;
		}
	} else if (OXD4A1D0C4->cnt_ > 0 && OXD4A1D0C4->cnt_ < 8) {
		OX99F7B1F2 = OXD4A1D0C4->cnt_;
		OX9B9C5F1A = OX6A7B8C4B >> (7 - OX99F7B1F2);
	} else {
		OX99F7B1F2 = 0;
		OX9B9C5F1A = 0;
		return 0;
	}

	if (OX99F7B1F2 > 0) {
		if (OX9B96F6E6(OXD4A1D0C4, OX99F7B1F2, OX9B9C5F1A)) {
			return -1;
		}
	}
	if (OXD4A1D0C4->cnt_ < 8) {
		assert(OXD4A1D0C4->cnt_ >= 0 && OXD4A1D0C4->cnt_ < 8);
		assert((OXD4A1D0C4->buf_ & 0xff) != 0xff);
		if (jas_stream_putc(OXD4A1D0C4->stream_, OXD4A1D0C4->buf_ & 0xff) == EOF) {
			return -1;
		}
		OXD4A1D0C4->cnt_ = 8;
		OXD4A1D0C4->buf_ = (OXD4A1D0C4->buf_ << 8) & 0xffff;
	}

	return 0;
}