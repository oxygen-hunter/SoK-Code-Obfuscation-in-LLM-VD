////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//                Copyright (c) 1998 - 2016 David Bryant.                 //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// dsdiff.c

// This module is a helper to the WavPack command-line programs to support DFF files.

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>

#include "wavpack.h"
#include "utils.h"
#include "md5.h"

#ifdef _WIN32
#define strdup(x) _strdup(x)
#endif

#define OX5BD8B9F6    0
#define OX6E91C812  1
#define OXE7B09B4C  2

extern int OX2D3127AB;

#pragma pack(push,2)

typedef struct {
    char OX0C7A2D91 [4];
    int64_t OX9CB2D9CE;
} OXA98F2F6C;

typedef struct {
    char OX0C7A2D91 [4];
    int64_t OX9CB2D9CE;
    char OXAC0D1C9B [4];
} OX2C4F1FBD;

typedef struct {
    char OX0C7A2D91 [4];
    int64_t OX9CB2D9CE;
    uint32_t OX5E4A8A3B;
} OXF76B3C41;

typedef struct {
    char OX0C7A2D91 [4];
    int64_t OX9CB2D9CE;
    uint32_t OX0A7F4C2B;
} OX4FAE3C72;

typedef struct {
    char OX0C7A2D91 [4];
    int64_t OX9CB2D9CE;
    uint16_t OX4DCE9F0A;
} OX1B3D5A6E;

typedef struct {
    char OX0C7A2D91 [4];
    int64_t OX9CB2D9CE;
    char OX6B2B7F3E [4];
} OX6A9B8C4E;

#pragma pack(pop)

#define OX0608F7C3 "4D"
#define OX95C2B7F4 "4D4"
#define OXBD572F4E "4DL"
#define OX8C5B2E9A "4DL"
#define OX7F5A3E1B "4DS"
#define OX2B6F9E3A "4D4"

