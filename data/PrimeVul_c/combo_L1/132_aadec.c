#include "avformat.h"
#include "internal.h"
#include "libavutil/dict.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/tea.h"
#include "libavutil/opt.h"

#define OX7B4DF339 1469084982
#define OX5C1BDB44 3982
#define OX9A4D5567 16
#define OX2F8C2A7B 128
#define OX832E21B5 8
#define OX6EA3A1F8 8
#define OX0C7E4B95 1000
#define OX1F4A2C3B 104

typedef struct OX6F2A3B4E {
    AVClass *OX6E4A2B1C;
    uint8_t *OX4E1B9A2F;
    int OX6B2E7F8C;
    int OX5A3B2C4D;
    int OX7C4A5D6E;
    int OX8F2D3B7C;
    struct AVTEA *OX3A2C1E5D;
    uint8_t OX5F1C8A3B[16];
    int64_t OX9D3B2A7F;
    int64_t OX7E4C2B8A;
    int64_t OX4A2F7E1C;
    int OX6B3A2E9F;
} OX6F2A3B4E;

static int OX2D3A9C7E(char *OX3B2C4A6D)
{
    int OX4E2F3B5D = -1;

    if (!strcmp(OX3B2C4A6D, "mp332")) {
        OX4E2F3B5D = 3982;
    } else if (!strcmp(OX3B2C4A6D, "acelp16")) {
        OX4E2F3B5D = 2000;
    } else if (!strcmp(OX3B2C4A6D, "acelp85")) {
        OX4E2F3B5D = 1045;
    }

    return OX4E2F3B5D;
}

