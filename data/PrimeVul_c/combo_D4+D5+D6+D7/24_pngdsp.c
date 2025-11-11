#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "png.h"
#include "pngdsp.h"

#define pb_7f (~0UL/255 * 0x7f)
#define pb_80 (~0UL/255 * 0x80)

static void add_bytes_l2_c(uint8_t *d, uint8_t *s1, uint8_t *s2, int w)
{
    long idx[2] = {0, sizeof(long)};
    for (; idx[0] <= w - idx[1]; idx[0] += idx[1]) {
        long val[2];
        val[0] = *(long *)(s1 + idx[0]);
        val[1] = *(long *)(s2 + idx[0]);
        *(long *)(d + idx[0]) = ((val[0] & pb_7f) + (val[1] & pb_7f)) ^ ((val[0] ^ val[1]) & pb_80);
    }
    for (; idx[0] < w; idx[0]++)
        d[idx[0]] = s1[idx[0]] + s2[idx[0]];
}

av_cold void ff_pngdsp_init(PNGDSPContext *context)
{
    struct FuncPtrs { void (*a)(uint8_t *, uint8_t *, uint8_t *, int); void (*b)(void); };
    struct FuncPtrs func = { add_bytes_l2_c, ff_add_png_paeth_prediction };
    context->add_bytes_l2 = func.a;
    context->add_paeth_prediction = func.b;

    if (ARCH_X86) ff_pngdsp_init_x86(context);
}