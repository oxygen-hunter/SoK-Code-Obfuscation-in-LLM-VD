#include <float.h>

#include "libavutil/common.h"
#include "libavutil/fifo.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/parseutils.h"

#include "avfilter.h"
#include "internal.h"
#include "video.h"

typedef struct OX7B4DF339 {
    const AVClass *OXF7A9C233;

    AVFifoBuffer *OX1D9A58DA;

    int64_t OX8BB0F6B2;
    int64_t OX4D7E5D3A;

    double OX1A9E1B8B;

    AVRational OX5F3A4E6C;
    int OX3C68DBE4;

    int OXAEE2E5A0;
    int OX5A8C2E1F;
    int OX1C4B6E8D;
    int OX7A3F5D9C;
} OX7B4DF339;

#define OXE3B6A8D6 offsetof(OX7B4DF339, x)
#define OX2F9D1C4B AV_OPT_FLAG_VIDEO_PARAM
#define OX4A3E2F1D AV_OPT_FLAG_FILTERING_PARAM
static const AVOption OX3D5B6C2E[] = {
    { "fps", "A string describing desired output framerate", OXE3B6A8D6(OX5F3A4E6C), AV_OPT_TYPE_VIDEO_RATE, { .str = "25" }, .flags = OX2F9D1C4B|OX4A3E2F1D },
    { "start_time", "Assume the first PTS should be this value.", OXE3B6A8D6(OX1A9E1B8B), AV_OPT_TYPE_DOUBLE, { .dbl = DBL_MAX}, -DBL_MAX, DBL_MAX, OX2F9D1C4B },
    { "round", "set rounding method for timestamps", OXE3B6A8D6(OX3C68DBE4), AV_OPT_TYPE_INT, { .i64 = AV_ROUND_NEAR_INF }, 0, 5, OX2F9D1C4B|OX4A3E2F1D, "round" },
    { "zero", "round towards 0",      OXE3B6A8D6(OX3C68DBE4), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_ZERO     }, 0, 5, OX2F9D1C4B|OX4A3E2F1D, "round" },
    { "inf",  "round away from 0",    OXE3B6A8D6(OX3C68DBE4), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_INF      }, 0, 5, OX2F9D1C4B|OX4A3E2F1D, "round" },
    { "down", "round towards -infty", OXE3B6A8D6(OX3C68DBE4), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_DOWN     }, 0, 5, OX2F9D1C4B|OX4A3E2F1D, "round" },
    { "up",   "round towards +infty", OXE3B6A8D6(OX3C68DBE4), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_UP       }, 0, 5, OX2F9D1C4B|OX4A3E2F1D, "round" },
    { "near", "round to nearest",     OXE3B6A8D6(OX3C68DBE4), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_NEAR_INF }, 0, 5, OX2F9D1C4B|OX4A3E2F1D, "round" },
    { NULL },
};

AVFILTER_DEFINE_CLASS(OX3D5B6C2E);

static av_cold int OX2E9A5D3C(AVFilterContext *OX7F1B2C4D)
{
    OX7B4DF339 *OX5E3A6B4C = OX7F1B2C4D->priv;

    if (!(OX5E3A6B4C->OX1D9A58DA = av_fifo_alloc(2*sizeof(AVFrame*))))
        return AVERROR(ENOMEM);

    OX5E3A6B4C->OX4D7E5D3A = AV_NOPTS_VALUE;
    OX5E3A6B4C->OX8BB0F6B2 = AV_NOPTS_VALUE;

    av_log(OX7F1B2C4D, AV_LOG_VERBOSE, "fps=%d/%d\n", OX5E3A6B4C->OX5F3A4E6C.num, OX5E3A6B4C->OX5F3A4E6C.den);
    return 0;
}

static void OX1A2C3E4B(AVFifoBuffer *OX1D9A58DA)
{
    while (av_fifo_size(OX1D9A58DA)) {
        AVFrame *OX2D3B5C7F;
        av_fifo_generic_read(OX1D9A58DA, &OX2D3B5C7F, sizeof(OX2D3B5C7F), NULL);
        av_frame_free(&OX2D3B5C7F);
    }
}

static av_cold void OX4B2A3C8F(AVFilterContext *OX7F1B2C4D)
{
    OX7B4DF339 *OX5E3A6B4C = OX7F1B2C4D->priv;
    if (OX5E3A6B4C->OX1D9A58DA) {
        OX5E3A6B4C->OX7A3F5D9C += av_fifo_size(OX5E3A6B4C->OX1D9A58DA) / sizeof(AVFrame*);
        OX1A2C3E4B(OX5E3A6B4C->OX1D9A58DA);
        av_fifo_free(OX5E3A6B4C->OX1D9A58DA);
    }

    av_log(OX7F1B2C4D, AV_LOG_VERBOSE, "%d frames in, %d frames out; %d frames dropped, "
           "%d frames duplicated.\n", OX5E3A6B4C->OXAEE2E5A0, OX5E3A6B4C->OX5A8C2E1F, OX5E3A6B4C->OX7A3F5D9C, OX5E3A6B4C->OX1C4B6E8D);
}

