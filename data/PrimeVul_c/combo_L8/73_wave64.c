#include <Python.h>
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
    char ckID [16];
    int64_t ckSize;
    char formType [16];
} Wave64FileHeader;

typedef struct {
    char ckID [16];
    int64_t ckSize;
} Wave64ChunkHeader;

#define Wave64ChunkHeaderFormat "88D"

static const unsigned char riff_guid [16] = { 'r','i','f','f', 0x2e,0x91,0xcf,0x11,0xa5,0xd6,0x28,0xdb,0x04,0xc1,0x00,0x00 };
static const unsigned char wave_guid [16] = { 'w','a','v','e', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };
static const unsigned char  fmt_guid [16] = { 'f','m','t',' ', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };
static const unsigned char data_guid [16] = { 'd','a','t','a', 0xf3,0xac,0xd3,0x11,0x8c,0xd1,0x00,0xc0,0x4f,0x8e,0xdb,0x8a };

#define WAVPACK_NO_ERROR    0
#define WAVPACK_SOFT_ERROR  1
#define WAVPACK_HARD_ERROR  2

extern int debug_logging_mode;

PyObject* ParseWave64HeaderConfig(PyObject* self, PyObject* args) {
    FILE *infile;
    char *infilename, *fourcc;
    WavpackContext *wpc;
    WavpackConfig *config;
    int64_t total_samples = 0, infilesize;
    Wave64ChunkHeader chunk_header;
    Wave64FileHeader filehdr;
    WaveHeader WaveHeader;
    uint32_t bcount;

    if (!PyArg_ParseTuple(args, "ssOO", &infilename, &fourcc, &wpc, &config)) {
        return NULL;
    }

    infile = fopen(infilename, "rb");
    if (!infile) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot open input file.");
        return NULL;
    }

    infilesize = DoGetFileSize(infile);
    memcpy(&filehdr, fourcc, 4);

    if (!DoReadFile(infile, ((char *)&filehdr) + 4, sizeof(Wave64FileHeader) - 4, &bcount) ||
        bcount != sizeof(Wave64FileHeader) - 4 || memcmp(filehdr.ckID, riff_guid, sizeof(riff_guid)) ||
        memcmp(filehdr.formType, wave_guid, sizeof(wave_guid))) {
        fclose(infile);
        PyErr_SetString(PyExc_RuntimeError, "File is not a valid .W64 file.");
        return NULL;
    }
    else if (!(config->qmode & QMODE_NO_STORE_WRAPPER) &&
             !WavpackAddWrapper(wpc, &filehdr, sizeof(filehdr))) {
        fclose(infile);
        PyErr_SetString(PyExc_RuntimeError, WavpackGetErrorMessage(wpc));
        return NULL;
    }

    WavpackLittleEndianToNative(&filehdr, Wave64ChunkHeaderFormat);

    if (infilesize && !(config->qmode & QMODE_IGNORE_LENGTH) &&
        filehdr.ckSize && filehdr.ckSize + 1 && filehdr.ckSize != infilesize) {
        fclose(infile);
        PyErr_SetString(PyExc_RuntimeError, "File is not a valid .W64 file.");
        return NULL;
    }

    while (1) {
        if (!DoReadFile(infile, &chunk_header, sizeof(Wave64ChunkHeader), &bcount) ||
            bcount != sizeof(Wave64ChunkHeader)) {
            fclose(infile);
            PyErr_SetString(PyExc_RuntimeError, "File is not a valid .W64 file.");
            return NULL;
        }
        else if (!(config->qmode & QMODE_NO_STORE_WRAPPER) &&
                 !WavpackAddWrapper(wpc, &chunk_header, sizeof(Wave64ChunkHeader))) {
            fclose(infile);
            PyErr_SetString(PyExc_RuntimeError, WavpackGetErrorMessage(wpc));
            return NULL;
        }

        WavpackLittleEndianToNative(&chunk_header, Wave64ChunkHeaderFormat);
        chunk_header.ckSize -= sizeof(chunk_header);

        if (!memcmp(chunk_header.ckID, fmt_guid, sizeof(fmt_guid))) {
            int supported = TRUE, format;

            chunk_header.ckSize = (chunk_header.ckSize + 7) & ~7L;

            if (chunk_header.ckSize < 16 || chunk_header.ckSize > sizeof(WaveHeader) ||
                !DoReadFile(infile, &WaveHeader, (uint32_t)chunk_header.ckSize, &bcount) ||
                bcount != chunk_header.ckSize) {
                fclose(infile);
                PyErr_SetString(PyExc_RuntimeError, "File is not a valid .W64 file.");
                return NULL;
            }
            else if (!(config->qmode & QMODE_NO_STORE_WRAPPER) &&
                     !WavpackAddWrapper(wpc, &WaveHeader, (uint32_t)chunk_header.ckSize)) {
                fclose(infile);
                PyErr_SetString(PyExc_RuntimeError, WavpackGetErrorMessage(wpc));
                return NULL;
            }

            WavpackLittleEndianToNative(&WaveHeader, WaveHeaderFormat);

            if (debug_logging_mode) {
                error_line("format tag size = %d", chunk_header.ckSize);
                error_line("FormatTag = %x, NumChannels = %d, BitsPerSample = %d",
                           WaveHeader.FormatTag, WaveHeader.NumChannels, WaveHeader.BitsPerSample);
                error_line("BlockAlign = %d, SampleRate = %d, BytesPerSecond = %d",
                           WaveHeader.BlockAlign, WaveHeader.SampleRate, WaveHeader.BytesPerSecond);

                if (chunk_header.ckSize > 16)
                    error_line("cbSize = %d, ValidBitsPerSample = %d", WaveHeader.cbSize,
                               WaveHeader.ValidBitsPerSample);

                if (chunk_header.ckSize > 20)
                    error_line("ChannelMask = %x, SubFormat = %d",
                               WaveHeader.ChannelMask, WaveHeader.SubFormat);
            }

            if (chunk_header.ckSize > 16 && WaveHeader.cbSize == 2)
                config->qmode |= QMODE_ADOBE_MODE;

            format = (WaveHeader.FormatTag == 0xfffe && chunk_header.ckSize == 40) ?
                     WaveHeader.SubFormat : WaveHeader.FormatTag;

            config->bits_per_sample = (chunk_header.ckSize == 40 && WaveHeader.ValidBitsPerSample) ?
                                      WaveHeader.ValidBitsPerSample : WaveHeader.BitsPerSample;

            if (format != 1 && format != 3)
                supported = FALSE;

            if (format == 3 && config->bits_per_sample != 32)
                supported = FALSE;

            if (!WaveHeader.NumChannels || WaveHeader.NumChannels > 256 ||
                WaveHeader.BlockAlign / WaveHeader.NumChannels < (config->bits_per_sample + 7) / 8 ||
                WaveHeader.BlockAlign / WaveHeader.NumChannels > 4 ||
                WaveHeader.BlockAlign % WaveHeader.NumChannels)
                supported = FALSE;

            if (config->bits_per_sample < 1 || config->bits_per_sample > 32)
                supported = FALSE;

            if (!supported) {
                fclose(infile);
                PyErr_SetString(PyExc_RuntimeError, "Unsupported .W64 format.");
                return NULL;
            }

            if (chunk_header.ckSize < 40) {
                if (!config->channel_mask && !(config->qmode & QMODE_CHANS_UNASSIGNED)) {
                    if (WaveHeader.NumChannels <= 2)
                        config->channel_mask = 0x5 - WaveHeader.NumChannels;
                    else if (WaveHeader.NumChannels <= 18)
                        config->channel_mask = (1 << WaveHeader.NumChannels) - 1;
                    else
                        config->channel_mask = 0x3ffff;
                }
            }
            else if (WaveHeader.ChannelMask && (config->channel_mask || (config->qmode & QMODE_CHANS_UNASSIGNED))) {
                fclose(infile);
                PyErr_SetString(PyExc_RuntimeError, "W64 file already has channel order information.");
                return NULL;
            }
            else if (WaveHeader.ChannelMask)
                config->channel_mask = WaveHeader.ChannelMask;

            if (format == 3)
                config->float_norm_exp = 127;
            else if ((config->qmode & QMODE_ADOBE_MODE) &&
                     WaveHeader.BlockAlign / WaveHeader.NumChannels == 4) {
                if (WaveHeader.BitsPerSample == 24)
                    config->float_norm_exp = 127 + 23;
                else if (WaveHeader.BitsPerSample == 32)
                    config->float_norm_exp = 127 + 15;
            }

            if (debug_logging_mode) {
                if (config->float_norm_exp == 127)
                    error_line("data format: normalized 32-bit floating point");
                else
                    error_line("data format: %d-bit integers stored in %d byte(s)",
                               config->bits_per_sample, WaveHeader.BlockAlign / WaveHeader.NumChannels);
            }
        }
        else if (!memcmp(chunk_header.ckID, data_guid, sizeof(data_guid))) {
            if (!WaveHeader.NumChannels) {
                fclose(infile);
                PyErr_SetString(PyExc_RuntimeError, "File is not a valid .W64 file.");
                return NULL;
            }

            if ((config->qmode & QMODE_IGNORE_LENGTH) || chunk_header.ckSize <= 0) {
                config->qmode |= QMODE_IGNORE_LENGTH;

                if (infilesize && DoGetFilePosition(infile) != -1)
                    total_samples = (infilesize - DoGetFilePosition(infile)) / WaveHeader.BlockAlign;
                else
                    total_samples = -1;
            }
            else {
                if (infilesize && infilesize - chunk_header.ckSize > 16777216) {
                    fclose(infile);
                    PyErr_SetString(PyExc_RuntimeError, "File has over 16 MB of extra RIFF data, possibly corrupt.");
                    return NULL;
                }

                total_samples = chunk_header.ckSize / WaveHeader.BlockAlign;

                if (!total_samples) {
                    fclose(infile);
                    PyErr_SetString(PyExc_RuntimeError, "File has no audio samples, possibly corrupt.");
                    return NULL;
                }

                if (total_samples > MAX_WAVPACK_SAMPLES) {
                    fclose(infile);
                    PyErr_SetString(PyExc_RuntimeError, "File has too many samples for WavPack.");
                    return NULL;
                }
            }

            config->bytes_per_sample = WaveHeader.BlockAlign / WaveHeader.NumChannels;
            config->num_channels = WaveHeader.NumChannels;
            config->sample_rate = WaveHeader.SampleRate;
            break;
        }
        else {
            int bytes_to_copy = (chunk_header.ckSize + 7) & ~7L;
            char *buff = malloc(bytes_to_copy);

            if (debug_logging_mode)
                error_line("extra unknown chunk \"%c%c%c%c\" of %d bytes",
                           chunk_header.ckID[0], chunk_header.ckID[1], chunk_header.ckID[2],
                           chunk_header.ckID[3], chunk_header.ckSize);

            if (!DoReadFile(infile, buff, bytes_to_copy, &bcount) ||
                bcount != bytes_to_copy ||
                (!(config->qmode & QMODE_NO_STORE_WRAPPER) &&
                 !WavpackAddWrapper(wpc, buff, bytes_to_copy))) {
                fclose(infile);
                free(buff);
                PyErr_SetString(PyExc_RuntimeError, WavpackGetErrorMessage(wpc));
                return NULL;
            }

            free(buff);
        }
    }

    if (!WavpackSetConfiguration64(wpc, config, total_samples, NULL)) {
        fclose(infile);
        PyErr_SetString(PyExc_RuntimeError, WavpackGetErrorMessage(wpc));
        return NULL;
    }

    fclose(infile);
    Py_RETURN_NONE;
}

