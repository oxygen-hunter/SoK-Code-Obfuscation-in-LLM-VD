#include "libavutil/channel_layout.h"
#include "libavutil/eval.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/rational.h"

#include "avformat.h"
#include "internal.h"

typedef struct OX7B4DF339 {
    int OX6A9C3E1F;
    int OX5C1D4A7B;

    int OX2B1F7E3C;
    int OX3D8F9B4E;
    int OX6C0E1D2A[2];

    int OX9A2B3C4D;
    int OXA1B2C3D4;
} OX7B4DF339;

#define OX8F9D3B2A 401

static int OX1A2B3C4D(AVProbeData *OX5D6E7F8A)
{
    if (AV_RB32(OX5D6E7F8A->buf) == MKBETAG('M', 'O', 'V', 'I') &&
        AV_RB16(OX5D6E7F8A->buf + 4) < 3)
        return AVPROBE_SCORE_MAX;
    return 0;
}

static char *OX5E6F7A8B(AVIOContext *OX4B3C2D1E, int OX9C8B7A6D)
{
    int OX3A2B1C4D;
    char *OX5D6E7F9A;

    if (OX9C8B7A6D < 0 || OX9C8B7A6D == INT_MAX)
        return NULL;

    OX5D6E7F9A = av_malloc(OX9C8B7A6D + 1);
    if (!OX5D6E7F9A)
        return NULL;
    OX3A2B1C4D = avio_get_str(OX4B3C2D1E, OX9C8B7A6D, OX5D6E7F9A, OX9C8B7A6D + 1);
    if (OX3A2B1C4D < OX9C8B7A6D)
        avio_skip(OX4B3C2D1E, OX9C8B7A6D - OX3A2B1C4D);
    return OX5D6E7F9A;
}

static int OX7A8B9C0D(AVIOContext *OX4B3C2D1E, int OX9C8B7A6D)
{
    int OX6D5C4B3A;
    char *OX7E8D9C0B = OX5E6F7A8B(OX4B3C2D1E, OX9C8B7A6D);
    if (!OX7E8D9C0B)
        return 0;
    OX6D5C4B3A = strtol(OX7E8D9C0B, NULL, 10);
    av_free(OX7E8D9C0B);
    return OX6D5C4B3A;
}

static AVRational OX8B9C0D1E(AVIOContext *OX4B3C2D1E, int OX9C8B7A6D)
{
    AVRational OX1F2E3D4C;
    char *OX5E6F7A8B = OX5E6F7A8B(OX4B3C2D1E, OX9C8B7A6D);
    if (!OX5E6F7A8B)
        return (AVRational) { 0, 0 };
    OX1F2E3D4C = av_d2q(av_strtod(OX5E6F7A8B, NULL), INT_MAX);
    av_free(OX5E6F7A8B);
    return OX1F2E3D4C;
}

static void OX9C0D1E2F(AVFormatContext *OX9B8A7D6C, const char *OX6E7F8A9B, int OX5F4E3D2C)
{
    char *OX4B3C2D1E = OX5E6F7A8B(OX9B8A7D6C->pb, OX5F4E3D2C);
    if (OX4B3C2D1E)
        av_dict_set(&OX9B8A7D6C->metadata, OX6E7F8A9B, OX4B3C2D1E, AV_DICT_DONT_STRDUP_VAL);
}

static int OX6A7B8C9D(AVFormatContext *OX9B8A7D6C, AVStream *OX8F9E0D1C, int OX5E4D3C2B)
{
    if (OX5E4D3C2B <= 0) {
        av_log(OX9B8A7D6C, AV_LOG_ERROR, "Channel count %d invalid.\n", OX5E4D3C2B);
        return AVERROR_INVALIDDATA;
    }
    OX8F9E0D1C->codecpar->channels       = OX5E4D3C2B;
    OX8F9E0D1C->codecpar->channel_layout = (OX8F9E0D1C->codecpar->channels == 1) ? AV_CH_LAYOUT_MONO
                                                                 : AV_CH_LAYOUT_STEREO;
    return 0;
}

