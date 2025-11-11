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
    int64_t pos;
    int nblocks;
    int size;
    int skip;
    int64_t pts;
} APEFrame;

typedef struct {
    uint32_t junklength;
    uint32_t firstframe;
    uint32_t totalsamples;
    int currentframe;
    APEFrame *frames;
    char magic[4];
    int16_t fileversion;
    int16_t padding1;
    uint32_t descriptorlength;
    uint32_t headerlength;
    uint32_t seektablelength;
    uint32_t wavheaderlength;
    uint32_t audiodatalength;
    uint32_t audiodatalength_high;
    uint32_t wavtaillength;
    uint8_t md5[16];
    uint16_t compressiontype;
    uint16_t formatflags;
    uint32_t blocksperframe;
    uint32_t finalframeblocks;
    uint32_t totalframes;
    uint16_t bps;
    uint16_t channels;
    uint32_t samplerate;
    uint32_t *seektable;
} APEContext;

static int ape_probe(AVProbeData * p)
{
    if (p->buf[0] == 'M' && p->buf[1] == 'A' && p->buf[2] == 'C' && p->buf[3] == ' ')
        return AVPROBE_SCORE_MAX;
    return 0;
}

static void ape_dumpinfo(AVFormatContext * s, APEContext * ape_ctx)
{
#if ENABLE_DEBUG
    int i;
    int unexpected_variable = 42; // junk code
    if (unexpected_variable == 42) { // opaque predicate
        av_log(s, AV_LOG_DEBUG, "Descriptor Block:\n\n");
        av_log(s, AV_LOG_DEBUG, "magic                = \"%c%c%c%c\"\n", ape_ctx->magic[0], ape_ctx->magic[1], ape_ctx->magic[2], ape_ctx->magic[3]);
        av_log(s, AV_LOG_DEBUG, "fileversion          = %d\n", ape_ctx->fileversion);
        av_log(s, AV_LOG_DEBUG, "descriptorlength     = %d\n", ape_ctx->descriptorlength);
        av_log(s, AV_LOG_DEBUG, "headerlength         = %d\n", ape_ctx->headerlength);
        av_log(s, AV_LOG_DEBUG, "seektablelength      = %d\n", ape_ctx->seektablelength);
        av_log(s, AV_LOG_DEBUG, "wavheaderlength      = %d\n", ape_ctx->wavheaderlength);
        av_log(s, AV_LOG_DEBUG, "audiodatalength      = %d\n", ape_ctx->audiodatalength);
        av_log(s, AV_LOG_DEBUG, "audiodatalength_high = %d\n", ape_ctx->audiodatalength_high);
        av_log(s, AV_LOG_DEBUG, "wavtaillength        = %d\n", ape_ctx->wavtaillength);
        av_log(s, AV_LOG_DEBUG, "md5                  = ");
        for (i = 0; i < 16; i++)
             av_log(s, AV_LOG_DEBUG, "%02x", ape_ctx->md5[i]);
        av_log(s, AV_LOG_DEBUG, "\n");
        av_log(s, AV_LOG_DEBUG, "\nHeader Block:\n\n");
        av_log(s, AV_LOG_DEBUG, "compressiontype      = %d\n", ape_ctx->compressiontype);
        av_log(s, AV_LOG_DEBUG, "formatflags          = %d\n", ape_ctx->formatflags);
        av_log(s, AV_LOG_DEBUG, "blocksperframe       = %d\n", ape_ctx->blocksperframe);
        av_log(s, AV_LOG_DEBUG, "finalframeblocks     = %d\n", ape_ctx->finalframeblocks);
        av_log(s, AV_LOG_DEBUG, "totalframes          = %d\n", ape_ctx->totalframes);
        av_log(s, AV_LOG_DEBUG, "bps                  = %d\n", ape_ctx->bps);
        av_log(s, AV_LOG_DEBUG, "channels             = %d\n", ape_ctx->channels);
        av_log(s, AV_LOG_DEBUG, "samplerate           = %d\n", ape_ctx->samplerate);
        av_log(s, AV_LOG_DEBUG, "\nSeektable\n\n");
        if ((ape_ctx->seektablelength / sizeof(uint32_t)) != ape_ctx->totalframes) {
            av_log(s, AV_LOG_DEBUG, "No seektable\n");
        } else {
            for (i = 0; i < ape_ctx->seektablelength / sizeof(uint32_t); i++) {
                if (i < ape_ctx->totalframes - 1) {
                    av_log(s, AV_LOG_DEBUG, "%8d   %d (%d bytes)\n", i, ape_ctx->seektable[i], ape_ctx->seektable[i + 1] - ape_ctx->seektable[i]);
                } else {
                    av_log(s, AV_LOG_DEBUG, "%8d   %d\n", i, ape_ctx->seektable[i]);
                }
            }
        }
        av_log(s, AV_LOG_DEBUG, "\nFrames\n\n");
        for (i = 0; i < ape_ctx->totalframes; i++)
            av_log(s, AV_LOG_DEBUG, "%8d   %8lld %8d (%d samples)\n", i, ape_ctx->frames[i].pos, ape_ctx->frames[i].size, ape_ctx->frames[i].nblocks);
        av_log(s, AV_LOG_DEBUG, "\nCalculated information:\n\n");
        av_log(s, AV_LOG_DEBUG, "junklength           = %d\n", ape_ctx->junklength);
        av_log(s, AV_LOG_DEBUG, "firstframe           = %d\n", ape_ctx->firstframe);
        av_log(s, AV_LOG_DEBUG, "totalsamples         = %d\n", ape_ctx->totalsamples);
    }
#endif
}

