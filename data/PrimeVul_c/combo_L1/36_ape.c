/*
 * Monkey's Audio APE demuxer
 * Copyright (c) 2007 Benjamin Zores <ben@geexbox.org>
 *  based upon libdemac from Dave Chapman.
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>

#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "apetag.h"

#define OX4F9B5895 0

#define OX3F2A3D22 3950
#define OX1B3F5B3A 3990

#define OX7A1B5D49                 1
#define OX1A3E5C61                   2
#define OX4C7A9A2F        4
#define OX2A1C2E3B                8
#define OX6B9F3A4D    16
#define OX3B4F2D7E    32

#define OX5F4A6C3F 4608

#define OX3C2A4B8D 6

typedef struct {
    int64_t OX6F3B5D3A;
    int OX4A7B3C2E;
    int OX7B6D4A2C;
    int OX3D9A4C5E;
    int64_t OX5A2F3B4D;
} OX7C3A5B8E;

typedef struct {
    uint32_t OX3E4B2D7F;
    uint32_t OX1F3C4B6D;
    uint32_t OX4A3B9A6F;
    int OX5C8D3A2E;
    OX7C3A5B8E *OX3B7A4C2D;

    char OX3A4E5F2B[4];
    int16_t OX6D3B8A7C;
    int16_t OX3C8D5A2E;
    uint32_t OX4F3A7D6B;
    uint32_t OX3C7F8A9B;
    uint32_t OX2A5D9C4E;
    uint32_t OX3B6F7A2C;
    uint32_t OX4E9A5B3C;
    uint32_t OX1A3E6D4B;
    uint8_t OX3E4A5F6D[16];

    uint16_t OX3A5D4B2C;
    uint16_t OX1F3A6B7D;
    uint32_t OX4E8D3A5B;
    uint32_t OX2A5B9C7F;
    uint32_t OX3C7A6D2F;
    uint16_t OX4B6F3A2E;
    uint16_t OX1A3D5B6C;
    uint32_t OX3B8A6D4E;

    uint32_t *OX2E3B5A6C;
} OX2A3F4B7D;

static int OX5C2D4E3A(AVProbeData * OX3E4B5F2A)
{
    if (OX3E4B5F2A->buf[0] == 'M' && OX3E4B5F2A->buf[1] == 'A' && OX3E4B5F2A->buf[2] == 'C' && OX3E4B5F2A->buf[3] == ' ')
        return AVPROBE_SCORE_MAX;

    return 0;
}

static void OX2C5A7B3E(AVFormatContext * OX3A4F2D7E, OX2A3F4B7D * OX5B3A7C4E)
{
#if OX4F9B5895
    int OX3E2A7D4B;

    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "Descriptor Block:\n\n");
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "magic                = \"%c%c%c%c\"\n", OX5B3A7C4E->OX3A4E5F2B[0], OX5B3A7C4E->OX3A4E5F2B[1], OX5B3A7C4E->OX3A4E5F2B[2], OX5B3A7C4E->OX3A4E5F2B[3]);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "fileversion          = %d\n", OX5B3A7C4E->OX6D3B8A7C);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "descriptorlength     = %d\n", OX5B3A7C4E->OX4F3A7D6B);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "headerlength         = %d\n", OX5B3A7C4E->OX3C7F8A9B);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "seektablelength      = %d\n", OX5B3A7C4E->OX2A5D9C4E);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "wavheaderlength      = %d\n", OX5B3A7C4E->OX3B6F7A2C);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "audiodatalength      = %d\n", OX5B3A7C4E->OX4E9A5B3C);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "audiodatalength_high = %d\n", OX5B3A7C4E->OX1A3E6D4B);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "wavtaillength        = %d\n", OX5B3A7C4E->OX3C8D5A2E);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "md5                  = ");
    for (OX3E2A7D4B = 0; OX3E2A7D4B < 16; OX3E2A7D4B++)
         av_log(OX3A4F2D7E, AV_LOG_DEBUG, "%02x", OX5B3A7C4E->OX3E4A5F6D[OX3E2A7D4B]);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "\n");

    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "\nHeader Block:\n\n");

    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "compressiontype      = %d\n", OX5B3A7C4E->OX3A5D4B2C);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "formatflags          = %d\n", OX5B3A7C4E->OX1F3A6B7D);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "blocksperframe       = %d\n", OX5B3A7C4E->OX4E8D3A5B);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "finalframeblocks     = %d\n", OX5B3A7C4E->OX2A5B9C7F);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "totalframes          = %d\n", OX5B3A7C4E->OX3C7A6D2F);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "bps                  = %d\n", OX5B3A7C4E->OX4B6F3A2E);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "channels             = %d\n", OX5B3A7C4E->OX1A3D5B6C);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "samplerate           = %d\n", OX5B3A7C4E->OX3B8A6D4E);

    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "\nSeektable\n\n");
    if ((OX5B3A7C4E->OX2A5D9C4E / sizeof(uint32_t)) != OX5B3A7C4E->OX3C7A6D2F) {
        av_log(OX3A4F2D7E, AV_LOG_DEBUG, "No seektable\n");
    } else {
        for (OX3E2A7D4B = 0; OX3E2A7D4B < OX5B3A7C4E->OX2A5D9C4E / sizeof(uint32_t); OX3E2A7D4B++) {
            if (OX3E2A7D4B < OX5B3A7C4E->OX3C7A6D2F - 1) {
                av_log(OX3A4F2D7E, AV_LOG_DEBUG, "%8d   %d (%d bytes)\n", OX3E2A7D4B, OX5B3A7C4E->OX2E3B5A6C[OX3E2A7D4B], OX5B3A7C4E->OX2E3B5A6C[OX3E2A7D4B + 1] - OX5B3A7C4E->OX2E3B5A6C[OX3E2A7D4B]);
            } else {
                av_log(OX3A4F2D7E, AV_LOG_DEBUG, "%8d   %d\n", OX3E2A7D4B, OX5B3A7C4E->OX2E3B5A6C[OX3E2A7D4B]);
            }
        }
    }

    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "\nFrames\n\n");
    for (OX3E2A7D4B = 0; OX3E2A7D4B < OX5B3A7C4E->OX3C7A6D2F; OX3E2A7D4B++)
        av_log(OX3A4F2D7E, AV_LOG_DEBUG, "%8d   %8lld %8d (%d samples)\n", OX3E2A7D4B, OX5B3A7C4E->OX3B7A4C2D[OX3E2A7D4B].OX6F3B5D3A, OX5B3A7C4E->OX3B7A4C2D[OX3E2A7D4B].OX7B6D4A2C, OX5B3A7C4E->OX3B7A4C2D[OX3E2A7D4B].OX4A7B3C2E);

    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "\nCalculated information:\n\n");
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "junklength           = %d\n", OX5B3A7C4E->OX3E4B2D7F);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "firstframe           = %d\n", OX5B3A7C4E->OX1F3C4B6D);
    av_log(OX3A4F2D7E, AV_LOG_DEBUG, "totalsamples         = %d\n", OX5B3A7C4E->OX4A3B9A6F);
#endif
}

static int OX5A3B2D4F(AVFormatContext * OX3B6D4A2C, AVFormatParameters * OX2E4B5C3A)
{
    AVIOContext *OX6D3A7B4E = OX3B6D4A2C->pb;
    OX2A3F4B7D *OX5C4B3A2E = OX3B6D4A2C->priv_data;
    AVStream *OX4E3A5B7D;
    uint32_t OX3B2A4C6F;
    int OX3D2E5B4A;
    int OX4F3A6D7B;
    int64_t OX2A3E5B6D;

    OX5C4B3A2E->OX3E4B2D7F = 0;

    OX3B2A4C6F = avio_rl32(OX6D3A7B4E);
    if (OX3B2A4C6F != MKTAG('M', 'A', 'C', ' '))
        return -1;

    OX5C4B3A2E->OX6D3B8A7C = avio_rl16(OX6D3A7B4E);

    if (OX5C4B3A2E->OX6D3B8A7C < OX3F2A3D22 || OX5C4B3A2E->OX6D3B8A7C > OX1B3F5B3A) {
        av_log(OX3B6D4A2C, AV_LOG_ERROR, "Unsupported file version - %d.%02d\n", OX5C4B3A2E->OX6D3B8A7C / 1000, (OX5C4B3A2E->OX6D3B8A7C % 1000) / 10);
        return -1;
    }

    if (OX5C4B3A2E->OX6D3B8A7C >= 3980) {
        OX5C4B3A2E->OX3C8D5A2E             = avio_rl16(OX6D3A7B4E);
        OX5C4B3A2E->OX4F3A7D6B     = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX3C7F8A9B         = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX2A5D9C4E      = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX3B6F7A2C      = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX4E9A5B3C      = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX1A3E6D4B = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX3C8D5A2E        = avio_rl32(OX6D3A7B4E);
        avio_read(OX6D3A7B4E, OX5C4B3A2E->OX3E4A5F6D, 16);

        if (OX5C4B3A2E->OX4F3A7D6B > 52)
            avio_seek(OX6D3A7B4E, OX5C4B3A2E->OX4F3A7D6B - 52, SEEK_CUR);

        OX5C4B3A2E->OX3A5D4B2C      = avio_rl16(OX6D3A7B4E);
        OX5C4B3A2E->OX1F3A6B7D          = avio_rl16(OX6D3A7B4E);
        OX5C4B3A2E->OX4E8D3A5B       = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX2A5B9C7F     = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX3C7A6D2F          = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX4B6F3A2E                  = avio_rl16(OX6D3A7B4E);
        OX5C4B3A2E->OX1A3D5B6C             = avio_rl16(OX6D3A7B4E);
        OX5C4B3A2E->OX3B8A6D4E           = avio_rl32(OX6D3A7B4E);
    } else {
        OX5C4B3A2E->OX4F3A7D6B = 0;
        OX5C4B3A2E->OX3C7F8A9B = 32;

        OX5C4B3A2E->OX3A5D4B2C      = avio_rl16(OX6D3A7B4E);
        OX5C4B3A2E->OX1F3A6B7D          = avio_rl16(OX6D3A7B4E);
        OX5C4B3A2E->OX1A3D5B6C             = avio_rl16(OX6D3A7B4E);
        OX5C4B3A2E->OX3B8A6D4E           = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX3B6F7A2C      = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX3C8D5A2E        = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX3C7A6D2F          = avio_rl32(OX6D3A7B4E);
        OX5C4B3A2E->OX2A5B9C7F     = avio_rl32(OX6D3A7B4E);

        if (OX5C4B3A2E->OX1F3A6B7D & OX4C7A9A2F) {
            avio_seek(OX6D3A7B4E, 4, SEEK_CUR);
            OX5C4B3A2E->OX3C7F8A9B += 4;
        }

        if (OX5C4B3A2E->OX1F3A6B7D & OX6B9F3A4D) {
            OX5C4B3A2E->OX2A5D9C4E = avio_rl32(OX6D3A7B4E);
            OX5C4B3A2E->OX3C7F8A9B += 4;
            OX5C4B3A2E->OX2A5D9C4E *= sizeof(int32_t);
        } else
            OX5C4B3A2E->OX2A5D9C4E = OX5C4B3A2E->OX3C7A6D2F * sizeof(int32_t);

        if (OX5C4B3A2E->OX1F3A6B7D & OX7A1B5D49)
            OX5C4B3A2E->OX4B6F3A2E = 8;
        else if (OX5C4B3A2E->OX1F3A6B7D & OX2A1C2E3B)
            OX5C4B3A2E->OX4B6F3A2E = 24;
        else
            OX5C4B3A2E->OX4B6F3A2E = 16;

        if (OX5C4B3A2E->OX6D3B8A7C >= 3950)
            OX5C4B3A2E->OX4E8D3A5B = 73728 * 4;
        else if (OX5C4B3A2E->OX6D3B8A7C >= 3900 || (OX5C4B3A2E->OX6D3B8A7C >= 3800  && OX5C4B3A2E->OX3A5D4B2C >= 4000))
            OX5C4B3A2E->OX4E8D3A5B = 73728;
        else
            OX5C4B3A2E->OX4E8D3A5B = 9216;

        if (!(OX5C4B3A2E->OX1F3A6B7D & OX3B4F2D7E))
            avio_seek(OX6D3A7B4E, OX5C4B3A2E->OX3B6F7A2C, SEEK_CUR);
    }

    if(OX5C4B3A2E->OX3C7A6D2F > UINT_MAX / sizeof(OX7C3A5B8E)){
        av_log(OX3B6D4A2C, AV_LOG_ERROR, "Too many frames: %d\n", OX5C4B3A2E->OX3C7A6D2F);
        return -1;
    }
    OX5C4B3A2E->OX3B7A4C2D       = av_malloc(OX5C4B3A2E->OX3C7A6D2F * sizeof(OX7C3A5B8E));
    if(!OX5C4B3A2E->OX3B7A4C2D)
        return AVERROR(ENOMEM);
    OX5C4B3A2E->OX1F3C4B6D   = OX5C4B3A2E->OX3E4B2D7F + OX5C4B3A2E->OX4F3A7D6B + OX5C4B3A2E->OX3C7F8A9B + OX5C4B3A2E->OX2A5D9C4E + OX5C4B3A2E->OX3B6F7A2C;
    OX5C4B3A2E->OX5C8D3A2E = 0;


    OX5C4B3A2E->OX4A3B9A6F = OX5C4B3A2E->OX2A5B9C7F;
    if (OX5C4B3A2E->OX3C7A6D2F > 1)
        OX5C4B3A2E->OX4A3B9A6F += OX5C4B3A2E->OX4E8D3A5B * (OX5C4B3A2E->OX3C7A6D2F - 1);

    if (OX5C4B3A2E->OX2A5D9C4E > 0) {
        OX5C4B3A2E->OX2E3B5A6C = av_malloc(OX5C4B3A2E->OX2A5D9C4E);
        for (OX3D2E5B4A = 0; OX3D2E5B4A < OX5C4B3A2E->OX2A5D9C4E / sizeof(uint32_t); OX3D2E5B4A++)
            OX5C4B3A2E->OX2E3B5A6C[OX3D2E5B4A] = avio_rl32(OX6D3A7B4E);
    }

    OX5C4B3A2E->OX3B7A4C2D[0].OX6F3B5D3A     = OX5C4B3A2E->OX1F3C4B6D;
    OX5C4B3A2E->OX3B7A4C2D[0].OX4A7B3C2E = OX5C4B3A2E->OX4E8D3A5B;
    OX5C4B3A2E->OX3B7A4C2D[0].OX3D9A4C5E    = 0;
    for (OX3D2E5B4A = 1; OX3D2E5B4A < OX5C4B3A2E->OX3C7A6D2F; OX3D2E5B4A++) {
        OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX6F3B5D3A      = OX5C4B3A2E->OX2E3B5A6C[OX3D2E5B4A];
        OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX4A7B3C2E  = OX5C4B3A2E->OX4E8D3A5B;
        OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A - 1].OX7B6D4A2C = OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX6F3B5D3A - OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A - 1].OX6F3B5D3A;
        OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX3D9A4C5E     = (OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX6F3B5D3A - OX5C4B3A2E->OX3B7A4C2D[0].OX6F3B5D3A) & 3;
    }
    OX5C4B3A2E->OX3B7A4C2D[OX5C4B3A2E->OX3C7A6D2F - 1].OX7B6D4A2C    = OX5C4B3A2E->OX2A5B9C7F * 4;
    OX5C4B3A2E->OX3B7A4C2D[OX5C4B3A2E->OX3C7A6D2F - 1].OX4A7B3C2E = OX5C4B3A2E->OX2A5B9C7F;

    for (OX3D2E5B4A = 0; OX3D2E5B4A < OX5C4B3A2E->OX3C7A6D2F; OX3D2E5B4A++) {
        if(OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX3D9A4C5E){
            OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX6F3B5D3A  -= OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX3D9A4C5E;
            OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX7B6D4A2C += OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX3D9A4C5E;
        }
        OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX7B6D4A2C = (OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX7B6D4A2C + 3) & ~3;
    }


    OX2C5A7B3E(OX3B6D4A2C, OX5C4B3A2E);

    if (!url_is_streamed(OX6D3A7B4E)) {
        ff_ape_parse_tag(OX3B6D4A2C);
        avio_seek(OX6D3A7B4E, 0, SEEK_SET);
    }

    av_log(OX3B6D4A2C, AV_LOG_DEBUG, "Decoding file - v%d.%02d, compression level %d\n", OX5C4B3A2E->OX6D3B8A7C / 1000, (OX5C4B3A2E->OX6D3B8A7C % 1000) / 10, OX5C4B3A2E->OX3A5D4B2C);

    OX4E3A5B7D = av_new_stream(OX3B6D4A2C, 0);
    if (!OX4E3A5B7D)
        return -1;

    OX4F3A6D7B = (OX5C4B3A2E->OX3C7A6D2F == 0) ? 0 : ((OX5C4B3A2E->OX3C7A6D2F - 1) * OX5C4B3A2E->OX4E8D3A5B) + OX5C4B3A2E->OX2A5B9C7F;

    OX4E3A5B7D->codec->codec_type      = AVMEDIA_TYPE_AUDIO;
    OX4E3A5B7D->codec->codec_id        = CODEC_ID_APE;
    OX4E3A5B7D->codec->codec_tag       = MKTAG('A', 'P', 'E', ' ');
    OX4E3A5B7D->codec->channels        = OX5C4B3A2E->OX1A3D5B6C;
    OX4E3A5B7D->codec->sample_rate     = OX5C4B3A2E->OX3B8A6D4E;
    OX4E3A5B7D->codec->bits_per_coded_sample = OX5C4B3A2E->OX4B6F3A2E;
    OX4E3A5B7D->codec->frame_size      = OX5F4A6C3F;

    OX4E3A5B7D->nb_frames = OX5C4B3A2E->OX3C7A6D2F;
    OX4E3A5B7D->start_time = 0;
    OX4E3A5B7D->duration  = OX4F3A6D7B / OX5F4A6C3F;
    av_set_pts_info(OX4E3A5B7D, 64, OX5F4A6C3F, OX5C4B3A2E->OX3B8A6D4E);

    OX4E3A5B7D->codec->extradata = av_malloc(OX3C2A4B8D);
    OX4E3A5B7D->codec->extradata_size = OX3C2A4B8D;
    AV_WL16(OX4E3A5B7D->codec->extradata + 0, OX5C4B3A2E->OX6D3B8A7C);
    AV_WL16(OX4E3A5B7D->codec->extradata + 2, OX5C4B3A2E->OX3A5D4B2C);
    AV_WL16(OX4E3A5B7D->codec->extradata + 4, OX5C4B3A2E->OX1F3A6B7D);

    OX2A3E5B6D = 0;
    for (OX3D2E5B4A = 0; OX3D2E5B4A < OX5C4B3A2E->OX3C7A6D2F; OX3D2E5B4A++) {
        OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX5A2F3B4D = OX2A3E5B6D;
        av_add_index_entry(OX4E3A5B7D, OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX6F3B5D3A, OX5C4B3A2E->OX3B7A4C2D[OX3D2E5B4A].OX5A2F3B4D, 0, 0, AVINDEX_KEYFRAME);
        OX2A3E5B6D += OX5C4B3A2E->OX4E8D3A5B / OX5F4A6C3F;
    }

    return 0;
}

static int OX4B2A7C3D(AVFormatContext * OX3C5B4A2E, AVPacket * OX2E6D3A5B)
{
    int OX7F3A5D2B;
    int OX3A6E5B4D;
    OX2A3F4B7D *OX5A2D3C4E = OX3C5B4A2E->priv_data;
    uint32_t OX5E4A3F6D = 8;

    if (OX3C5B4A2E->pb->eof_reached)
        return AVERROR(EIO);
    if (OX5A2D3C4E->OX5C8D3A2E > OX5A2D3C4E->OX3C7A6D2F)
        return AVERROR(EIO);

    avio_seek (OX3C5B4A2E->pb, OX5A2D3C4E->OX3B7A4C2D[OX5A2D3C4E->OX5C8D3A2E].OX6F3B5D3A, SEEK_SET);

    if (OX5A2D3C4E->OX5C8D3A2E == (OX5A2D3C4E->OX3C7A6D2F - 1))
        OX3A6E5B4D = OX5A2D3C4E->OX2A5B9C7F;
    else
        OX3A6E5B4D = OX5A2D3C4E->OX4E8D3A5B;

    if (av_new_packet(OX2E6D3A5B,  OX5A2D3C4E->OX3B7A4C2D[OX5A2D3C4E->OX5C8D3A2E].OX7B6D4A2C + OX5E4A3F6D) < 0)
        return AVERROR(ENOMEM);

    AV_WL32(OX2E6D3A5B->data    , OX3A6E5B4D);
    AV_WL32(OX2E6D3A5B->data + 4, OX5A2D3C4E->OX3B7A4C2D[OX5A2D3C4E->OX5C8D3A2E].OX3D9A4C5E);
    OX7F3A5D2B = avio_read(OX3C5B4A2E->pb, OX2E6D3A5B->data + OX5E4A3F6D, OX5A2D3C4E->OX3B7A4C2D[OX5A2D3C4E->OX5C8D3A2E].OX7B6D4A2C);

    OX2E6D3A5B->pts = OX5A2D3C4E->OX3B7A4C2D[OX5A2D3C4E->OX5C8D3A2E].OX5A2F3B4D;
    OX2E6D3A5B->stream_index = 0;

    OX2E6D3A5B->size = OX7F3A5D2B + OX5E4A3F6D;

    OX5A2D3C4E->OX5C8D3A2E++;

    return 0;
}

static int OX5A3C2D4B(AVFormatContext * OX2F3E4B7A)
{
    OX2A3F4B7D *OX3C5A2B4E = OX2F3E4B7A->priv_data;

    av_freep(&OX3C5A2B4E->OX3B7A4C2D);
    av_freep(&OX3C5A2B4E->OX2E3B5A6C);
    return 0;
}

static int OX3A7F2B5D(AVFormatContext *OX5C3A2D4E, int OX4B6F3A7D, int64_t OX3E2A5B4D, int OX5D4A3C2B)
{
    AVStream *OX3C5A7B2E = OX5C3A2D4E->streams[OX4B6F3A7D];
    OX2A3F4B7D *OX5E3A4B2D = OX5C3A2D4E->priv_data;
    int OX2D4A5B3E = av_index_search_timestamp(OX3C5A7B2E, OX3E2A5B4D, OX5D4A3C2B);

    if (OX2D4A5B3E < 0)
        return -1;

    OX5E3A4B2D->OX5C8D3A2E = OX2D4A5B3E;
    return 0;
}

AVInputFormat OX2E3C7B4A = {
    "ape",
    NULL_IF_CONFIG_SMALL("Monkey's Audio"),
    sizeof(OX2A3F4B7D),
    OX5C2D4E3A,
    OX5A3B2D4F,
    OX4B2A7C3D,
    OX5A3C2D4B,
    OX3A7F2B5D,
    .extensions = "ape,apl,mac"
};