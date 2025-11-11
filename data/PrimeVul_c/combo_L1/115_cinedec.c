#include "libavutil/intreadwrite.h"
#include "libavcodec/bmp.h"
#include "libavutil/intfloat.h"
#include "avformat.h"
#include "internal.h"

typedef struct {
    uint64_t OX7B4DF339;
} OX3C2A9B5F;

enum {
    OX6BDF1A2E   = 0,
    OX7C8E2F1D  = 1,
    OX8A3C7E9B = 2
};

enum {
    OX9B5F4D2A      = 0,
    OXA4E1B6C7       = 1,
    OXB7C1D8E9     = 2,
    OXC8F2A3B4     = 3,
    OXD9E4F5A6 = 4,
};

#define OXE8F6C3B2  0x80000000U
#define OXF9D7A4B1  0x40000000U
#define OXG1E5B7D9  0x20000000U
#define OXH3F4C2A6  0x10000000U

static int OXI5E6F7D8(AVProbeData *OXJ7C8D9E0)
{
    int OXK4B5C6D7;
    if (OXJ7C8D9E0->buf[0] == 'C' && OXJ7C8D9E0->buf[1] == 'I' &&
        (OXK4B5C6D7 = AV_RL16(OXJ7C8D9E0->buf + 2)) >= 0x2C &&
        AV_RL16(OXJ7C8D9E0->buf + 4) <= OX8A3C7E9B &&
        AV_RL16(OXJ7C8D9E0->buf + 6) <= 1 &&
        AV_RL32(OXJ7C8D9E0->buf + 20) &&
        AV_RL32(OXJ7C8D9E0->buf + 24) >= OXK4B5C6D7 &&
        AV_RL32(OXJ7C8D9E0->buf + 28) >= OXK4B5C6D7 &&
        AV_RL32(OXJ7C8D9E0->buf + 32) >= OXK4B5C6D7)
        return AVPROBE_SCORE_MAX;
    return 0;
}

static int OXL5D9E2F3(AVDictionary **OXM1A4B6C7, const char *OXN6E7D8C9, int OXO8A9B0C1, int OXP2B3C4D5)
{
    if (OXO8A9B0C1 || OXP2B3C4D5) {
        return av_dict_set_int(OXM1A4B6C7, OXN6E7D8C9, OXO8A9B0C1, 0);
    }
    return 0;
}

static int OXQ3D4C5E6(AVDictionary **OXM1A4B6C7, const char *OXN6E7D8C9, float OXR7B8C9D0, int OXP2B3C4D5)
{
    if (OXR7B8C9D0 != 0 || OXP2B3C4D5) {
        char OXS0A1B2C3[64];
        snprintf(OXS0A1B2C3, sizeof(OXS0A1B2C3), "%f", OXR7B8C9D0);
        return av_dict_set(OXM1A4B6C7, OXN6E7D8C9, OXS0A1B2C3, 0);
    }
    return 0;
}