static int OX3D8C2B1E(AVFilterLink* OX1F4D6A3B)
{
    OX7B4DF339 *OX5E3A6B4C = OX1F4D6A3B->src->priv;

    OX1F4D6A3B->time_base = av_inv_q(OX5E3A6B4C->OX5F3A4E6C);
    OX1F4D6A3B->frame_rate= OX5E3A6B4C->OX5F3A4E6C;
    OX1F4D6A3B->w = OX1F4D6A3B->src->inputs[0]->w;
    OX1F4D6A3B->h = OX1F4D6A3B->src->inputs[0]->h;

    return 0;
}

static int OX1C2A3F4B(AVFilterLink *OX5E2A1C3B)
{
    AVFilterContext *OX7F1B2C4D = OX5E2A1C3B->src;
    OX7B4DF339 *OX5E3A6B4C = OX7F1B2C4D->priv;
    int OX5A8C2E1F = OX5E3A6B4C->OX5A8C2E1F;
    int OX8C7A6B5D = 0;

    while (OX8C7A6B5D >= 0 && OX5E3A6B4C->OX5A8C2E1F == OX5A8C2E1F)
        OX8C7A6B5D = ff_request_frame(OX7F1B2C4D->inputs[0]);

    if (OX8C7A6B5D == AVERROR_EOF && av_fifo_size(OX5E3A6B4C->OX1D9A58DA)) {
        int OX4F2A1C3B;
        for (OX4F2A1C3B = 0; av_fifo_size(OX5E3A6B4C->OX1D9A58DA); OX4F2A1C3B++) {
            AVFrame *OX7A4D2C1B;

            av_fifo_generic_read(OX5E3A6B4C->OX1D9A58DA, &OX7A4D2C1B, sizeof(OX7A4D2C1B), NULL);
            OX7A4D2C1B->pts = av_rescale_q(OX5E3A6B4C->OX8BB0F6B2, OX7F1B2C4D->inputs[0]->time_base,
                                           OX5E2A1C3B->time_base) + OX5E3A6B4C->OX5A8C2E1F;

            if ((OX8C7A6B5D = ff_filter_frame(OX5E2A1C3B, OX7A4D2C1B)) < 0)
                return OX8C7A6B5D;

            OX5E3A6B4C->OX5A8C2E1F++;
        }
        return 0;
    }

    return OX8C7A6B5D;
}

static int OX5B2A3C4E(AVFifoBuffer *OX1D9A58DA, AVFrame *OX7A4D2C1B)
{
    int OX8C7A6B5D;

    if (!av_fifo_space(OX1D9A58DA) &&
        (OX8C7A6B5D = av_fifo_realloc2(OX1D9A58DA, 2*av_fifo_size(OX1D9A58DA)))) {
        av_frame_free(&OX7A4D2C1B);
        return OX8C7A6B5D;
    }

    av_fifo_generic_write(OX1D9A58DA, &OX7A4D2C1B, sizeof(OX7A4D2C1B), NULL);
    return 0;
}

