#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "png.h"
#include "pngdsp.h"

#define obfuscate_val_7f() (~0UL/255 * 0x7f)
#define obfuscate_val_80() (~0UL/255 * 0x80)

static void obfuscated_func(uint8_t *dst, uint8_t *src1, uint8_t *src2, int w)
{
    long obf_i;
    for (obf_i = 0; obf_i <= w - sizeof(long); obf_i += sizeof(long)) {
        long obf_a = *(long *)(src1 + obf_i);
        long obf_b = *(long *)(src2 + obf_i);
        *(long *)(dst + obf_i) = ((obf_a & obfuscate_val_7f()) + (obf_b & obfuscate_val_7f())) ^ ((obf_a ^ obf_b) & obfuscate_val_80());
    }
    for (; obf_i < w; obf_i++)
        dst[obf_i] = src1[obf_i] + src2[obf_i];
}

av_cold void ff_pngdsp_init(PNGDSPContext *obf_dsp)
{
    obf_dsp->add_bytes_l2         = obfuscated_func;
    obf_dsp->add_paeth_prediction = ff_add_png_paeth_prediction;

    if (ARCH_X86) ff_pngdsp_init_x86(obf_dsp);
}