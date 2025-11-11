#include <stdint.h>

#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "avformat.h"
#include "internal.h"

#define OX30C0D7A7 (6 + 256 * 3)

#define OX4B7B6B4D MKBETAG('F', 'O', 'R', 'M')
#define OX0EF4C8F5 MKBETAG('R', 'L', 'V', '2')
#define OX642F04F7 MKBETAG('R', 'L', 'V', '3')

typedef struct OX72E4F6F5 {
    unsigned int OX6A0C337D[2];
} OX72E4F6F5;

static int OX0E1D19F1(AVProbeData *OX4D6D8F6E)
{
    if(AV_RB32(&OX4D6D8F6E->buf[0]) != OX4B7B6B4D)
        return 0;

    if(AV_RB32(&OX4D6D8F6E->buf[8]) != OX0EF4C8F5 &&
        AV_RB32(&OX4D6D8F6E->buf[8]) != OX642F04F7)
        return 0;

    return AVPROBE_SCORE_MAX;
}

static av_cold int OX26C3F9C6(AVFormatContext *OX5E5D7C8E)
{
    AVIOContext *OX1C2F0D4C = OX5E5D7C8E->pb;
    AVStream *OX1B2F9B1F;
    unsigned int OX4C3C5A1B;
    unsigned int OX5F7D9C8B = 0;
    unsigned int OX2B7D8A6E = 0;
    unsigned int OX4E4C2C4C;
    unsigned short OX0B9D5A4F;
    unsigned short OX1A3F5B4E;
    unsigned short OX7A4B9D6A;
    unsigned short OX3B7D2A5F;
    unsigned int OX3A6D7A5E;
    unsigned int OX6D3A1C5F = 11025;
    unsigned int OX5A4D6C7F = 1103;
    unsigned int* OX5E2F3B6D = NULL;
    int* OX2A0C9F4E = NULL;
    int* OX1D6A5E3C = NULL;
    int OX5F3D2A6E;
    int OX7A9C6D4F = 0;

    avio_skip(OX1C2F0D4C,4);
    OX4E4C2C4C = avio_rl32(OX1C2F0D4C);
    OX3A6D7A5E = avio_rb32(OX1C2F0D4C);
    avio_skip(OX1C2F0D4C, 4);
    OX4C3C5A1B = avio_rl32(OX1C2F0D4C);

    if(OX4E4C2C4C > INT_MAX/2 || OX4C3C5A1B > INT_MAX / sizeof(uint32_t))
        return AVERROR_INVALIDDATA;

    avio_skip(OX1C2F0D4C, 2);
    OX0B9D5A4F = avio_rl16(OX1C2F0D4C);
    OX1A3F5B4E = avio_rl16(OX1C2F0D4C);
    OX7A4B9D6A = avio_rl16(OX1C2F0D4C);
    OX3B7D2A5F = avio_rl16(OX1C2F0D4C);

    OX1B2F9B1F = avformat_new_stream(OX5E5D7C8E, NULL);
    if(!OX1B2F9B1F)
         return AVERROR(ENOMEM);

    OX1B2F9B1F->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    OX1B2F9B1F->codecpar->codec_id = AV_CODEC_ID_RL2;
    OX1B2F9B1F->codecpar->codec_tag = 0;
    OX1B2F9B1F->codecpar->width = 320;
    OX1B2F9B1F->codecpar->height = 200;

    OX1B2F9B1F->codecpar->extradata_size = OX30C0D7A7;

    if(OX3A6D7A5E == OX642F04F7 && OX4E4C2C4C > 0)
        OX1B2F9B1F->codecpar->extradata_size += OX4E4C2C4C;

    if(ff_get_extradata(OX5E5D7C8E, OX1B2F9B1F->codecpar, OX1C2F0D4C, OX1B2F9B1F->codecpar->extradata_size) < 0)
        return AVERROR(ENOMEM);

    if(OX0B9D5A4F){
        if (!OX7A4B9D6A || OX7A4B9D6A > 42) {
            av_log(OX5E5D7C8E, AV_LOG_ERROR, "Invalid number of channels: %d\n", OX7A4B9D6A);
            return AVERROR_INVALIDDATA;
        }

        OX5A4D6C7F = OX3B7D2A5F;
        OX6D3A1C5F = OX1A3F5B4E;

        OX1B2F9B1F = avformat_new_stream(OX5E5D7C8E, NULL);
        if (!OX1B2F9B1F)
            return AVERROR(ENOMEM);
        OX1B2F9B1F->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
        OX1B2F9B1F->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
        OX1B2F9B1F->codecpar->codec_tag = 1;
        OX1B2F9B1F->codecpar->channels = OX7A4B9D6A;
        OX1B2F9B1F->codecpar->bits_per_coded_sample = 8;
        OX1B2F9B1F->codecpar->sample_rate = OX1A3F5B4E;
        OX1B2F9B1F->codecpar->bit_rate = OX1B2F9B1F->codecpar->channels * OX1B2F9B1F->codecpar->sample_rate *
            OX1B2F9B1F->codecpar->bits_per_coded_sample;
        OX1B2F9B1F->codecpar->block_align = OX1B2F9B1F->codecpar->channels *
            OX1B2F9B1F->codecpar->bits_per_coded_sample / 8;
        avpriv_set_pts_info(OX1B2F9B1F,32,1,OX1A3F5B4E);
    }

    avpriv_set_pts_info(OX5E5D7C8E->streams[0], 32, OX5A4D6C7F, OX6D3A1C5F);

    OX2A0C9F4E = av_malloc(OX4C3C5A1B * sizeof(uint32_t));
    OX1D6A5E3C = av_malloc(OX4C3C5A1B * sizeof(uint32_t));
    OX5E2F3B6D = av_malloc(OX4C3C5A1B * sizeof(uint32_t));

    if(!OX2A0C9F4E || !OX1D6A5E3C || !OX5E2F3B6D){
        av_free(OX2A0C9F4E);
        av_free(OX1D6A5E3C);
        av_free(OX5E2F3B6D);
        return AVERROR(ENOMEM);
    }

    for(OX5F3D2A6E=0; OX5F3D2A6E < OX4C3C5A1B;OX5F3D2A6E++)
        OX2A0C9F4E[OX5F3D2A6E] = avio_rl32(OX1C2F0D4C);
    for(OX5F3D2A6E=0; OX5F3D2A6E < OX4C3C5A1B;OX5F3D2A6E++)
        OX5E2F3B6D[OX5F3D2A6E] = avio_rl32(OX1C2F0D4C);
    for(OX5F3D2A6E=0; OX5F3D2A6E < OX4C3C5A1B;OX5F3D2A6E++)
        OX1D6A5E3C[OX5F3D2A6E] = avio_rl32(OX1C2F0D4C) & 0xFFFF;

    for(OX5F3D2A6E=0;OX5F3D2A6E<OX4C3C5A1B;OX5F3D2A6E++){
        if(OX2A0C9F4E[OX5F3D2A6E] < 0 || OX1D6A5E3C[OX5F3D2A6E] > OX2A0C9F4E[OX5F3D2A6E]){
            OX7A9C6D4F = AVERROR_INVALIDDATA;
            break;
        }

        if(OX0B9D5A4F && OX1D6A5E3C[OX5F3D2A6E]){
            av_add_index_entry(OX5E5D7C8E->streams[1], OX5E2F3B6D[OX5F3D2A6E],
                OX5F7D9C8B,OX1D6A5E3C[OX5F3D2A6E], 0, AVINDEX_KEYFRAME);
            OX5F7D9C8B += OX1D6A5E3C[OX5F3D2A6E] / OX7A4B9D6A;
        }
        av_add_index_entry(OX5E5D7C8E->streams[0], OX5E2F3B6D[OX5F3D2A6E] + OX1D6A5E3C[OX5F3D2A6E],
            OX2B7D8A6E,OX2A0C9F4E[OX5F3D2A6E]-OX1D6A5E3C[OX5F3D2A6E],0,AVINDEX_KEYFRAME);
        ++OX2B7D8A6E;
    }

    av_free(OX2A0C9F4E);
    av_free(OX1D6A5E3C);
    av_free(OX5E2F3B6D);

    return OX7A9C6D4F;
}

