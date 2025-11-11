#include <stdio.h>

#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "apetag.h"

#define ENABLE_DEBUG 0

#define APE_MIN_VERSION 3950
#define APE_MAX_VERSION 3990

#define MAC_FORMAT_FLAG_8_BIT                 1
#define MAC_FORMAT_FLAG_CRC                   2
#define MAC_FORMAT_FLAG_HAS_PEAK_LEVEL        4
#define MAC_FORMAT_FLAG_24_BIT                8
#define MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS    16
#define MAC_FORMAT_FLAG_CREATE_WAV_HEADER    32

#define MAC_SUBFRAME_SIZE 4608

#define APE_EXTRADATA_SIZE 6

typedef struct {
    int64_t a; // pos
    int b;     // nblocks
    int c;     // size
    int d;     // skip
    int64_t e; // pts
} ConvertedAPEFrame;

typedef struct {
    uint32_t d1; // junklength
    uint32_t d2; // firstframe
    uint32_t d3; // totalsamples
    int d4;      // currentframe
    ConvertedAPEFrame *d5; // frames

    char b1[4]; // magic
    int16_t b2; // fileversion
    int16_t b3; // padding1
    uint32_t b4; // descriptorlength
    uint32_t b5; // headerlength
    uint32_t b6; // seektablelength
    uint32_t b7; // wavheaderlength
    uint32_t b8; // audiodatalength
    uint32_t b9; // audiodatalength_high
    uint32_t b10; // wavtaillength
    uint8_t b11[16]; // md5

    uint16_t c1; // compressiontype
    uint16_t c2; // formatflags
    uint32_t c3; // blocksperframe
    uint32_t c4; // finalframeblocks
    uint32_t c5; // totalframes
    uint16_t c6; // bps
    uint16_t c7; // channels
    uint32_t c8; // samplerate

    uint32_t *c9; // seektable
} ConvertedAPEContext;

static int ape_probe(AVProbeData * p)
{
    if (p->buf[0] == 'M' && p->buf[1] == 'A' && p->buf[2] == 'C' && p->buf[3] == ' ')
        return AVPROBE_SCORE_MAX;

    return 0;
}

static void ape_dumpinfo(AVFormatContext * s, ConvertedAPEContext * ape_ctx)
{
#if ENABLE_DEBUG
    int i;

    av_log(s, AV_LOG_DEBUG, "Descriptor Block:\n\n");
    av_log(s, AV_LOG_DEBUG, "magic                = \"%c%c%c%c\"\n", ape_ctx->b1[0], ape_ctx->b1[1], ape_ctx->b1[2], ape_ctx->b1[3]);
    av_log(s, AV_LOG_DEBUG, "fileversion          = %d\n", ape_ctx->b2);
    av_log(s, AV_LOG_DEBUG, "descriptorlength     = %d\n", ape_ctx->b4);
    av_log(s, AV_LOG_DEBUG, "headerlength         = %d\n", ape_ctx->b5);
    av_log(s, AV_LOG_DEBUG, "seektablelength      = %d\n", ape_ctx->b6);
    av_log(s, AV_LOG_DEBUG, "wavheaderlength      = %d\n", ape_ctx->b7);
    av_log(s, AV_LOG_DEBUG, "audiodatalength      = %d\n", ape_ctx->b8);
    av_log(s, AV_LOG_DEBUG, "audiodatalength_high = %d\n", ape_ctx->b9);
    av_log(s, AV_LOG_DEBUG, "wavtaillength        = %d\n", ape_ctx->b10);
    av_log(s, AV_LOG_DEBUG, "md5                  = ");
    for (i = 0; i < 16; i++)
         av_log(s, AV_LOG_DEBUG, "%02x", ape_ctx->b11[i]);
    av_log(s, AV_LOG_DEBUG, "\n");

    av_log(s, AV_LOG_DEBUG, "\nHeader Block:\n\n");

    av_log(s, AV_LOG_DEBUG, "compressiontype      = %d\n", ape_ctx->c1);
    av_log(s, AV_LOG_DEBUG, "formatflags          = %d\n", ape_ctx->c2);
    av_log(s, AV_LOG_DEBUG, "blocksperframe       = %d\n", ape_ctx->c3);
    av_log(s, AV_LOG_DEBUG, "finalframeblocks     = %d\n", ape_ctx->c4);
    av_log(s, AV_LOG_DEBUG, "totalframes          = %d\n", ape_ctx->c5);
    av_log(s, AV_LOG_DEBUG, "bps                  = %d\n", ape_ctx->c6);
    av_log(s, AV_LOG_DEBUG, "channels             = %d\n", ape_ctx->c7);
    av_log(s, AV_LOG_DEBUG, "samplerate           = %d\n", ape_ctx->c8);

    av_log(s, AV_LOG_DEBUG, "\nSeektable\n\n");
    if ((ape_ctx->b6 / sizeof(uint32_t)) != ape_ctx->c5) {
        av_log(s, AV_LOG_DEBUG, "No seektable\n");
    } else {
        for (i = 0; i < ape_ctx->b6 / sizeof(uint32_t); i++) {
            if (i < ape_ctx->c5 - 1) {
                av_log(s, AV_LOG_DEBUG, "%8d   %d (%d bytes)\n", i, ape_ctx->c9[i], ape_ctx->c9[i + 1] - ape_ctx->c9[i]);
            } else {
                av_log(s, AV_LOG_DEBUG, "%8d   %d\n", i, ape_ctx->c9[i]);
            }
        }
    }

    av_log(s, AV_LOG_DEBUG, "\nFrames\n\n");
    for (i = 0; i < ape_ctx->c5; i++)
        av_log(s, AV_LOG_DEBUG, "%8d   %8lld %8d (%d samples)\n", i, ape_ctx->d5[i].a, ape_ctx->d5[i].c, ape_ctx->d5[i].b);

    av_log(s, AV_LOG_DEBUG, "\nCalculated information:\n\n");
    av_log(s, AV_LOG_DEBUG, "junklength           = %d\n", ape_ctx->d1);
    av_log(s, AV_LOG_DEBUG, "firstframe           = %d\n", ape_ctx->d2);
    av_log(s, AV_LOG_DEBUG, "totalsamples         = %d\n", ape_ctx->d3);
#endif
}