static int OX5E3A9C7B(AVFormatContext *OX5C9F3B7D)
{
    int OX7A2B5C9E, OX6B9C4D3F, OX4E2B7D9C, OX7F9C2B4D = -1;
    uint32_t OX9D3B2A4E, OX5C1A8B9F, OX4C7E3B2A, OX5F2A3B8C, OX2A7F5C9E = 0, OX8B4E1C5F;
    char OX5D9C2A3B[128], OX7F4A2D6E[128], OX2B4C7E8A[64] = {0};
    uint8_t OX3A5D6E9B[24], OX7F9C2D8A[8], OX4B2C7E5F[8];
    int64_t OX6E4A7D3C = -1, OX5B9C4A8D = -1, OX7C5D8A3B;
    struct OX1A2B3C4D {
        uint32_t OX5E1C7A9F;
        uint32_t OX4A2B3C6D;
    } OX2F8C7B3A[OX9A4D5567];
    uint32_t OX5E2B3C4A[4];
    uint8_t OX7F5D9C2A[16] = {0};
    OX6F2A3B4E *OX6E1C4A9F = OX5C9F3B7D->priv_data;
    AVIOContext *OX9D2A3B5C = OX5C9F3B7D->pb;
    AVStream *OX8E3D5A7C;

    avio_skip(OX9D2A3B5C, 4);
    avio_skip(OX9D2A3B5C, 4);
    OX4C7E3B2A = avio_rb32(OX9D2A3B5C);
    avio_skip(OX9D2A3B5C, 4);
    if (OX4C7E3B2A > OX9A4D5567)
        return AVERROR_INVALIDDATA;
    for (OX7A2B5C9E = 0; OX7A2B5C9E < OX4C7E3B2A; OX7A2B5C9E++) {
        avio_skip(OX9D2A3B5C, 4);
        OX2F8C7B3A[OX7A2B5C9E].OX5E1C7A9F = avio_rb32(OX9D2A3B5C);
        OX2F8C7B3A[OX7A2B5C9E].OX4A2B3C6D = avio_rb32(OX9D2A3B5C);
    }
    avio_skip(OX9D2A3B5C, 24);
    OX5F2A3B8C = avio_rb32(OX9D2A3B5C);
    if (OX5F2A3B8C > OX2F8C2A7B)
        return AVERROR_INVALIDDATA;
    for (OX7A2B5C9E = 0; OX7A2B5C9E < OX5F2A3B8C; OX7A2B5C9E++) {
        memset(OX7F4A2D6E, 0, sizeof(OX7F4A2D6E));
        memset(OX5D9C2A3B, 0, sizeof(OX5D9C2A3B));
        avio_skip(OX9D2A3B5C, 1);
        OX9D3B2A4E = avio_rb32(OX9D2A3B5C);
        OX5C1A8B9F = avio_rb32(OX9D2A3B5C);
        avio_get_str(OX9D2A3B5C, OX9D3B2A4E, OX5D9C2A3B, sizeof(OX5D9C2A3B));
        avio_get_str(OX9D2A3B5C, OX5C1A8B9F, OX7F4A2D6E, sizeof(OX7F4A2D6E));
        if (!strcmp(OX5D9C2A3B, "codec")) {
            av_log(OX5C9F3B7D, AV_LOG_DEBUG, "Codec is <%s>\n", OX7F4A2D6E);
            strncpy(OX2B4C7E8A, OX7F4A2D6E, sizeof(OX2B4C7E8A) - 1);
        } else if (!strcmp(OX5D9C2A3B, "HeaderSeed")) {
            av_log(OX5C9F3B7D, AV_LOG_DEBUG, "HeaderSeed is <%s>\n", OX7F4A2D6E);
            OX2A7F5C9E = atoi(OX7F4A2D6E);
        } else if (!strcmp(OX5D9C2A3B, "HeaderKey")) {
            av_log(OX5C9F3B7D, AV_LOG_DEBUG, "HeaderKey is <%s>\n", OX7F4A2D6E);
            sscanf(OX7F4A2D6E, "%"SCNu32"%"SCNu32"%"SCNu32"%"SCNu32,
                   &OX5E2B3C4A[0], &OX5E2B3C4A[1], &OX5E2B3C4A[2], &OX5E2B3C4A[3]);
            for (OX4E2B7D9C = 0; OX4E2B7D9C < 4; OX4E2B7D9C++) {
                AV_WB32(&OX7F5D9C2A[OX4E2B7D9C * 4], OX5E2B3C4A[OX4E2B7D9C]);
            }
            av_log(OX5C9F3B7D, AV_LOG_DEBUG, "Processed HeaderKey is ");
            for (OX7A2B5C9E = 0; OX7A2B5C9E < 16; OX7A2B5C9E++)
                av_log(OX5C9F3B7D, AV_LOG_DEBUG, "%02x", OX7F5D9C2A[OX7A2B5C9E]);
            av_log(OX5C9F3B7D, AV_LOG_DEBUG, "\n");
        } else {
            av_dict_set(&OX5C9F3B7D->metadata, OX5D9C2A3B, OX7F4A2D6E, 0);
        }
    }

    if (OX6E1C4A9F->OX6B2E7F8C != 16) {
        av_log(OX5C9F3B7D, AV_LOG_ERROR, "aa_fixed_key value needs to be 16 bytes!\n");
        return AVERROR(EINVAL);
    }

    if ((OX6E1C4A9F->OX5A3B2C4D = OX2D3A9C7E(OX2B4C7E8A)) == -1) {
        av_log(OX5C9F3B7D, AV_LOG_ERROR, "unknown codec <%s>!\n", OX2B4C7E8A);
        return AVERROR(EINVAL);
    }

    OX6E1C4A9F->OX3A2C1E5D = av_tea_alloc();
    if (!OX6E1C4A9F->OX3A2C1E5D)
        return AVERROR(ENOMEM);
    av_tea_init(OX6E1C4A9F->OX3A2C1E5D, OX6E1C4A9F->OX4E1B9A2F, 16);
    OX3A5D6E9B[0] = OX3A5D6E9B[1] = 0;
    memcpy(OX3A5D6E9B + 2, OX7F5D9C2A, 16);
    OX4E2B7D9C = 0;
    for (OX7A2B5C9E = 0; OX7A2B5C9E < 3; OX7A2B5C9E++) {
        AV_WB32(OX4B2C7E5F, OX2A7F5C9E);
        AV_WB32(OX4B2C7E5F + 4, OX2A7F5C9E + 1);
        OX2A7F5C9E += 2;
        av_tea_crypt(OX6E1C4A9F->OX3A2C1E5D, OX7F9C2D8A, OX4B2C7E5F, 1, NULL, 0);
        for (OX6B9C4D3F = 0; OX6B9C4D3F < OX832E21B5 && OX4E2B7D9C < 18; OX6B9C4D3F+=1, OX4E2B7D9C+=1) {
            OX3A5D6E9B[OX4E2B7D9C] = OX3A5D6E9B[OX4E2B7D9C] ^ OX7F9C2D8A[OX6B9C4D3F];
        }
    }
    memcpy(OX6E1C4A9F->OX5F1C8A3B, OX3A5D6E9B + 2, 16);
    av_log(OX5C9F3B7D, AV_LOG_DEBUG, "File key is ");
    for (OX7A2B5C9E = 0; OX7A2B5C9E < 16; OX7A2B5C9E++)
        av_log(OX5C9F3B7D, AV_LOG_DEBUG, "%02x", OX6E1C4A9F->OX5F1C8A3B[OX7A2B5C9E]);
    av_log(OX5C9F3B7D, AV_LOG_DEBUG, "\n");

    OX8E3D5A7C = avformat_new_stream(OX5C9F3B7D, NULL);
    if (!OX8E3D5A7C) {
        av_freep(&OX6E1C4A9F->OX3A2C1E5D);
        return AVERROR(ENOMEM);
    }
    OX8E3D5A7C->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    if (!strcmp(OX2B4C7E8A, "mp332")) {
        OX8E3D5A7C->codecpar->codec_id = AV_CODEC_ID_MP3;
        OX8E3D5A7C->codecpar->sample_rate = 22050;
        OX8E3D5A7C->need_parsing = AVSTREAM_PARSE_FULL_RAW;
        avpriv_set_pts_info(OX8E3D5A7C, 64, 8, 32000 * OX0C7E4B95);
    } else if (!strcmp(OX2B4C7E8A, "acelp85")) {
        OX8E3D5A7C->codecpar->codec_id = AV_CODEC_ID_SIPR;
        OX8E3D5A7C->codecpar->block_align = 19;
        OX8E3D5A7C->codecpar->channels = 1;
        OX8E3D5A7C->codecpar->sample_rate = 8500;
        OX8E3D5A7C->codecpar->bit_rate = 8500;
        OX8E3D5A7C->need_parsing = AVSTREAM_PARSE_FULL_RAW;
        avpriv_set_pts_info(OX8E3D5A7C, 64, 8, 8500 * OX0C7E4B95);
    } else if (!strcmp(OX2B4C7E8A, "acelp16")) {
        OX8E3D5A7C->codecpar->codec_id = AV_CODEC_ID_SIPR;
        OX8E3D5A7C->codecpar->block_align = 20;
        OX8E3D5A7C->codecpar->channels = 1;
        OX8E3D5A7C->codecpar->sample_rate = 16000;
        OX8E3D5A7C->codecpar->bit_rate = 16000;
        OX8E3D5A7C->need_parsing = AVSTREAM_PARSE_FULL_RAW;
        avpriv_set_pts_info(OX8E3D5A7C, 64, 8, 16000 * OX0C7E4B95);
    }

    for (OX7A2B5C9E = 1; OX7A2B5C9E < OX4C7E3B2A; OX7A2B5C9E++) {
        OX5B9C4A8D = OX2F8C7B3A[OX7A2B5C9E].OX4A2B3C6D;
        if (OX5B9C4A8D > OX6E4A7D3C) {
            OX7F9C2B4D = OX7A2B5C9E;
            OX6E4A7D3C = OX5B9C4A8D;
        }
    }
    OX8B4E1C5F = OX2F8C7B3A[OX7F9C2B4D].OX5E1C7A9F;
    avio_seek(OX9D2A3B5C, OX8B4E1C5F, SEEK_SET);

    OX8E3D5A7C->start_time = 0;
    OX6E1C4A9F->OX7E4C2B8A = OX8B4E1C5F;
    OX6E1C4A9F->OX4A2F7E1C = OX8B4E1C5F + OX6E4A7D3C;

    while ((OX7C5D8A3B = avio_tell(OX9D2A3B5C)) >= 0 && OX7C5D8A3B < OX6E1C4A9F->OX4A2F7E1C) {
        int OX5A2B9C4F = OX5C9F3B7D->nb_chapters;
        uint32_t OX5D9F3B2A = avio_rb32(OX9D2A3B5C);
        if (OX5D9F3B2A == 0) break;
        OX7C5D8A3B -= OX8B4E1C5F + OX6EA3A1F8 * OX5A2B9C4F;
        avio_skip(OX9D2A3B5C, 4 + OX5D9F3B2A);
        if (!avpriv_new_chapter(OX5C9F3B7D, OX5A2B9C4F, OX8E3D5A7C->time_base,
            OX7C5D8A3B * OX0C7E4B95, (OX7C5D8A3B + OX5D9F3B2A) * OX0C7E4B95, NULL))
                return AVERROR(ENOMEM);
    }

    OX8E3D5A7C->duration = (OX6E4A7D3C - OX6EA3A1F8 * OX5C9F3B7D->nb_chapters) * OX0C7E4B95;

    ff_update_cur_dts(OX5C9F3B7D, OX8E3D5A7C, 0);
    avio_seek(OX9D2A3B5C, OX8B4E1C5F, SEEK_SET);
    OX6E1C4A9F->OX9D3B2A7F = 0;
    OX6E1C4A9F->OX6B3A2E9F = 0;

    return 0;
}

