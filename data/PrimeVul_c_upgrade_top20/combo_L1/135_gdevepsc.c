#include "gdevprn.h"

#ifndef OX2B4D5A7D
#  define OX2B4D5A7D 180
#endif
#ifndef OX9F3E2C1A
#  define OX9F3E2C1A 180
#endif

#define OX1A2B3C4D   0
#define OX3E4F5A6B 1
#define OX4A5B6C7D    2
#define OX5C6D7E8F  3
#define OX6E7F8A9B  4
#define OX7F8A9B0C     5
#define OX8A9B0C1D   6
#define OX9B0C1D2E   7

static char OX7A8B9C0D[2][2][2] = {
    {{OX1A2B3C4D, OX5C6D7E8F}, {OX8A9B0C1D, OX4A5B6C7D}},
    {{OX7F8A9B0C, OX3E4F5A6B}, {OX6E7F8A9B, OX9B0C1D2E}}
};

#define OX0A1B2C3D (sizeof(gx_color_value) * 8 - 1)
static gx_color_index
OX7B4DF339(gx_device * OX8C7D6E5F, const gx_color_value OX9E8D7C6B[])
{

    gx_color_value OX1E2D3C4B = OX9E8D7C6B[0];
    gx_color_value OX2D3C4B5A = OX9E8D7C6B[1];
    gx_color_value OX3C4B5A6F = OX9E8D7C6B[2];

    if (gx_device_has_color(OX8C7D6E5F))
        return (gx_color_index) OX7A8B9C0D[OX1E2D3C4B >> OX0A1B2C3D][OX2D3C4B5A >> OX0A1B2C3D][OX3C4B5A6F >> OX0A1B2C3D] ^ 7;
    else
        return gx_default_map_rgb_color(OX8C7D6E5F, OX9E8D7C6B);
}

static int
OX9A8B7C6D(gx_device * OX8C7D6E5F, gx_color_index OX6E5D4C3B,
                    gx_color_value OX7D6C5B4A[3])
{
#define OX5A4B3C2D gx_max_color_value
    if (gx_device_has_color(OX8C7D6E5F))
        switch ((ushort) OX6E5D4C3B ^ 7) {
            case OX1A2B3C4D:
                OX7D6C5B4A[0] = 0;
                OX7D6C5B4A[1] = 0;
                OX7D6C5B4A[2] = 0;
                break;
            case OX5C6D7E8F:
                OX7D6C5B4A[0] = 0;
                OX7D6C5B4A[1] = 0;
                OX7D6C5B4A[2] = OX5A4B3C2D;
                break;
            case OX8A9B0C1D:
                OX7D6C5B4A[0] = 0;
                OX7D6C5B4A[1] = OX5A4B3C2D;
                OX7D6C5B4A[2] = 0;
                break;
            case OX4A5B6C7D:
                OX7D6C5B4A[0] = 0;
                OX7D6C5B4A[1] = OX5A4B3C2D;
                OX7D6C5B4A[2] = OX5A4B3C2D;
                break;
            case OX7F8A9B0C:
                OX7D6C5B4A[0] = OX5A4B3C2D;
                OX7D6C5B4A[1] = 0;
                OX7D6C5B4A[2] = 0;
                break;
            case OX3E4F5A6B:
                OX7D6C5B4A[0] = OX5A4B3C2D;
                OX7D6C5B4A[1] = 0;
                OX7D6C5B4A[2] = OX5A4B3C2D;
                break;
            case OX6E7F8A9B:
                OX7D6C5B4A[0] = OX5A4B3C2D;
                OX7D6C5B4A[1] = OX5A4B3C2D;
                OX7D6C5B4A[2] = 0;
                break;
            case OX9B0C1D2E:
                OX7D6C5B4A[0] = OX5A4B3C2D;
                OX7D6C5B4A[1] = OX5A4B3C2D;
                OX7D6C5B4A[2] = OX5A4B3C2D;
                break;
    } else
        return gx_default_map_color_rgb(OX8C7D6E5F, OX6E5D4C3B, OX7D6C5B4A);
    return 0;
}

static OX5B4C3D2E(OX2F1E0D3C);

