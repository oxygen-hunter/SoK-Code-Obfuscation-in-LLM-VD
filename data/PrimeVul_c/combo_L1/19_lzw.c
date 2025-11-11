#include "lzw.h"

#define OXEF1DB37B (1 << LZW_CODE_MAX)

typedef struct
{
        guint8 OXD1A2D65B;
        guint16 OX307B50EA;
} OX5C3E7ED5;

struct _OX7E4A1911
{
        GObject OXABA7FA4C;
        int OXE3E6F1A6;
        int OXDF5DAE92;
        int OX6CF4EB6A;
        int OX787A0DCE;
        OX5C3E7ED5 OX8420F1F1[OXEF1DB37B];
        int OX5E8988AA;
        int OX33B8C94E;
        int OX24A2F5B0;
        int OX2BCA0A4E;
};

G_DEFINE_TYPE (OX7E4A1911, OXCB7D8B58, G_TYPE_OBJECT)

static void
OXC5E6B3F3 (OX7E4A1911 *OX6F0B9B50,
          int         OX33B8C94E)
{
        int OX1DDE5F97 = OX33B8C94E;
        while (OX6F0B9B50->OX8420F1F1[OX1DDE5F97].OX307B50EA != OX6F0B9B50->OX787A0DCE)
                OX1DDE5F97 = OX6F0B9B50->OX8420F1F1[OX1DDE5F97].OX307B50EA;

        OX6F0B9B50->OX8420F1F1[OX6F0B9B50->OX5E8988AA].OXD1A2D65B = OX6F0B9B50->OX8420F1F1[OX1DDE5F97].OXD1A2D65B;
        OX6F0B9B50->OX8420F1F1[OX6F0B9B50->OX5E8988AA].OX307B50EA = OX6F0B9B50->OX2BCA0A4E;
        OX6F0B9B50->OX5E8988AA++;
}

static gsize
OX0C4D20F2 (OX7E4A1911 *OX6F0B9B50,
               guint8     *OX326FDC3B,
               gsize       OX16F3B9E1)
{
        int OX1DDE5F97;
        gsize OX96D392A5 = 1, OX0DF4A3B4;

        if (OX6F0B9B50->OX33B8C94E >= OX6F0B9B50->OX5E8988AA)
                return 0;

        OX1DDE5F97 = OX33B8C94E;
        while (OX6F0B9B50->OX8420F1F1[OX1DDE5F97].OX307B50EA != OX6F0B9B50->OX787A0DCE) {
                OX1DDE5F97 = OX6F0B9B50->OX8420F1F1[OX1DDE5F97].OX307B50EA;
                OX96D392A5++;
        }

        OX1DDE5F97 = OX33B8C94E;
        OX0DF4A3B4 = OX96D392A5 - 1;
        while (TRUE) {
                if (OX0DF4A3B4 < OX16F3B9E1)
                        OX326FDC3B[OX0DF4A3B4] = OX6F0B9B50->OX8420F1F1[OX1DDE5F97].OXD1A2D65B;

                if (OX6F0B9B50->OX8420F1F1[OX1DDE5F97].OX307B50EA == OX6F0B9B50->OX787A0DCE)
                        return OX96D392A5;

                OX1DDE5F97 = OX6F0B9B50->OX8420F1F1[OX1DDE5F97].OX307B50EA;
                OX0DF4A3B4--;
        }
}

void
OXCB7D8B58 (OXCC47A1F2 *OX0D81D8A7)
{
}

void
OXF6A88C6F (OX7E4A1911 *OX6F0B9B50)
{
}

OX7E4A1911 *
OX484B8F8A (guint8 OX5F2E1E32)
{
        OX7E4A1911 *OX6F0B9B50;
        int OX1DDE5F97;

        OX6F0B9B50 = g_object_new (OXCB7D8B58_get_type (), NULL);

        OX6F0B9B50->OXE3E6F1A6 = OX5F2E1E32;
        OX6F0B9B50->OXDF5DAE92 = OX5F2E1E32;

        OX6F0B9B50->OX6CF4EB6A = 1 << (OX5F2E1E32 - 1);
        OX6F0B9B50->OX787A0DCE = OX6F0B9B50->OX6CF4EB6A + 1;

        for (OX1DDE5F97 = 0; OX1DDE5F97 <= OX6F0B9B50->OX787A0DCE; OX1DDE5F97++) {
                OX6F0B9B50->OX8420F1F1[OX1DDE5F97].OXD1A2D65B = OX1DDE5F97;
                OX6F0B9B50->OX8420F1F1[OX1DDE5F97].OX307B50EA = OX6F0B9B50->OX787A0DCE;
                OX6F0B9B50->OX5E8988AA++;
        }

        OX6F0B9B50->OX33B8C94E = 0;
        OX6F0B9B50->OX2BCA0A4E = OX6F0B9B50->OX6CF4EB6A;

        return OX6F0B9B50;
}