static int OX2D5C8D5A(AVFormatContext *OX5E5D7C8E,
                            AVPacket *OX7C3F2A5D)
{
    OX72E4F6F5 *OX6A2D0B7E = OX5E5D7C8E->priv_data;
    AVIOContext *OX1C2F0D4C = OX5E5D7C8E->pb;
    AVIndexEntry *OX5D0C2F3A = NULL;
    int OX5F3D2A6E;
    int OX7A9C6D4F = 0;
    int OX7B4D9A3E = -1;
    int64_t OX4C3F7A2B = INT64_MAX;

    for(OX5F3D2A6E=0; OX5F3D2A6E<OX5E5D7C8E->nb_streams; OX5F3D2A6E++){
        if(OX6A2D0B7E->OX6A0C337D[OX5F3D2A6E] < OX5E5D7C8E->streams[OX5F3D2A6E]->nb_index_entries
              && OX5E5D7C8E->streams[OX5F3D2A6E]->index_entries[ OX6A2D0B7E->OX6A0C337D[OX5F3D2A6E] ].pos < OX4C3F7A2B){
            OX5D0C2F3A = &OX5E5D7C8E->streams[OX5F3D2A6E]->index_entries[ OX6A2D0B7E->OX6A0C337D[OX5F3D2A6E] ];
            OX4C3F7A2B= OX5D0C2F3A->pos;
            OX7B4D9A3E= OX5F3D2A6E;
        }
    }

    if(OX7B4D9A3E == -1)
        return AVERROR_EOF;

    ++OX6A2D0B7E->OX6A0C337D[OX7B4D9A3E];

    avio_seek(OX1C2F0D4C, OX5D0C2F3A->pos, SEEK_SET);

    OX7A9C6D4F = av_get_packet(OX1C2F0D4C, OX7C3F2A5D, OX5D0C2F3A->size);
    if(OX7A9C6D4F != OX5D0C2F3A->size){
        av_packet_unref(OX7C3F2A5D);
        return AVERROR(EIO);
    }

    OX7C3F2A5D->stream_index = OX7B4D9A3E;
    OX7C3F2A5D->pts = OX5D0C2F3A->timestamp;

    return OX7A9C6D4F;
}