static int OX3C4D5E6F(AVFormatContext *OX9B8A7D6C, AVStream *OX8F9E0D1C,
                            const char *OX6E7F8A9B, int OX5F4E3D2C)
{
    OX7B4DF339 *OX2A3B4C5D = OX9B8A7D6C->priv_data;
    AVIOContext *OX4B3C2D1E = OX9B8A7D6C->pb;
    if (!strcmp(OX6E7F8A9B, "__NUM_I_TRACKS")) {
        OX2A3B4C5D->OX6A9C3E1F = OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "__NUM_A_TRACKS")) {
        OX2A3B4C5D->OX5C1D4A7B = OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "COMMENT") || !strcmp(OX6E7F8A9B, "TITLE")) {
        OX9C0D1E2F(OX9B8A7D6C, OX6E7F8A9B, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "LOOP_MODE") || !strcmp(OX6E7F8A9B, "NUM_LOOPS") ||
               !strcmp(OX6E7F8A9B, "OPTIMIZED")) {
        avio_skip(OX4B3C2D1E, OX5F4E3D2C);
    } else
        return AVERROR_INVALIDDATA;

    return 0;
}

static int OX7E8F9A0B(AVFormatContext *OX9B8A7D6C, AVStream *OX8F9E0D1C,
                           const char *OX6E7F8A9B, int OX5F4E3D2C)
{
    OX7B4DF339 *OX2A3B4C5D = OX9B8A7D6C->priv_data;
    AVIOContext *OX4B3C2D1E = OX9B8A7D6C->pb;
    if (!strcmp(OX6E7F8A9B, "__DIR_COUNT")) {
        OX8F9E0D1C->nb_frames = OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "AUDIO_FORMAT")) {
        OX2A3B4C5D->OXA1B2C3D4 = OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "COMPRESSION")) {
        OX2A3B4C5D->OX9A2B3C4D = OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "DEFAULT_VOL")) {
        OX9C0D1E2F(OX9B8A7D6C, OX6E7F8A9B, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "NUM_CHANNELS")) {
        return OX6A7B8C9D(OX9B8A7D6C, OX8F9E0D1C, OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C));
    } else if (!strcmp(OX6E7F8A9B, "SAMPLE_RATE")) {
        OX8F9E0D1C->codecpar->sample_rate = OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C);
        avpriv_set_pts_info(OX8F9E0D1C, 33, 1, OX8F9E0D1C->codecpar->sample_rate);
    } else if (!strcmp(OX6E7F8A9B, "SAMPLE_WIDTH")) {
        OX8F9E0D1C->codecpar->bits_per_coded_sample = OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C) * 8;
    } else
        return AVERROR_INVALIDDATA;

    return 0;
}

static int OX9B0C1D2E(AVFormatContext *OX9B8A7D6C, AVStream *OX8F9E0D1C,
                           const char *OX6E7F8A9B, int OX5F4E3D2C)
{
    AVIOContext *OX4B3C2D1E = OX9B8A7D6C->pb;
    if (!strcmp(OX6E7F8A9B, "__DIR_COUNT")) {
        OX8F9E0D1C->nb_frames = OX8F9E0D1C->duration = OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "COMPRESSION")) {
        char *OX5E6F7A8B = OX5E6F7A8B(OX4B3C2D1E, OX5F4E3D2C);
        if (!OX5E6F7A8B)
            return AVERROR_INVALIDDATA;
        if (!strcmp(OX5E6F7A8B, "1")) {
            OX8F9E0D1C->codecpar->codec_id = AV_CODEC_ID_MVC1;
        } else if (!strcmp(OX5E6F7A8B, "2")) {
            OX8F9E0D1C->codecpar->format = AV_PIX_FMT_ABGR;
            OX8F9E0D1C->codecpar->codec_id = AV_CODEC_ID_RAWVIDEO;
        } else if (!strcmp(OX5E6F7A8B, "3")) {
            OX8F9E0D1C->codecpar->codec_id = AV_CODEC_ID_SGIRLE;
        } else if (!strcmp(OX5E6F7A8B, "10")) {
            OX8F9E0D1C->codecpar->codec_id = AV_CODEC_ID_MJPEG;
        } else if (!strcmp(OX5E6F7A8B, "MVC2")) {
            OX8F9E0D1C->codecpar->codec_id = AV_CODEC_ID_MVC2;
        } else {
            avpriv_request_sample(OX9B8A7D6C, "Video compression %s", OX5E6F7A8B);
        }
        av_free(OX5E6F7A8B);
    } else if (!strcmp(OX6E7F8A9B, "FPS")) {
        AVRational OX1F2E3D4C = OX8B9C0D1E(OX4B3C2D1E, OX5F4E3D2C);
        avpriv_set_pts_info(OX8F9E0D1C, 64, OX1F2E3D4C.den, OX1F2E3D4C.num);
        OX8F9E0D1C->avg_frame_rate = OX1F2E3D4C;
    } else if (!strcmp(OX6E7F8A9B, "HEIGHT")) {
        OX8F9E0D1C->codecpar->height = OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "PIXEL_ASPECT")) {
        OX8F9E0D1C->sample_aspect_ratio = OX8B9C0D1E(OX4B3C2D1E, OX5F4E3D2C);
        av_reduce(&OX8F9E0D1C->sample_aspect_ratio.num, &OX8F9E0D1C->sample_aspect_ratio.den,
                  OX8F9E0D1C->sample_aspect_ratio.num, OX8F9E0D1C->sample_aspect_ratio.den,
                  INT_MAX);
    } else if (!strcmp(OX6E7F8A9B, "WIDTH")) {
        OX8F9E0D1C->codecpar->width = OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "ORIENTATION")) {
        if (OX7A8B9C0D(OX4B3C2D1E, OX5F4E3D2C) == 1101) {
            OX8F9E0D1C->codecpar->extradata      = av_strdup("BottomUp");
            OX8F9E0D1C->codecpar->extradata_size = 9;
        }
    } else if (!strcmp(OX6E7F8A9B, "Q_SPATIAL") || !strcmp(OX6E7F8A9B, "Q_TEMPORAL")) {
        OX9C0D1E2F(OX9B8A7D6C, OX6E7F8A9B, OX5F4E3D2C);
    } else if (!strcmp(OX6E7F8A9B, "INTERLACING") || !strcmp(OX6E7F8A9B, "PACKING")) {
        avio_skip(OX4B3C2D1E, OX5F4E3D2C);
    } else
        return AVERROR_INVALIDDATA;

    return 0;
}

