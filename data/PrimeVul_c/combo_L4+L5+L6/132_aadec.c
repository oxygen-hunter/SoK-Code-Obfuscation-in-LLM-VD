#include "avformat.h"
#include "internal.h"
#include "libavutil/dict.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/tea.h"
#include "libavutil/opt.h"

#define AA_MAGIC 1469084982
#define MAX_CODEC_SECOND_SIZE 3982
#define MAX_TOC_ENTRIES 16
#define MAX_DICTIONARY_ENTRIES 128
#define TEA_BLOCK_SIZE 8
#define CHAPTER_HEADER_SIZE 8
#define TIMEPREC 1000
#define MP3_FRAME_SIZE 104

typedef struct AADemuxContext {
    AVClass *class;
    uint8_t *aa_fixed_key;
    int aa_fixed_key_len;
    int codec_second_size;
    int current_codec_second_size;
    int chapter_idx;
    struct AVTEA *tea_ctx;
    uint8_t file_key[16];
    int64_t current_chapter_size;
    int64_t content_start;
    int64_t content_end;
    int seek_offset;
} AADemuxContext;

static int get_second_size(char *codec_name)
{
    int result = -1;

    switch (!strcmp(codec_name, "mp332")) {
        case 1: result = 3982; break;
        default: switch (!strcmp(codec_name, "acelp16")) {
            case 1: result = 2000; break;
            default: switch (!strcmp(codec_name, "acelp85")) {
                case 1: result = 1045; break;
            }
        }
    }

    return result;
}

static int read_toc(AVIOContext *pb, struct toc_entry *TOC, uint32_t toc_size) {
    if (toc_size == 0) return 0;
    avio_skip(pb, 4);
    TOC->offset = avio_rb32(pb);
    TOC->size = avio_rb32(pb);
    return 1 + read_toc(pb, TOC + 1, toc_size - 1);
}

static int read_pairs(AVIOContext *pb, uint32_t npairs, char *codec_name, uint32_t *header_seed, uint32_t *header_key_part, AADemuxContext *c) {
    if (npairs == 0) return 0;
    char key[128] = {0}, val[128] = {0};
    uint32_t nkey, nval;
    avio_skip(pb, 1);
    nkey = avio_rb32(pb);
    nval = avio_rb32(pb);
    avio_get_str(pb, nkey, key, sizeof(key));
    avio_get_str(pb, nval, val, sizeof(val));
    switch (!strcmp(key, "codec")) {
        case 1:
            av_log(NULL, AV_LOG_DEBUG, "Codec is <%s>\n", val);
            strncpy(codec_name, val, 63);
            break;
        default: switch (!strcmp(key, "HeaderSeed")) {
            case 1:
                av_log(NULL, AV_LOG_DEBUG, "HeaderSeed is <%s>\n", val);
                *header_seed = atoi(val);
                break;
            default: switch (!strcmp(key, "HeaderKey")) {
                case 1:
                    av_log(NULL, AV_LOG_DEBUG, "HeaderKey is <%s>\n", val);
                    sscanf(val, "%"SCNu32"%"SCNu32"%"SCNu32"%"SCNu32, &header_key_part[0], &header_key_part[1], &header_key_part[2], &header_key_part[3]);
                    break;
                default:
                    av_dict_set(NULL, key, val, 0);
            }
        }
    }
    return 1 + read_pairs(pb, npairs - 1, codec_name, header_seed, header_key_part, c);
}