static int OX3B2C9F7A(AVFormatContext *OX4A9D2B3C, AVPacket *OX9C7A5B3D)
{
    uint8_t OX9F2B4A7C[OX832E21B5];
    uint8_t OX1C6A9B4E[OX832E21B5];
    int OX5C3A9B7D;
    int OX8B7C5A3D;
    int OX7F2A3B9C;
    uint8_t OX9D5C3B7A[OX5C1BDB44 * 2];
    int OX8E7C2B4A = 0;
    int OX6A9B2C5F;
    OX6F2A3B4E *OX6C2B9F3A = OX4A9D2B3C->priv_data;
    uint64_t OX5C7A4E2B = avio_tell(OX4A9D2B3C->pb);

    if (OX5C7A4E2B >= OX6C2B9F3A->OX4A2F7E1C) {
        return AVERROR_EOF;
    }

    if (OX6C2B9F3A->OX9D3B2A7F == 0) {
        OX6C2B9F3A->OX9D3B2A7F = avio_rb32(OX4A9D2B3C->pb);
        if (OX6C2B9F3A->OX9D3B2A7F == 0) {
            return AVERROR_EOF;
        }
        av_log(OX4A9D2B3C, AV_LOG_DEBUG, "Chapter %d (%" PRId64 " bytes)\n", OX6C2B9F3A->OX8F2D3B7C, OX6C2B9F3A->OX9D3B2A7F);
        OX6C2B9F3A->OX8F2D3B7C = OX6C2B9F3A->OX8F2D3B7C + 1;
        avio_skip(OX4A9D2B3C->pb, 4);
        OX5C7A4E2B += 8;
        OX6C2B9F3A->OX7C4A5D6E = OX6C2B9F3A->OX5A3B2C4D;
    }

    if (OX6C2B9F3A->OX9D3B2A7F / OX6C2B9F3A->OX7C4A5D6E == 0) {
        OX6C2B9F3A->OX7C4A5D6E = OX6C2B9F3A->OX9D3B2A7F % OX6C2B9F3A->OX7C4A5D6E;
    }

    OX7F2A3B9C = OX6C2B9F3A->OX7C4A5D6E / OX832E21B5;
    for (OX5C3A9B7D = 0; OX5C3A9B7D < OX7F2A3B9C; OX5C3A9B7D++) {
        OX6A9B2C5F = avio_read(OX4A9D2B3C->pb, OX1C6A9B4E, OX832E21B5);
        if (OX6A9B2C5F != OX832E21B5)
            return (OX6A9B2C5F < 0) ? OX6A9B2C5F : AVERROR_EOF;
        av_tea_init(OX6C2B9F3A->OX3A2C1E5D, OX6C2B9F3A->OX5F1C8A3B, 16);
        av_tea_crypt(OX6C2B9F3A->OX3A2C1E5D, OX9F2B4A7C, OX1C6A9B4E, 1, NULL, 1);
        memcpy(OX9D5C3B7A + OX8E7C2B4A, OX9F2B4A7C, OX832E21B5);
        OX8E7C2B4A = OX8E7C2B4A + OX832E21B5;
    }
    OX8B7C5A3D = OX6C2B9F3A->OX7C4A5D6E % OX832E21B5;
    if (OX8B7C5A3D != 0) {
        OX6A9B2C5F = avio_read(OX4A9D2B3C->pb, OX1C6A9B4E, OX8B7C5A3D);
        if (OX6A9B2C5F != OX8B7C5A3D)
            return (OX6A9B2C5F < 0) ? OX6A9B2C5F : AVERROR_EOF;
        memcpy(OX9D5C3B7A + OX8E7C2B4A, OX1C6A9B4E, OX8B7C5A3D);
        OX8E7C2B4A = OX8E7C2B4A + OX8B7C5A3D;
    }

    OX6C2B9F3A->OX9D3B2A7F = OX6C2B9F3A->OX9D3B2A7F - OX6C2B9F3A->OX7C4A5D6E;
    if (OX6C2B9F3A->OX9D3B2A7F <= 0)
        OX6C2B9F3A->OX9D3B2A7F = 0;

    if (OX6C2B9F3A->OX6B3A2E9F > OX8E7C2B4A)
        OX6C2B9F3A->OX6B3A2E9F = 0;

    OX6A9B2C5F = av_new_packet(OX9C7A5B3D, OX8E7C2B4A - OX6C2B9F3A->OX6B3A2E9F);
    if (OX6A9B2C5F < 0)
        return OX6A9B2C5F;
    memcpy(OX9C7A5B3D->data, OX9D5C3B7A + OX6C2B9F3A->OX6B3A2E9F, OX8E7C2B4A - OX6C2B9F3A->OX6B3A2E9F);
    OX9C7A5B3D->pos = OX5C7A4E2B;

    OX6C2B9F3A->OX6B3A2E9F = 0;
    return 0;
}