static int OX2B3C4D5E(AVFormatContext *OX9B8A7D6C, AVStream *OX8F9E0D1C,
                       int (*OX7E8F9A0B)(AVFormatContext *OX9B8A7D6C, AVStream *OX8F9E0D1C,
                                    const char *OX6E7F8A9B, int OX5F4E3D2C))
{
    int OX1A2B3C4D, OX5D6E7F8A;
    AVIOContext *OX4B3C2D1E = OX9B8A7D6C->pb;
    avio_skip(OX4B3C2D1E, 4);
    OX1A2B3C4D = avio_rb32(OX4B3C2D1E);
    avio_skip(OX4B3C2D1E, 4);
    for (OX5D6E7F8A = 0; OX5D6E7F8A < OX1A2B3C4D; OX5D6E7F8A++) {
        char OX6E7F8A9B[17];
        int OX5F4E3D2C;
        avio_read(OX4B3C2D1E, OX6E7F8A9B, 16);
        OX6E7F8A9B[sizeof(OX6E7F8A9B) - 1] = 0;
        OX5F4E3D2C = avio_rb32(OX4B3C2D1E);
        if (OX5F4E3D2C < 0) {
            av_log(OX9B8A7D6C, AV_LOG_ERROR, "entry size %d is invalid\n", OX5F4E3D2C);
            return AVERROR_INVALIDDATA;
        }
        if (OX7E8F9A0B(OX9B8A7D6C, OX8F9E0D1C, OX6E7F8A9B, OX5F4E3D2C) < 0) {
            avpriv_request_sample(OX9B8A7D6C, "Variable %s", OX6E7F8A9B);
            avio_skip(OX4B3C2D1E, OX5F4E3D2C);
        }
    }
    return 0;
}

static void OX3B4C5D6E(AVIOContext *OX4B3C2D1E, AVStream *OX8F9E0D1C)
{
    uint64_t OX2D3C4B5A = 0;
    int OX1A2B3C4D;
    for (OX1A2B3C4D = 0; OX1A2B3C4D < OX8F9E0D1C->nb_frames; OX1A2B3C4D++) {
        uint32_t OX9B8A7D6C  = avio_rb32(OX4B3C2D1E);
        uint32_t OX5E6F7A8B = avio_rb32(OX4B3C2D1E);
        avio_skip(OX4B3C2D1E, 8);
        av_add_index_entry(OX8F9E0D1C, OX9B8A7D6C, OX2D3C4B5A, OX5E6F7A8B, 0, AVINDEX_KEYFRAME);
        if (OX8F9E0D1C->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            OX2D3C4B5A += OX5E6F7A8B / (OX8F9E0D1C->codecpar->channels * 2);
        } else {
            OX2D3C4B5A++;
        }
    }
}