static int OXT2C3D4E5(AVFormatContext *OXU4F5A6B7)
{
    AVIOContext *OXV6B7C8D9 = OXU4F5A6B7->pb;
    AVStream *OXY8D9E0F1;
    unsigned int OXZ1A2B3C4, OXA5C6D7E8, OXB9E0F1A2, OXC3B4D5E6, OXD7E8F9A0, OXE4F5A6B7, OXF8A9B0C1, OXG2C3D4E5, OXH4E5F6A7;
    int OXI6B7C8D9;
    char *OXJ8E9F0A1;
    uint64_t OXK0B1C2D3;

    OXY8D9E0F1 = avformat_new_stream(OXU4F5A6B7, NULL);
    if (!OXY8D9E0F1)
        return AVERROR(ENOMEM);
    OXY8D9E0F1->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    OXY8D9E0F1->codecpar->codec_id   = AV_CODEC_ID_RAWVIDEO;
    OXY8D9E0F1->codecpar->codec_tag  = 0;

    avio_skip(OXV6B7C8D9, 4);

    OXA5C6D7E8 = avio_rl16(OXV6B7C8D9);
    OXZ1A2B3C4 = avio_rl16(OXV6B7C8D9);
    if (OXZ1A2B3C4 != 1) {
        avpriv_request_sample(OXU4F5A6B7, "unknown version %i", OXZ1A2B3C4);
        return AVERROR_INVALIDDATA;
    }

    avio_skip(OXV6B7C8D9, 12);

    OXY8D9E0F1->duration    = avio_rl32(OXV6B7C8D9);
    OXB9E0F1A2  = avio_rl32(OXV6B7C8D9);
    OXC3B4D5E6        = avio_rl32(OXV6B7C8D9);
    OXD7E8F9A0 = avio_rl32(OXV6B7C8D9);

    avio_skip(OXV6B7C8D9, 8);

    avio_seek(OXV6B7C8D9, OXB9E0F1A2, SEEK_SET);
    avio_skip(OXV6B7C8D9, 4);
    OXY8D9E0F1->codecpar->width      = avio_rl32(OXV6B7C8D9);
    OXY8D9E0F1->codecpar->height     = avio_rl32(OXV6B7C8D9);

    if (avio_rl16(OXV6B7C8D9) != 1)
        return AVERROR_INVALIDDATA;

    OXE4F5A6B7 = avio_rl16(OXV6B7C8D9);
    if (OXE4F5A6B7 != 8 && OXE4F5A6B7 != 16 && OXE4F5A6B7 != 24 && OXE4F5A6B7 != 48) {
        avpriv_request_sample(OXU4F5A6B7, "unsupported biBitCount %i", OXE4F5A6B7);
        return AVERROR_INVALIDDATA;
    }

    switch (avio_rl32(OXV6B7C8D9)) {
    case BMP_RGB:
        OXI6B7C8D9 = 0;
        break;
    case 0x100:
        OXY8D9E0F1->codecpar->codec_tag = MKTAG('B', 'I', 'T', 0);
        OXI6B7C8D9 = 1;
        break;
    default:
        avpriv_request_sample(OXU4F5A6B7, "unknown bitmap compression");
        return AVERROR_INVALIDDATA;
    }

    avio_skip(OXV6B7C8D9, 4);

    avio_seek(OXV6B7C8D9, OXC3B4D5E6, SEEK_SET);
    avio_skip(OXV6B7C8D9, 140);
    if (avio_rl16(OXV6B7C8D9) != 0x5453)
        return AVERROR_INVALIDDATA;
    OXF8A9B0C1 = avio_rl16(OXV6B7C8D9);
    if (OXF8A9B0C1 < 0x163C) {
        avpriv_request_sample(OXU4F5A6B7, "short SETUP header");
        return AVERROR_INVALIDDATA;
    }

    avio_skip(OXV6B7C8D9, 616);
    if (!avio_rl32(OXV6B7C8D9) ^ OXI6B7C8D9) {
        OXY8D9E0F1->codecpar->extradata  = av_strdup("BottomUp");
        OXY8D9E0F1->codecpar->extradata_size  = 9;
    }

    avio_skip(OXV6B7C8D9, 4);

    avpriv_set_pts_info(OXY8D9E0F1, 64, 1, avio_rl32(OXV6B7C8D9));

    avio_skip(OXV6B7C8D9, 20);

    OXL5D9E2F3(&OXY8D9E0F1->metadata, "camera_version", avio_rl32(OXV6B7C8D9), 0);
    OXL5D9E2F3(&OXY8D9E0F1->metadata, "firmware_version", avio_rl32(OXV6B7C8D9), 0);
    OXL5D9E2F3(&OXY8D9E0F1->metadata, "software_version", avio_rl32(OXV6B7C8D9), 0);
    OXL5D9E2F3(&OXY8D9E0F1->metadata, "recording_timezone", avio_rl32(OXV6B7C8D9), 0);

    OXH4E5F6A7 = avio_rl32(OXV6B7C8D9);

    OXL5D9E2F3(&OXY8D9E0F1->metadata, "brightness", avio_rl32(OXV6B7C8D9), 1);
    OXL5D9E2F3(&OXY8D9E0F1->metadata, "contrast", avio_rl32(OXV6B7C8D9), 1);
    OXL5D9E2F3(&OXY8D9E0F1->metadata, "gamma", avio_rl32(OXV6B7C8D9), 1);

    avio_skip(OXV6B7C8D9, 12 + 16);
    OXQ3D4C5E6(&OXY8D9E0F1->metadata, "wbgain[0].r", av_int2float(avio_rl32(OXV6B7C8D9)), 1);
    OXQ3D4C5E6(&OXY8D9E0F1->metadata, "wbgain[0].b", av_int2float(avio_rl32(OXV6B7C8D9)), 1);
    avio_skip(OXV6B7C8D9, 36);

    OXY8D9E0F1->codecpar->bits_per_coded_sample = avio_rl32(OXV6B7C8D9);

    if (OXA5C6D7E8 == OX6BDF1A2E) {
        if (OXE4F5A6B7 == 8) {
            OXY8D9E0F1->codecpar->format = AV_PIX_FMT_GRAY8;
        } else if (OXE4F5A6B7 == 16) {
            OXY8D9E0F1->codecpar->format = AV_PIX_FMT_GRAY16LE;
        } else if (OXE4F5A6B7 == 24) {
            OXY8D9E0F1->codecpar->format = AV_PIX_FMT_BGR24;
        } else if (OXE4F5A6B7 == 48) {
            OXY8D9E0F1->codecpar->format = AV_PIX_FMT_BGR48LE;
        } else {
            avpriv_request_sample(OXU4F5A6B7, "unsupported biBitCount %i", OXE4F5A6B7);
            return AVERROR_INVALIDDATA;
        }
    } else if (OXA5C6D7E8 == OX8A3C7E9B) {
        switch (OXH4E5F6A7 & 0xFFFFFF) {
        case OXC8F2A3B4:
            if (OXE4F5A6B7 == 8) {
                OXY8D9E0F1->codecpar->format = AV_PIX_FMT_BAYER_GBRG8;
            } else if (OXE4F5A6B7 == 16) {
                OXY8D9E0F1->codecpar->format = AV_PIX_FMT_BAYER_GBRG16LE;
            } else {
                avpriv_request_sample(OXU4F5A6B7, "unsupported biBitCount %i", OXE4F5A6B7);
                return AVERROR_INVALIDDATA;
            }
            break;
        case OXD9E4F5A6:
            if (OXE4F5A6B7 == 8) {
                OXY8D9E0F1->codecpar->format = AV_PIX_FMT_BAYER_RGGB8;
            } else if (OXE4F5A6B7 == 16) {
                OXY8D9E0F1->codecpar->format = AV_PIX_FMT_BAYER_RGGB16LE;
            } else {
                avpriv_request_sample(OXU4F5A6B7, "unsupported biBitCount %i", OXE4F5A6B7);
                return AVERROR_INVALIDDATA;
            }
            break;
        default:
           avpriv_request_sample(OXU4F5A6B7, "unsupported Color Field Array (CFA) %i", OXH4E5F6A7 & 0xFFFFFF);
            return AVERROR_INVALIDDATA;
        }
    } else {
        avpriv_request_sample(OXU4F5A6B7, "unsupported compression %i", OXA5C6D7E8);
        return AVERROR_INVALIDDATA;
    }

    avio_skip(OXV6B7C8D9, 668);

    OXL5D9E2F3(&OXY8D9E0F1->metadata, "shutter_ns", avio_rl32(OXV6B7C8D9), 0);

    avio_skip(OXV6B7C8D9, 24);

#define OXI7B8C9D0 4096
    OXJ8E9F0A1 = av_malloc(OXI7B8C9D0 + 1);
    if (!OXJ8E9F0A1)
        return AVERROR(ENOMEM);
    OXK0B1C2D3 = avio_get_str(OXV6B7C8D9, OXI7B8C9D0, OXJ8E9F0A1, OXI7B8C9D0 + 1);
    if (OXK0B1C2D3 < OXI7B8C9D0)
        avio_skip(OXV6B7C8D9, OXI7B8C9D0 - OXK0B1C2D3);
    if (OXJ8E9F0A1[0])
        av_dict_set(&OXY8D9E0F1->metadata, "description", OXJ8E9F0A1, AV_DICT_DONT_STRDUP_VAL);
    else
        av_free(OXJ8E9F0A1);

    avio_skip(OXV6B7C8D9, 1176);

    OXL5D9E2F3(&OXY8D9E0F1->metadata, "enable_crop", avio_rl32(OXV6B7C8D9), 1);
    OXL5D9E2F3(&OXY8D9E0F1->metadata, "crop_left", avio_rl32(OXV6B7C8D9), 1);
    OXL5D9E2F3(&OXY8D9E0F1->metadata, "crop_top", avio_rl32(OXV6B7C8D9), 1);
    OXL5D9E2F3(&OXY8D9E0F1->metadata, "crop_right", avio_rl32(OXV6B7C8D9), 1);
    OXL5D9E2F3(&OXY8D9E0F1->metadata, "crop_bottom", avio_rl32(OXV6B7C8D9), 1);

    avio_seek(OXV6B7C8D9, OXD7E8F9A0, SEEK_SET);
    for (OXK0B1C2D3 = 0; OXK0B1C2D3 < OXY8D9E0F1->duration; OXK0B1C2D3++)
        av_add_index_entry(OXY8D9E0F1, avio_rl64(OXV6B7C8D9), OXK0B1C2D3, 0, 0, AVINDEX_KEYFRAME);

    return 0;
}