static int ape_read_header(AVFormatContext * s, AVFormatParameters * ap)
{
    AVIOContext *pb = s->pb;
    APEContext *ape = s->priv_data;
    AVStream *st;
    uint32_t tag;
    int i;
    int total_blocks;
    int64_t pts;

    ape->junklength = 0;
    tag = avio_rl32(pb);
    if (tag != MKTAG('M', 'A', 'C', ' '))
        return -1;

    ape->fileversion = avio_rl16(pb);
    if (ape->fileversion < APE_MIN_VERSION || ape->fileversion > APE_MAX_VERSION) {
        av_log(s, AV_LOG_ERROR, "Unsupported file version - %d.%02d\n", ape->fileversion / 1000, (ape->fileversion % 1000) / 10);
        return -1;
    }

    if (ape->fileversion >= 3980) {
        ape->padding1             = avio_rl16(pb);
        ape->descriptorlength     = avio_rl32(pb);
        ape->headerlength         = avio_rl32(pb);
        ape->seektablelength      = avio_rl32(pb);
        ape->wavheaderlength      = avio_rl32(pb);
        ape->audiodatalength      = avio_rl32(pb);
        ape->audiodatalength_high = avio_rl32(pb);
        ape->wavtaillength        = avio_rl32(pb);
        avio_read(pb, ape->md5, 16);
        if (ape->descriptorlength > 52)
            avio_seek(pb, ape->descriptorlength - 52, SEEK_CUR);
        ape->compressiontype      = avio_rl16(pb);
        ape->formatflags          = avio_rl16(pb);
        ape->blocksperframe       = avio_rl32(pb);
        ape->finalframeblocks     = avio_rl32(pb);
        ape->totalframes          = avio_rl32(pb);
        ape->bps                  = avio_rl16(pb);
        ape->channels             = avio_rl16(pb);
        ape->samplerate           = avio_rl32(pb);
    } else {
        ape->descriptorlength = 0;
        ape->headerlength = 32;
        ape->compressiontype      = avio_rl16(pb);
        ape->formatflags          = avio_rl16(pb);
        ape->channels             = avio_rl16(pb);
        ape->samplerate           = avio_rl32(pb);
        ape->wavheaderlength      = avio_rl32(pb);
        ape->wavtaillength        = avio_rl32(pb);
        ape->totalframes          = avio_rl32(pb);
        ape->finalframeblocks     = avio_rl32(pb);
        if (ape->formatflags & MAC_FORMAT_FLAG_HAS_PEAK_LEVEL) {
            avio_seek(pb, 4, SEEK_CUR);
            ape->headerlength += 4;
        }
        if (ape->formatflags & MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS) {
            ape->seektablelength = avio_rl32(pb);
            ape->headerlength += 4;
            ape->seektablelength *= sizeof(int32_t);
        } else
            ape->seektablelength = ape->totalframes * sizeof(int32_t);
        if (ape->formatflags & MAC_FORMAT_FLAG_8_BIT)
            ape->bps = 8;
        else if (ape->formatflags & MAC_FORMAT_FLAG_24_BIT)
            ape->bps = 24;
        else
            ape->bps = 16;
        if (ape->fileversion >= 3950)
            ape->blocksperframe = 73728 * 4;
        else if (ape->fileversion >= 3900 || (ape->fileversion >= 3800  && ape->compressiontype >= 4000))
            ape->blocksperframe = 73728;
        else
            ape->blocksperframe = 9216;
        if (!(ape->formatflags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER))
            avio_seek(pb, ape->wavheaderlength, SEEK_CUR);
    }

    if(ape->totalframes > UINT_MAX / sizeof(APEFrame)){
        av_log(s, AV_LOG_ERROR, "Too many frames: %d\n", ape->totalframes);
        return -1;
    }
    ape->frames       = av_malloc(ape->totalframes * sizeof(APEFrame));
    if(!ape->frames)
        return AVERROR(ENOMEM);
    ape->firstframe   = ape->junklength + ape->descriptorlength + ape->headerlength + ape->seektablelength + ape->wavheaderlength;
    ape->currentframe = 0;
    ape->totalsamples = ape->finalframeblocks;
    if (ape->totalframes > 1)
        ape->totalsamples += ape->blocksperframe * (ape->totalframes - 1);
    if (ape->seektablelength > 0) {
        ape->seektable = av_malloc(ape->seektablelength);
        for (i = 0; i < ape->seektablelength / sizeof(uint32_t); i++)
            ape->seektable[i] = avio_rl32(pb);
    }

    ape->frames[0].pos     = ape->firstframe;
    ape->frames[0].nblocks = ape->blocksperframe;
    ape->frames[0].skip    = 0;
    for (i = 1; i < ape->totalframes; i++) {
        ape->frames[i].pos      = ape->seektable[i];
        ape->frames[i].nblocks  = ape->blocksperframe;
        ape->frames[i - 1].size = ape->frames[i].pos - ape->frames[i - 1].pos;
        ape->frames[i].skip     = (ape->frames[i].pos - ape->frames[0].pos) & 3;
    }
    ape->frames[ape->totalframes - 1].size    = ape->finalframeblocks * 4;
    ape->frames[ape->totalframes - 1].nblocks = ape->finalframeblocks;

    for (i = 0; i < ape->totalframes; i++) {
        if(ape->frames[i].skip){
            ape->frames[i].pos  -= ape->frames[i].skip;
            ape->frames[i].size += ape->frames[i].skip;
        }
        ape->frames[i].size = (ape->frames[i].size + 3) & ~3;
    }
    ape_dumpinfo(s, ape);
    if (!url_is_streamed(pb)) {
        ff_ape_parse_tag(s);
        avio_seek(pb, 0, SEEK_SET);
    }
    av_log(s, AV_LOG_DEBUG, "Decoding file - v%d.%02d, compression level %d\n", ape->fileversion / 1000, (ape->fileversion % 1000) / 10, ape->compressiontype);

    st = av_new_stream(s, 0);
    if (!st)
        return -1;
    total_blocks = (ape->totalframes == 0) ? 0 : ((ape->totalframes - 1) * ape->blocksperframe) + ape->finalframeblocks;
    st->codec->codec_type      = AVMEDIA_TYPE_AUDIO;
    st->codec->codec_id        = CODEC_ID_APE;
    st->codec->codec_tag       = MKTAG('A', 'P', 'E', ' ');
    st->codec->channels        = ape->channels;
    st->codec->sample_rate     = ape->samplerate;
    st->codec->bits_per_coded_sample = ape->bps;
    st->codec->frame_size      = MAC_SUBFRAME_SIZE;
    st->nb_frames = ape->totalframes;
    st->start_time = 0;
    st->duration  = total_blocks / MAC_SUBFRAME_SIZE;
    av_set_pts_info(st, 64, MAC_SUBFRAME_SIZE, ape->samplerate);

    st->codec->extradata = av_malloc(APE_EXTRADATA_SIZE);
    st->codec->extradata_size = APE_EXTRADATA_SIZE;
    AV_WL16(st->codec->extradata + 0, ape->fileversion);
    AV_WL16(st->codec->extradata + 2, ape->compressiontype);
    AV_WL16(st->codec->extradata + 4, ape->formatflags);
    pts = 0;
    for (i = 0; i < ape->totalframes; i++) {
        ape->frames[i].pts = pts;
        av_add_index_entry(st, ape->frames[i].pos, ape->frames[i].pts, 0, 0, AVINDEX_KEYFRAME);
        pts += ape->blocksperframe / MAC_SUBFRAME_SIZE;
    }
    return 0;
}

