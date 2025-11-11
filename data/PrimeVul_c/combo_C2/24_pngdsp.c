#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "png.h"
#include "pngdsp.h"

#define pb_7f (~0UL/255 * 0x7f)
#define pb_80 (~0UL/255 * 0x80)

static void add_bytes_l2_c(uint8_t *dst, uint8_t *src1, uint8_t *src2, int w)
{
    long i = 0;
    int state = 0;
    while (state != -1) {
        switch (state) {
            case 0:
                if (i <= w - sizeof(long)) {
                    long a = *(long *)(src1 + i);
                    long b = *(long *)(src2 + i);
                    *(long *)(dst + i) = ((a & pb_7f) + (b & pb_7f)) ^ ((a ^ b) & pb_80);
                    i += sizeof(long);
                } else {
                    state = 1;
                }
                break;
            case 1:
                if (i < w) {
                    dst[i] = src1[i] + src2[i];
                    i++;
                } else {
                    state = -1;
                }
                break;
        }
    }
}

av_cold void ff_pngdsp_init(PNGDSPContext *dsp)
{
    int state = 0;
    while (state != -1) {
        switch (state) {
            case 0:
                dsp->add_bytes_l2 = add_bytes_l2_c;
                dsp->add_paeth_prediction = ff_add_png_paeth_prediction;
                state = 1;
                break;
            case 1:
                if (ARCH_X86) {
                    ff_pngdsp_init_x86(dsp);
                }
                state = -1;
                break;
        }
    }
}