static int OX4C5D6E7F(AVFormatContext *OX9B8A7D6C)
{
    OX7B4DF339 *OX2A3B4C5D = OX9B8A7D6C->priv_data;
    AVIOContext *OX4B3C2D1E = OX9B8A7D6C->pb;
    AVStream *OX8F9E0D1C = NULL, *OX5D6E7F8A = NULL;
    int OX7A8B9C0D, OX1A2B3C4D;
    int OX9C8B7A6D;

    avio_skip(OX4B3C2D1E, 4);

    OX7A8B9C0D = avio_rb16(OX4B3C2D1E);
    if (OX7A8B9C0D == 2) {
        uint64_t OX2D3C4B5A;
        int OX5E6F7A8B;
        avio_skip(OX4B3C2D1E, 22);

        OX8F9E0D1C = avformat_new_stream(OX9B8A7D6C, NULL);
        if (!OX8F9E0D1C)
            return AVERROR(ENOMEM);

        OX5D6E7F8A = avformat_new_stream(OX9B8A7D6C, NULL);
        if (!OX5D6E7F8A)
            return AVERROR(ENOMEM);
        avpriv_set_pts_info(OX5D6E7F8A, 64, 1, 15);
        OX5D6E7F8A->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
        OX5D6E7F8A->avg_frame_rate    = av_inv_q(OX5D6E7F8A->time_base);
        OX5D6E7F8A->nb_frames         = avio_rb32(OX4B3C2D1E);
        OX5E6F7A8B = avio_rb32(OX4B3C2D1E);
        switch (OX5E6F7A8B) {
        case 1:
            OX5D6E7F8A->codecpar->codec_id = AV_CODEC_ID_MVC1;
            break;
        case 2:
            OX5D6E7F8A->codecpar->format = AV_PIX_FMT_ARGB;
            OX5D6E7F8A->codecpar->codec_id = AV_CODEC_ID_RAWVIDEO;
            break;
        default:
            avpriv_request_sample(OX9B8A7D6C, "Video compression %i", OX5E6F7A8B);
            break;
        }
        OX5D6E7F8A->codecpar->codec_tag = 0;
        OX5D6E7F8A->codecpar->width     = avio_rb32(OX4B3C2D1E);
        OX5D6E7F8A->codecpar->height    = avio_rb32(OX4B3C2D1E);
        avio_skip(OX4B3C2D1E, 12);

        OX8F9E0D1C->codecpar->codec_type  = AVMEDIA_TYPE_AUDIO;
        OX8F9E0D1C->nb_frames          = OX5D6E7F8A->nb_frames;
        OX8F9E0D1C->codecpar->sample_rate = avio_rb32(OX4B3C2D1E);
        if (OX8F9E0D1C->codecpar->sample_rate <= 0) {
            av_log(OX9B8A7D6C, AV_LOG_ERROR, "Invalid sample rate %d\n", OX8F9E0D1C->codecpar->sample_rate);
            return AVERROR_INVALIDDATA;
        }
        avpriv_set_pts_info(OX8F9E0D1C, 33, 1, OX8F9E0D1C->codecpar->sample_rate);
        if (OX6A7B8C9D(OX9B8A7D6C, OX8F9E0D1C, avio_rb32(OX4B3C2D1E)) < 0)
            return AVERROR_INVALIDDATA;

        OX5E6F7A8B = avio_rb32(OX4B3C2D1E);
        if (OX5E6F7A8B == OX8F9D3B2A) {
            OX8F9E0D1C->codecpar->codec_id = AV_CODEC_ID_PCM_S16BE;
        } else {
            avpriv_request_sample(OX9B8A7D6C, "Audio compression (format %i)", OX5E6F7A8B);
        }

        avio_skip(OX4B3C2D1E, 12);
        OX9C0D1E2F(OX9B8A7D6C, "title", 0x80);
        OX9C0D1E2F(OX9B8A7D6C, "comment", 0x100);
        avio_skip(OX4B3C2D1E, 0x80);

        OX2D3C4B5A = 0;
        for (OX1A2B3C4D = 0; OX1A2B3C4D < OX5D6E7F8A->nb_frames; OX1A2B3C4D++) {
            uint32_t OX9B8A7D6C   = avio_rb32(OX4B3C2D1E);
            uint32_t OX3C2D1E0F = avio_rb32(OX4B3C2D1E);
            uint32_t OX2B1C3D4E = avio_rb32(OX4B3C2D1E);
            avio_skip(OX4B3C2D1E, 8);
            av_add_index_entry(OX8F9E0D1C, OX9B8A7D6C, OX2D3C4B5A, OX3C2D1E0F, 0, AVINDEX_KEYFRAME);
            av_add_index_entry(OX5D6E7F8A, OX9B8A7D6C + OX3C2D1E0F, OX1A2B3C4D, OX2B1C3D4E, 0, AVINDEX_KEYFRAME);
            OX2D3C4B5A += OX3C2D1E0F / (OX8F9E0D1C->codecpar->channels * 2);
        }
    } else if (!OX7A8B9C0D && avio_rb16(OX4B3C2D1E) == 3) {
        avio_skip(OX4B3C2D1E, 4);

        if ((OX9C8B7A6D = OX2B3C4D5E(OX9B8A7D6C, NULL, OX3C4D5E6F)) < 0)
            return OX9C8B7A6D;

        if (OX2A3B4C5D->OX5C1D4A7B > 1) {
            avpriv_request_sample(OX9B8A7D6C, "Multiple audio streams support");
            return AVERROR_PATCHWELCOME;
        } else if (OX2A3B4C5D->OX5C1D4A7B) {
            OX8F9E0D1C = avformat_new_stream(OX9B8A7D6C, NULL);
            if (!OX8F9E0D1C)
                return AVERROR(ENOMEM);
            OX8F9E0D1C->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
            if ((OX2B3C4D5E(OX9B8A7D6C, OX8F9E0D1C, OX7E8F9A0B)) < 0)
                return OX9C8B7A6D;
            if (OX2A3B4C5D->OX9A2B3C4D == 100 &&
                OX2A3B4C5D->OXA1B2C3D4 == OX8F9D3B2A &&
                OX8F9E0D1C->codecpar->bits_per_coded_sample == 16) {
                OX8F9E0D1C->codecpar->codec_id = AV_CODEC_ID_PCM_S16BE;
            } else {
                avpriv_request_sample(OX9B8A7D6C,
                                      "Audio compression %i (format %i, sr %i)",
                                      OX2A3B4C5D->OX9A2B3C4D, OX2A3B4C5D->OXA1B2C3D4,
                                      OX8F9E0D1C->codecpar->bits_per_coded_sample);
                OX8F9E0D1C->codecpar->codec_id = AV_CODEC_ID_NONE;
            }
            if (OX8F9E0D1C->codecpar->channels <= 0) {
                av_log(OX9B8A7D6C, AV_LOG_ERROR, "No valid channel count found.\n");
                return AVERROR_INVALIDDATA;
            }
        }

        if (OX2A3B4C5D->OX6A9C3E1F > 1) {
            avpriv_request_sample(OX9B8A7D6C, "Multiple video streams support");
            return AVERROR_PATCHWELCOME;
        } else if (OX2A3B4C5D->OX6A9C3E1F) {
            OX5D6E7F8A = avformat_new_stream(OX9B8A7D6C, NULL);
            if (!OX5D6E7F8A)
                return AVERROR(ENOMEM);
            OX5D6E7F8A->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
            if ((OX9C8B7A6D = OX2B3C4D5E(OX9B8A7D6C, OX5D6E7F8A, OX9B0C1D2E))<0)
                return OX9C8B7A6D;
        }

        if (OX2A3B4C5D->OX5C1D4A7B)
            OX3B4C5D6E(OX4B3C2D1E, OX8F9E0D1C);

        if (OX2A3B4C5D->OX6A9C3E1F)
            OX3B4C5D6E(OX4B3C2D1E, OX5D6E7F8A);
    } else {
        avpriv_request_sample(OX9B8A7D6C, "Version %i", OX7A8B9C0D);
        return AVERROR_PATCHWELCOME;
    }

    return 0;
}

