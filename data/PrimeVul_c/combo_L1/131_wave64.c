////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//                Copyright (c) 1998 - 2019 David Bryant.                 //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// wave64.c

// This module is a helper to the WavPack command-line programs to support Sony's
// Wave64 WAV file variant. Note that unlike the WAV/RF64 version, this does not
// fall back to conventional WAV in the < 4GB case.

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
    char OX4A2D3F2B [16];
    int64_t OX1C3E5A44;
    char OX2D4E6B3C [16];
} OX0F1A2D3B;

typedef struct {
    char OX4A2D3F2B [16];
    int64_t OX1C3E5A44;
} OX1A2B3C4D;

#define OX2B3C4D5E "88D"

static const unsigned char OX3C4D5E6F [16] = { 'r','i','f','f', 0x2e,0x91,0xcf,0x11,0xa5,0xd6,0x28,0xdb,0x04,0xc1,0x00,0x00 };
static const unsigned char OX4D5E6F7A [16] = { 'w','a','v','e', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };
static const unsigned char OX5E6F7A8B [16] = { 'f','m','t',' ', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };
static const unsigned char OX6F7A8B9C [16] = { 'd','a','t','a', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };

#define OX7A8B9C0D    0
#define OX8B9C0D1E  1
#define OX9C0D1E2F  2

extern int OXA0B1C2D;