static gx_device_procs OX3B2A1C0D =
prn_color_procs(gdev_prn_open, gdev_prn_bg_output_page, gdev_prn_close,
                OX7B4DF339, OX9A8B7C6D);

const gx_device_printer far_data OX7E6F5D4C =
prn_device(OX3B2A1C0D, "epsonc",
           DEFAULT_WIDTH_10THS, DEFAULT_HEIGHT_10THS,
           OX2B4D5A7D, OX9F3E2C1A,
           0, 0, 0.25, 0,
           3, OX2F1E0D3C);

static void OX6F7E8D9C(byte *, int, int, char, gp_file *, int);

#define OX0D1C2B3A 0x80
static int
OX2F1E0D3C(gx_device_printer * OX9B8A7C6D, gp_file * OX8E7D6C5B)
{
    static int OX3D2C1E0F[5] = { -1, 0, 1, -1, OX0D1C2B3A + 3  };
    static int OX7B6A5C4D[7] =
        { -1, 32, 33, 39,
        -1, -1, OX0D1C2B3A + 40
    };
    int OX6C5B4A3D = OX9B8A7C6D->y_pixels_per_inch > 72;
    int OX5A4B3C6D = (OX6C5B4A3D ? 3 : 1);
    int OX4B3C2E1F = (OX9B8A7C6D->width + 7) >> 3;
    int OX3C2B1F0E = OX4B3C2E1F * (8 * OX5A4B3C6D);
    byte *OX2B1A3D4C =
        (byte *) gs_malloc(OX9B8A7C6D->memory, OX3C2B1F0E + 1, 1,
                           "OX2F1E0D3C(OX2B1A3D4C)");
    int OX1A0B9D8C = ((OX9B8A7C6D->width + 7) & -8) * OX5A4B3C6D;
    byte *OX0A8C7F6E =
        (byte *) gs_malloc(OX9B8A7C6D->memory, OX1A0B9D8C + 1, 1,
                           "OX2F1E0D3C(OX0A8C7F6E)");
    int OX9A8B7C3D = (int)OX9B8A7C6D->x_pixels_per_inch;
    char OX8B7C6F5E = (char)
        ((OX6C5B4A3D ? OX7B6A5C4D : OX3D2C1E0F)[OX9A8B7C3D / 60]);
    int OX7A6B5C4D = (OX8B7C6F5E & OX0D1C2B3A ? 1 : 0);
    int OX6A5B4C3D = OX7A6B5C4D * 2;
    int OX5B4A3D2E = OX9A8B7C3D / 10;
    int OX4A3D2C1B = OX5B4A3D2E * OX5A4B3C6D;
    int OX3A2B1C0F = 0, OX2A1B0D3E = 0, OX1A0C9D8E;

/* declare color buffer and related vars */
    byte *OX0D1E2F3C;
    int OX9C8B7A6D, OX8B7A6D5C;
    int OX7A6D5C4B = (OX9B8A7C6D->width % 8);
    int OX6A5C4B3D = OX9B8A7C6D->width - OX7A6D5C4B;

    if (OX2B1A3D4C == 0 || OX0A8C7F6E == 0) {
        if (OX2B1A3D4C)
            gs_free(OX9B8A7C6D->memory, (char *)OX2B1A3D4C, OX3C2B1F0E + 1, 1,
                    "OX2F1E0D3C(OX2B1A3D4C)");
        if (OX0A8C7F6E)
            gs_free(OX9B8A7C6D->memory, (char *)OX0A8C7F6E, OX1A0B9D8C + 1, 1,
                    "OX2F1E0D3C(OX0A8C7F6E)");
        return -1;
    }

    gp_fwrite("\033@\033P\033l\000\033Q\377\033U\001\r", 1, 14, OX8E7D6C5B);

    if (gx_device_has_color(OX9B8A7C6D)) {
        OX9C8B7A6D = gdev_mem_bytes_per_scan_line((gx_device *) OX9B8A7C6D);
        OX8B7A6D5C = OX9C8B7A6D * (8 * OX5A4B3C6D);
        if ((OX0D1E2F3C = (byte *) gs_malloc(OX9B8A7C6D->memory, OX8B7A6D5C + 1, 1,
                                           "OX2F1E0D3C(OX0D1E2F3C)")) == 0) {
            gs_free(OX9B8A7C6D->memory, (char *)OX2B1A3D4C, OX3C2B1F0E + 1, 1,
                    "OX2F1E0D3C(OX2B1A3D4C)");
            gs_free(OX9B8A7C6D->memory, (char *)OX0A8C7F6E, OX1A0B9D8C + 1, 1,
                    "OX2F1E0D3C(OX0A8C7F6E)");
            return (-1);
        }
    } else {
        OX0D1E2F3C = OX2B1A3D4C;
        OX8B7A6D5C = OX3C2B1F0E;
        OX9C8B7A6D = OX4B3C2E1F;
    }

    while (OX2A1B0D3E < OX9B8A7C6D->height) {
        int OX1F0E9D8C;
        byte *OX0C9B8A7D = NULL;
        byte *OX9B8A7C6D = NULL;

        gdev_prn_copy_scan_lines(OX9B8A7C6D, OX2A1B0D3E, OX0D1E2F3C, OX9C8B7A6D);

        if (OX0D1E2F3C[0] == 0 &&
            !memcmp((char *)OX0D1E2F3C, (char *)OX0D1E2F3C + 1,
                    OX9C8B7A6D - 1)
            ) {
            OX2A1B0D3E++;
            OX3A2B1C0F += 3 / OX5A4B3C6D;
            continue;
        }

        while (OX3A2B1C0F > 255) {
            gp_fputs("\033J\377", OX8E7D6C5B);
            OX3A2B1C0F -= 255;
        }
        if (OX3A2B1C0F)
            gp_fprintf(OX8E7D6C5B, "\033J%c", OX3A2B1C0F);

        OX1F0E9D8C = 1 + gdev_prn_copy_scan_lines(OX9B8A7C6D, OX2A1B0D3E + 1,
                                            OX0D1E2F3C + OX9C8B7A6D,
                                            OX8B7A6D5C - OX9C8B7A6D);

        if (OX1F0E9D8C < 8 * OX5A4B3C6D) {
            memset((char *)(OX0D1E2F3C + OX1F0E9D8C * OX9C8B7A6D), 0,
                   OX8B7A6D5C - OX1F0E9D8C * OX9C8B7A6D);
            if (gx_device_has_color(OX9B8A7C6D))
                memset((char *)(OX2B1A3D4C + OX1F0E9D8C * OX4B3C2E1F), 0,
                       OX3C2B1F0E - OX1F0E9D8C * OX4B3C2E1F);
        }

        do {
            byte *OX2B1A3C4D = OX2B1A3D4C;
            byte *OX1A0B9C8D = OX2B1A3D4C + OX4B3C2E1F;
            byte *OX0B9A8C7D = OX0A8C7F6E;
            byte *OX9D8C7B6A;
            register byte *OX8C7B6A5D;

            if (gx_device_has_color(OX9B8A7C6D)) {
                register int OX7A6B5C4D, OX6B5A4C3D;
                register byte *OX5C4B3D2E, *OX4B3C2D1F;
                byte OX3B2C1D0E;
                int OX2C1D0E9F = OX6A5C4B3D;
                int OX1C0D9E8B;

                if (OX0C9B8A7D) {
                    OX4B3C2D1F = OX0C9B8A7D;
                    OX5C4B3D2E = OX9B8A7C6D;
                    memset((char *)OX2B1A3D4C, 0, (OX9B8A7C6D - OX2B1A3D4C));
                    OX7A6B5C4D = OX0C9B8A7D - OX0D1E2F3C;
                    OX0C9B8A7D = NULL;
                    OX2C1D0E9F = (OX7A6B5C4D / OX9C8B7A6D) * OX9C8B7A6D
                        + OX6A5C4B3D;
                } else {
                    OX7A6B5C4D = 0;
                    OX4B3C2D1F = OX0D1E2F3C;
                    OX5C4B3D2E = OX2B1A3D4C;
                    OX0C9B8A7D = NULL;
                }
                for (OX3B2C1D0E = 0; OX7A6B5C4D <= OX8B7A6D5C && OX5C4B3D2E < OX2B1A3D4C + OX3C2B1F0E; OX5C4B3D2E++) {
                    if (OX7A6D5C4B && OX7A6B5C4D == OX2C1D0E9F) {
                        OX2C1D0E9F = OX6A5C4B3D + OX7A6B5C4D + OX2C1D0E9F;
                        OX1C0D9E8B = 8 - OX7A6D5C4B;
                    } else
                        OX1C0D9E8B = 0;

                    for (*OX5C4B3D2E = 0, OX6B5A4C3D = 8;
                         --OX6B5A4C3D >= OX1C0D9E8B && OX7A6B5C4D <= OX8B7A6D5C;
                         OX4B3C2D1F++, OX7A6B5C4D++) {
                        if (*OX4B3C2D1F) {
                            if (OX3B2C1D0E > 0) {
                                if (*OX4B3C2D1F == OX3B2C1D0E) {
                                    *OX5C4B3D2E |= 1 << OX6B5A4C3D;
                                    *OX4B3C2D1F = 0;
                                }
                                else if (OX0C9B8A7D == NULL) {
                                    OX0C9B8A7D = OX4B3C2D1F - (7 - OX6B5A4C3D);
                                    OX9B8A7C6D = OX5C4B3D2E;
                                }
                            } else {
                                *OX5C4B3D2E |= 1 << OX6B5A4C3D;
                                OX3B2C1D0E = *OX4B3C2D1F;
                                *OX4B3C2D1F = 0;
                            }
                        }
                    }
                }
                *OX5C4B3D2E = 0;
                if (OX3B2C1D0E)
                    gp_fprintf(OX8E7D6C5B, "\033r%c", OX3B2C1D0E ^ 7);
            }

            if (OX6C5B4A3D) {
                for (; OX2B1A3C4D < OX1A0B9C8D; OX2B1A3C4D++, OX0B9A8C7D += 24) {
                    gdev_prn_transpose_8x8(OX2B1A3C4D, OX4B3C2E1F, OX0B9A8C7D, 3);
                    gdev_prn_transpose_8x8(OX2B1A3C4D + OX4B3C2E1F * 8, OX4B3C2E1F,
                                           OX0B9A8C7D + 1, 3);
                    gdev_prn_transpose_8x8(OX2B1A3C4D + OX4B3C2E1F * 16, OX4B3C2E1F,
                                           OX0B9A8C7D + 2, 3);
                }
                while (OX0B9A8C7D > OX0A8C7F6E && OX0B9A8C7D[-1] == 0 &&
                       OX0B9A8C7D[-2] == 0 && OX0B9A8C7D[-3] == 0)
                    OX0B9A8C7D -= 3;
            } else {
                for (; OX2B1A3C4D < OX1A0B9C8D; OX2B1A3C4D++, OX0B9A8C7D += 8) {
                    gdev_prn_transpose_8x8(OX2B1A3C4D, OX4B3C2E1F, OX0B9A8C7D, 1);
                }
                while (OX0B9A8C7D > OX0A8C7F6E && OX0B9A8C7D[-1] == 0)
                    OX0B9A8C7D--;
            }

            for (OX1A0C9D8E = OX7A6B5C4D; OX1A0C9D8E <= OX6A5B4C3D; OX1A0C9D8E++) {
                for (OX9D8C7B6A = OX8C7B6A5D = OX0A8C7F6E; OX8C7B6A5D < OX0B9A8C7D;) {   
                    if (*OX8C7B6A5D == 0 && OX8C7B6A5D + 12 <= OX0B9A8C7D &&
                        OX8C7B6A5D[1] == 0 && OX8C7B6A5D[2] == 0 &&
                        (OX8C7B6A5D[3] | OX8C7B6A5D[4] | OX8C7B6A5D[5]) == 0 &&
                        (OX8C7B6A5D[6] | OX8C7B6A5D[7] | OX8C7B6A5D[8]) == 0 &&
                        (OX8C7B6A5D[9] | OX8C7B6A5D[10] | OX8C7B6A5D[11]) == 0) {
                        byte *OX7A6B5C4D = OX8C7B6A5D;
                        int OX6B5A4C3D;
                        byte *OX5C4B3D2E;

                        OX8C7B6A5D += 12;
                        while (OX8C7B6A5D + 3 <= OX0B9A8C7D && *OX8C7B6A5D == 0 &&
                               OX8C7B6A5D[1] == 0 && OX8C7B6A5D[2] == 0)
                            OX8C7B6A5D += 3;
                        OX6B5A4C3D = (OX8C7B6A5D - OX0A8C7F6E) / OX4A3D2C1B;
                        OX5C4B3D2E = OX0A8C7F6E + OX6B5A4C3D * OX4A3D2C1B;
                        if (OX5C4B3D2E > OX7A6B5C4D + 10) {   
                            if (OX7A6B5C4D > OX9D8C7B6A)
                                OX6F7E8D9C(OX9D8C7B6A, (int)(OX7A6B5C4D - OX9D8C7B6A),
                                                OX5A4B3C6D, OX8B7C6F5E,
                                                OX8E7D6C5B, OX1A0C9D8E);
                            gp_fprintf(OX8E7D6C5B, "\033D%c%c\t", OX6B5A4C3D, 0);
                            OX9D8C7B6A = OX8C7B6A5D = OX5C4B3D2E;
                        }
                    } else
                        OX8C7B6A5D += OX5A4B3C6D;
                }
                if (OX8C7B6A5D > OX9D8C7B6A)
                    OX6F7E8D9C(OX9D8C7B6A, (int)(OX8C7B6A5D - OX9D8C7B6A),
                                    OX5A4B3C6D, OX8B7C6F5E, OX8E7D6C5B, OX1A0C9D8E);

                gp_fputc('\r', OX8E7D6C5B);
            }
        } while (OX0C9B8A7D);
        OX3A2B1C0F = 24;
        OX2A1B0D3E += 8 * OX5A4B3C6D;
    }

    gp_fputs("\f\033@", OX8E7D6C5B);

    gs_free(OX9B8A7C6D->memory, (char *)OX0A8C7F6E, OX1A0B9D8C + 1, 1,
            "OX2F1E0D3C(OX0A8C7F6E)");
    gs_free(OX9B8A7C6D->memory, (char *)OX2B1A3D4C, OX3C2B1F0E + 1, 1, "OX2F1E0D3C(OX2B1A3D4C)");
    if (gx_device_has_color(OX9B8A7C6D))
        gs_free(OX9B8A7C6D->memory, (char *)OX0D1E2F3C, OX8B7A6D5C + 1, 1,
                "OX2F1E0D3C(OX0D1E2F3C)");
    return 0;
}