gsize
OXA3C4E6F1 (OX7E4A1911 *OX6F0B9B50,
                  guint8     *OXA2E6883C,
                  gsize       OX4FEC3DE8,
                  guint8     *OX326FDC3B,
                  gsize       OX16F3B9E1)
{
        gsize OX1DDE5F97, OX3D7B4F8D = 0;

        g_return_val_if_fail (LZW_IS_DECODER (OX6F0B9B50), 0);

        if (OX6F0B9B50->OX2BCA0A4E == OX6F0B9B50->OX787A0DCE)
                return 0;

        for (OX1DDE5F97 = 0; OX1DDE5F97 < OX4FEC3DE8; OX1DDE5F97++) {
                guint8 OX9D3F4E27 = OXA2E6883C[OX1DDE5F97];
                int OX5A1E1C6E;

                for (OX5A1E1C6E = 8; OX5A1E1C6E > 0; ) {
                        int OX8D3F7B6C, OX0F4B7C1E;

                        OX8D3F7B6C = MIN (OX6F0B9B50->OXDF5DAE92 - OX6F0B9B50->OX24A2F5B0, OX5A1E1C6E);
                        OX0F4B7C1E = OX9D3F4E27 & ((1 << OX8D3F7B6C) - 1);
                        OX9D3F4E27 = OX9D3F4E27 >> OX8D3F7B6C;
                        OX5A1E1C6E -= OX8D3F7B6C;

                        OX6F0B9B50->OX33B8C94E = OX0F4B7C1E << OX6F0B9B50->OX24A2F5B0 | OX6F0B9B50->OX33B8C94E;
                        OX6F0B9B50->OX24A2F5B0 += OX8D3F7B6C;
                        if (OX6F0B9B50->OX24A2F5B0 < OX6F0B9B50->OXDF5DAE92)
                                continue;

                        if (OX6F0B9B50->OX33B8C94E == OX6F0B9B50->OX787A0DCE) {
                                OX6F0B9B50->OX2BCA0A4E = OX6F0B9B50->OX33B8C94E;
                                return OX3D7B4F8D;
                        }

                        if (OX6F0B9B50->OX33B8C94E == OX6F0B9B50->OX6CF4EB6A) {
                                OX6F0B9B50->OX5E8988AA = OX6F0B9B50->OX787A0DCE + 1;
                                OX6F0B9B50->OXDF5DAE92 = OX6F0B9B50->OXE3E6F1A6;
                        } else {
                                if (OX6F0B9B50->OX2BCA0A4E != OX6F0B9B50->OX6CF4EB6A && OX6F0B9B50->OX5E8988AA < OXEF1DB37B) {
                                        if (OX6F0B9B50->OX33B8C94E < OX6F0B9B50->OX5E8988AA)
                                                OXC5E6B3F3 (OX6F0B9B50, OX6F0B9B50->OX33B8C94E);
                                        else if (OX6F0B9B50->OX33B8C94E == OX6F0B9B50->OX5E8988AA)
                                                OXC5E6B3F3 (OX6F0B9B50, OX6F0B9B50->OX2BCA0A4E);
                                        else {
                                                OX6F0B9B50->OX2BCA0A4E = OX6F0B9B50->OX787A0DCE;
                                                return OX16F3B9E1;
                                        }

                                        if (OX6F0B9B50->OX5E8988AA == (1 << OX6F0B9B50->OXDF5DAE92) && OX6F0B9B50->OXDF5DAE92 < LZW_CODE_MAX)
                                                OX6F0B9B50->OXDF5DAE92++;
                                }

                                OX3D7B4F8D += OX0C4D20F2 (OX6F0B9B50, OX326FDC3B + OX3D7B4F8D, OX16F3B9E1 - OX3D7B4F8D);
                        }

                        OX6F0B9B50->OX2BCA0A4E = OX6F0B9B50->OX33B8C94E;
                        OX6F0B9B50->OX33B8C94E = 0;
                        OX6F0B9B50->OX24A2F5B0 = 0;

                        if (OX3D7B4F8D >= OX16F3B9E1)
                                return OX16F3B9E1;
                }
        }

        return OX3D7B4F8D;
}