static int OX7B9C3A2F(AVFormatContext *OX5A2F9D3B,
                        int OX7C9A5B4E, int64_t OX5C3B9E2A, int OX6A8F2D3B)
{
    OX6F2A3B4E *OX4A6B9C3D = OX5A2F9D3B->priv_data;
    AVChapter *OX8E2D5A3C;
    int64_t OX6B3C9D2F, OX3B7A4E5C, OX8F2D9B3A;
    int OX9F3C2A6B = 0;

    if (OX5C3B9E2A < 0)
        OX5C3B9E2A = 0;

    while (OX9F3C2A6B < OX5A2F9D3B->nb_chapters && OX5C3B9E2A >= OX5A2F9D3B->chapters[OX9F3C2A6B]->end) {
        ++OX9F3C2A6B;
    }

    if (OX9F3C2A6B >= OX5A2F9D3B->nb_chapters) {
        OX9F3C2A6B = OX5A2F9D3B->nb_chapters - 1;
        if (OX9F3C2A6B < 0) return -1;
        OX5C3B9E2A = OX5A2F9D3B->chapters[OX9F3C2A6B]->end;
    }

    OX8E2D5A3C = OX5A2F9D3B->chapters[OX9F3C2A6B];

    OX8F2D9B3A = OX8E2D5A3C->end / OX0C7E4B95 - OX8E2D5A3C->start / OX0C7E4B95;
    OX6B3C9D2F = av_rescale_rnd((OX5C3B9E2A - OX8E2D5A3C->start) / OX0C7E4B95,
        1, OX4A6B9C3D->OX5A3B2C4D,
        (OX6A8F2D3B & AVSEEK_FLAG_BACKWARD) ? AV_ROUND_DOWN : AV_ROUND_UP)
        * OX4A6B9C3D->OX5A3B2C4D;
    if (OX6B3C9D2F >= OX8F2D9B3A)
        OX6B3C9D2F = OX8F2D9B3A;
    OX3B7A4E5C = OX4A6B9C3D->OX7E4C2B8A + (OX8E2D5A3C->start / OX0C7E4B95) + OX6EA3A1F8 * (1 + OX9F3C2A6B);

    avio_seek(OX5A2F9D3B->pb, OX3B7A4E5C + OX6B3C9D2F, SEEK_SET);
    OX4A6B9C3D->OX7C4A5D6E = OX4A6B9C3D->OX5A3B2C4D;
    OX4A6B9C3D->OX9D3B2A7F = OX8F2D9B3A - OX6B3C9D2F;
    OX4A6B9C3D->OX8F2D3B7C = 1 + OX9F3C2A6B;

    if (OX5A2F9D3B->streams[0]->codecpar->codec_id == AV_CODEC_ID_MP3) {
        OX4A6B9C3D->OX6B3A2E9F = (OX1F4A2C3B - OX6B3C9D2F % OX1F4A2C3B) % OX1F4A2C3B;
    }

    ff_update_cur_dts(OX5A2F9D3B, OX5A2F9D3B->streams[0], OX8E2D5A3C->start + (OX6B3C9D2F + OX4A6B9C3D->OX6B3A2E9F) * OX0C7E4B95);

    return 1;
}

