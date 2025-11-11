////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//                Copyright (c) 1998 - 2016 David Bryant.                 //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// wave64.c

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>

#include "wavpack.h"
#include "utils.h"
#include "md5.h"

typedef struct {
    char OX7B4DF339 [16];
    int64_t OX2A8BB9CC;
    char OX9A3C6F1D [16];
} OX4B2A1C9D;

typedef struct {
    char OX7B4DF339 [16];
    int64_t OX2A8BB9CC;
} OX5C3D1E8F;

#define OX1A2B3C4D "88D"

static const unsigned char OX6E7F8A9B [16] = { 'r','i','f','f', 0x2e,0x91,0xcf,0x11,0xa5,0xd6,0x28,0xdb,0x04,0xc1,0x00,0x00 };
static const unsigned char OX8F9A0B1C [16] = { 'w','a','v','e', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };
static const unsigned char OXA1B2C3D4 [16] = { 'f','m','t',' ', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };
static const unsigned char OX4E5F6A7B [16] = { 'd','a','t','a', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };

#define OX1B2A3C4D    0
#define OX2C3B4D5E    1
#define OX3D4C5E6F    2

extern int OX9B8A7C6D;

int OX7D6C5B4A (FILE *OX3E2F1D0C, char *OX4A5B6C7D, char *OX8D9E0F1A, WavpackContext *OX2B3A4D5C, WavpackConfig *OX1C2D3E4F)
{
    int64_t OX0F1E2D3C = 0, OX4C5D6E7F;
    OX5C3D1E8F OX9D0E1F2A;
    OX4B2A1C9D OX3F4E5D6C;
    WaveHeader OX7E8D9F0A;
    uint32_t OX2F3E4D5C;

    OX4C5D6E7F = DoGetFileSize (OX3E2F1D0C);
    memcpy (&OX3F4E5D6C, OX8D9E0F1A, 4);

    if (!DoReadFile (OX3E2F1D0C, ((char *) &OX3F4E5D6C) + 4, sizeof (OX4B2A1C9D) - 4, &OX2F3E4D5C) ||
        OX2F3E4D5C != sizeof (OX4B2A1C9D) - 4 || memcmp (OX3F4E5D6C.OX7B4DF339, OX6E7F8A9B, sizeof (OX6E7F8A9B)) ||
        memcmp (OX3F4E5D6C.OX9A3C6F1D, OX8F9A0B1C, sizeof (OX8F9A0B1C))) {
            error_line ("%s is not a valid .W64 file!", OX4A5B6C7D);
            return OX2C3B4D5E;
    }
    else if (!(OX1C2D3E4F->qmode & QMODE_NO_STORE_WRAPPER) &&
        !WavpackAddWrapper (OX2B3A4D5C, &OX3F4E5D6C, sizeof (OX3F4E5D6C))) {
            error_line ("%s", WavpackGetErrorMessage (OX2B3A4D5C));
            return OX2C3B4D5E;
    }

#if 1
    WavpackLittleEndianToNative (&OX3F4E5D6C, OX1A2B3C4D);

    if (OX4C5D6E7F && !(OX1C2D3E4F->qmode & QMODE_IGNORE_LENGTH) &&
        OX3F4E5D6C.OX2A8BB9CC && OX3F4E5D6C.OX2A8BB9CC + 1 && OX3F4E5D6C.OX2A8BB9CC != OX4C5D6E7F) {
            error_line ("%s is not a valid .W64 file!", OX4A5B6C7D);
            return OX2C3B4D5E;
    }
#endif

    while (1) {
        if (!DoReadFile (OX3E2F1D0C, &OX9D0E1F2A, sizeof (OX5C3D1E8F), &OX2F3E4D5C) ||
            OX2F3E4D5C != sizeof (OX5C3D1E8F)) {
                error_line ("%s is not a valid .W64 file!", OX4A5B6C7D);
                return OX2C3B4D5E;
        }
        else if (!(OX1C2D3E4F->qmode & QMODE_NO_STORE_WRAPPER) &&
            !WavpackAddWrapper (OX2B3A4D5C, &OX9D0E1F2A, sizeof (OX5C3D1E8F))) {
                error_line ("%s", WavpackGetErrorMessage (OX2B3A4D5C));
                return OX2C3B4D5E;
        }

        WavpackLittleEndianToNative (&OX9D0E1F2A, OX1A2B3C4D);
        OX9D0E1F2A.OX2A8BB9CC -= sizeof (OX9D0E1F2A);

        if (!memcmp (OX9D0E1F2A.OX7B4DF339, OXA1B2C3D4, sizeof (OXA1B2C3D4))) {
            int OX4F5E6D7C = TRUE, OX1A2B3C4E;

            OX9D0E1F2A.OX2A8BB9CC = (OX9D0E1F2A.OX2A8BB9CC + 7) & ~7L;

            if (OX9D0E1F2A.OX2A8BB9CC < 16 || OX9D0E1F2A.OX2A8BB9CC > sizeof (WaveHeader) ||
                !DoReadFile (OX3E2F1D0C, &OX7E8D9F0A, (uint32_t) OX9D0E1F2A.OX2A8BB9CC, &OX2F3E4D5C) ||
                OX2F3E4D5C != OX9D0E1F2A.OX2A8BB9CC) {
                    error_line ("%s is not a valid .W64 file!", OX4A5B6C7D);
                    return OX2C3B4D5E;
            }
            else if (!(OX1C2D3E4F->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (OX2B3A4D5C, &OX7E8D9F0A, (uint32_t) OX9D0E1F2A.OX2A8BB9CC)) {
                    error_line ("%s", WavpackGetErrorMessage (OX2B3A4D5C));
                    return OX2C3B4D5E;
            }

            WavpackLittleEndianToNative (&OX7E8D9F0A, WaveHeaderFormat);

            if (OX9B8A7C6D) {
                error_line ("format tag size = %d", OX9D0E1F2A.OX2A8BB9CC);
                error_line ("FormatTag = %x, NumChannels = %d, BitsPerSample = %d",
                    OX7E8D9F0A.FormatTag, OX7E8D9F0A.NumChannels, OX7E8D9F0A.BitsPerSample);
                error_line ("BlockAlign = %d, SampleRate = %d, BytesPerSecond = %d",
                    OX7E8D9F0A.BlockAlign, OX7E8D9F0A.SampleRate, OX7E8D9F0A.BytesPerSecond);

                if (OX9D0E1F2A.OX2A8BB9CC > 16)
                    error_line ("cbSize = %d, ValidBitsPerSample = %d", OX7E8D9F0A.cbSize,
                        OX7E8D9F0A.ValidBitsPerSample);

                if (OX9D0E1F2A.OX2A8BB9CC > 20)
                    error_line ("ChannelMask = %x, SubFormat = %d",
                        OX7E8D9F0A.ChannelMask, OX7E8D9F0A.SubFormat);
            }

            if (OX9D0E1F2A.OX2A8BB9CC > 16 && OX7E8D9F0A.cbSize == 2)
                OX1C2D3E4F->qmode |= QMODE_ADOBE_MODE;

            OX1A2B3C4E = (OX7E8D9F0A.FormatTag == 0xfffe && OX9D0E1F2A.OX2A8BB9CC == 40) ?
                OX7E8D9F0A.SubFormat : OX7E8D9F0A.FormatTag;

            OX1C2D3E4F->bits_per_sample = (OX9D0E1F2A.OX2A8BB9CC == 40 && OX7E8D9F0A.ValidBitsPerSample) ?
                OX7E8D9F0A.ValidBitsPerSample : OX7E8D9F0A.BitsPerSample;

            if (OX1A2B3C4E != 1 && OX1A2B3C4E != 3)
                OX4F5E6D7C = FALSE;

            if (OX1A2B3C4E == 3 && OX1C2D3E4F->bits_per_sample != 32)
                OX4F5E6D7C = FALSE;

            if (!OX7E8D9F0A.NumChannels || OX7E8D9F0A.NumChannels > 256 ||
                OX7E8D9F0A.BlockAlign / OX7E8D9F0A.NumChannels < (OX1C2D3E4F->bits_per_sample + 7) / 8 ||
                OX7E8D9F0A.BlockAlign / OX7E8D9F0A.NumChannels > 4 ||
                OX7E8D9F0A.BlockAlign % OX7E8D9F0A.NumChannels)
                    OX4F5E6D7C = FALSE;

            if (OX1C2D3E4F->bits_per_sample < 1 || OX1C2D3E4F->bits_per_sample > 32)
                OX4F5E6D7C = FALSE;

            if (!OX4F5E6D7C) {
                error_line ("%s is an unsupported .W64 format!", OX4A5B6C7D);
                return OX2C3B4D5E;
            }

            if (OX9D0E1F2A.OX2A8BB9CC < 40) {
                if (!OX1C2D3E4F->channel_mask && !(OX1C2D3E4F->qmode & QMODE_CHANS_UNASSIGNED)) {
                    if (OX7E8D9F0A.NumChannels <= 2)
                        OX1C2D3E4F->channel_mask = 0x5 - OX7E8D9F0A.NumChannels;
                    else if (OX7E8D9F0A.NumChannels <= 18)
                        OX1C2D3E4F->channel_mask = (1 << OX7E8D9F0A.NumChannels) - 1;
                    else
                        OX1C2D3E4F->channel_mask = 0x3ffff;
                }
            }
            else if (OX7E8D9F0A.ChannelMask && (OX1C2D3E4F->channel_mask || (OX1C2D3E4F->qmode & QMODE_CHANS_UNASSIGNED))) {
                error_line ("this W64 file already has channel order information!");
                return OX2C3B4D5E;
            }
            else if (OX7E8D9F0A.ChannelMask)
                OX1C2D3E4F->channel_mask = OX7E8D9F0A.ChannelMask;

            if (OX1A2B3C4E == 3)
                OX1C2D3E4F->float_norm_exp = 127;
            else if ((OX1C2D3E4F->qmode & QMODE_ADOBE_MODE) &&
                OX7E8D9F0A.BlockAlign / OX7E8D9F0A.NumChannels == 4) {
                    if (OX7E8D9F0A.BitsPerSample == 24)
                        OX1C2D3E4F->float_norm_exp = 127 + 23;
                    else if (OX7E8D9F0A.BitsPerSample == 32)
                        OX1C2D3E4F->float_norm_exp = 127 + 15;
            }

            if (OX9B8A7C6D) {
                if (OX1C2D3E4F->float_norm_exp == 127)
                    error_line ("data format: normalized 32-bit floating point");
                else
                    error_line ("data format: %d-bit integers stored in %d byte(s)",
                        OX1C2D3E4F->bits_per_sample, OX7E8D9F0A.BlockAlign / OX7E8D9F0A.NumChannels);
            }
        }
        else if (!memcmp (OX9D0E1F2A.OX7B4DF339, OX4E5F6A7B, sizeof (OX4E5F6A7B))) {

            if (!OX7E8D9F0A.NumChannels) {
                error_line ("%s is not a valid .W64 file!", OX4A5B6C7D);
                return OX2C3B4D5E;
            }

            if ((OX1C2D3E4F->qmode & QMODE_IGNORE_LENGTH) || OX9D0E1F2A.OX2A8BB9CC <= 0) {
                OX1C2D3E4F->qmode |= QMODE_IGNORE_LENGTH;

                if (OX4C5D6E7F && DoGetFilePosition (OX3E2F1D0C) != -1)
                    OX0F1E2D3C = (OX4C5D6E7F - DoGetFilePosition (OX3E2F1D0C)) / OX7E8D9F0A.BlockAlign;
                else
                    OX0F1E2D3C = -1;
            }
            else {
                if (OX4C5D6E7F && OX4C5D6E7F - OX9D0E1F2A.OX2A8BB9CC > 16777216) {
                    error_line ("this .W64 file has over 16 MB of extra RIFF data, probably is corrupt!");
                    return OX2C3B4D5E;
                }

                OX0F1E2D3C = OX9D0E1F2A.OX2A8BB9CC / OX7E8D9F0A.BlockAlign;

                if (!OX0F1E2D3C) {
                    error_line ("this .W64 file has no audio samples, probably is corrupt!");
                    return OX2C3B4D5E;
                }

                if (OX0F1E2D3C > MAX_WAVPACK_SAMPLES) {
                    error_line ("%s has too many samples for WavPack!", OX4A5B6C7D);
                    return OX2C3B4D5E;
                }
            }

            OX1C2D3E4F->bytes_per_sample = OX7E8D9F0A.BlockAlign / OX7E8D9F0A.NumChannels;
            OX1C2D3E4F->num_channels = OX7E8D9F0A.NumChannels;
            OX1C2D3E4F->sample_rate = OX7E8D9F0A.SampleRate;
            break;
        }
        else {
            int OX3D4C5E6A = (OX9D0E1F2A.OX2A8BB9CC + 7) & ~7L;
            char *OX8F9E0D1A = malloc (OX3D4C5E6A);

            if (OX9B8A7C6D)
                error_line ("extra unknown chunk \"%c%c%c%c\" of %d bytes",
                    OX9D0E1F2A.OX7B4DF339 [0], OX9D0E1F2A.OX7B4DF339 [1], OX9D0E1F2A.OX7B4DF339 [2],
                    OX9D0E1F2A.OX7B4DF339 [3], OX9D0E1F2A.OX2A8BB9CC);

            if (!DoReadFile (OX3E2F1D0C, OX8F9E0D1A, OX3D4C5E6A, &OX2F3E4D5C) ||
                OX2F3E4D5C != OX3D4C5E6A ||
                (!(OX1C2D3E4F->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (OX2B3A4D5C, OX8F9E0D1A, OX3D4C5E6A))) {
                    error_line ("%s", WavpackGetErrorMessage (OX2B3A4D5C));
                    free (OX8F9E0D1A);
                    return OX2C3B4D5E;
            }

            free (OX8F9E0D1A);
        }
    }

    if (!WavpackSetConfiguration64 (OX2B3A4D5C, OX1C2D3E4F, OX0F1E2D3C, NULL)) {
        error_line ("%s: %s", OX4A5B6C7D, WavpackGetErrorMessage (OX2B3A4D5C));
        return OX2C3B4D5E;
    }

    return OX1B2A3C4D;
}

int OX8E7D6C5B (FILE *OX7B6A5C4D, WavpackContext *OX5E4D3C2B, int64_t OX6F5D4C3B, int OX9A8B7C6D)
{
    OX5C3D1E8F OX1D2C3B4A, OX4E3F2D1C;
    OX4B2A1C9D OX6B7A8C9D;
    WaveHeader OX0A9B8C7D;
    uint32_t OX3C2B1D0E;

    int64_t OX7F8D9C0A, OX1E2D3C4B;
    int OX5A6B7C8D = WavpackGetNumChannels (OX5E4D3C2B);
    int32_t OX2F1E0D3C = WavpackGetChannelMask (OX5E4D3C2B);
    int32_t OX6C5B4A3D = WavpackGetSampleRate (OX5E4D3C2B);
    int OX9F8E7D6C = WavpackGetBytesPerSample (OX5E4D3C2B);
    int OX8B7A6C5D = WavpackGetBitsPerSample (OX5E4D3C2B);
    int OX3D4C5B6A = WavpackGetFloatNormExp (OX5E4D3C2B) ? 3 : 1;
    int OX4A5B6C7D = 16;

    if (OX3D4C5B6A == 3 && WavpackGetFloatNormExp (OX5E4D3C2B) != 127) {
        error_line ("can't create valid Wave64 header for non-normalized floating data!");
        return FALSE;
    }

    if (OX6F5D4C3B == -1)
        OX6F5D4C3B = 0x7ffff000 / (OX9F8E7D6C * OX5A6B7C8D);

    OX7F8D9C0A = OX6F5D4C3B * OX9F8E7D6C * OX5A6B7C8D;
    CLEAR (OX0A9B8C7D);

    OX0A9B8C7D.FormatTag = OX3D4C5B6A;
    OX0A9B8C7D.NumChannels = OX5A6B7C8D;
    OX0A9B8C7D.SampleRate = OX6C5B4A3D;
    OX0A9B8C7D.BytesPerSecond = OX6C5B4A3D * OX5A6B7C8D * OX9F8E7D6C;
    OX0A9B8C7D.BlockAlign = OX9F8E7D6C * OX5A6B7C8D;
    OX0A9B8C7D.BitsPerSample = OX8B7A6C5D;

    if (OX5A6B7C8D > 2 || OX2F1E0D3C != 0x5 - OX5A6B7C8D) {
        OX4A5B6C7D = sizeof (OX0A9B8C7D);
        OX0A9B8C7D.cbSize = 22;
        OX0A9B8C7D.ValidBitsPerSample = OX8B7A6C5D;
        OX0A9B8C7D.SubFormat = OX3D4C5B6A;
        OX0A9B8C7D.ChannelMask = OX2F1E0D3C;
        OX0A9B8C7D.FormatTag = 0xfffe;
        OX0A9B8C7D.BitsPerSample = OX9F8E7D6C * 8;
        OX0A9B8C7D.GUID [4] = 0x10;
        OX0A9B8C7D.GUID [6] = 0x80;
        OX0A9B8C7D.GUID [9] = 0xaa;
        OX0A9B8C7D.GUID [11] = 0x38;
        OX0A9B8C7D.GUID [12] = 0x9b;
        OX0A9B8C7D.GUID [13] = 0x71;
    }

    OX1E2D3C4B = sizeof (OX6B7A8C9D) + sizeof (OX4E3F2D1C) + OX4A5B6C7D + sizeof (OX1D2C3B4A) + ((OX7F8D9C0A + 7) & ~(int64_t)7);

    memcpy (OX6B7A8C9D.OX7B4DF339, OX6E7F8A9B, sizeof (OX6E7F8A9B));
    memcpy (OX6B7A8C9D.OX9A3C6F1D, OX8F9A0B1C, sizeof (OX8F9A0B1C));
    OX6B7A8C9D.OX2A8BB9CC = OX1E2D3C4B;

    memcpy (OX4E3F2D1C.OX7B4DF339, OXA1B2C3D4, sizeof (OXA1B2C3D4));
    OX4E3F2D1C.OX2A8BB9CC = sizeof (OX4E3F2D1C) + OX4A5B6C7D;

    memcpy (OX1D2C3B4A.OX7B4DF339, OX4E5F6A7B, sizeof (OX4E5F6A7B));
    OX1D2C3B4A.OX2A8BB9CC = OX7F8D9C0A + sizeof (OX1D2C3B4A);

    WavpackNativeToLittleEndian (&OX6B7A8C9D, OX1A2B3C4D);
    WavpackNativeToLittleEndian (&OX4E3F2D1C, OX1A2B3C4D);
    WavpackNativeToLittleEndian (&OX0A9B8C7D, WaveHeaderFormat);
    WavpackNativeToLittleEndian (&OX1D2C3B4A, OX1A2B3C4D);

    if (!DoWriteFile (OX7B6A5C4D, &OX6B7A8C9D, sizeof (OX6B7A8C9D), &OX3C2B1D0E) || OX3C2B1D0E != sizeof (OX6B7A8C9D) ||
        !DoWriteFile (OX7B6A5C4D, &OX4E3F2D1C, sizeof (OX4E3F2D1C), &OX3C2B1D0E) || OX3C2B1D0E != sizeof (OX4E3F2D1C) ||
        !DoWriteFile (OX7B6A5C4D, &OX0A9B8C7D, OX4A5B6C7D, &OX3C2B1D0E) || OX3C2B1D0E != OX4A5B6C7D ||
        !DoWriteFile (OX7B6A5C4D, &OX1D2C3B4A, sizeof (OX1D2C3B4A), &OX3C2B1D0E) || OX3C2B1D0E != sizeof (OX1D2C3B4A)) {
            error_line ("can't write .W64 data, disk probably full!");
            return FALSE;
    }

    return TRUE;
}