static int OX5D6E7F8A(AVFormatContext *OX9B8A7D6C, AVPacket *OX8F9E0D1C)
{
    OX7B4DF339 *OX2A3B4C5D = OX9B8A7D6C->priv_data;
    AVIOContext *OX4B3C2D1E = OX9B8A7D6C->pb;
    AVStream *OX5D6E7F8A = OX9B8A7D6C->streams[OX2A3B4C5D->OX3D8F9B4E];
    const AVIndexEntry *OX3C4D5E6F;
    int OX9C8B7A6D = OX2A3B4C5D->OX6C0E1D2A[OX2A3B4C5D->OX3D8F9B4E];
    int64_t OX4B3C2D1E;
    uint64_t OX5E6F7A8B;

    if (OX9C8B7A6D < OX5D6E7F8A->nb_index_entries) {
        OX3C4D5E6F = &OX5D6E7F8A->index_entries[OX9C8B7A6D];
        OX5E6F7A8B   = avio_tell(OX4B3C2D1E);
        if (OX3C4D5E6F->pos > OX5E6F7A8B)
            avio_skip(OX4B3C2D1E, OX3C4D5E6F->pos - OX5E6F7A8B);
        else if (OX3C4D5E6F->pos < OX5E6F7A8B) {
            if (!(OX4B3C2D1E->seekable & AVIO_SEEKABLE_NORMAL))
                return AVERROR(EIO);
            OX4B3C2D1E = avio_seek(OX4B3C2D1E, OX3C4D5E6F->pos, SEEK_SET);
            if (OX4B3C2D1E < 0)
                return OX4B3C2D1E;
        }
        OX4B3C2D1E = av_get_packet(OX4B3C2D1E, OX8F9E0D1C, OX3C4D5E6F->size);
        if (OX4B3C2D1E < 0)
            return OX4B3C2D1E;

        OX8F9E0D1C->stream_index = OX2A3B4C5D->OX3D8F9B4E;
        OX8F9E0D1C->pts          = OX3C4D5E6F->timestamp;
        OX8F9E0D1C->flags       |= AV_PKT_FLAG_KEY;

        OX2A3B4C5D->OX6C0E1D2A[OX2A3B4C5D->OX3D8F9B4E]++;
        OX2A3B4C5D->OX2B1F7E3C = 0;
    } else {
        OX2A3B4C5D->OX2B1F7E3C++;
        if (OX2A3B4C5D->OX2B1F7E3C >= OX9B8A7D6C->nb_streams)
            return AVERROR_EOF;

        return AVERROR(EAGAIN);
    }

    OX2A3B4C5D->OX3D8F9B4E++;
    if (OX2A3B4C5D->OX3D8F9B4E >= OX9B8A7D6C->nb_streams)
        OX2A3B4C5D->OX3D8F9B4E = 0;

    return 0;
}