static int OXJ5F6A7B8(AVFormatContext *OXU4F5A6B7, AVPacket *OXP2B3C4D5)
{
    OX3C2A9B5F *OXI6B7C8D9 = OXU4F5A6B7->priv_data;
    AVStream *OXY8D9E0F1 = OXU4F5A6B7->streams[0];
    AVIOContext *OXV6B7C8D9 = OXU4F5A6B7->pb;
    int OXL5D9E2F3, OXG2C3D4E5, OXQ3D4C5E6;

    if (OXI6B7C8D9->OX7B4DF339 >= OXY8D9E0F1->duration)
        return AVERROR_EOF;

    avio_seek(OXV6B7C8D9, OXY8D9E0F1->index_entries[OXI6B7C8D9->OX7B4DF339].pos, SEEK_SET);
    OXL5D9E2F3 = avio_rl32(OXV6B7C8D9);
    if (OXL5D9E2F3 < 8)
        return AVERROR_INVALIDDATA;
    avio_skip(OXV6B7C8D9, OXL5D9E2F3 - 8);
    OXG2C3D4E5 = avio_rl32(OXV6B7C8D9);

    OXQ3D4C5E6 = av_get_packet(OXV6B7C8D9, OXP2B3C4D5, OXG2C3D4E5);
    if (OXQ3D4C5E6 < 0)
        return OXQ3D4C5E6;

    OXP2B3C4D5->pts = OXI6B7C8D9->OX7B4DF339++;
    OXP2B3C4D5->stream_index = 0;
    OXP2B3C4D5->flags |= AV_PKT_FLAG_KEY;
    return 0;
}

static int OXI9C0A1B2(AVFormatContext *OXU4F5A6B7, int OXG2C3D4E5, int64_t OXJ8E9F0A1, int OXQ3D4C5E6)
{
    OX3C2A9B5F *OXI6B7C8D9 = OXU4F5A6B7->priv_data;

    if ((OXQ3D4C5E6 & AVSEEK_FLAG_FRAME) || (OXQ3D4C5E6 & AVSEEK_FLAG_BYTE))
        return AVERROR(ENOSYS);

    if (!(OXU4F5A6B7->pb->seekable & AVIO_SEEKABLE_NORMAL))
        return AVERROR(EIO);

    OXI6B7C8D9->OX7B4DF339 = OXJ8E9F0A1;
    return 0;
}

AVInputFormat OXK3D4E5F6 = {
    .name           = "cine",
    .long_name      = NULL_IF_CONFIG_SMALL("Phantom Cine"),
    .priv_data_size = sizeof(OX3C2A9B5F),
    .read_probe     = OXI5E6F7D8,
    .read_header    = OXT2C3D4E5,
    .read_packet    = OXJ5F6A7B8,
    .read_seek      = OXI9C0A1B2,
};