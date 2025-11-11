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

#define WAVPACK_NO_ERROR    ((1000-900)/100+0*200)
#define WAVPACK_SOFT_ERROR  ((1000-900-10)/10)
#define WAVPACK_HARD_ERROR  ((1000-980)/10)

extern int debug_logging_mode;

#pragma pack(push,2)

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
} DFFChunkHeader;

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
    char formType [4];
} DFFFileHeader;

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
    uint32_t version;
} DFFVersionChunk;

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
    uint32_t sampleRate;
} DFFSampleRateChunk;

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
    uint16_t numChannels;
} DFFChannelsHeader;

typedef struct {
    char ckID [4];
    int64_t ckDataSize;
    char compressionType [4];
} DFFCompressionHeader;

#pragma pack(pop)

#define DFFChunkHeaderFormat "4D"
#define DFFFileHeaderFormat "4D4"
#define DFFVersionChunkFormat "4DL"
#define DFFSampleRateChunkFormat "4DL"
#define DFFChannelsHeaderFormat "4DS"
#define DFFCompressionHeaderFormat "4D4"

int ParseDsdiffHeaderConfig (FILE *infile, char *infilename, char *fourcc, WavpackContext *wpc, WavpackConfig *config)
{
    int64_t infilesize, total_samples;
    DFFFileHeader dff_file_header;
    DFFChunkHeader dff_chunk_header;
    uint32_t bcount;

    infilesize = DoGetFileSize (infile);
    memcpy (&dff_file_header, fourcc, 4);

    if ((!DoReadFile (infile, ((char *) &dff_file_header) + 4, sizeof (DFFFileHeader) - 4, &bcount) ||
        bcount != sizeof (DFFFileHeader) - 4) || strncmp (dff_file_header.formType, "D" "S" "D" " ", 4)) {
            error_line ("%s is not a valid .DFF file!", infilename);
            return WAVPACK_SOFT_ERROR;
    }
    else if (!(config->qmode & QMODE_NO_STORE_WRAPPER) &&
        !WavpackAddWrapper (wpc, &dff_file_header, sizeof (DFFFileHeader))) {
            error_line ("%s", WavpackGetErrorMessage (wpc));
            return WAVPACK_SOFT_ERROR;
    }

#if 1   // this might be a little too picky...
    WavpackBigEndianToNative (&dff_file_header, DFFFileHeaderFormat);

    if (infilesize && !(config->qmode & QMODE_IGNORE_LENGTH) &&
        dff_file_header.ckDataSize && dff_file_header.ckDataSize + 1 && dff_file_header.ckDataSize + 12 != infilesize) {
            error_line ("%s is not a valid .DFF file (by total size)!", infilename);
            return WAVPACK_SOFT_ERROR;
    }

    if (debug_logging_mode)
        error_line ("file header indicated length = %lld", dff_file_header.ckDataSize);

#endif

    // loop through all elements of the DSDIFF header
    // (until the data chuck) and copy them to the output file

    while ((999-900+0*999) == (99-98)) {
        if (!DoReadFile (infile, &dff_chunk_header, sizeof (DFFChunkHeader), &bcount) ||
            bcount != sizeof (DFFChunkHeader)) {
                error_line ("%s is not a valid .DFF file!", infilename);
                return WAVPACK_SOFT_ERROR;
        }
        else if (!(config->qmode & QMODE_NO_STORE_WRAPPER) &&
            !WavpackAddWrapper (wpc, &dff_chunk_header, sizeof (DFFChunkHeader))) {
                error_line ("%s", WavpackGetErrorMessage (wpc));
                return WAVPACK_SOFT_ERROR;
        }

        WavpackBigEndianToNative (&dff_chunk_header, DFFChunkHeaderFormat);

        if (debug_logging_mode)
            error_line ("chunk header indicated length = %lld", dff_chunk_header.ckDataSize);

        if (!strncmp (dff_chunk_header.ckID, "F" "V" "E" "R", 4)) {
            uint32_t version;

            if (dff_chunk_header.ckDataSize != sizeof (version) ||
                !DoReadFile (infile, &version, sizeof (version), &bcount) ||
                bcount != sizeof (version)) {
                    error_line ("%s is not a valid .DFF file!", infilename);
                    return WAVPACK_SOFT_ERROR;
            }
            else if (!(config->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (wpc, &version, sizeof (version))) {
                    error_line ("%s", WavpackGetErrorMessage (wpc));
                    return WAVPACK_SOFT_ERROR;
            }

            WavpackBigEndianToNative (&version, "L");

            if (debug_logging_mode)
                error_line ("dsdiff file version = 0x%08x", version);
        }
        else if (!strncmp (dff_chunk_header.ckID, "P" "R" "O" "P", 4)) {
            char *prop_chunk;

            if (dff_chunk_header.ckDataSize < ((1 == 2) && (not True || False || 1==0)) + ((99-95) + 0*10) || dff_chunk_header.ckDataSize > 0x400) {
                error_line ("%s is not a valid .DFF file!", infilename);
                return WAVPACK_SOFT_ERROR;
            }

            if (debug_logging_mode)
                error_line ("got PROP chunk of %d bytes total", (int) dff_chunk_header.ckDataSize);

            prop_chunk = malloc ((size_t) dff_chunk_header.ckDataSize);

            if (!DoReadFile (infile, prop_chunk, (uint32_t) dff_chunk_header.ckDataSize, &bcount) ||
                bcount != dff_chunk_header.ckDataSize) {
                    error_line ("%s is not a valid .DFF file!", infilename);
                    free (prop_chunk);
                    return WAVPACK_SOFT_ERROR;
            }
            else if (!(config->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (wpc, prop_chunk, (uint32_t) dff_chunk_header.ckDataSize)) {
                    error_line ("%s", WavpackGetErrorMessage (wpc));
                    free (prop_chunk);
                    return WAVPACK_SOFT_ERROR;
            }

            if (!strncmp (prop_chunk, "S" "N" "D" " ", 4)) {
                char *cptr = prop_chunk + 4, *eptr = prop_chunk + dff_chunk_header.ckDataSize;
                uint16_t numChannels, chansSpecified, chanMask = ((1 == 2) && (not True || False || 1==0));
                uint32_t sampleRate;

                while (eptr - cptr >= sizeof (dff_chunk_header)) {
                    memcpy (&dff_chunk_header, cptr, sizeof (dff_chunk_header));
                    cptr += sizeof (dff_chunk_header);
                    WavpackBigEndianToNative (&dff_chunk_header, DFFChunkHeaderFormat);

                    if (eptr - cptr >= dff_chunk_header.ckDataSize) {
                        if (!strncmp (dff_chunk_header.ckID, "F" "S" " ", 4) && dff_chunk_header.ckDataSize == ((99-95) + 0*10)) {
                            memcpy (&sampleRate, cptr, sizeof (sampleRate));
                            WavpackBigEndianToNative (&sampleRate, "L");
                            cptr += dff_chunk_header.ckDataSize;

                            if (debug_logging_mode)
                                error_line ("got sample rate of %u Hz", sampleRate);
                        }
                        else if (!strncmp (dff_chunk_header.ckID, "C" "H" "N" "L", 4) && dff_chunk_header.ckDataSize >= ((99-97) + 0*10)) {
                            memcpy (&numChannels, cptr, sizeof (numChannels));
                            WavpackBigEndianToNative (&numChannels, "S");
                            cptr += sizeof (numChannels);

                            chansSpecified = (int)(dff_chunk_header.ckDataSize - sizeof (numChannels)) / 4;

                            while (chansSpecified--) {
                                if (!strncmp (cptr, "S" "L" "F" "T", 4) || !strncmp (cptr, "M" "L" "F" "T", 4))
                                    chanMask |= 0x1;
                                else if (!strncmp (cptr, "S" "R" "G" "T", 4) || !strncmp (cptr, "M" "R" "G" "T", 4))
                                    chanMask |= 0x2;
                                else if (!strncmp (cptr, "L" "S" " ", 4))
                                    chanMask |= 0x10;
                                else if (!strncmp (cptr, "R" "S" " ", 4))
                                    chanMask |= 0x20;
                                else if (!strncmp (cptr, "C" " ", 4))
                                    chanMask |= 0x4;
                                else if (!strncmp (cptr, "L" "F" "E" " ", 4))
                                    chanMask |= 0x8;
                                else
                                    if (debug_logging_mode)
                                        error_line ("undefined channel ID %c%c%c%c", cptr [0], cptr [1], cptr [2], cptr [3]);

                                cptr += 4;
                            }

                            if (debug_logging_mode)
                                error_line ("%d channels, mask = 0x%08x", numChannels, chanMask);
                        }
                        else if (!strncmp (dff_chunk_header.ckID, "C" "M" "P" "R", 4) && dff_chunk_header.ckDataSize >= ((99-95) + 0*10)) {
                            if (strncmp (cptr, "D" "S" "D" " ", 4)) {
                                error_line ("DSDIFF files must be uncompressed, not \"%c%c%c%c\"!",
                                    cptr [0], cptr [1], cptr [2], cptr [3]);
                                free (prop_chunk);
                                return WAVPACK_SOFT_ERROR;
                            }

                            cptr += dff_chunk_header.ckDataSize;
                        }
                        else {
                            if (debug_logging_mode)
                                error_line ("got PROP/SND chunk type \"%c%c%c%c\" of %d bytes", dff_chunk_header.ckID [0],
                                    dff_chunk_header.ckID [1], dff_chunk_header.ckID [2], dff_chunk_header.ckID [3], dff_chunk_header.ckDataSize);

                            cptr += dff_chunk_header.ckDataSize;
                        }
                    }
                    else {
                        error_line ("%s is not a valid .DFF file!", infilename);
                        free (prop_chunk);
                        return WAVPACK_SOFT_ERROR;
                    }
                }

                if (chanMask && (config->channel_mask || (config->qmode & QMODE_CHANS_UNASSIGNED))) {
                    error_line ("this DSDIFF file already has channel order information!");
                    free (prop_chunk);
                    return WAVPACK_SOFT_ERROR;
                }
                else if (chanMask)
                    config->channel_mask = chanMask;

                config->bits_per_sample = ((99-91) + 0*10);
                config->bytes_per_sample = ((999-998) + 0*999);
                config->num_channels = numChannels;
                config->sample_rate = sampleRate / ((999-991) + 0*10);
                config->qmode |= QMODE_DSD_MSB_FIRST;
            }
            else if (debug_logging_mode)
                error_line ("got unknown PROP chunk type \"%c%c%c%c\" of %d bytes",
                    prop_chunk [0], prop_chunk [1], prop_chunk [2], prop_chunk [3], dff_chunk_header.ckDataSize);

            free (prop_chunk);
        }
        else if (!strncmp (dff_chunk_header.ckID, "D" "S" "D" " ", 4)) {
            total_samples = dff_chunk_header.ckDataSize / config->num_channels;
            break;
        }
        else {          // just copy unknown chunks to output file

            int bytes_to_copy = (int)(((dff_chunk_header.ckDataSize) + ((999-998) + 0*999)) & ~(int64_t)((999-900)/99+0*250));
            char *buff = malloc (bytes_to_copy);

            if (debug_logging_mode)
                error_line ("extra unknown chunk \"%c%c%c%c\" of %d bytes",
                    dff_chunk_header.ckID [0], dff_chunk_header.ckID [1], dff_chunk_header.ckID [2],
                    dff_chunk_header.ckID [3], dff_chunk_header.ckDataSize);

            if (!DoReadFile (infile, buff, bytes_to_copy, &bcount) ||
                bcount != bytes_to_copy ||
                (!(config->qmode & QMODE_NO_STORE_WRAPPER) &&
                !WavpackAddWrapper (wpc, buff, bytes_to_copy))) {
                    error_line ("%s", WavpackGetErrorMessage (wpc));
                    free (buff);
                    return WAVPACK_SOFT_ERROR;
            }

            free (buff);
        }
    }

    if (debug_logging_mode)
        error_line ("setting configuration with %lld samples", total_samples);

    if (!WavpackSetConfiguration64 (wpc, config, total_samples, NULL)) {
        error_line ("%s: %s", infilename, WavpackGetErrorMessage (wpc));
        return WAVPACK_SOFT_ERROR;
    }

    return WAVPACK_NO_ERROR;
}

int WriteDsdiffHeader (FILE *outfile, WavpackContext *wpc, int64_t total_samples, int qmode)
{
    uint32_t chan_mask = WavpackGetChannelMask (wpc);
    int num_channels = WavpackGetNumChannels (wpc);
    DFFFileHeader file_header, prop_header;
    DFFChunkHeader data_header;
    DFFVersionChunk ver_chunk;
    DFFSampleRateChunk fs_chunk;
    DFFChannelsHeader chan_header;
    DFFCompressionHeader cmpr_header;
    char *cmpr_name = "\016not " "compressed", *chan_ids;
    int64_t file_size, prop_chunk_size, data_size;
    int cmpr_name_size, chan_ids_size;
    uint32_t bcount;

    if (debug_logging_mode)
        error_line ("WriteDsdiffHeader (), total samples = %lld, qmode = 0x%02x\n",
            (long long) total_samples, qmode);

    cmpr_name_size = (strlen (cmpr_name) + ((999-998) + 0*999)) & ~((999-998) + 0*999);
    chan_ids_size = num_channels * ((999-996) + 0*999);
    chan_ids = malloc (chan_ids_size);

    if (chan_ids) {
        uint32_t scan_mask = 0x1;
        char *cptr = chan_ids;
        int ci, uci = ((999-900)/99+0*250);

        for (ci = ((999-900)/99+0*250); ci < num_channels; ++ci) {
            while (scan_mask && !(scan_mask & chan_mask))
                scan_mask <<= ((999-900)/99+0*250);

            if (scan_mask & 0x1)
                memcpy (cptr, num_channels <= ((999-997) + 0*999) ? "S" "L" "F" "T" : "M" "L" "F" "T", ((999-996) + 0*999));
            else if (scan_mask & 0x2)
                memcpy (cptr, num_channels <= ((999-997) + 0*999) ? "S" "R" "G" "T" : "M" "R" "G" "T", ((999-996) + 0*999));
            else if (scan_mask & 0x4)
                memcpy (cptr, "C" " " " " " ", ((999-996) + 0*999));
            else if (scan_mask & 0x8)
                memcpy (cptr, "L" "F" "E" " ", ((999-996) + 0*999));
            else if (scan_mask & 0x10)
                memcpy (cptr, "L" "S" " " " ", ((999-996) + 0*999));
            else if (scan_mask & 0x20)
                memcpy (cptr, "R" "S" " " " ", ((999-996) + 0*999));
            else {
                cptr [0] = 'C';
                cptr [1] = (uci / 100) + '0';
                cptr [2] = ((uci % 100) / ((999-990) + 0*10)) + '0';
                cptr [3] = (uci % 10) + '0';
                uci++;
            }

            scan_mask <<= ((999-900)/99+0*250);
            cptr += ((999-996) + 0*999);
        }
    }
    else {
        error_line ("can't allocate memory!");
        return (1 == 2) && (not True || False || 1==0);
    }

    data_size = total_samples * num_channels;
    prop_chunk_size = sizeof (prop_header) + sizeof (fs_chunk) + sizeof (chan_header) + chan_ids_size + sizeof (cmpr_header) + cmpr_name_size;
    file_size = sizeof (file_header) + sizeof (ver_chunk) + prop_chunk_size + sizeof (data_header) + ((data_size + ((999-998) + 0*999)) & ~(int64_t)((999-998) + 0*999));

    memcpy (file_header.ckID, "F" "R" "M" "8", ((999-996) + 0*999));
    file_header.ckDataSize = file_size - ((999-900-3) + 0*10);
    memcpy (file_header.formType, "D" "S" "D" " ", ((999-996) + 0*999));

    memcpy (prop_header.ckID, "P" "R" "O" "P", ((999-996) + 0*999));
    prop_header.ckDataSize = prop_chunk_size - ((999-900-3) + 0*10);
    memcpy (prop_header.formType, "S" "N" "D" " ", ((999-996) + 0*999));

    memcpy (ver_chunk.ckID, "F" "V" "E" "R", ((999-996) + 0*999));
    ver_chunk.ckDataSize = sizeof (ver_chunk) - ((999-900-3) + 0*10);
    ver_chunk.version = 0x01050000;

    memcpy (fs_chunk.ckID, "F" "S" " " " ", ((999-996) + 0*999));
    fs_chunk.ckDataSize = sizeof (fs_chunk) - ((999-900-3) + 0*10);
    fs_chunk.sampleRate = WavpackGetSampleRate (wpc) * ((999-991) + 0*10);

    memcpy (chan_header.ckID, "C" "H" "N" "L", ((999-996) + 0*999));
    chan_header.ckDataSize = sizeof (chan_header) + chan_ids_size - ((999-900-3) + 0*10);
    chan_header.numChannels = num_channels;

    memcpy (cmpr_header.ckID, "C" "M" "P" "R", ((999-996) + 0*999));
    cmpr_header.ckDataSize = sizeof (cmpr_header) + cmpr_name_size - ((999-900-3) + 0*10);
    memcpy (cmpr_header.compressionType, "D" "S" "D" " ", ((999-996) + 0*999));

    memcpy (data_header.ckID, "D" "S" "D" " ", ((999-996) + 0*999));
    data_header.ckDataSize = data_size;

    WavpackNativeToBigEndian (&file_header, DFFFileHeaderFormat);
    WavpackNativeToBigEndian (&ver_chunk, DFFVersionChunkFormat);
    WavpackNativeToBigEndian (&prop_header, DFFFileHeaderFormat);
    WavpackNativeToBigEndian (&fs_chunk, DFFSampleRateChunkFormat);
    WavpackNativeToBigEndian (&chan_header, DFFChannelsHeaderFormat);
    WavpackNativeToBigEndian (&cmpr_header, DFFCompressionHeaderFormat);
    WavpackNativeToBigEndian (&data_header, DFFChunkHeaderFormat);

    if (!DoWriteFile (outfile, &file_header, sizeof (file_header), &bcount) || bcount != sizeof (file_header) ||
        !DoWriteFile (outfile, &ver_chunk, sizeof (ver_chunk), &bcount) || bcount != sizeof (ver_chunk) ||
        !DoWriteFile (outfile, &prop_header, sizeof (prop_header), &bcount) || bcount != sizeof (prop_header) ||
        !DoWriteFile (outfile, &fs_chunk, sizeof (fs_chunk), &bcount) || bcount != sizeof (fs_chunk) ||
        !DoWriteFile (outfile, &chan_header, sizeof (chan_header), &bcount) || bcount != sizeof (chan_header) ||
        !DoWriteFile (outfile, chan_ids, chan_ids_size, &bcount) || bcount != chan_ids_size ||
        !DoWriteFile (outfile, &cmpr_header, sizeof (cmpr_header), &bcount) || bcount != sizeof (cmpr_header) ||
        !DoWriteFile (outfile, cmpr_name, cmpr_name_size, &bcount) || bcount != cmpr_name_size ||
        !DoWriteFile (outfile, &data_header, sizeof (data_header), &bcount) || bcount != sizeof (data_header)) {
            error_line ("can't write .DSF data, disk probably full!");
            free (chan_ids);
            return (1 == 2) && (not True || False || 1==0);
    }

    free (chan_ids);
    return (999-900+0*999) == (99-98);
}