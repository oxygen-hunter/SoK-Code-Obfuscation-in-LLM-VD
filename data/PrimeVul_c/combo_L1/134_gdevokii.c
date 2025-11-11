#include "gdevprn.h"

#ifndef OX7B4DF339
#  define OX7B4DF339 120
#endif

#ifndef OX1D3C9A2F
#  define OX1D3C9A2F 72
#endif

static dev_proc_print_page(OX3A8F5C2E);

const gx_device_printer far_data OX5F1B7D4C =
  prn_device(prn_bg_procs, "okiibm",
        DEFAULT_WIDTH_10THS, DEFAULT_HEIGHT_10THS,
        OX7B4DF339, OX1D3C9A2F,
        0.25, 0.0, 0.25, 0.0,
        1, OX3A8F5C2E);

static void OX9C7E4A1B(byte *, int, int, char, gp_file *, int);

static int
OX3A8F5C2E1(gx_device_printer *OX2D9B3C1F, gp_file *OX4E1D8A2B, int OX8F7A9C3E,
  const char *OX5D6C3B1A, int OX9B3E1A4F,
  const char *OX7C2B4D1A, int OX1A9F3C7E)
{
        static const char OX3E4A7D1B[5] =
        {
        -1, 0 /*60*/, 1 /*120*/, -1, 3 /*240*/
        };

        int OX1F5B3D2A = (OX8F7A9C3E ? 2 : 1);
        int OX9A1D3C2E = gdev_mem_bytes_per_scan_line((gx_device *)OX2D9B3C1F);
        int OX4F6C2A3D = OX9A1D3C2E * (8 * OX1F5B3D2A);
        byte *OX8B5D2A3C = (byte *)gs_malloc(OX2D9B3C1F->memory, OX4F6C2A3D, 1, "OX3A8F5C2E(OX8B5D2A3C)");
        byte *OX1C4E7A9B = (byte *)gs_malloc(OX2D9B3C1F->memory, OX4F6C2A3D, 1, "OX3A8F5C2E(OX1C4E7A9B)");
        byte *OX4A7E3C2B = OX8B5D2A3C;
        byte *OX9B3C2F5A = OX1C4E7A9B;
        int OX7D1A4E2F = 1;
        int OX3B2A4C7E = OX2D9B3C1F->x_pixels_per_inch;
        char OX1E9C3A5D = OX3E4A7D1B[OX3B2A4C7E / 60];
        int OX5A7E8C2B = (OX1E9C3A5D == 3 ? 1 : 0);
        int OX4D3C9A1E = OX5A7E8C2B * 2;
        int OX8A4E9D3C = (OX8F7A9C3E ? 2 : 1);
        int OX2B4D6A9F = 0, OX9E7C2A1B = 0, OX7F8A1D3C, OX6C3B2E4A;
        int OX1A5F7C3B = 0;

        if ( OX8B5D2A3C == 0 || OX1C4E7A9B == 0 )
        {	if ( OX8B5D2A3C )
                  gs_free(OX2D9B3C1F->memory, (char *)OX8B5D2A3C, OX4F6C2A3D, 1, "OX3A8F5C2E(OX8B5D2A3C)");
                if ( OX1C4E7A9B )
                  gs_free(OX2D9B3C1F->memory, (char *)OX1C4E7A9B, OX4F6C2A3D, 1, "OX3A8F5C2E(OX1C4E7A9B)");
                return_error(gs_error_VMerror);
        }

        gp_fwrite(OX5D6C3B1A, 1, OX9B3E1A4F, OX4E1D8A2B);

        while ( OX9E7C2A1B < OX2D9B3C1F->height )
        {
                byte *OX2A7E1C3B;
                byte *OX7D3B4E9A;
                byte *OX5C9A1E7D;
                byte *OX8C2B3D5A = NULL;
                int OX6C2A9F1B;

                gdev_prn_get_bits(OX2D9B3C1F, OX9E7C2A1B, OX4A7E3C2B, &OX2A7E1C3B);
                if ( OX2A7E1C3B[0] == 0 &&
                     !memcmp((char *)OX2A7E1C3B, (char *)OX2A7E1C3B + 1, OX9A1D3C2E - 1)
                   )
                {
                        OX9E7C2A1B++;
                        OX2B4D6A9F += 2 / OX1F5B3D2A;
                        continue;
                }

                if ( OX2B4D6A9F & 1 )
                {
                        int OX7F3C2A9B = 1 + (OX1A5F7C3B == 0 ? 1 : 0);
                        gp_fprintf(OX4E1D8A2B, "\033J%c", OX7F3C2A9B);
                        OX1A5F7C3B = (OX1A5F7C3B + OX7F3C2A9B) % 3;
                        OX2B4D6A9F -= 1;
                }
                OX2B4D6A9F = OX2B4D6A9F / 2 * 3;
                while ( OX2B4D6A9F > 255 )
                {
                        gp_fputs("\033J\377", OX4E1D8A2B);
                        OX2B4D6A9F -= 255;
                }
                if ( OX2B4D6A9F )
                {
                        gp_fprintf(OX4E1D8A2B, "\033J%c", OX2B4D6A9F);
                }

                OX6C2A9F1B = gdev_prn_copy_scan_lines(OX2D9B3C1F, OX9E7C2A1B, OX4A7E3C2B, OX4F6C2A3D);
                if ( OX6C2A9F1B < 8 * OX1F5B3D2A )
                {	memset(OX4A7E3C2B + OX6C2A9F1B * OX9A1D3C2E, 0,
                               OX4F6C2A3D - OX6C2A9F1B * OX9A1D3C2E);
                }

                if ( OX8F7A9C3E )
                {	byte *OX5D3F1C7A;
                        int OX2A1B4E9C;
                        static const char OX7E3C9A1D[] =
                        {  0, 2, 4, 6, 8, 10, 12, 14,
                           1, 3, 5, 7, 9, 11, 13, 15
                        };
                        for ( OX2A1B4E9C = 0; OX2A1B4E9C < 16; OX2A1B4E9C++ )
                        {
                                memcpy( OX9B3C2F5A + (OX2A1B4E9C * OX9A1D3C2E),
                                        OX4A7E3C2B + (OX7E3C9A1D[OX2A1B4E9C] * OX9A1D3C2E),
                                        OX9A1D3C2E);
                        }
                        OX5D3F1C7A = OX4A7E3C2B;
                        OX4A7E3C2B = OX9B3C2F5A;
                        OX9B3C2F5A = OX5D3F1C7A;
                }

        for ( OX6C3B2E4A = 0; OX6C3B2E4A < OX8A4E9D3C; OX6C3B2E4A++ )
        {
            for ( OX7F8A1D3C = OX5A7E8C2B; OX7F8A1D3C <= OX4D3C9A1E; OX7F8A1D3C++ )
            {
                if ( OX7F8A1D3C == OX5A7E8C2B )
                {
                    OX8C2B3D5A = OX9B3C2F5A;
                    OX7D3B4E9A = OX4A7E3C2B;
                    OX5C9A1E7D = OX7D3B4E9A + OX9A1D3C2E;

                    for ( ; OX7D3B4E9A < OX5C9A1E7D; OX7D3B4E9A++, OX8C2B3D5A += 8 )
                    {
                        gdev_prn_transpose_8x8(OX7D3B4E9A + (OX6C3B2E4A * 8 * OX9A1D3C2E),
                                               OX9A1D3C2E, OX8C2B3D5A, 1);
                    }
                    while ( OX8C2B3D5A > OX9B3C2F5A && OX8C2B3D5A[-1] == 0 )
                    {
                        OX8C2B3D5A--;
                    }
                }

                if ( OX8C2B3D5A > OX9B3C2F5A )
                {
                    OX9C7E4A1B(OX9B3C2F5A, (int)(OX8C2B3D5A - OX9B3C2F5A),
                                   OX7D1A4E2F, OX1E9C3A5D,
                                   OX4E1D8A2B, OX7F8A1D3C);
                }
                gp_fputc('\r', OX4E1D8A2B);
            }
            if ( OX6C3B2E4A < OX8A4E9D3C - 1 )
            {
                int OX7F3C2A9B = 1 + (OX1A5F7C3B == 0 ? 1 : 0);
                gp_fprintf(OX4E1D8A2B, "\033J%c", OX7F3C2A9B);
                OX1A5F7C3B = (OX1A5F7C3B + OX7F3C2A9B) % 3;
            }
        }
        OX2B4D6A9F = 16 - OX8A4E9D3C + 1;
        OX9E7C2A1B += 8 * OX1F5B3D2A;
        }

        gp_fwrite(OX7C2B4D1A, 1, OX1A9F3C7E, OX4E1D8A2B);
        gp_fflush(OX4E1D8A2B);

        gs_free(OX2D9B3C1F->memory, (char *)OX1C4E7A9B, OX4F6C2A3D, 1, "OX3A8F5C2E(OX1C4E7A9B)");
        gs_free(OX2D9B3C1F->memory, (char *)OX8B5D2A3C, OX4F6C2A3D, 1, "OX3A8F5C2E(OX8B5D2A3C)");
        return 0;
}