PyObject* WriteWave64Header(PyObject* self, PyObject* args) {
    FILE *outfile;
    WavpackContext *wpc;
    int64_t total_samples;
    int qmode;

    if (!PyArg_ParseTuple(args, "OLi", &outfile, &wpc, &total_samples, &qmode)) {
        return NULL;
    }

    Wave64ChunkHeader datahdr, fmthdr;
    Wave64FileHeader filehdr;
    WaveHeader wavhdr;
    uint32_t bcount;

    int64_t total_data_bytes, total_file_bytes;
    int num_channels = WavpackGetNumChannels(wpc);
    int32_t channel_mask = WavpackGetChannelMask(wpc);
    int32_t sample_rate = WavpackGetSampleRate(wpc);
    int bytes_per_sample = WavpackGetBytesPerSample(wpc);
    int bits_per_sample = WavpackGetBitsPerSample(wpc);
    int format = WavpackGetFloatNormExp(wpc) ? 3 : 1;
    int wavhdrsize = 16;

    if (format == 3 && WavpackGetFloatNormExp(wpc) != 127) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot create valid Wave64 header for non-normalized floating data.");
        return NULL;
    }

    if (total_samples == -1)
        total_samples = 0x7ffff000 / (bytes_per_sample * num_channels);

    total_data_bytes = total_samples * bytes_per_sample * num_channels;
    CLEAR(wavhdr);

    wavhdr.FormatTag = format;
    wavhdr.NumChannels = num_channels;
    wavhdr.SampleRate = sample_rate;
    wavhdr.BytesPerSecond = sample_rate * num_channels * bytes_per_sample;
    wavhdr.BlockAlign = bytes_per_sample * num_channels;
    wavhdr.BitsPerSample = bits_per_sample;

    if (num_channels > 2 || channel_mask != 0x5 - num_channels) {
        wavhdrsize = sizeof(wavhdr);
        wavhdr.cbSize = 22;
        wavhdr.ValidBitsPerSample = bits_per_sample;
        wavhdr.SubFormat = format;
        wavhdr.ChannelMask = channel_mask;
        wavhdr.FormatTag = 0xfffe;
        wavhdr.BitsPerSample = bytes_per_sample * 8;
        wavhdr.GUID[4] = 0x10;
        wavhdr.GUID[6] = 0x80;
        wavhdr.GUID[9] = 0xaa;
        wavhdr.GUID[11] = 0x38;
        wavhdr.GUID[12] = 0x9b;
        wavhdr.GUID[13] = 0x71;
    }

    total_file_bytes = sizeof(filehdr) + sizeof(fmthdr) + wavhdrsize + sizeof(datahdr) + ((total_data_bytes + 7) & ~(int64_t)7);

    memcpy(filehdr.ckID, riff_guid, sizeof(riff_guid));
    memcpy(filehdr.formType, wave_guid, sizeof(wave_guid));
    filehdr.ckSize = total_file_bytes;

    memcpy(fmthdr.ckID, fmt_guid, sizeof(fmt_guid));
    fmthdr.ckSize = sizeof(fmthdr) + wavhdrsize;

    memcpy(datahdr.ckID, data_guid, sizeof(data_guid));
    datahdr.ckSize = total_data_bytes + sizeof(datahdr);

    WavpackNativeToLittleEndian(&filehdr, Wave64ChunkHeaderFormat);
    WavpackNativeToLittleEndian(&fmthdr, Wave64ChunkHeaderFormat);
    WavpackNativeToLittleEndian(&wavhdr, WaveHeaderFormat);
    WavpackNativeToLittleEndian(&datahdr, Wave64ChunkHeaderFormat);

    if (!DoWriteFile(outfile, &filehdr, sizeof(filehdr), &bcount) || bcount != sizeof(filehdr) ||
        !DoWriteFile(outfile, &fmthdr, sizeof(fmthdr), &bcount) || bcount != sizeof(fmthdr) ||
        !DoWriteFile(outfile, &wavhdr, wavhdrsize, &bcount) || bcount != wavhdrsize ||
        !DoWriteFile(outfile, &datahdr, sizeof(datahdr), &bcount) || bcount != sizeof(datahdr)) {
        PyErr_SetString(PyExc_RuntimeError, "Cannot write .W64 data, disk probably full.");
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyMethodDef WavepackMethods[] = {
    {"parse_wave64_header_config", ParseWave64HeaderConfig, METH_VARARGS, "Parse Wave64 Header Config"},
    {"write_wave64_header", WriteWave64Header, METH_VARARGS, "Write Wave64 Header"},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef wavpackmodule = {
    PyModuleDef_HEAD_INIT,
    "wavpack",
    NULL,
    -1,
    WavepackMethods
};

PyMODINIT_FUNC PyInit_wavpack(void) {
    return PyModule_Create(&wavpackmodule);
}