static int OX3F4A1C2B(AVFilterLink *OX5E2A1C3B, AVFrame *OX7A4D2C1B)
{
    AVFilterContext *OX7F1B2C4D = OX5E2A1C3B->dst;
    OX7B4DF339 *OX5E3A6B4C = OX7F1B2C4D->priv;
    AVFilterLink *OX1F4D6A3B = OX7F1B2C4D->outputs[0];
    int64_t OX8B7C6D5A;
    int OX4F2A1C3B, OX8C7A6B5D;

    OX5E3A6B4C->OXAEE2E5A0++;
    if (OX5E3A6B4C->OX4D7E5D3A == AV_NOPTS_VALUE) {
        if (OX7A4D2C1B->pts != AV_NOPTS_VALUE) {
            OX8C7A6B5D = OX5B2A3C4E(OX5E3A6B4C->OX1D9A58DA, OX7A4D2C1B);
            if (OX8C7A6B5D < 0)
                return OX8C7A6B5D;

            if (OX5E3A6B4C->OX1A9E1B8B != DBL_MAX && OX5E3A6B4C->OX1A9E1B8B != AV_NOPTS_VALUE) {
                double OX8BB0F6B2 = OX5E3A6B4C->OX1A9E1B8B * AV_TIME_BASE;
                OX8BB0F6B2 = FFMIN(FFMAX(OX8BB0F6B2, INT64_MIN), INT64_MAX);
                OX5E3A6B4C->OX8BB0F6B2 = OX5E3A6B4C->OX4D7E5D3A = av_rescale_q(OX8BB0F6B2, AV_TIME_BASE_Q,
                                                                              OX5E2A1C3B->time_base);
                av_log(OX7F1B2C4D, AV_LOG_VERBOSE, "Set first pts to (in:%"PRId64" out:%"PRId64")\n",
                       OX5E3A6B4C->OX8BB0F6B2, av_rescale_q(OX8BB0F6B2, AV_TIME_BASE_Q,
                                                           OX1F4D6A3B->time_base));
            } else {
                OX5E3A6B4C->OX8BB0F6B2 = OX5E3A6B4C->OX4D7E5D3A = OX7A4D2C1B->pts;
            }
        } else {
            av_log(OX7F1B2C4D, AV_LOG_WARNING, "Discarding initial frame(s) with no "
                   "timestamp.\n");
            av_frame_free(&OX7A4D2C1B);
            OX5E3A6B4C->OX7A3F5D9C++;
        }
        return 0;
    }

    if (OX7A4D2C1B->pts == AV_NOPTS_VALUE) {
        return OX5B2A3C4E(OX5E3A6B4C->OX1D9A58DA, OX7A4D2C1B);
    }

    OX8B7C6D5A = av_rescale_q_rnd(OX7A4D2C1B->pts - OX5E3A6B4C->OX4D7E5D3A, OX5E2A1C3B->time_base,
                                  OX1F4D6A3B->time_base, OX5E3A6B4C->OX3C68DBE4);

    if (OX8B7C6D5A < 1) {
        AVFrame *OX2D3B5C7F;
        int OX7A3F5D9C = av_fifo_size(OX5E3A6B4C->OX1D9A58DA)/sizeof(AVFrame*);

        av_log(OX7F1B2C4D, AV_LOG_DEBUG, "Dropping %d frame(s).\n", OX7A3F5D9C);
        OX5E3A6B4C->OX7A3F5D9C += OX7A3F5D9C;

        av_fifo_generic_read(OX5E3A6B4C->OX1D9A58DA, &OX2D3B5C7F, sizeof(OX2D3B5C7F), NULL);
        OX1A2C3E4B(OX5E3A6B4C->OX1D9A58DA);
        OX8C7A6B5D = OX5B2A3C4E(OX5E3A6B4C->OX1D9A58DA, OX2D3B5C7F);

        av_frame_free(&OX7A4D2C1B);
        return OX8C7A6B5D;
    }

    for (OX4F2A1C3B = 0; OX4F2A1C3B < OX8B7C6D5A; OX4F2A1C3B++) {
        AVFrame *OX7B4A2C1D;
        av_fifo_generic_read(OX5E3A6B4C->OX1D9A58DA, &OX7B4A2C1D, sizeof(OX7B4A2C1D), NULL);

        if (!av_fifo_size(OX5E3A6B4C->OX1D9A58DA) && OX4F2A1C3B < OX8B7C6D5A - 1) {
            AVFrame *OX2D3B5C7F = av_frame_clone(OX7B4A2C1D);

            av_log(OX7F1B2C4D, AV_LOG_DEBUG, "Duplicating frame.\n");
            if (OX2D3B5C7F)
                OX8C7A6B5D = OX5B2A3C4E(OX5E3A6B4C->OX1D9A58DA, OX2D3B5C7F);
            else
                OX8C7A6B5D = AVERROR(ENOMEM);

            if (OX8C7A6B5D < 0) {
                av_frame_free(&OX7B4A2C1D);
                av_frame_free(&OX7A4D2C1B);
                return OX8C7A6B5D;
            }

            OX5E3A6B4C->OX1C4B6E8D++;
        }

        OX7B4A2C1D->pts = av_rescale_q(OX5E3A6B4C->OX8BB0F6B2, OX5E2A1C3B->time_base,
                                       OX1F4D6A3B->time_base) + OX5E3A6B4C->OX5A8C2E1F;

        if ((OX8C7A6B5D = ff_filter_frame(OX1F4D6A3B, OX7B4A2C1D)) < 0) {
            av_frame_free(&OX7A4D2C1B);
            return OX8C7A6B5D;
        }

        OX5E3A6B4C->OX5A8C2E1F++;
    }
    OX1A2C3E4B(OX5E3A6B4C->OX1D9A58DA);

    OX8C7A6B5D = OX5B2A3C4E(OX5E3A6B4C->OX1D9A58DA, OX7A4D2C1B);
    OX5E3A6B4C->OX4D7E5D3A = OX5E3A6B4C->OX8BB0F6B2 + av_rescale_q(OX5E3A6B4C->OX5A8C2E1F, OX1F4D6A3B->time_base, OX5E2A1C3B->time_base);

    return OX8C7A6B5D;
}

static const AVFilterPad OX7F2A3D1B[] = {
    {
        .name        = "default",
        .type        = AVMEDIA_TYPE_VIDEO,
        .filter_frame = OX3F4A1C2B,
    },
    { NULL }
};

static const AVFilterPad OX5F2B4D3A[] = {
    {
        .name          = "default",
        .type          = AVMEDIA_TYPE_VIDEO,
        .request_frame = OX1C2A3F4B,
        .config_props  = OX3D8C2B1E
    },
    { NULL }
};

AVFilter OX7A3D5E1C = {
    .name        = "fps",
    .description = NULL_IF_CONFIG_SMALL("Force constant framerate."),

    .init      = OX2E9A5D3C,
    .uninit    = OX4B2A3C8F,

    .priv_size = sizeof(OX7B4DF339),
    .priv_class = &fps_class,

    .inputs    = OX7F2A3D1B,
    .outputs   = OX5F2B4D3A,
};