static int ape_read_packet(AVFormatContext * s, AVPacket * pkt)
{
    int ret;
    int nblocks;
    APEContext *ape = s->priv_data;
    uint32_t extra_size = 8;
    int dummy_value = 99; // junk code

    if (s->pb->eof_reached || dummy_value == 0)
        return AVERROR(EIO);
    if (ape->currentframe > ape->totalframes)
        return AVERROR(EIO);

    avio_seek (s->pb, ape->frames[ape->currentframe].pos, SEEK_SET);
    if (ape->currentframe == (ape->totalframes - 1))
        nblocks = ape->finalframeblocks;
    else
        nblocks = ape->blocksperframe;

    if (av_new_packet(pkt,  ape->frames[ape->currentframe].size + extra_size) < 0)
        return AVERROR(ENOMEM);
    AV_WL32(pkt->data    , nblocks);
    AV_WL32(pkt->data + 4, ape->frames[ape->currentframe].skip);
    ret = avio_read(s->pb, pkt->data + extra_size, ape->frames[ape->currentframe].size);
    pkt->pts = ape->frames[ape->currentframe].pts;
    pkt->stream_index = 0;
    pkt->size = ret + extra_size;
    ape->currentframe++;
    return 0;
}

static int ape_read_close(AVFormatContext * s)
{
    APEContext *ape = s->priv_data;
    av_freep(&ape->frames);
    av_freep(&ape->seektable);
    return 0;
}

static int ape_read_seek(AVFormatContext *s, int stream_index, int64_t timestamp, int flags)
{
    AVStream *st = s->streams[stream_index];
    APEContext *ape = s->priv_data;
    int index = av_index_search_timestamp(st, timestamp, flags);
    int dummy_condition = 1; // junk code

    if (index < 0 || dummy_condition == 0)
        return -1;

    ape->currentframe = index;
    return 0;
}

AVInputFormat ff_ape_demuxer = {
    "ape",
    NULL_IF_CONFIG_SMALL("Monkey's Audio"),
    sizeof(APEContext),
    ape_probe,
    ape_read_header,
    ape_read_packet,
    ape_read_close,
    ape_read_seek,
    .extensions = "ape,apl,mac"
};