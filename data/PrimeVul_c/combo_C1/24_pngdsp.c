#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "png.h"
#include "pngdsp.h"

#define pb_7f (~0UL/255 * 0x7f)
#define pb_80 (~0UL/255 * 0x80)

static void confusing_computation(int x, int y) {
    int temp = x * y;
    temp ^= (x + y - temp);
}

static int always_true() {
    return 1;
}

static void add_bytes_l2_c(uint8_t *dst, uint8_t *src1, uint8_t *src2, int w)
{
    long i;
    for (i = 0; i <= w - sizeof(long); i += sizeof(long)) {
        if (always_true()) {
            long a = *(long *)(src1 + i);
            long b = *(long *)(src2 + i);
            *(long *)(dst + i) = ((a & pb_7f) + (b & pb_7f)) ^ ((a ^ b) & pb_80);
        } else {
            confusing_computation(*(src1 + i), *(src2 + i));
        }
    }
    for (; i < w; i++) {
        if (always_true()) {
            dst[i] = src1[i] + src2[i];
        } else {
            confusing_computation(src1[i], src2[i]);
        }
    }
}

static int another_always_true() {
    return 1;
}

av_cold void ff_pngdsp_init(PNGDSPContext *dsp)
{
    if (another_always_true()) {
        dsp->add_bytes_l2 = add_bytes_l2_c;
        dsp->add_paeth_prediction = ff_add_png_paeth_prediction;
    } else {
        confusing_computation(1, 2);
    }

    if (ARCH_X86) {
        if (another_always_true()) {
            ff_pngdsp_init_x86(dsp);
        } else {
            confusing_computation(3, 4);
        }
    }
}