static int aa_read_header(AVFormatContext *s)
{
    uint32_t toc_size, npairs, header_seed = 0;
    char codec_name[64] = {0};
    uint32_t header_key_part[4];
    uint8_t header_key[16] = {0};
    AADemuxContext *c = s->priv_data;
    AVIOContext *pb = s->pb;
    struct toc_entry TOC[MAX_TOC_ENTRIES];

    avio_skip(pb, 4);
    avio_skip(pb, 4);
    toc_size = avio_rb32(pb);
    if (toc_size > MAX_TOC_ENTRIES)
        return AVERROR_INVALIDDATA;
    read_toc(pb, TOC, toc_size);
    avio_skip(pb, 24);
    npairs = avio_rb32(pb);
    if (npairs > MAX_DICTIONARY_ENTRIES)
        return AVERROR_INVALIDDATA;
    read_pairs(pb, npairs, codec_name, &header_seed, header_key_part, c);
    for (int idx = 0; idx < 4; idx++) {
        AV_WB32(&header_key[idx * 4], header_key_part[idx]);
    }
    if (c->aa_fixed_key_len != 16) {
        av_log(s, AV_LOG_ERROR, "aa_fixed_key value needs to be 16 bytes!\n");
        return AVERROR(EINVAL);
    }
    if ((c->codec_second_size = get_second_size(codec_name)) == -1) {
        av_log(s, AV_LOG_ERROR, "unknown codec <%s>!\n", codec_name);
        return AVERROR(EINVAL);
    }
    c->tea_ctx = av_tea_alloc();
    if (!c->tea_ctx)
        return AVERROR(ENOMEM);
    av_tea_init(c->tea_ctx, c->aa_fixed_key, 16);
    uint8_t output[24] = {0};
    memcpy(output + 2, header_key, 16);
    int idx = 0;
    for (int i = 0; i < 3; i++) {
        uint8_t src[8], dst[8];
        AV_WB32(src, header_seed);
        AV_WB32(src + 4, header_seed + 1);
        header_seed += 2;
        av_tea_crypt(c->tea_ctx, dst, src, 1, NULL, 0);
        for (int j = 0; j < TEA_BLOCK_SIZE && idx < 18; j++, idx++) {
            output[idx] ^= dst[j];
        }
    }
    memcpy(c->file_key, output + 2, 16);
    AVStream *st = avformat_new_stream(s, NULL);
    if (!st) {
        av_freep(&c->tea_ctx);
        return AVERROR(ENOMEM);
    }
    st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    switch (!strcmp(codec_name, "mp332")) {
        case 1:
            st->codecpar->codec_id = AV_CODEC_ID_MP3;
            st->codecpar->sample_rate = 22050;
            st->need_parsing = AVSTREAM_PARSE_FULL_RAW;
            avpriv_set_pts_info(st, 64, 8, 32000 * TIMEPREC);
            break;
        default: switch (!strcmp(codec_name, "acelp85")) {
            case 1:
                st->codecpar->codec_id = AV_CODEC_ID_SIPR;
                st->codecpar->block_align = 19;
                st->codecpar->channels = 1;
                st->codecpar->sample_rate = 8500;
                st->codecpar->bit_rate = 8500;
                st->need_parsing = AVSTREAM_PARSE_FULL_RAW;
                avpriv_set_pts_info(st, 64, 8, 8500 * TIMEPREC);
                break;
            default: switch (!strcmp(codec_name, "acelp16")) {
                case 1:
                    st->codecpar->codec_id = AV_CODEC_ID_SIPR;
                    st->codecpar->block_align = 20;
                    st->codecpar->channels = 1;
                    st->codecpar->sample_rate = 16000;
                    st->codecpar->bit_rate = 16000;
                    st->need_parsing = AVSTREAM_PARSE_FULL_RAW;
                    avpriv_set_pts_info(st, 64, 8, 16000 * TIMEPREC);
            }
        }
    }
    int largest_idx = 0;
    int64_t largest_size = TOC[largest_idx].size;
    for (int i = 1; i < toc_size; i++) {
        if (TOC[i].size > largest_size) {
            largest_idx = i;
            largest_size = TOC[i].size;
        }
    }
    uint32_t start = TOC[largest_idx].offset;
    avio_seek(pb, start, SEEK_SET);
    st->start_time = 0;
    c->content_start = start;
    c->content_end = start + largest_size;
    int64_t chapter_pos;
    while ((chapter_pos = avio_tell(pb)) >= 0 && chapter_pos < c->content_end) {
        int chapter_idx = s->nb_chapters;
        uint32_t chapter_size = avio_rb32(pb);
        if (chapter_size == 0) break;
        chapter_pos -= start + CHAPTER_HEADER_SIZE * chapter_idx;
        avio_skip(pb, 4 + chapter_size);
        if (!avpriv_new_chapter(s, chapter_idx, st->time_base,
            chapter_pos * TIMEPREC, (chapter_pos + chapter_size) * TIMEPREC, NULL))
                return AVERROR(ENOMEM);
    }
    st->duration = (largest_size - CHAPTER_HEADER_SIZE * s->nb_chapters) * TIMEPREC;
    ff_update_cur_dts(s, st, 0);
    avio_seek(pb, start, SEEK_SET);
    c->current_chapter_size = 0;
    c->seek_offset = 0;
    return 0;
}

static int decrypt_and_read(AVIOContext *pb, int blocks, uint8_t *buf, uint8_t *dst, uint8_t *src, AADemuxContext *c, int written) {
    if (blocks == 0) return written;
    int ret = avio_read(pb, src, TEA_BLOCK_SIZE);
    if (ret != TEA_BLOCK_SIZE)
        return (ret < 0) ? ret : AVERROR_EOF;
    av_tea_init(c->tea_ctx, c->file_key, 16);
    av_tea_crypt(c->tea_ctx, dst, src, 1, NULL, 1);
    memcpy(buf + written, dst, TEA_BLOCK_SIZE);
    return decrypt_and_read(pb, blocks - 1, buf, dst, src, c, written + TEA_BLOCK_SIZE);
}

