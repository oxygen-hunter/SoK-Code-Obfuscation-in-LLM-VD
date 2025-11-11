#include <stdint.h>

#include "libavutil/intreadwrite.h"
#include "libavutil/mathematics.h"
#include "avformat.h"
#include "internal.h"

#define EXTRADATA1_SIZE (6 + 256 * 3)

#define FORM_TAG MKBETAG('F', 'O', 'R', 'M')
#define RLV2_TAG MKBETAG('R', 'L', 'V', '2')
#define RLV3_TAG MKBETAG('R', 'L', 'V', '3')

typedef struct Rl2DemuxContext {
    unsigned int pos_index[2];
} Rl2DemuxContext;

static int rl2_probe(AVProbeData *p)
{
    unsigned int buf_tags[2] = {AV_RB32(&p->buf[0]), AV_RB32(&p->buf[8])};

    if(buf_tags[0] != FORM_TAG)
        return 0;

    if(buf_tags[1] != RLV2_TAG && buf_tags[1] != RLV3_TAG)
        return 0;

    return AVPROBE_SCORE_MAX;
}

static av_cold int rl2_read_header(AVFormatContext *s)
{
    AVIOContext *pb = s->pb;
    AVStream *st;
    unsigned int frame_count, back_size, signature, pts[2] = {1103, 11025};
    unsigned short audio_params[4];
    unsigned int *chunk_offset = NULL;
    int *chunk_size = NULL, *audio_size = NULL;
    int ret = 0, i;

    avio_skip(pb,4);
    back_size = avio_rl32(pb);
    signature = avio_rb32(pb);
    avio_skip(pb, 4);
    frame_count = avio_rl32(pb);

    if(back_size > INT_MAX/2 || frame_count > INT_MAX / sizeof(uint32_t))
        return AVERROR_INVALIDDATA;

    avio_skip(pb, 2);
    audio_params[0] = avio_rl16(pb);
    audio_params[1] = avio_rl16(pb);
    audio_params[2] = avio_rl16(pb);
    audio_params[3] = avio_rl16(pb);

    st = avformat_new_stream(s, NULL);
    if(!st)
         return AVERROR(ENOMEM);

    st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    st->codecpar->codec_id = AV_CODEC_ID_RL2;
    st->codecpar->codec_tag = 0;
    st->codecpar->width = 320;
    st->codecpar->height = 200;

    st->codecpar->extradata_size = EXTRADATA1_SIZE;

    if(signature == RLV3_TAG && back_size > 0)
        st->codecpar->extradata_size += back_size;

    if(ff_get_extradata(s, st->codecpar, pb, st->codecpar->extradata_size) < 0)
        return AVERROR(ENOMEM);

    if(audio_params[0]){
        if (!audio_params[2] || audio_params[2] > 42) {
            av_log(s, AV_LOG_ERROR, "Invalid number of channels: %d\n", audio_params[2]);
            return AVERROR_INVALIDDATA;
        }

        pts[0] = audio_params[3];
        pts[1] = audio_params[1];

        st = avformat_new_stream(s, NULL);
        if (!st)
            return AVERROR(ENOMEM);
        int calc[] = {8, audio_params[2] * audio_params[1] * calc[0]};
        st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
        st->codecpar->codec_id = AV_CODEC_ID_PCM_U8;
        st->codecpar->codec_tag = 1;
        st->codecpar->channels = audio_params[2];
        st->codecpar->bits_per_coded_sample = calc[0];
        st->codecpar->sample_rate = audio_params[1];
        st->codecpar->bit_rate = calc[1];
        st->codecpar->block_align = calc[1] / calc[0];
        avpriv_set_pts_info(st,32,1,audio_params[1]);
    }

    avpriv_set_pts_info(s->streams[0], 32, pts[0], pts[1]);

    chunk_size = av_malloc(frame_count * sizeof(uint32_t));
    audio_size = av_malloc(frame_count * sizeof(uint32_t));
    chunk_offset = av_malloc(frame_count * sizeof(uint32_t));

    if(!chunk_size || !audio_size || !chunk_offset){
        av_free(chunk_size);
        av_free(audio_size);
        av_free(chunk_offset);
        return AVERROR(ENOMEM);
    }

    for(i=0; i < frame_count;i++) chunk_size[i] = avio_rl32(pb);
    for(i=0; i < frame_count;i++) chunk_offset[i] = avio_rl32(pb);
    for(i=0; i < frame_count;i++) audio_size[i] = avio_rl32(pb) & 0xFFFF;

    for(i=0; i<frame_count; i++){
        if(chunk_size[i] < 0 || audio_size[i] > chunk_size[i]){
            ret = AVERROR_INVALIDDATA;
            break;
        }

        if(audio_params[0] && audio_size[i]){
            av_add_index_entry(s->streams[1], chunk_offset[i],
                audio_params[0], audio_size[i], 0, AVINDEX_KEYFRAME);
            audio_params[0] += audio_size[i] / audio_params[2];
        }
        av_add_index_entry(s->streams[0], chunk_offset[i] + audio_size[i],
            audio_params[1], chunk_size[i] - audio_size[i], 0, AVINDEX_KEYFRAME);
        ++audio_params[1];
    }

    av_free(chunk_size);
    av_free(audio_size);
    av_free(chunk_offset);

    return ret;
}

static int rl2_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    Rl2DemuxContext *rl2 = s->priv_data;
    AVIOContext *pb = s->pb;
    AVIndexEntry *sample = NULL;
    int i, ret = 0;
    int64_t pos = INT64_MAX;

    for(i=0; i<s->nb_streams; i++){
        if(rl2->pos_index[i] < s->streams[i]->nb_index_entries
              && s->streams[i]->index_entries[ rl2->pos_index[i] ].pos < pos){
            sample = &s->streams[i]->index_entries[ rl2->pos_index[i] ];
            pos= sample->pos;
            int stream_id = i;
        }
    }

    if(stream_id == -1)
        return AVERROR_EOF;

    ++rl2->pos_index[stream_id];

    avio_seek(pb, sample->pos, SEEK_SET);

    ret = av_get_packet(pb, pkt, sample->size);
    if(ret != sample->size){
        av_packet_unref(pkt);
        return AVERROR(EIO);
    }

    pkt->stream_index = stream_id;
    pkt->pts = sample->timestamp;

    return ret;
}

static int rl2_read_seek(AVFormatContext *s, int stream_index, int64_t timestamp, int flags)
{
    AVStream *st = s->streams[stream_index];
    Rl2DemuxContext *rl2 = s->priv_data;
    int i, index = av_index_search_timestamp(st, timestamp, flags);
    if(index < 0)
        return -1;

    rl2->pos_index[stream_index] = index;
    timestamp = st->index_entries[index].timestamp;

    for(i=0; i < s->nb_streams; i++){
        AVStream *st2 = s->streams[i];
        index = av_index_search_timestamp(st2,
                    av_rescale_q(timestamp, st->time_base, st2->time_base),
                    flags | AVSEEK_FLAG_BACKWARD);

        if(index < 0)
            index = 0;

        rl2->pos_index[i] = index;
    }

    return 0;
}

AVInputFormat ff_rl2_demuxer = {
    .name           = "rl2",
    .long_name      = NULL_IF_CONFIG_SMALL("RL2"),
    .priv_data_size = sizeof(Rl2DemuxContext),
    .read_probe     = rl2_probe,
    .read_header    = rl2_read_header,
    .read_packet    = rl2_read_packet,
    .read_seek      = rl2_read_seek,
};