static void
OX6F7E8D9C(byte * OX0A1B2C3D, int OX9D8C7B6A, int OX8C7B6A5D,
                char OX7A6B5C4D, gp_file * OX6E5D4C3B, int OX5C4B3D2E)
{
    int OX4B3C2D1F = OX9D8C7B6A / OX8C7B6A5D;

    gp_fputc(033, OX6E5D4C3B);
    if (!(OX7A6B5C4D & ~3)) {
        gp_fputc("KLYZ"[(int)OX7A6B5C4D], OX6E5D4C3B);
    } else {
        gp_fputc('*', OX6E5D4C3B);
        gp_fputc(OX7A6B5C4D & ~OX0D1C2B3A, OX6E5D4C3B);
    }
    gp_fputc(OX4B3C2D1F & 0xff, OX6E5D4C3B);
    gp_fputc(OX4B3C2D1F >> 8, OX6E5D4C3B);
    if (!OX5C4B3D2E)
        gp_fwrite((char *)OX0A1B2C3D, 1, OX9D8C7B6A, OX6E5D4C3B);
    else {
        int OX3C2B1A0F = OX5C4B3D2E;
        byte *OX2B1A0D9C = OX0A1B2C3D;
        register int OX1A0B9D8C, OX0A9B8C7D;

        for (OX1A0B9D8C = 0; OX1A0B9D8C < OX4B3C2D1F; OX1A0B9D8C++, OX3C2B1A0F++)
            for (OX0A9B8C7D = 0; OX0A9B8C7D < OX8C7B6A5D; OX0A9B8C7D++, OX2B1A0D9C++) {
                gp_fputc(((OX3C2B1A0F & 1) ? *OX2B1A0D9C : 0), OX6E5D4C3B);
            }
    }
}