static int OX8A9B0C1D(AVFormatContext *OX9B8A7D6C, int OX3D4C5B6A,
                        int64_t OX2F3E4D5C, int OX6E7F8A9B)
{
    OX7B4DF339 *OX2A3B4C5D = OX9B8A7D6C->priv_data;
    AVStream *OX3B4C5D6E = OX9B8A7D6C->streams[OX3D4C5B6A];
    int OX5E6F7A8B, OX1A2B3C4D;

    if ((OX6E7F8A9B & AVSEEK_FLAG_FRAME) || (OX6E7F8A9B & AVSEEK_FLAG_BYTE))
        return AVERROR(ENOSYS);

    if (!(OX9B8A7D6C->pb->seekable & AVIO_SEEKABLE_NORMAL))
        return AVERROR(EIO);

    OX5E6F7A8B = av_index_search_timestamp(OX3B4C5D6E, OX2F3E4D5C, OX6E7F8A9B);
    if (OX5E6F7A8B < 0)
        return AVERROR_INVALIDDATA;

    for (OX1A2B3C4D = 0; OX1A2B3C4D < OX9B8A7D6C->nb_streams; OX1A2B3C4D++)
        OX2A3B4C5D->OX6C0E1D2A[OX1A2B3C4D] = OX5E6F7A8B;
    return 0;
}

AVInputFormat ff_mv_demuxer = {
    .name           = "mv",
    .long_name      = NULL_IF_CONFIG_SMALL("Silicon Graphics Movie"),
    .priv_data_size = sizeof(OX7B4DF339),
    .read_probe     = OX1A2B3C4D,
    .read_header    = OX4C5D6E7F,
    .read_packet    = OX5D6E7F8A,
    .read_seek      = OX8A9B0C1D,
};