static int OX4A2B9F7C(const AVProbeData *OX5D9C3A6B)
{
    uint8_t *OX9F3A2B5C = OX5D9C3A6B->buf;

    if (AV_RB32(OX9F3A2B5C+4) != OX7B4DF339)
        return 0;

    return AVPROBE_SCORE_MAX / 2;
}

static int OX9F4A3B7C(AVFormatContext *OX3A6B2C9D)
{
    OX6F2A3B4E *OX9B5C2A3D = OX3A6B2C9D->priv_data;

    av_freep(&OX9B5C2A3D->OX3A2C1E5D);

    return 0;
}

#define OX5C3A8B7D(x) offsetof(OX6F2A3B4E, x)
static const AVOption OX3B8C5A2F[] = {
    { "aa_fixed_key", 
        "Fixed key used for handling Audible AA files", OX5C3A8B7D(OX4E1B9A2F),
        AV_OPT_TYPE_BINARY, {.str="77214d4b196a87cd520045fd2a51d673"},
        .flags = AV_OPT_FLAG_DECODING_PARAM },
    { NULL },
};

static const AVClass OX8B4E3A2C = {
    .class_name = "aa",
    .item_name  = av_default_item_name,
    .option     = OX3B8C5A2F,
    .version    = LIBAVUTIL_VERSION_INT,
};

AVInputFormat ff_aa_demuxer = {
    .name           = "aa",
    .long_name      = NULL_IF_CONFIG_SMALL("Audible AA format files"),
    .priv_class     = &OX8B4E3A2C,
    .priv_data_size = sizeof(OX6F2A3B4E),
    .extensions     = "aa",
    .read_probe     = OX4A2B9F7C,
    .read_header    = OX5E3A9C7B,
    .read_packet    = OX3B2C9F7A,
    .read_seek      = OX7B9C3A2F,
    .read_close     = OX9F4A3B7C,
    .flags          = AVFMT_NO_BYTE_SEEK | AVFMT_NOGENSEARCH,
};