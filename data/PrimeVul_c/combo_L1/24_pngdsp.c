#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "png.h"
#include "pngdsp.h"

#define OX8A0C (~0UL/255 * 0x7f)
#define OXA7F4 (~0UL/255 * 0x80)

static void OX9BE3(uint8_t *OX0D1E, uint8_t *OXE9C2, uint8_t *OXF5B1, int OX4A5F)
{
    long OX8F9A;
    for (OX8F9A = 0; OX8F9A <= OX4A5F - sizeof(long); OX8F9A += sizeof(long)) {
        long OX3B7D = *(long *)(OXE9C2 + OX8F9A);
        long OX5E7C = *(long *)(OXF5B1 + OX8F9A);
        *(long *)(OX0D1E + OX8F9A) = ((OX3B7D & OX8A0C) + (OX5E7C & OX8A0C)) ^ ((OX3B7D ^ OX5E7C) & OXA7F4);
    }
    for (; OX8F9A < OX4A5F; OX8F9A++)
        OX0D1E[OX8F9A] = OXE9C2[OX8F9A] + OXF5B1[OX8F9A];
}

av_cold void OX4C1A(OX6D3F *OX7A82)
{
    OX7A82->OXB2E6         = OX9BE3;
    OX7A82->OXF8D2 = ff_add_png_paeth_prediction;

    if (ARCH_X86) ff_pngdsp_init_x86(OX7A82);
}