static int ape_read_header(AVFormatContext * s, AVFormatParameters * ap)
{
    AVIOContext *pb = s->pb;
    ConvertedAPEContext *ape = s->priv_data;
    AVStream *st;
    uint32_t tag;
    int i;
    int total_blocks;
    int64_t pts;

    ape->d1 = 0;

    tag = avio_rl32(pb);
    if (tag != MKTAG('M', 'A', 'C', ' '))
        return -1;

    ape->b2 = avio_rl16(pb);

    if (ape->b2 < APE_MIN_VERSION || ape->b2 > APE_MAX_VERSION) {
        av_log(s, AV_LOG_ERROR, "Unsupported file version - %d.%02d\n", ape->b2 / 1000, (ape->b2 % 1000) / 10);
        return -1;
    }

    if (ape->b2 >= 3980) {
        ape->b3 = avio_rl16(pb);
        ape->b4 = avio_rl32(pb);
        ape->b5 = avio_rl32(pb);
        ape->b6 = avio_rl32(pb);
        ape->b7 = avio_rl32(pb);
        ape->b8 = avio_rl32(pb);
        ape->b9 = avio_rl32(pb);
        ape->b10 = avio_rl32(pb);
        avio_read(pb, ape->b11, 16);

        if (ape->b4 > 52)
            avio_seek(pb, ape->b4 - 52, SEEK_CUR);

        ape->c1 = avio_rl16(pb);
        ape->c2 = avio_rl16(pb);
        ape->c3 = avio_rl32(pb);
        ape->c4 = avio_rl32(pb);
        ape->c5 = avio_rl32(pb);
        ape->c6 = avio_rl16(pb);
        ape->c7 = avio_rl16(pb);
        ape->c8 = avio_rl32(pb);
    } else {
        ape->b4 = 0;
        ape->b5 = 32;

        ape->c1 = avio_rl16(pb);
        ape->c2 = avio_rl16(pb);
        ape->c7 = avio_rl16(pb);
        ape->c8 = avio_rl32(pb);
        ape->b7 = avio_rl32(pb);
        ape->b10 = avio_rl32(pb);
        ape->c5 = avio_rl32(pb);
        ape->c4 = avio_rl32(pb);

        if (ape->c2 & MAC_FORMAT_FLAG_HAS_PEAK_LEVEL) {
            avio_seek(pb, 4, SEEK_CUR);
            ape->b5 += 4;
        }

        if (ape->c2 & MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS) {
            ape->b6 = avio_rl32(pb);
            ape->b5 += 4;
            ape->b6 *= sizeof(int32_t);
        } else
            ape->b6 = ape->c5 * sizeof(int32_t);

        if (ape->c2 & MAC_FORMAT_FLAG_8_BIT)
            ape->c6 = 8;
        else if (ape->c2 & MAC_FORMAT_FLAG_24_BIT)
            ape->c6 = 24;
        else
            ape->c6 = 16;

        if (ape->b2 >= 3950)
            ape->c3 = 73728 * 4;
        else if (ape->b2 >= 3900 || (ape->b2 >= 3800  && ape->c1 >= 4000))
            ape->c3 = 73728;
        else
            ape->c3 = 9216;

        if (!(ape->c2 & MAC_FORMAT_FLAG_CREATE_WAV_HEADER))
            avio_seek(pb, ape->b7, SEEK_CUR);
    }

    if(ape->c5 > UINT_MAX / sizeof(ConvertedAPEFrame)){
        av_log(s, AV_LOG_ERROR, "Too many frames: %d\n", ape->c5);
        return -1;
    }
    ape->d5 = av_malloc(ape->c5 * sizeof(ConvertedAPEFrame));
    if(!ape->d5)
        return AVERROR(ENOMEM);
    ape->d2 = ape->d1 + ape->b4 + ape->b5 + ape->b6 + ape->b7;
    ape->d4 = 0;

    ape->d3 = ape->c4;
    if (ape->c5 > 1)
        ape->d3 += ape->c3 * (ape->c5 - 1);

    if (ape->b6 > 0) {
        ape->c9 = av_malloc(ape->b6);
        for (i = 0; i < ape->b6 / sizeof(uint32_t); i++)
            ape->c9[i] = avio_rl32(pb);
    }

    ape->d5[0].a = ape->d2;
    ape->d5[0].b = ape->c3;
    ape->d5[0].d = 0;
    for (i = 1; i < ape->c5; i++) {
        ape->d5[i].a = ape->c9[i];
        ape->d5[i].b = ape->c3;
        ape->d5[i - 1].c = ape->d5[i].a - ape->d5[i - 1].a;
        ape->d5[i].d = (ape->d5[i].a - ape->d5[0].a) & 3;
    }
    ape->d5[ape->c5 - 1].c = ape->c4 * 4;
    ape->d5[ape->c5 - 1].b = ape->c4;

    for (i = 0; i < ape->c5; i++) {
        if(ape->d5[i].d){
            ape->d5[i].a -= ape->d5[i].d;
            ape->d5[i].c += ape->d5[i].d;
        }
        ape->d5[i].c = (ape->d5[i].c + 3) & ~3;
    }

    ape_dumpinfo(s, ape);

    if (!url_is_streamed(pb)) {
        ff_ape_parse_tag(s);
        avio_seek(pb, 0, SEEK_SET);
    }

    av_log(s, AV_LOG_DEBUG, "Decoding file - v%d.%02d, compression level %d\n", ape->b2 / 1000, (ape->b2 % 1000) / 10, ape->c1);

    st = av_new_stream(s, 0);
    if (!st)
        return -1;

    total_blocks = (ape->c5 == 0) ? 0 : ((ape->c5 - 1) * ape->c3) + ape->c4;

    st->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codec->codec_id = CODEC_ID_APE;
    st->codec->codec_tag = MKTAG('A', 'P', 'E', ' ');
    st->codec->channels = ape->c7;
    st->codec->sample_rate = ape->c8;
    st->codec->bits_per_coded_sample = ape->c6;
    st->codec->frame_size = MAC_SUBFRAME_SIZE;

    st->nb_frames = ape->c5;
    st->start_time = 0;
    st->duration = total_blocks / MAC_SUBFRAME_SIZE;
    av_set_pts_info(st, 64, MAC_SUBFRAME_SIZE, ape->c8);

    st->codec->extradata = av_malloc(APE_EXTRADATA_SIZE);
    st->codec->extradata_size = APE_EXTRADATA_SIZE;
    AV_WL16(st->codec->extradata + 0, ape->b2);
    AV_WL16(st->codec->extradata + 2, ape->c1);
    AV_WL16(st->codec->extradata + 4, ape->c2);

    pts = 0;
    for (i = 0; i < ape->c5; i++) {
        ape->d5[i].e = pts;
        av_add_index_entry(st, ape->d5[i].a, ape->d5[i].e, 0, 0, AVINDEX_KEYFRAME);
        pts += ape->c3 / MAC_SUBFRAME_SIZE;
    }

    return 0;
}