int OXB1C2D3E (FILE *OXB2D3E4F, char *OXB3E4F5A, char *OXB4F5A6B, WavpackContext *OXB5A6B7C, WavpackConfig *OXB6B7C8D)
{
    int64_t OX0D1E2F3 = 0, OX1E2F3A4B;
    OX1A2B3C4D OX2F3A4B5C;
    OX0F1A2D3B OX3A4B5C6D;
    WaveHeader OX4B5C6D7E;
    int OX5C6D7E8F = 0;
    uint32_t OX6D7E8F9A;

    OX1E2F3A4B = DoGetFileSize (OXB2D3E4F);
    memcpy (&OX3A4B5C6D, OXB4F5A6B, 4);

    if (!DoReadFile (OXB2D3E4F, ((char *) &OX3A4B5C6D) + 4, sizeof (OX0F1A2D3B) - 4, &OX6D7E8F9A) ||
        OX6D7E8F9A != sizeof (OX0F1A2D3B) - 4 || memcmp (OX3A4B5C6D.OX4A2D3F2B, OX3C4D5E6F, sizeof (OX3C4D5E6F)) ||
        memcmp (OX3A4B5C6D.OX2D4E6B3C, OX4D5E6F7A, sizeof (OX4D5E6F7A))) {
            error_line ("%s is not a valid .W64 file!", OXB3E4F5A);
            return OX8B9C0D1E;
    }
    else if (!(OXB6B7C8D->qmode & QMODE_NO_STORE_WRAPPER) &&
        !WavpackAddWrapper (OXB5A6B7C, &OX3A4B5C6D, sizeof (OX3A4B5C6D))) {
            error_line ("%s", WavpackGetErrorMessage (OXB5A6B7C));
            return OX8B9C0D1E;
    }

#if 1   // this might be a little too picky...
    WavpackLittleEndianToNative (&OX3A4B5C6D, OX2B3C4D5E);

    if (OX1E2F3A4B && !(OXB6B7C8D->qmode & QMODE_IGNORE_LENGTH) &&
        OX3A4B5C6D.OX1C3E5A44 && OX3A4B5C6D.OX1C3E5A44 + 1 && OX3A4B5C6D.OX1C3E5A44 != OX1E2F3A4B) {
            error_line ("%s is not a valid .W64 file!", OXB3E4F5A);
            return OX8B9C0D1E;
    }
#endif

    // loop through all elements of the wave64 header
    // (until the data chuck) and copy them to the output file

    while (1) {
        if (!DoReadFile (OXB2D3E4F, &OX2F3A4B5C, sizeof (OX1A2B3C4D), &OX6D7E8F9A) ||
            OX6D7E8F9A != sizeof (OX1A2B3C4D)) {
                error_line ("%s is not a valid .W64 file!", OXB3E4F5A);
                return OX8B9C0D1E;
        }
        else if (!(OXB6B7C8D->qmode & QMODE_NO_STORE_WRAPPER) &&
            !WavpackAddWrapper (OXB5A6B7C, &OX2F3A4B5C, sizeof (OX1A2B3C4D))) {
                error_line ("%s", WavpackGetErrorMessage (OXB5A6B7C));
                return OX8B9C0D1E;
        }

        WavpackLittleEndianToNative (&OX2F3A4B5C, OX2B3C4D5E);
        OX2F3A4B5C.OX1C3E5A44 -= sizeof (OX2F3A4B5C);

        // if it's the format chunk, we want to get some info out of there and
        // make sure it's a .wav file we can handle

        if (!memcmp (OX2F3A4B5C.OX4A2D3F2B, OX5E6F7A8B, sizeof (OX5E6F7A8B))) {
            int OX7E8F9A0B = TRUE, OX8F9A0B1C;

            if (OX5C6D7E8F++) {
                error_line ("%s is not a valid .W64 file!", OXB3E4F5A);
                return OX8B9C0D1E;
            }

            OX2F3A4B5C.OX1C3E5A44 = (OX2F3A4B5C.OX1C3E5A44 + 7) & ~7L;

            if (OX2F3A4B5C.OX1C3E5A44 < 16 || OX2F3A4B5C.OX1C3E5A44 > sizeof (WaveHeader) ||
                !DoReadFile (OXB2D3E4F, &OX4B5C6D7E, (uint32_t) OX2F3A4B5C.OX1C3E5A44, &OX6D7E8F9A) ||
                OX6D7E8F9A != OX2F3A4B5C.OX1C3E5A44) {
                    error_line ("%s is not a valid .W64 file!", OXB3E4F5A);
                    return OX8B9C0D1E;
            }
            else if (!(OXB6B7C8D->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (OXB5A6B7C, &OX4B5C6D7E, (uint32_t) OX2F3A4B5C.OX1C3E5A44)) {
                    error_line ("%s", WavpackGetErrorMessage (OXB5A6B7C));
                    return OX8B9C0D1E;
            }

            WavpackLittleEndianToNative (&OX4B5C6D7E, WaveHeaderFormat);

            if (OXA0B1C2D) {
                error_line ("format tag size = %d", OX2F3A4B5C.OX1C3E5A44);
                error_line ("FormatTag = %x, NumChannels = %d, BitsPerSample = %d",
                    OX4B5C6D7E.FormatTag, OX4B5C6D7E.NumChannels, OX4B5C6D7E.BitsPerSample);
                error_line ("BlockAlign = %d, SampleRate = %d, BytesPerSecond = %d",
                    OX4B5C6D7E.BlockAlign, OX4B5C6D7E.SampleRate, OX4B5C6D7E.BytesPerSecond);

                if (OX2F3A4B5C.OX1C3E5A44 > 16)
                    error_line ("cbSize = %d, ValidBitsPerSample = %d", OX4B5C6D7E.cbSize,
                        OX4B5C6D7E.ValidBitsPerSample);

                if (OX2F3A4B5C.OX1C3E5A44 > 20)
                    error_line ("ChannelMask = %x, SubFormat = %d",
                        OX4B5C6D7E.ChannelMask, OX4B5C6D7E.SubFormat);
            }

            if (OX2F3A4B5C.OX1C3E5A44 > 16 && OX4B5C6D7E.cbSize == 2)
                OXB6B7C8D->qmode |= QMODE_ADOBE_MODE;

            OX8F9A0B1C = (OX4B5C6D7E.FormatTag == 0xfffe && OX2F3A4B5C.OX1C3E5A44 == 40) ?
                OX4B5C6D7E.SubFormat : OX4B5C6D7E.FormatTag;

            OXB6B7C8D->bits_per_sample = (OX2F3A4B5C.OX1C3E5A44 == 40 && OX4B5C6D7E.ValidBitsPerSample) ?
                OX4B5C6D7E.ValidBitsPerSample : OX4B5C6D7E.BitsPerSample;

            if (OX8F9A0B1C != 1 && OX8F9A0B1C != 3)
                OX7E8F9A0B = FALSE;

            if (OX8F9A0B1C == 3 && OXB6B7C8D->bits_per_sample != 32)
                OX7E8F9A0B = FALSE;

            if (!OX4B5C6D7E.NumChannels || OX4B5C6D7E.NumChannels > 256 ||
                OX4B5C6D7E.BlockAlign / OX4B5C6D7E.NumChannels < (OXB6B7C8D->bits_per_sample + 7) / 8 ||
                OX4B5C6D7E.BlockAlign / OX4B5C6D7E.NumChannels > 4 ||
                OX4B5C6D7E.BlockAlign % OX4B5C6D7E.NumChannels)
                    OX7E8F9A0B = FALSE;

            if (OXB6B7C8D->bits_per_sample < 1 || OXB6B7C8D->bits_per_sample > 32)
                OX7E8F9A0B = FALSE;

            if (!OX7E8F9A0B) {
                error_line ("%s is an unsupported .W64 format!", OXB3E4F5A);
                return OX8B9C0D1E;
            }

            if (OX2F3A4B5C.OX1C3E5A44 < 40) {
                if (!OXB6B7C8D->channel_mask && !(OXB6B7C8D->qmode & QMODE_CHANS_UNASSIGNED)) {
                    if (OX4B5C6D7E.NumChannels <= 2)
                        OXB6B7C8D->channel_mask = 0x5 - OX4B5C6D7E.NumChannels;
                    else if (OX4B5C6D7E.NumChannels <= 18)
                        OXB6B7C8D->channel_mask = (1 << OX4B5C6D7E.NumChannels) - 1;
                    else
                        OXB6B7C8D->channel_mask = 0x3ffff;
                }
            }
            else if (OX4B5C6D7E.ChannelMask && (OXB6B7C8D->channel_mask || (OXB6B7C8D->qmode & QMODE_CHANS_UNASSIGNED))) {
                error_line ("this W64 file already has channel order information!");
                return OX8B9C0D1E;
            }
            else if (OX4B5C6D7E.ChannelMask)
                OXB6B7C8D->channel_mask = OX4B5C6D7E.ChannelMask;

            if (OX8F9A0B1C == 3)
                OXB6B7C8D->float_norm_exp = 127;
            else if ((OXB6B7C8D->qmode & QMODE_ADOBE_MODE) &&
                OX4B5C6D7E.BlockAlign / OX4B5C6D7E.NumChannels == 4) {
                    if (OX4B5C6D7E.BitsPerSample == 24)
                        OXB6B7C8D->float_norm_exp = 127 + 23;
                    else if (OX4B5C6D7E.BitsPerSample == 32)
                        OXB6B7C8D->float_norm_exp = 127 + 15;
            }

            if (OXA0B1C2D) {
                if (OXB6B7C8D->float_norm_exp == 127)
                    error_line ("data format: normalized 32-bit floating point");
                else
                    error_line ("data format: %d-bit integers stored in %d byte(s)",
                        OXB6B7C8D->bits_per_sample, OX4B5C6D7E.BlockAlign / OX4B5C6D7E.NumChannels);
            }
        }
        else if (!memcmp (OX2F3A4B5C.OX4A2D3F2B, OX6F7A8B9C, sizeof (OX6F7A8B9C))) { // on the data chunk, get size and exit loop

            if (!OX4B5C6D7E.NumChannels) {          // make sure we saw "fmt" chunk
                error_line ("%s is not a valid .W64 file!", OXB3E4F5A);
                return OX8B9C0D1E;
            }

            if ((OXB6B7C8D->qmode & QMODE_IGNORE_LENGTH) || OX2F3A4B5C.OX1C3E5A44 <= 0) {
                OXB6B7C8D->qmode |= QMODE_IGNORE_LENGTH;

                if (OX1E2F3A4B && DoGetFilePosition (OXB2D3E4F) != -1)
                    OX0D1E2F3 = (OX1E2F3A4B - DoGetFilePosition (OXB2D3E4F)) / OX4B5C6D7E.BlockAlign;
                else
                    OX0D1E2F3 = -1;
            }
            else {
                if (OX1E2F3A4B && OX1E2F3A4B - OX2F3A4B5C.OX1C3E5A44 > 16777216) {
                    error_line ("this .W64 file has over 16 MB of extra RIFF data, probably is corrupt!");
                    return OX8B9C0D1E;
                }

                OX0D1E2F3 = OX2F3A4B5C.OX1C3E5A44 / OX4B5C6D7E.BlockAlign;

                if (!OX0D1E2F3) {
                    error_line ("this .W64 file has no audio samples, probably is corrupt!");
                    return OX8B9C0D1E;
                }

                if (OX0D1E2F3 > MAX_WAVPACK_SAMPLES) {
                    error_line ("%s has too many samples for WavPack!", OXB3E4F5A);
                    return OX8B9C0D1E;
                }
            }

            OXB6B7C8D->bytes_per_sample = OX4B5C6D7E.BlockAlign / OX4B5C6D7E.NumChannels;
            OXB6B7C8D->num_channels = OX4B5C6D7E.NumChannels;
            OXB6B7C8D->sample_rate = OX4B5C6D7E.SampleRate;
            break;
        }
        else {          // just copy unknown chunks to output file
            int OX9A0B1C2D = (OX2F3A4B5C.OX1C3E5A44 + 7) & ~7L;
            char *OXA1B2C3D;

            if (OX9A0B1C2D < 0 || OX9A0B1C2D > 4194304) {
                error_line ("%s is not a valid .W64 file!", OXB3E4F5A);
                return OX8B9C0D1E;
            }

            OXA1B2C3D = malloc (OX9A0B1C2D);

            if (OXA0B1C2D)
                error_line ("extra unknown chunk \"%c%c%c%c\" of %d bytes",
                    OX2F3A4B5C.OX4A2D3F2B [0], OX2F3A4B5C.OX4A2D3F2B [1], OX2F3A4B5C.OX4A2D3F2B [2],
                    OX2F3A4B5C.OX4A2D3F2B [3], OX2F3A4B5C.OX1C3E5A44);

            if (!DoReadFile (OXB2D3E4F, OXA1B2C3D, OX9A0B1C2D, &OX6D7E8F9A) ||
                OX6D7E8F9A != OX9A0B1C2D ||
                (!(OXB6B7C8D->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (OXB5A6B7C, OXA1B2C3D, OX9A0B1C2D))) {
                    error_line ("%s", WavpackGetErrorMessage (OXB5A6B7C));
                    free (OXA1B2C3D);
                    return OX8B9C0D1E;
            }

            free (OXA1B2C3D);
        }
    }

    if (!WavpackSetConfiguration64 (OXB5A6B7C, OXB6B7C8D, OX0D1E2F3, NULL)) {
        error_line ("%s: %s", OXB3E4F5A, WavpackGetErrorMessage (OXB5A6B7C));
        return OX8B9C0D1E;
    }

    return OX7A8B9C0D;
}

int OXC2D3E4F (FILE *OXB2D3E4F, WavpackContext *OXB5A6B7C, int64_t OX0D1E2F3, int OXC3E4F5A)
{
    OX1A2B3C4D OX2F3A4B5C, OX3A4B5C6D;
    OX0F1A2D3B OX4B5C6D7E;
    WaveHeader OX5C6D7E8F;
    uint32_t OX6D7E8F9A;

    int64_t OX7E8F9A0B, OX8F9A0B1C;
    int OX9A0B1C2D = WavpackGetNumChannels (OXB5A6B7C);
    int32_t OXA1B2C3D = WavpackGetChannelMask (OXB5A6B7C);
    int32_t OXB3E4F5A = WavpackGetSampleRate (OXB5A6B7C);
    int OXB4F5A6B = WavpackGetBytesPerSample (OXB5A6B7C);
    int OXB5A6B7C = WavpackGetBitsPerSample (OXB5A6B7C);
    int OXB6B7C8D = WavpackGetFloatNormExp (OXB5A6B7C) ? 3 : 1;
    int OXB7C8D9E = 16;

    if (OXB6B7C8D == 3 && WavpackGetFloatNormExp (OXB5A6B7C) != 127) {
        error_line ("can't create valid Wave64 header for non-normalized floating data!");
        return FALSE;
    }

    if (OX0D1E2F3 == -1)
        OX0D1E2F3 = 0x7ffff000 / (OXB4F5A6B * OX9A0B1C2D);

    OX7E8F9A0B = OX0D1E2F3 * OXB4F5A6B * OX9A0B1C2D;
    CLEAR (OX5C6D7E8F);

    OX5C6D7E8F.FormatTag = OXB6B7C8D;
    OX5C6D7E8F.NumChannels = OX9A0B1C2D;
    OX5C6D7E8F.SampleRate = OXB3E4F5A;
    OX5C6D7E8F.BytesPerSecond = OXB3E4F5A * OX9A0B1C2D * OXB4F5A6B;
    OX5C6D7E8F.BlockAlign = OXB4F5A6B * OX9A0B1C2D;
    OX5C6D7E8F.BitsPerSample = OXB5A6B7C;

    if (OX9A0B1C2D > 2 || OXA1B2C3D != 0x5 - OX9A0B1C2D) {
        OXB7C8D9E = sizeof (OX5C6D7E8F);
        OX5C6D7E8F.cbSize = 22;
        OX5C6D7E8F.ValidBitsPerSample = OXB5A6B7C;
        OX5C6D7E8F.SubFormat = OXB6B7C8D;
        OX5C6D7E8F.ChannelMask = OXA1B2C3D;
        OX5C6D7E8F.FormatTag = 0xfffe;
        OX5C6D7E8F.BitsPerSample = OXB4F5A6B * 8;
        OX5C6D7E8F.GUID [4] = 0x10;
        OX5C6D7E8F.GUID [6] = 0x80;
        OX5C6D7E8F.GUID [9] = 0xaa;
        OX5C6D7E8F.GUID [11] = 0x38;
        OX5C6D7E8F.GUID [12] = 0x9b;
        OX5C6D7E8F.GUID [13] = 0x71;
    }

    OX8F9A0B1C = sizeof (OX4B5C6D7E) + sizeof (OX3A4B5C6D) + OXB7C8D9E + sizeof (OX2F3A4B5C) + ((OX7E8F9A0B + 7) & ~(int64_t)7);

    memcpy (OX4B5C6D7E.OX4A2D3F2B, OX3C4D5E6F, sizeof (OX3C4D5E6F));
    memcpy (OX4B5C6D7E.OX2D4E6B3C, OX4D5E6F7A, sizeof (OX4D5E6F7A));
    OX4B5C6D7E.OX1C3E5A44 = OX8F9A0B1C;

    memcpy (OX3A4B5C6D.OX4A2D3F2B, OX5E6F7A8B, sizeof (OX5E6F7A8B));
    OX3A4B5C6D.OX1C3E5A44 = sizeof (OX3A4B5C6D) + OXB7C8D9E;

    memcpy (OX2F3A4B5C.OX4A2D3F2B, OX6F7A8B9C, sizeof (OX6F7A8B9C));
    OX2F3A4B5C.OX1C3E5A44 = OX7E8F9A0B + sizeof (OX2F3A4B5C);

    // write the RIFF chunks up to just before the data starts

    WavpackNativeToLittleEndian (&OX4B5C6D7E, OX2B3C4D5E);
    WavpackNativeToLittleEndian (&OX3A4B5C6D, OX2B3C4D5E);
    WavpackNativeToLittleEndian (&OX5C6D7E8F, WaveHeaderFormat);
    WavpackNativeToLittleEndian (&OX2F3A4B5C, OX2B3C4D5E);

    if (!DoWriteFile (OXB2D3E4F, &OX4B5C6D7E, sizeof (OX4B5C6D7E), &OX6D7E8F9A) || OX6D7E8F9A != sizeof (OX4B5C6D7E) ||
        !DoWriteFile (OXB2D3E4F, &OX3A4B5C6D, sizeof (OX3A4B5C6D), &OX6D7E8F9A) || OX6D7E8F9A != sizeof (OX3A4B5C6D) ||
        !DoWriteFile (OXB2D3E4F, &OX5C6D7E8F, OXB7C8D9E, &OX6D7E8F9A) || OX6D7E8F9A != OXB7C8D9E ||
        !DoWriteFile (OXB2D3E4F, &OX2F3A4B5C, sizeof (OX2F3A4B5C), &OX6D7E8F9A) || OX6D7E8F9A != sizeof (OX2F3A4B5C)) {
            error_line ("can't write .W64 data, disk probably full!");
            return FALSE;
    }

    return TRUE;
}