static int aa_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    uint8_t dst[TEA_BLOCK_SIZE], src[TEA_BLOCK_SIZE];
    AADemuxContext *c = s->priv_data;
    uint64_t pos = avio_tell(s->pb);
    if (pos >= c->content_end) {
        return AVERROR_EOF;
    }
    if (c->current_chapter_size == 0) {
        c->current_chapter_size = avio_rb32(s->pb);
        if (c->current_chapter_size == 0) {
            return AVERROR_EOF;
        }
        av_log(s, AV_LOG_DEBUG, "Chapter %d (%" PRId64 " bytes)\n", c->chapter_idx, c->current_chapter_size);
        c->chapter_idx++;
        avio_skip(s->pb, 4);
        pos += 8;
        c->current_codec_second_size = c->codec_second_size;
    }
    if (c->current_chapter_size / c->current_codec_second_size == 0) {
        c->current_codec_second_size = c->current_chapter_size % c->current_codec_second_size;
    }
    int blocks = c->current_codec_second_size / TEA_BLOCK_SIZE;
    uint8_t buf[MAX_CODEC_SECOND_SIZE * 2];
    int written = decrypt_and_read(s->pb, blocks, buf, dst, src, c, 0);
    int trailing_bytes = c->current_codec_second_size % TEA_BLOCK_SIZE;
    if (trailing_bytes != 0) {
        int ret = avio_read(s->pb, src, trailing_bytes);
        if (ret != trailing_bytes)
            return (ret < 0) ? ret : AVERROR_EOF;
        memcpy(buf + written, src, trailing_bytes);
        written += trailing_bytes;
    }
    c->current_chapter_size -= c->current_codec_second_size;
    if (c->current_chapter_size <= 0)
        c->current_chapter_size = 0;
    if (c->seek_offset > written)
        c->seek_offset = 0;
    int ret = av_new_packet(pkt, written - c->seek_offset);
    if (ret < 0)
        return ret;
    memcpy(pkt->data, buf + c->seek_offset, written - c->seek_offset);
    pkt->pos = pos;
    c->seek_offset = 0;
    return 0;
}

static int seek_chapter_idx(AVFormatContext *s, int64_t timestamp) {
    if (s->nb_chapters == 0 || timestamp < 0) return 0;
    if (timestamp >= s->chapters[s->nb_chapters - 1]->end) return s->nb_chapters - 1;
    return 1 + seek_chapter_idx(s, timestamp - s->chapters[0]->end);
}

static int aa_read_seek(AVFormatContext *s, int stream_index, int64_t timestamp, int flags) {
    AADemuxContext *c = s->priv_data;
    int chapter_idx = seek_chapter_idx(s, timestamp);
    if (chapter_idx >= s->nb_chapters) {
        chapter_idx = s->nb_chapters - 1;
        if (chapter_idx < 0) return -1;
        timestamp = s->chapters[chapter_idx]->end;
    }
    AVChapter *ch = s->chapters[chapter_idx];
    int64_t chapter_size = ch->end / TIMEPREC - ch->start / TIMEPREC;
    int64_t chapter_pos = av_rescale_rnd((timestamp - ch->start) / TIMEPREC, 1, c->codec_second_size, (flags & AVSEEK_FLAG_BACKWARD) ? AV_ROUND_DOWN : AV_ROUND_UP) * c->codec_second_size;
    if (chapter_pos >= chapter_size) chapter_pos = chapter_size;
    int64_t chapter_start = c->content_start + (ch->start / TIMEPREC) + CHAPTER_HEADER_SIZE * (1 + chapter_idx);
    avio_seek(s->pb, chapter_start + chapter_pos, SEEK_SET);
    c->current_codec_second_size = c->codec_second_size;
    c->current_chapter_size = chapter_size - chapter_pos;
    c->chapter_idx = 1 + chapter_idx;
    if (s->streams[0]->codecpar->codec_id == AV_CODEC_ID_MP3) {
        c->seek_offset = (MP3_FRAME_SIZE - chapter_pos % MP3_FRAME_SIZE) % MP3_FRAME_SIZE;
    }
    ff_update_cur_dts(s, s->streams[0], ch->start + (chapter_pos + c->seek_offset) * TIMEPREC);
    return 1;
}

static int aa_probe(const AVProbeData *p)
{
    uint8_t *buf = p->buf;
    if (AV_RB32(buf+4) != AA_MAGIC)
        return 0;

    return AVPROBE_SCORE_MAX / 2;
}

static int aa_read_close(AVFormatContext *s)
{
    AADemuxContext *c = s->priv_data;
    av_freep(&c->tea_ctx);
    return 0;
}

#define OFFSET(x) offsetof(AADemuxContext, x)
static const AVOption aa_options[] = {
    { "aa_fixed_key", "Fixed key used for handling Audible AA files", OFFSET(aa_fixed_key), AV_OPT_TYPE_BINARY, {.str="77214d4b196a87cd520045fd2a51d673"}, .flags = AV_OPT_FLAG_DECODING_PARAM },
    { NULL },
};

static const AVClass aa_class = {
    .class_name = "aa",
    .item_name  = av_default_item_name,
    .option     = aa_options,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ff_aa_demuxer = {
    .name           = "aa",
    .long_name      = NULL_IF_CONFIG_SMALL("Audible AA format files"),
    .priv_class     = &aa_class,
    .priv_data_size = sizeof(AADemuxContext),
    .extensions     = "aa",
    .read_probe     = aa_probe,
    .read_header    = aa_read_header,
    .read_packet    = aa_read_packet,
    .read_seek      = aa_read_seek,
    .read_close     = aa_read_close,
    .flags          = AVFMT_NO_BYTE_SEEK | AVFMT_NOGENSEARCH,
};