static int ape_read_packet(AVFormatContext * s, AVPacket * pkt)
{
    int ret;
    int nblocks;
    ConvertedAPEContext *ape = s->priv_data;
    uint32_t extra_size = 8;

    if (s->pb->eof_reached)
        return AVERROR(EIO);
    if (ape->d4 > ape->c5)
        return AVERROR(EIO);

    avio_seek(s->pb, ape->d5[ape->d4].a, SEEK_SET);

    if (ape->d4 == (ape->c5 - 1))
        nblocks = ape->c4;
    else
        nblocks = ape->c3;

    if (av_new_packet(pkt, ape->d5[ape->d4].c + extra_size) < 0)
        return AVERROR(ENOMEM);

    AV_WL32(pkt->data, nblocks);
    AV_WL32(pkt->data + 4, ape->d5[ape->d4].d);
    ret = avio_read(s->pb, pkt->data + extra_size, ape->d5[ape->d4].c);

    pkt->pts = ape->d5[ape->d4].e;
    pkt->stream_index = 0;

    pkt->size = ret + extra_size;

    ape->d4++;

    return 0;
}

static int ape_read_close(AVFormatContext * s)
{
    ConvertedAPEContext *ape = s->priv_data;

    av_freep(&ape->d5);
    av_freep(&ape->c9);
    return 0;
}

static int ape_read_seek(AVFormatContext *s, int stream_index, int64_t timestamp, int flags)
{
    AVStream *st = s->streams[stream_index];
    ConvertedAPEContext *ape = s->priv_data;
    int index = av_index_search_timestamp(st, timestamp, flags);

    if (index < 0)
        return -1;

    ape->d4 = index;
    return 0;
}

AVInputFormat ff_ape_demuxer = {
    "ape",
    NULL_IF_CONFIG_SMALL("Monkey's Audio"),
    sizeof(ConvertedAPEContext),
    ape_probe,
    ape_read_header,
    ape_read_packet,
    ape_read_close,
    ape_read_seek,
    .extensions = "ape,apl,mac"
};