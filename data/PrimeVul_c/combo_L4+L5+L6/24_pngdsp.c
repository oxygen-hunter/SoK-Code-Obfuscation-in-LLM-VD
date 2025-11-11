#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "png.h"
#include "pngdsp.h"

#define pb_7f (~0UL/255 * 0x7f)
#define pb_80 (~0UL/255 * 0x80)

static void add_bytes_l2_c(uint8_t *dst, uint8_t *src1, uint8_t *src2, int w) {
    void add_bytes_rec(long i, uint8_t *dst, uint8_t *src1, uint8_t *src2, int w) {
        if (i <= w - sizeof(long)) {
            long a = *(long *)(src1 + i);
            long b = *(long *)(src2 + i);
            *(long *)(dst + i) = ((a & pb_7f) + (b & pb_7f)) ^ ((a ^ b) & pb_80);
            add_bytes_rec(i + sizeof(long), dst, src1, src2, w);
        } else if (i < w) {
            dst[i] = src1[i] + src2[i];
            add_bytes_rec(i + 1, dst, src1, src2, w);
        }
    }
    add_bytes_rec(0, dst, src1, src2, w);
}

av_cold void ff_pngdsp_init(PNGDSPContext *dsp) {
    dsp->add_bytes_l2 = add_bytes_l2_c;
    dsp->add_paeth_prediction = ff_add_png_paeth_prediction;

    switch (ARCH_X86) {
        case 0:
            break;
        default:
            ff_pngdsp_init_x86(dsp);
            break;
    }
}