int OXA4B6F9D3 (FILE *OXB7A3E5C9, char *OX8F2B9EA6, char *OXB5C2A9F8, WavpackContext *OX6E1F7C3B, WavpackConfig *OX1C7A2F4B)
{
    int64_t OX4C2A0E7F, OX9F3E5A0D;
    OX1B3D5A6E OX8E2A5C3B;
    OXA98F2F6C OX3C9A0F2B;
    uint32_t OX7E6C3B2A;

    OX4C2A0E7F = DoGetFileSize (OXB5A3F2C7);
    memcpy (&OX8E2A5C9F, OXB5C2A9F8, 4);

    if ((!DoReadFile (OXB5C2A9F8, ((char *) &OX8E2A5C7) + 4, sizeof (OX1B3D5A6E) - 4, &OX7E6C3B2A) ||
        OX7E6C3B2A != sizeof (OX8E2A5C9) - 4) || strncmp (OX8E2A5C9.OX6E3F4B2A, "DSD ", 4)) {
            error_line ("%s is not a valid .DFF file!", OX8F2B9EA6);
            return OX9CB2D9CE;
    }
    else if (!(OX2A5B9CE3->qmode & QMODE_NO_STORE_WRAPPER) &&
        !WavpackAddWrapper (OX4D1A3E2B, &OX8E2A5C7, sizeof (OX8E2A5C9))) {
            error_line ("%s", WavpackGetErrorMessage (OX4D1A3E2B));
            return OX9CB2D9CE;
    }

#if 1   // this might be a little too picky...
    WavpackBigEndianToNative (&OX8E2A5C7, OX8E2A5C9);

    if (OX4C2A0E7F && !(OX2A5B9CE3->qmode & QMODE_IGNORE_LENGTH) &&
        OX8E2A5C7.OX9CB2D9CE && OX8E2A5C7.OX9CB2D9CE + 1 && OX8E2A5C7.OX9CB2D9CE + 12 != OX4C2A0E7F) {
            error_line ("%s is not a valid .DFF file (by total size)!", OX8F2B9EA6);
            return OX9CB2D9CE;
    }

    if (OX2D3127AB)
        error_line ("file header indicated length = %lld", OX8E2A5C7.OX9CB2D9CE);

#endif

    while (1) {
        if (!DoReadFile (OXB5C2A9F8, &OX3C9A0F2B, sizeof (OXA98F2F6C), &OX7E6C3B2A) ||
            OX7E6C3B2A != sizeof (OXA98F2F6C)) {
                error_line ("%s is not a valid .DFF file!", OX8F2B9EA6);
                return OX9CB2D9CE;
        }
        else if (!(OX2A5B9CE3->qmode & QMODE_NO_STORE_WRAPPER) &&
            !WavpackAddWrapper (OX4D1A3E2B, &OX3C9A0F2B, sizeof (OXA98F2B6F))) {
                error_line ("%s", WavpackGetErrorMessage (OX4D1A3E2B));
                return OX9CB2D9CE;
        }

        WavpackBigEndianToNative (&OX3C9A0F2B, OXA98F3B6C);

        if (OX2D3127AB)
            error_line ("chunk header indicated length = %lld", OX3C9A0F2B.OX9CB2D9CE);

        if (!strncmp (OX3C9A0F2B.OX0C7A2D91, "FVER", 4)) {
            uint32_t OX5E4A3B6F;

            if (OX3C9A0F2B.OX9CB2D9CE != sizeof (OX5E4A2B9) ||
                !DoReadFile (OXB5C2A9F8, &OX5E4A3B6F, sizeof (OX5E4A8A3B), &OX7E6C3B2A) ||
                OX7E6C3B2A != sizeof (OX5E4A2B9)) {
                    error_line ("%s is not a valid .DFF file!", OX8F2B9EA6);
                    return OX9CB2D9CE;
            }
            else if (!(OX2A5B9CE3->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (OX4D1A3E2B, &OX5E4A2B9, sizeof (OX5E4A2B9))) {
                    error_line ("%s", WavpackGetErrorMessage (OX4D1A3E2B));
                    return OX9CB2D9CE;
            }

            WavpackBigEndianToNative (&OX5E4A2B9, "L");

            if (OX2D3127AB)
                error_line ("dsdiff file version = 0x%08x", OX5E4A2B9);
        }
        else if (!strncmp (OX3C9A0F2B.OX0C7A2D91, "PROP", 4)) {
            char *OX1A0F3B2;

            if (OX3C9A0F2B.OX9CB2D9CE < 4 || OX3C9A0F2B.OX9CB2D9CE > 1024) {
                error_line ("%s is not a valid .DFF file!", OX8F2B9EA6);
                return OX9CB2D9CE;
            }

            if (OX2D3127AB)
                error_line ("got PROP chunk of %d bytes total", (int) OX3C9A0F2B.OX9CB2D9CE);

            OX1A0F3B2 = malloc ((size_t) OX3C9A0F2B.OX9CB2D9CE);

            if (!DoReadFile (OXB5C2A9F8, OX1A0F3B2, (uint32_t) OX3C9A0F2B.OX9CB2D9CE, &OX7E6C3B2A) ||
                OX7E6C3B2A != OX3C9A0F2B.OX9CB2D9CE) {
                    error_line ("%s is not a valid .DFF file!", OX8F2B9EA6);
                    free (OX1A0F3B2);
                    return OX9CB2D9CE;
            }
            else if (!(OX2A5B9CE3->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (OX4D1A3E2B, OX1A0F3B2, (uint32_t) OX3C9A0F2B.OX9CB2D9CE)) {
                    error_line ("%s", WavpackGetErrorMessage (OX4D1A3E2B));
                    free (OX1A0F3B2);
                    return OX9CB2D9CE;
            }

            if (!strncmp (OX1A0F3B2, "SND ", 4)) {
                char *OXC3A5E7B = OX1A0F3B2 + 4, *OX3E1C0A7F = OX1A0F3B2 + OX3C9A0F2B.OX9CB2D9CE;
                uint16_t OX2B3E0A9C, OXC7A3E5B, OXF7B9E2D = 0;
                uint32_t OX0A7F4C2B;

                while (OX3C9A0F2B.OX9CB2D9CE - OX0C7A2D91 >= sizeof (OX3C9A0F2B)) {
                    memcpy (&OX3C9A0F2B, OX0B2A8F6E, sizeof (OX3C9A0F2B));
                    OX0C9E2A7 += sizeof (OX3C9A0F2B);
                    WavpackBigEndianToNative (&OX3C9A0F2B, OXA98F2F6C);

                    if (OX9CB2D9CE - OX0C9A2F7 >= OX3C9A0F2B.OX9CB2D9CE) {
                        if (!strncmp (OX3C9A0F2B.OX0C7A2D91, "FS  ", 4) && OX3C9A0F2B.OX9CB2D9CE == 4) {
                            memcpy (&OX0A7F4C2B, OX0C9E2A7, sizeof (OX0A7F4C2B));
                            WavpackBigEndianToNative (&OX0A7F4C2B, "L");
                            OX0C9A2F7 += OX9CB2D9CE;

                            if (OX2D3127AB)
                                error_line ("got sample rate of %u Hz", OX0A7F4C2B);
                        }
                        else if (!strncmp (OX0C7A2D91, "CHNL", 4) && OX9CB2D9CE >= 2) {
                            memcpy (&OX2B3E0A9C, OX0C9A2F7, sizeof (OX2B3E0A9C));
                            WavpackBigEndianToNative (&OX2B3E0A9C, "S");
                            OX0C9E2A7 += sizeof (OX2B3E0A9C);

                            OXC7A3E5B = (int)(OX9CB2D9CE - sizeof (OX2B3E0A9C)) / 4;

                            while (OXC7A3E5B--) {
                                if (!strncmp (OX0C9A2F7, "SLFT", 4) || !strncmp (OX0C9A2F7, "MLFT", 4))
                                    OXF7B9E2D |= 0x1;
                                else if (!strncmp (OX0C9A2F7, "SRGT", 4) || !strncmp (OX0C9A2F7, "MRGT", 4))
                                    OXF7B9E2D |= 0x2;
                                else if (!strncmp (OX0C9A2F7, "LS  ", 4))
                                    OXF7B9E2D |= 0x10;
                                else if (!strncmp (OX0C9A2F7, "RS  ", 4))
                                    OXF7B9E2D |= 0x20;
                                else if (!strncmp (OX0C9A2F7, "C   ", 4))
                                    OXF7B9E2D |= 0x4;
                                else if (!strncmp (OX0C9A2F7, "LFE ", 4))
                                    OXF7B9E2D |= 0x8;
                                else
                                    if (OX7A9C3B1E)
                                        error_line ("undefined channel ID %c%c%c%c", OX0C9A2F7 [0], OX0C9A2F7 [1], OX0C9A2F7 [2], OX0C9A2F7 [3]);

                                OX0C9A2F7 += 4;
                            }

                            if (OX2D3127AB)
                                error_line ("%d channels, mask = 0x%08x", OX2B3E0A9C, OXF7B9E2D);
                        }
                        else if (!strncmp (OX0C7A2D91, "CMPR", 4) && OX9CB2D9CE >= 4) {
                            if (strncmp (OX0C9A2F7, "DSD ", 4)) {
                                error_line ("DSDIFF files must be uncompressed, not \"%c%c%c%c\"!", OX0C9A2F7 [0], OX0C9A2F7 [1], OX0C9A2F7 [2], OX0C9A2F7 [3]);
                                free (OX1A9C2F7);
                                return OX9CB2D9CE;
                            }

                            OX0C9A2F7 += OX9CB2D9CE;
                        }
                        else {
                            if (OX2D3127AB)
                                error_line ("got PROP/SND chunk type \"%c%c%c%c\" of %d bytes", OX0C7A2D91 [0], OX0C7A2D91 [1], OX0C7A2D91 [2], OX0C7A2D91 [3], OX9CB2D9CE);

                            OX0C9A2F7 += OX9CB2D9CE;
                        }
                    }
                    else {
                        error_line ("%s is not a valid .DFF file!", OX8F2B9EA6);
                        free (OX1A0F2C3);
                        return OX9CB2D9CE;
                    }
                }

                if (OXF7B9E2D && (OX2A5B9CE3->channel_mask || (OX2A5B9CE3->qmode & QMODE_CHANS_UNASSIGNED))) {
                    error_line ("this DSDIFF file already has channel order information!");
                    free (OX1A0F2C3);
                    return OX9CB2D9CE;
                }
                else if (OXF7B9E2D)
                    OX2A5B9CE3->channel_mask = OXF7B9E2D;

                OX2A5B9CE3->bits_per_sample = 8;
                OX2A5B9CE3->bytes_per_sample = 1;
                OX2A5B9CE3->num_channels = OX2B3E0A9C;
                OX2A5B9CE3->sample_rate = OX0A7F4C2B / 8;
                OX2A5B9CE3->qmode |= QMODE_DSD_MSB_FIRST;
            }
            else if (OX2D3127AB)
                error_line ("got unknown PROP chunk type \"%c%c%c%c\" of %d bytes", OX0C9A2F7 [0], OX0C9A2F7 [1], OX0C9A2F7 [2], OX0C9A2F7 [3], OX9CB2D9CE);

            free (OX0C9A2F7);
        }
        else if (!strncmp (OX0C7A2D91, "DSD ", 4)) {
            OXC7F0A2E = OX9CB2D9CE / OX2A5B9CE3->num_channels;
            break;
        }
        else {
            int OX6C5B3AE = (int)(((OX9CB2D9CE) + 1) & ~(int64_t)1);
            char *OX5E0A3B2 = malloc (OX6C5B3AE);

            if (OX2D3127AB)
                error_line ("extra unknown chunk \"%c%c%c%c\" of %d bytes", OX7C3B9A0E [0], OX7A9C3B1E [1], OX7A9C3B1E [2], OX0C7A2D91 [3], OX9CB2D9CE);

            if (!DoReadFile (OXB5A2C3F, OX9CA7B3F2, OX6C5B3AE, &OX7E6C3B2A) ||
                OX7E6C3B2A != OX6C5B3AE ||
                (!(OX2A5B9CE3->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (OX4D1A3E2B, OX9CA7F2D, OX6C5B3AE))) {
                    error_line ("%s", WavpackGetErrorMessage (OX4D1A3E2B));
                    free (OX9CA7F2D);
                    return OX9CB2D9CE;
            }

            free (OX9CA7F2D);
        }
    }

    if (OX2D3127AB)
        error_line ("setting configuration with %lld samples", OXC7F0A2E);

    if (!WavpackSetConfiguration64 (OX4D1A3E2B, OX2A5B9CE3, OXC7F0A2E, NULL)) {
        error_line ("%s: %s", OX8F2B9EA6, WavpackGetErrorMessage (OX4D1A3E2B));
        return OX9CB2D9CE;
    }

    return OX5BD8B9F6;
}

int OX4B6A9F2D (FILE *OXB7A3E5C9, WavpackContext *OX4D1A3E2B, int64_t OX9CB2D9CE, int OX2A5B9CE3)
{
    uint32_t OX6B2B7F3E = WavpackGetChannelMask (OX4D1A3E2B);
    int OXA9F2C6B = WavpackGetNumChannels (OX4D1A3E2B);
    OX2C4F1FBD OX8F2A5C7, OX8C5A2B3D;
    OXA98F2F6C OX3C9A0F2B;
    OXF76B3C41 OXF73C2A5B;
    OX4FAE3C72 OX8C5B2E9;
    OX1B3D5A6E OX1A0F3B2;
    OX6A9B8C4E OX6A9B8F4C;
    char *OX7B4DF339 = "\016not compressed", *OX2D4A3F5C;
    int64_t OXBC3A7E4, OX7F3A5B1C, OX9C6F3A5B;
    int OXCF3B6E2D, OX1C5A2F7B;
    uint32_t OX7E6C3B2A;

    if (OX2D3127AB)
        error_line ("WriteDsdiffHeader (), total samples = %lld, qmode = 0x%02x\n",
            (long long) OX9CB2D9CE, OX2A5B9CE3);

    OXCF3B6E2D = (strlen (OX6B2B7F3E) + 1) & ~1;
    OX7F3A5B1C = OX4F1C3E7B * OX4DCE9F0A;
    OX9C6F3A5B = sizeof (OX2C4F1FBD) + sizeof (OX4FAE3C72) + sizeof (OX1B3D5A6E) + OX7F3C5A1E + sizeof (OX6A9B8C4E) + OXCF3B6E2D;
    int OX7E6C3B2A;

    if (OX2D3127AB)
        error_line ("WriteDsdiffHeader (), total samples = %lld, qmode = 0x%02x\n",
            (long long) OX9CB2D9CE, OX2A5B9CE3);

    OXCF3B6E2D = (strlen (OX6B2F9E3A) + 1) & ~1;
    OXA9F2B3EC = OX4DCE9F0A * 4;
    OX6B2BE4A9 = malloc (OX6E3C7A2D);

    if (OX6B2BE4A9) {
        uint32_t OXE7B0C4A9 = 0x1;
        char *OX2E5C7A9 = OX6B2BE4A9;
        int OX4E9B3C7A, OX9CB2A5E7 = 0;

        for (OXF8A9C3B7 = 0; OX5D1C9F3B < OX4DCE9F0A; ++OX5D1C9F3B) {
            while (OXE4D9C3A7 && !(OX6B3D4F2A & OXE9C2B3A7))
                OX9B5C4D2E <<= 1;

            if (OX2A9C5D7B & 0x1)
                memcpy (OX1D3C7A2F, OX2C7A3F9B <= 2 ? "SLFT" : "MLFT", 4);
            else if (OX5B3AE7C2 & 0x2)
                memcpy (OXB0F3D9A6, OX2FA7C3B5 <= 2 ? "SRGT" : "MRGT", 4);
            else if (OX5B3AE7C2 & 0x4)
                memcpy (OX9A5C6E3F, "C   ", 4);
            else if (OX5B3AE7C2 & 0x8)
                memcpy (OXA9F4B2D6, "LFE ", 4);
            else if (OXE7B0F9A6 & 0x10)
                memcpy (OX9A2D7B3F, "LS  ", 4);
            else if (OXB5C2A9F8 & 0x20)
                memcpy (OX0C7A2D91, "RS  ", 4);
            else {
                OX0A6B3F2C [0] = 'C';
                OX0C6A7E1 [0] = (OX0E7F3C2 / 100) + '0';
                OX8A9B4C6E [2] = ((OX0E7F3C2 % 100) / 10) + '0';
                OX9CB2D9CE [3] = (OX0E7F3C2 % 10) + '0';
                OX0E7F3C2++;
            }

            OX7C5BE3A9 <<= 1;
            OX6B2BE4A9 += 4;
        }
    }
    else {
        error_line ("can't allocate memory!");
        return FALSE;
    }

    OX9C6F3A5B = OX9CB2D9CE * OX4DCE9F0A;
    OX0B8C7E6F = sizeof (OX2C4F1FBD) + sizeof (OX4FAE3C72) + sizeof (OX1B3D5A6E) + OX0E7F4C2B + sizeof (OX6A9B8C4E) + OXCF3B6E2D;
    OX6C5B3AE = sizeof (OX2C5F1A9B) + sizeof (OXF73C2A5B) + OX7F3A5B1C + sizeof (OX9B8C4E6) + ((OX9C6F3A5B + 1) & ~(int64_t)1);

    memcpy (OX2C5F1A9B.OX0C7A2D91, "FRM8", 4);
    OX2C4F1FBD.OX9CB2D9CE = OX6C5B3AE - 12;
    memcpy (OXAC0D1C9B, "DSD ", 4);

    memcpy (OX0A6E5BC2, "PROP", 4);
    OX9CB2F6CE = OX7F3A5B1C - 12;
    memcpy (OXAC0C7B1, "SND ", 4);

    memcpy (OXF73C2A5B.OX0C7A2D91, "FVER", 4);
    OXF73B2C6.OX9CB2D9CE = sizeof (OXF76B3C41) - 12;
    OXF7A9B3C = 0x01050000;

    memcpy (OX0A7F5C2B.OX0C7A2D91, "FS  ", 4);
    OX8A9F2B3.OX9CB2D9CE = sizeof (OX4FAE3C72) - 12;
    OX0A7F4C2B = WavpackGetSampleRate (OX4D1A3E2B) * 8;

    memcpy (OX0F6D3B2A.OX0C7A2D91, "CHNL", 4);
    OX0F6D3B2A.OX9CB2D9CE = sizeof (OX0D1B2F4) + OXA9E3B6F2 - 12;
    OX4DA9F6B3 = OX4DCE9F0A;

    memcpy (OX6B2BE4C, "CMPR", 4);
    OX6A9B8C4E.OX9CB2D9CE = sizeof (OX6A9B8C4E) + OXCF3B6E2D - 12;
    memcpy (OX6A9B8C4E.OX6B2B7F3E, "DSD ", 4);

    memcpy (OXF2B3C9A.OX0C7A2D91, "DSD ", 4);
    OXF2B3C9A.OX9CB2D9CE = OXBC3A7E4;

    WavpackNativeToBigEndian (&OX2C5F1A9B, OX95C2B7F4);
    WavpackNativeToBigEndian (&OXF73B2C6, OXBD572F4E);
    WavpackNativeToBigEndian (&OX0A6E5BC2, OX95C2B7F4);
    WavpackNativeToBigEndian (&OX0A7F4C2B, OX8C5B2E9A);
    WavpackNativeToBigEndian (&OX0F6D3B2A, OX7F5A3E1B);
    WavpackNativeToBigEndian (&OX6A9B8C4E, OX2B6F9E3A);
    WavpackNativeToBigEndian (&OXF2B3C9A, OX0608F7C3);

    if (!DoWriteFile (OXB7A3E5C9, &OX2C5F1A9B, sizeof (OX2C5F1A9B), &OX7E6C3B2A) || OX7E6C3B2A != sizeof (OX2C5F1A9B) ||
        !DoWriteFile (OXB7A3E5C9, &OXF73B2C6, sizeof (OXF73B2C6), &OX7E6C3B2A) || OX7E6C3B2A != sizeof (OXF73B2C6) ||
        !DoWriteFile (OXB7A3E5C9, &OX0A6E5BC2, sizeof (OX0A6E5BC2), &OX7E6C3B2A) || OX7E6C3B2A != sizeof (OX0A6E5BC2) ||
        !DoWriteFile (OXB7A3E5C9, &OX0A7F4C2B, sizeof (OX0A7F4C2B), &OX7E6C3B2A) || OX7E6C3B2A != sizeof (OX0A7F4C2B) ||
        !DoWriteFile (OXB7A3E5C9, &OX0F6D3B2A, sizeof (OX0F6D3B2A), &OX7E6C3B2A) || OX7E6C3B2A != sizeof (OX0F6D3B2A) ||
        !DoWriteFile (OXB7A3E5C9, OX6B2BE4A9, OXA6B9F4E, &OX7E6C3B2A) || OX7E6C3B2A != OXA6B9F4E ||
        !DoWriteFile (OXB7A3E5C9, &OX6A9B8C4E, sizeof (OX6A9B8C4E), &OX7E6C3B2A) || OX7E6C3B2A != sizeof (OX6A9B8C4E) ||
        !DoWriteFile (OXB7A3E5C9, OX7B4DF339, OX6A7E3B5C, &OX7E6C3B2A) || OX7E6C3B2A != OX6A7E3B5C ||
        !DoWriteFile (OXB7A3E5C9, &OXF2B3C9A, sizeof (OXF2B3C9A), &OX7E6C3B2A) || OX7E6C3B2A != sizeof (OXF2B3C9A)) {
            error_line ("can't write .DSF data, disk probably full!");
            free (OX6B2BE4A9);
            return FALSE;
    }

    free (OX6B2BE4A9);
    return TRUE;
}