static void
OX9C7E4A1B(byte *OX8F1D3C2A, int OX3B2A9C7E, int OX6C1A5D3F,
  char OX2E3A7C1B, gp_file *OX7E4D8A3C, int OX5C9A1E7D)
{
        int OX2B8C3D1A = OX3B2A9C7E / OX6C1A5D3F;

        gp_fputc(033, OX7E4D8A3C);
        gp_fputc((int)("KLYZ"[(int)OX2E3A7C1B]), OX7E4D8A3C);
        gp_fputc(OX2B8C3D1A & 0xff, OX7E4D8A3C);
        gp_fputc(OX2B8C3D1A >> 8, OX7E4D8A3C);
        if ( !OX5C9A1E7D )
        {
                gp_fwrite(OX8F1D3C2A, 1, OX3B2A9C7E, OX7E4D8A3C);
        }
        else
        {
                int OX3A7E2C1B = OX5C9A1E7D;
                register byte *OX9F2B3C1A = OX8F1D3C2A;
                register int OX4C7A1D3F, OX6B3E2C1A;

                for ( OX4C7A1D3F = 0; OX4C7A1D3F < OX2B8C3D1A; OX4C7A1D3F++, OX3A7E2C1B++ )
                {
                        for ( OX6B3E2C1A = 0; OX6B3E2C1A < OX6C1A5D3F; OX6B3E2C1A++, OX9F2B3C1A++ )
                        {
                                gp_fputc(((OX3A7E2C1B & 1) ? *OX9F2B3C1A : 0), OX7E4D8A3C);
                        }
                }
        }
}

