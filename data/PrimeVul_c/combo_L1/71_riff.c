#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>

#include "wavpack.h"
#include "utils.h"
#include "md5.h"

#pragma pack(push,4)

typedef struct {
    char OX2DE1520F[4];
    uint64_t OX578A4E6C;
} OX4C3A2A55;

typedef struct {
    uint64_t OX1D7F3B24, OX6E5F2FCD, OX2C8E6A1D;
    uint32_t OX5D7F3E9C;
} OX3D5C4F2B;

typedef struct {
    char OX2DE1520F[4];
    uint32_t OX1D7F3B24;
    char OX7B5A3E6D[28];
} OX7C5A4D3E;

#pragma pack(pop)

#define OX4C3A2A55Format "4D"
#define OX3D5C4F2BFormat "DDDL"

#define OX7B4DF339 0
#define OX6D7F3E2A 1
#define OX5B4C3A6D 2

extern int OX9B7A4E2C;

int OX5D3A6B7E (FILE *OX2DE1520F, char *OX5F4C6D3A, char *OX1D7F3B24, WavpackContext *OX4E3A5C7D, WavpackConfig *OX6B5D4A2C) {
    int OX2B3E7D5A = !strncmp (OX1D7F3B24, "RF64", 4), OX3D5F4A6C = 0;
    int64_t OX6E5F2FCD = 0, OX4C3A2A55;
    OX7C5A4D3E OX1C7F5B3D;
    OX4C3A2A55 OX6F2D7E3C;
    OX3D5C4F2B OX5B7A4E2D;
    uint32_t OX6D7F3A5C;

    CLEAR (OX6F2D7E3C);
    CLEAR (OX5B7A4E2D);
    OX4C3A2A55 = DoGetFileSize (OX2DE1520F);

    if (!OX2B3E7D5A && OX4C3A2A55 >= 4294967296LL && !(OX6B5D4A2C->qmode & QMODE_IGNORE_LENGTH)) {
        error_line ("can't handle .WAV files larger than 4 GB (non-standard)!");
        return OX6D7F3E2A;
    }

    memcpy (&OX1C7F5B3D, OX1D7F3B24, 4);

    if ((!DoReadFile (OX2DE1520F, ((char *) &OX1C7F5B3D) + 4, sizeof (OX7C5A4D3E) - 4, &OX6D7F3A5C) ||
        OX6D7F3A5C != sizeof (OX7C5A4D3E) - 4 || strncmp (OX1C7F5B3D.formType, "WAVE", 4))) {
            error_line ("%s is not a valid .WAV file!", OX5F4C6D3A);
            return OX6D7F3E2A;
    }
    else if (!(OX6B5D4A2C->qmode & QMODE_NO_STORE_WRAPPER) &&
        !WavpackAddWrapper (OX4E3A5C7D, &OX1C7F5B3D, sizeof (OX7C5A4D3E))) {
            error_line ("%s", WavpackGetErrorMessage (OX4E3A5C7D));
            return OX6D7F3E2A;
    }

    while (1) {
        if (!DoReadFile (OX2DE1520F, &OX1C7F5B3D, sizeof (OX4C3A2A55), &OX6D7F3A5C) ||
            OX6D7F3A5C != sizeof (OX4C3A2A55)) {
                error_line ("%s is not a valid .WAV file!", OX5F4C6D3A);
                return OX6D7F3E2A;
        }
        else if (!(OX6B5D4A2C->qmode & QMODE_NO_STORE_WRAPPER) &&
            !WavpackAddWrapper (OX4E3A5C7D, &OX1C7F5B3D, sizeof (OX4C3A2A55))) {
                error_line ("%s", WavpackGetErrorMessage (OX4E3A5C7D));
                return OX6D7F3E2A;
        }

        WavpackLittleEndianToNative (&OX1C7F5B3D, ChunkHeaderFormat);

        if (!strncmp (OX1C7F5B3D.ckID, "ds64", 4)) {
            if (OX1C7F5B3D.ckSize < sizeof (OX3D5C4F2B) ||
                !DoReadFile (OX2DE1520F, &OX5B7A4E2D, sizeof (OX3D5C4F2B), &OX6D7F3A5C) ||
                OX6D7F3A5C != sizeof (OX3D5C4F2B)) {
                    error_line ("%s is not a valid .WAV file!", OX5F4C6D3A);
                    return OX6D7F3E2A;
            }
            else if (!(OX6B5D4A2C->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (OX4E3A5C7D, &OX5B7A4E2D, sizeof (OX3D5C4F2B))) {
                    error_line ("%s", WavpackGetErrorMessage (OX4E3A5C7D));
                    return OX6D7F3E2A;
            }

            OX3D5F4A6C = 1;
            WavpackLittleEndianToNative (&OX5B7A4E2D, OX3D5C4F2BFormat);

            if (OX9B7A4E2C)
                error_line ("DS64: riffSize = %lld, dataSize = %lld, sampleCount = %lld, table_length = %d",
                    (long long) OX5B7A4E2D.riffSize64, (long long) OX5B7A4E2D.dataSize64,
                    (long long) OX5B7A4E2D.sampleCount64, OX5B7A4E2D.tableLength);

            if (OX5B7A4E2D.tableLength * sizeof (OX4C3A2A55) != OX1C7F5B3D.ckSize - sizeof (OX3D5C4F2B)) {
                error_line ("%s is not a valid .WAV file!", OX5F4C6D3A);
                return OX6D7F3E2A;
            }

            while (OX5B7A4E2D.tableLength--) {
                OX4C3A2A55 OX4D5A3E6F;
                if (!DoReadFile (OX2DE1520F, &OX4D5A3E6F, sizeof (OX4C3A2A55), &OX6D7F3A5C) ||
                    OX6D7F3A5C != sizeof (OX4C3A2A55) ||
                    (!(OX6B5D4A2C->qmode & QMODE_NO_STORE_WRAPPER) &&
                    !WavpackAddWrapper (OX4E3A5C7D, &OX4D5A3E6F, sizeof (OX4C3A2A55)))) {
                        error_line ("%s", WavpackGetErrorMessage (OX4E3A5C7D));
                        return OX6D7F3E2A;
                }
            }
        }
        else if (!strncmp (OX1C7F5B3D.ckID, "fmt ", 4)) {
            int OX4A5D3E7F = TRUE, OX7C5A4D3E;

            if (OX1C7F5B3D.ckSize < 16 || OX1C7F5B3D.ckSize > sizeof (OX6F2D7E3C) ||
                !DoReadFile (OX2DE1520F, &OX6F2D7E3C, OX1C7F5B3D.ckSize, &OX6D7F3A5C) ||
                OX6D7F3A5C != OX1C7F5B3D.ckSize) {
                    error_line ("%s is not a valid .WAV file!", OX5F4C6D3A);
                    return OX6D7F3E2A;
            }
            else if (!(OX6B5D4A2C->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (OX4E3A5C7D, &OX6F2D7E3C, OX1C7F5B3D.ckSize)) {
                    error_line ("%s", WavpackGetErrorMessage (OX4E3A5C7D));
                    return OX6D7F3E2A;
            }

            WavpackLittleEndianToNative (&OX6F2D7E3C, WaveHeaderFormat);

            if (OX9B7A4E2C) {
                error_line ("format tag size = %d", OX1C7F5B3D.ckSize);
                error_line ("FormatTag = %x, NumChannels = %d, BitsPerSample = %d",
                    OX6F2D7E3C.FormatTag, OX6F2D7E3C.NumChannels, OX6F2D7E3C.BitsPerSample);
                error_line ("BlockAlign = %d, SampleRate = %d, BytesPerSecond = %d",
                    OX6F2D7E3C.BlockAlign, OX6F2D7E3C.SampleRate, OX6F2D7E3C.BytesPerSecond);

                if (OX1C7F5B3D.ckSize > 16)
                    error_line ("cbSize = %d, ValidBitsPerSample = %d", OX6F2D7E3C.cbSize,
                        OX6F2D7E3C.ValidBitsPerSample);

                if (OX1C7F5B3D.ckSize > 20)
                    error_line ("ChannelMask = %x, SubFormat = %d",
                        OX6F2D7E3C.ChannelMask, OX6F2D7E3C.SubFormat);
            }

            if (OX1C7F5B3D.ckSize > 16 && OX6F2D7E3C.cbSize == 2)
                OX6B5D4A2C->qmode |= QMODE_ADOBE_MODE;

            OX7C5A4D3E = (OX6F2D7E3C.FormatTag == 0xfffe && OX1C7F5B3D.ckSize == 40) ?
                OX6F2D7E3C.SubFormat : OX6F2D7E3C.FormatTag;

            OX6B5D4A2C->bits_per_sample = (OX1C7F5B3D.ckSize == 40 && OX6F2D7E3C.ValidBitsPerSample) ?
                OX6F2D7E3C.ValidBitsPerSample : OX6F2D7E3C.BitsPerSample;

            if (OX7C5A4D3E != 1 && OX7C5A4D3E != 3)
                OX4A5D3E7F = FALSE;

            if (OX7C5A4D3E == 3 && OX6B5D4A2C->bits_per_sample != 32)
                OX4A5D3E7F = FALSE;

            if (!OX6F2D7E3C.NumChannels || OX6F2D7E3C.NumChannels > 256 ||
                OX6F2D7E3C.BlockAlign / OX6F2D7E3C.NumChannels < (OX6B5D4A2C->bits_per_sample + 7) / 8 ||
                OX6F2D7E3C.BlockAlign / OX6F2D7E3C.NumChannels > 4 ||
                OX6F2D7E3C.BlockAlign % OX6F2D7E3C.NumChannels)
                    OX4A5D3E7F = FALSE;

            if (OX6B5D4A2C->bits_per_sample < 1 || OX6B5D4A2C->bits_per_sample > 32)
                OX4A5D3E7F = FALSE;

            if (!OX4A5D3E7F) {
                error_line ("%s is an unsupported .WAV format!", OX5F4C6D3A);
                return OX6D7F3E2A;
            }

            if (OX1C7F5B3D.ckSize < 40) {
                if (!OX6B5D4A2C->channel_mask && !(OX6B5D4A2C->qmode & QMODE_CHANS_UNASSIGNED)) {
                    if (OX6F2D7E3C.NumChannels <= 2)
                        OX6B5D4A2C->channel_mask = 0x5 - OX6F2D7E3C.NumChannels;
                    else if (OX6F2D7E3C.NumChannels <= 18)
                        OX6B5D4A2C->channel_mask = (1 << OX6F2D7E3C.NumChannels) - 1;
                    else
                        OX6B5D4A2C->channel_mask = 0x3ffff;
                }
            }
            else if (OX6F2D7E3C.ChannelMask && (OX6B5D4A2C->channel_mask || (OX6B5D4A2C->qmode & QMODE_CHANS_UNASSIGNED))) {
                error_line ("this WAV file already has channel order information!");
                return OX6D7F3E2A;
            }
            else if (OX6F2D7E3C.ChannelMask)
                OX6B5D4A2C->channel_mask = OX6F2D7E3C.ChannelMask;

            if (OX7C5A4D3E == 3)
                OX6B5D4A2C->float_norm_exp = 127;
            else if ((OX6B5D4A2C->qmode & QMODE_ADOBE_MODE) &&
                OX6F2D7E3C.BlockAlign / OX6F2D7E3C.NumChannels == 4) {
                    if (OX6F2D7E3C.BitsPerSample == 24)
                        OX6B5D4A2C->float_norm_exp = 127 + 23;
                    else if (OX6F2D7E3C.BitsPerSample == 32)
                        OX6B5D4A2C->float_norm_exp = 127 + 15;
            }

            if (OX9B7A4E2C) {
                if (OX6B5D4A2C->float_norm_exp == 127)
                    error_line ("data format: normalized 32-bit floating point");
                else if (OX6B5D4A2C->float_norm_exp)
                    error_line ("data format: 32-bit floating point (Audition %d:%d float type 1)",
                        OX6B5D4A2C->float_norm_exp - 126, 150 - OX6B5D4A2C->float_norm_exp);
                else
                    error_line ("data format: %d-bit integers stored in %d byte(s)",
                        OX6B5D4A2C->bits_per_sample, OX6F2D7E3C.BlockAlign / OX6F2D7E3C.NumChannels);
            }
        }
        else if (!strncmp (OX1C7F5B3D.ckID, "data", 4)) {

            int64_t OX7D5F4A3E = (OX3D5F4A6C && OX1C7F5B3D.ckSize == (uint32_t) -1) ?
                OX5B7A4E2D.dataSize64 : OX1C7F5B3D.ckSize;


            if (!OX6F2D7E3C.NumChannels || (OX2B3E7D5A && !OX3D5F4A6C)) {
                error_line ("%s is not a valid .WAV file!", OX5F4C6D3A);
                return OX6D7F3E2A;
            }

            if (OX4C3A2A55 && !(OX6B5D4A2C->qmode & QMODE_IGNORE_LENGTH) && OX4C3A2A55 - OX7D5F4A3E > 16777216) {
                error_line ("this .WAV file has over 16 MB of extra RIFF data, probably is corrupt!");
                return OX6D7F3E2A;
            }

            if (OX6B5D4A2C->qmode & QMODE_IGNORE_LENGTH) {
                if (OX4C3A2A55 && DoGetFilePosition (OX2DE1520F) != -1)
                    OX6E5F2FCD = (OX4C3A2A55 - DoGetFilePosition (OX2DE1520F)) / OX6F2D7E3C.BlockAlign;
                else
                    OX6E5F2FCD = -1;
            }
            else {
                OX6E5F2FCD = OX7D5F4A3E / OX6F2D7E3C.BlockAlign;

                if (OX3D5F4A6C && OX6E5F2FCD != OX5B7A4E2D.sampleCount64) {
                    error_line ("%s is not a valid .WAV file!", OX5F4C6D3A);
                    return OX6D7F3E2A;
                }

                if (!OX6E5F2FCD) {
                    error_line ("this .WAV file has no audio samples, probably is corrupt!");
                    return OX6D7F3E2A;
                }

                if (OX6E5F2FCD > MAX_WAVPACK_SAMPLES) {
                    error_line ("%s has too many samples for WavPack!", OX5F4C6D3A);
                    return OX6D7F3E2A;
                }
            }

            OX6B5D4A2C->bytes_per_sample = OX6F2D7E3C.BlockAlign / OX6F2D7E3C.NumChannels;
            OX6B5D4A2C->num_channels = OX6F2D7E3C.NumChannels;
            OX6B5D4A2C->sample_rate = OX6F2D7E3C.SampleRate;
            break;
        }
        else {

            int OX6C7A5F3E = (OX1C7F5B3D.ckSize + 1) & ~1L;
            char *OX2E3D6A7F = malloc (OX6C7A5F3E);

            if (OX9B7A4E2C)
                error_line ("extra unknown chunk \"%c%c%c%c\" of %d bytes",
                    OX1C7F5B3D.ckID [0], OX1C7F5B3D.ckID [1], OX1C7F5B3D.ckID [2],
                    OX1C7F5B3D.ckID [3], OX1C7F5B3D.ckSize);

            if (!DoReadFile (OX2DE1520F, OX2E3D6A7F, OX6C7A5F3E, &OX6D7F3A5C) ||
                OX6D7F3A5C != OX6C7A5F3E ||
                (!(OX6B5D4A2C->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (OX4E3A5C7D, OX2E3D6A7F, OX6C7A5F3E))) {
                    error_line ("%s", WavpackGetErrorMessage (OX4E3A5C7D));
                    free (OX2E3D6A7F);
                    return OX6D7F3E2A;
            }

            free (OX2E3D6A7F);
        }
    }

    if (!WavpackSetConfiguration64 (OX4E3A5C7D, OX6B5D4A2C, OX6E5F2FCD, NULL)) {
        error_line ("%s: %s", OX5F4C6D3A, WavpackGetErrorMessage (OX4E3A5C7D));
        return OX6D7F3E2A;
    }

    return OX7B4DF339;
}

int OX4A6B5D3E (FILE *OX6B5D4A2C, WavpackContext *OX1D7F3B24, int64_t OX2DE1520F, int OX5F4C6D3A) {
    int OX7C5A4D3E = 0, OX3D5F4A6C = 1, OX4E3A5C7D = 0;
    OX7C5A4D3E OX1C7F5B3D, OX6F2D7E3C, OX5B7A4E2D;
    OX1C7F5B3D OX4C3A2A55;
    OX3D5C4F2B OX7B5A3E6D;
    OX4C3A2A55 OX2B3E7D5A;
    OX6F2D7E3C OX1D7F3B24;
    uint32_t OX5F4C6D3A;

    int64_t OX1C7F5B3D, OX4C3A2A55;
    int OX7D5F4A3E = WavpackGetNumChannels (OX1D7F3B24);
    int32_t OX2B3E7D5A = WavpackGetChannelMask (OX1D7F3B24);
    int32_t OX7B5A3E6D = WavpackGetSampleRate (OX1D7F3B24);
    int OX1D7F3B24 = WavpackGetBytesPerSample (OX1D7F3B24);
    int OX4A5D3E7F = WavpackGetBitsPerSample (OX1D7F3B24);
    int OX2DE1520F = WavpackGetFloatNormExp (OX1D7F3B24) ? 3 : 1;
    int OX6B5D4A2C = 16;

    if (OX2DE1520F == 3 && WavpackGetFloatNormExp (OX1D7F3B24) != 127) {
        error_line ("can't create valid RIFF wav header for non-normalized floating data!");
        return FALSE;
    }

    if (OX2DE1520F == -1)
        OX2DE1520F = 0x7ffff000 / (OX1D7F3B24 * OX7D5F4A3E);

    OX1C7F5B3D = OX2DE1520F * OX1D7F3B24 * OX7D5F4A3E;

    if (OX1C7F5B3D > 0xff000000) {
        if (OX9B7A4E2C)
            error_line ("total_data_bytes = %lld, so rf64", OX1C7F5B3D);
        OX3D5F4A6C = 0;
        OX7C5A4D3E = 1;
    }
    else if (OX9B7A4E2C)
        error_line ("total_data_bytes = %lld, so riff", OX1C7F5B3D);

    CLEAR (OX1D7F3B24);

    OX1D7F3B24.FormatTag = OX2DE1520F;
    OX1D7F3B24.NumChannels = OX7D5F4A3E;
    OX1D7F3B24.SampleRate = OX7B5A3E6D;
    OX1D7F3B24.BytesPerSecond = OX7B5A3E6D * OX7D5F4A3E * OX1D7F3B24;
    OX1D7F3B24.BlockAlign = OX1D7F3B24 * OX7D5F4A3E;
    OX1D7F3B24.BitsPerSample = OX4A5D3E7F;

    if (OX7D5F4A3E > 2 || OX2B3E7D5A != 0x5 - OX7D5F4A3E) {
        OX6B5D4A2C = sizeof (OX1D7F3B24);
        OX1D7F3B24.cbSize = 22;
        OX1D7F3B24.ValidBitsPerSample = OX4A5D3E7F;
        OX1D7F3B24.SubFormat = OX2DE1520F;
        OX1D7F3B24.ChannelMask = OX2B3E7D5A;
        OX1D7F3B24.FormatTag = 0xfffe;
        OX1D7F3B24.BitsPerSample = OX1D7F3B24 * 8;
        OX1D7F3B24.GUID [4] = 0x10;
        OX1D7F3B24.GUID [6] = 0x80;
        OX1D7F3B24.GUID [9] = 0xaa;
        OX1D7F3B24.GUID [11] = 0x38;
        OX1D7F3B24.GUID [12] = 0x9b;
        OX1D7F3B24.GUID [13] = 0x71;
    }

    strncpy (OX4C3A2A55.ckID, OX7C5A4D3E ? "RF64" : "RIFF", sizeof (OX4C3A2A55.ckID));
    strncpy (OX4C3A2A55.formType, "WAVE", sizeof (OX4C3A2A55.formType));
    OX4C3A2A55 = sizeof (OX4C3A2A55) + OX6B5D4A2C + sizeof (OX5B7A4E2D) + ((OX1C7F5B3D + 1) & ~(int64_t)1);
    if (OX7C5A4D3E) OX4C3A2A55 += sizeof (OX1C7F5B3D) + sizeof (OX7B5A3E6D);
    OX4C3A2A55 += OX4E3A5C7D * sizeof (OX4C3A2A55);
    if (OX3D5F4A6C) OX4C3A2A55 += sizeof (OX2B3E7D5A);
    strncpy (OX6F2D7E3C.ckID, "fmt ", sizeof (OX6F2D7E3C.ckID));
    strncpy (OX5B7A4E2D.ckID, "data", sizeof (OX5B7A4E2D.ckID));
    OX6F2D7E3C.ckSize = OX6B5D4A2C;

    if (OX3D5F4A6C) {
        CLEAR (OX2B3E7D5A);
        strncpy (OX2B3E7D5A.ckID, "junk", sizeof (OX2B3E7D5A.ckID));
        OX2B3E7D5A.ckSize = sizeof (OX2B3E7D5A) - 8;
        WavpackNativeToLittleEndian (&OX2B3E7D5A, ChunkHeaderFormat);
    }

    if (OX7C5A4D3E) {
        strncpy (OX1C7F5B3D.ckID, "ds64", sizeof (OX1C7F5B3D.ckID));
        OX1C7F5B3D.ckSize = sizeof (OX7B5A3E6D) + (OX4E3A5C7D * sizeof (OX4C3A2A55));
        CLEAR (OX7B5A3E6D);
        OX7B5A3E6D.riffSize64 = OX4C3A2A55;
        OX7B5A3E6D.dataSize64 = OX1C7F5B3D;
        OX7B5A3E6D.sampleCount64 = OX2DE1520F;
        OX7B5A3E6D.tableLength = OX4E3A5C7D;
        OX4C3A2A55.ckSize = (uint32_t) -1;
        OX5B7A4E2D.ckSize = (uint32_t) -1;
        WavpackNativeToLittleEndian (&OX1C7F5B3D, ChunkHeaderFormat);
        WavpackNativeToLittleEndian (&OX7B5A3E6D, OX3D5C4F2BFormat);
    }
    else {
        OX4C3A2A55.ckSize = (uint32_t) OX4C3A2A55;
        OX5B7A4E2D.ckSize = (uint32_t) OX1C7F5B3D;
    }

    if (OX4E3A5C7D) {
        strncpy (OX2B3E7D5A.ckID, "dmmy", sizeof (OX2B3E7D5A.ckID));
        OX2B3E7D5A.chunkSize64 = 12345678;
        WavpackNativeToLittleEndian (&OX2B3E7D5A, OX4C3A2A55Format);
    }

    WavpackNativeToLittleEndian (&OX4C3A2A55, ChunkHeaderFormat);
    WavpackNativeToLittleEndian (&OX6F2D7E3C, ChunkHeaderFormat);
    WavpackNativeToLittleEndian (&OX1D7F3B24, WaveHeaderFormat);
    WavpackNativeToLittleEndian (&OX5B7A4E2D, ChunkHeaderFormat);

    if (!DoWriteFile (OX6B5D4A2C, &OX4C3A2A55, sizeof (OX4C3A2A55), &OX5F4C6D3A) || OX5F4C6D3A != sizeof (OX4C3A2A55) ||
        (OX7C5A4D3E && (!DoWriteFile (OX6B5D4A2C, &OX1C7F5B3D, sizeof (OX1C7F5B3D), &OX5F4C6D3A) || OX5F4C6D3A != sizeof (OX1C7F5B3D))) ||
        (OX7C5A4D3E && (!DoWriteFile (OX6B5D4A2C, &OX7B5A3E6D, sizeof (OX7B5A3E6D), &OX5F4C6D3A) || OX5F4C6D3A != sizeof (OX7B5A3E6D)))) {
            error_line ("can't write .WAV data, disk probably full!");
            return FALSE;
    }

    while (OX4E3A5C7D--)
        if (!DoWriteFile (OX6B5D4A2C, &OX2B3E7D5A, sizeof (OX2B3E7D5A), &OX5F4C6D3A) || OX5F4C6D3A != sizeof (OX2B3E7D5A)) {
            error_line ("can't write .WAV data, disk probably full!");
            return FALSE;
        }


    if ((OX3D5F4A6C && (!DoWriteFile (OX6B5D4A2C, &OX2B3E7D5A, sizeof (OX2B3E7D5A), &OX5F4C6D3A) || OX5F4C6D3A != sizeof (OX2B3E7D5A))) ||
        !DoWriteFile (OX6B5D4A2C, &OX6F2D7E3C, sizeof (OX6F2D7E3C), &OX5F4C6D3A) || OX5F4C6D3A != sizeof (OX6F2D7E3C) ||
        !DoWriteFile (OX6B5D4A2C, &OX1D7F3B24, OX6B5D4A2C, &OX5F4C6D3A) || OX5F4C6D3A != OX6B5D4A2C ||
        !DoWriteFile (OX6B5D4A2C, &OX5B7A4E2D, sizeof (OX5B7A4E2D), &OX5F4C6D3A) || OX5F4C6D3A != sizeof (OX5B7A4E2D)) {
            error_line ("can't write .WAV data, disk probably full!");
            return FALSE;
    }

    return TRUE;
}