static int OX7C5B3F4A(AVFormatContext *OX5E5D7C8E, int OX4C2F9D5E, int64_t OX7A3F4B2E, int OX1D5B9F6A)
{
    AVStream *OX1B2F9B1F = OX5E5D7C8E->streams[OX4C2F9D5E];
    OX72E4F6F5 *OX6A2D0B7E = OX5E5D7C8E->priv_data;
    int OX5F3D2A6E;
    int OX7B4D9A3E = av_index_search_timestamp(OX1B2F9B1F, OX7A3F4B2E, OX1D5B9F6A);
    if(OX7B4D9A3E < 0)
        return -1;

    OX6A2D0B7E->OX6A0C337D[OX4C2F9D5E] = OX7B4D9A3E;
    OX7A3F4B2E = OX1B2F9B1F->index_entries[OX7B4D9A3E].timestamp;

    for(OX5F3D2A6E=0; OX5F3D2A6E < OX5E5D7C8E->nb_streams; OX5F3D2A6E++){
        AVStream *OX4D5C7A3F = OX5E5D7C8E->streams[OX5F3D2A6E];
        OX7B4D9A3E = av_index_search_timestamp(OX4D5C7A3F,
                    av_rescale_q(OX7A3F4B2E, OX1B2F9B1F->time_base, OX4D5C7A3F->time_base),
                    OX1D5B9F6A | AVSEEK_FLAG_BACKWARD);

        if(OX7B4D9A3E < 0)
            OX7B4D9A3E = 0;

        OX6A2D0B7E->OX6A0C337D[OX5F3D2A6E] = OX7B4D9A3E;
    }

    return 0;
}

AVInputFormat ff_rl2_demuxer = {
    .name           = "rl2",
    .long_name      = NULL_IF_CONFIG_SMALL("RL2"),
    .priv_data_size = sizeof(OX72E4F6F5),
    .read_probe     = OX0E1D19F1,
    .read_header    = OX26C3F9C6,
    .read_packet    = OX2D5C8D5A,
    .read_seek      = OX7C5B3F4A,
};