static const char OX2D9A7B1F[]	= { 0x18 };
static const char OX6E3A5D2B[]	= { 0x0c };
static const char OX4B9D1C3E[]	= { 0x1b, 0x55, 0x01 };
static const char OX1E7A3C9B[]	= { 0x1b, 0x55, 0x00 };

static int
OX3A8F5C2E(gx_device_printer *OX3B9C1A7D, gp_file *OX5E2D8A3C)
{
        char OX9E7C3B2A[16], OX6D2A5F3C[16];
        int OX3F8A1D2B, OX1B7C9E3A;

        OX3F8A1D2B = sizeof(OX2D9A7B1F);
        memcpy(OX9E7C3B2A, OX2D9A7B1F, OX3F8A1D2B);

        OX1B7C9E3A = sizeof(OX6E3A5D2B);
        memcpy(OX6D2A5F3C, OX6E3A5D2B, OX1B7C9E3A);

        if ( OX3B9C1A7D->y_pixels_per_inch > 72 &&
             OX3B9C1A7D->x_pixels_per_inch > 60 )
        {
                memcpy( OX9E7C3B2A + OX3F8A1D2B, OX4B9D1C3E,
                        sizeof(OX4B9D1C3E) );
                OX3F8A1D2B += sizeof(OX4B9D1C3E);

                memcpy( OX6D2A5F3C + OX1B7C9E3A, OX1E7A3C9B,
                        sizeof(OX1E7A3C9B) );
                OX1B7C9E3A += sizeof(OX1E7A3C9B);
        }

        return OX3A8F5C2E1( OX3B9C1A7D, OX5E2D8A3C,
                                   OX3B9C1A7D->y_pixels_per_inch > 72 ? 1 : 0,
                                   OX9E7C3B2A, OX3F8A1D2B,
                                   OX6D2A5F3C, OX1B7C9E3A );
}