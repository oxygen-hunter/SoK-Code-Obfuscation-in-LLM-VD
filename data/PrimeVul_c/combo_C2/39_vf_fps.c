#include <float.h>

#include "libavutil/common.h"
#include "libavutil/fifo.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/parseutils.h"

#include "avfilter.h"
#include "internal.h"
#include "video.h"

typedef struct FPSContext {
    const AVClass *class;
    AVFifoBuffer *fifo;
    int64_t first_pts;
    int64_t pts;
    double start_time;
    AVRational framerate;
    int rounding;
    int frames_in;
    int frames_out;
    int dup;
    int drop;
} FPSContext;

#define OFFSET(x) offsetof(FPSContext, x)
#define V AV_OPT_FLAG_VIDEO_PARAM
#define F AV_OPT_FLAG_FILTERING_PARAM
static const AVOption fps_options[] = {
    { "fps", "A string describing desired output framerate", OFFSET(framerate), AV_OPT_TYPE_VIDEO_RATE, { .str = "25" }, .flags = V|F },
    { "start_time", "Assume the first PTS should be this value.", OFFSET(start_time), AV_OPT_TYPE_DOUBLE, { .dbl = DBL_MAX}, -DBL_MAX, DBL_MAX, V },
    { "round", "set rounding method for timestamps", OFFSET(rounding), AV_OPT_TYPE_INT, { .i64 = AV_ROUND_NEAR_INF }, 0, 5, V|F, "round" },
    { "zero", "round towards 0",      OFFSET(rounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_ZERO     }, 0, 5, V|F, "round" },
    { "inf",  "round away from 0",    OFFSET(rounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_INF      }, 0, 5, V|F, "round" },
    { "down", "round towards -infty", OFFSET(rounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_DOWN     }, 0, 5, V|F, "round" },
    { "up",   "round towards +infty", OFFSET(rounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_UP       }, 0, 5, V|F, "round" },
    { "near", "round to nearest",     OFFSET(rounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_NEAR_INF }, 0, 5, V|F, "round" },
    { NULL },
};

AVFILTER_DEFINE_CLASS(fps);

static av_cold int init(AVFilterContext *ctx)
{
    int step = 0;
    FPSContext *s = ctx->priv;
    while (1) {
        switch (step) {
            case 0:
                if (!(s->fifo = av_fifo_alloc(2*sizeof(AVFrame*))))
                    return AVERROR(ENOMEM);
                s->pts = AV_NOPTS_VALUE;
                s->first_pts = AV_NOPTS_VALUE;
                av_log(ctx, AV_LOG_VERBOSE, "fps=%d/%d\n", s->framerate.num, s->framerate.den);
                return 0;
        }
    }
}

static void flush_fifo(AVFifoBuffer *fifo)
{
    int step = 0;
    while (1) {
        switch (step) {
            case 0:
                if (av_fifo_size(fifo) == 0) return;
                step = 1;
                break;
            case 1: {
                AVFrame *tmp;
                av_fifo_generic_read(fifo, &tmp, sizeof(tmp), NULL);
                av_frame_free(&tmp);
                step = 0;
                break;
            }
        }
    }
}

static av_cold void uninit(AVFilterContext *ctx)
{
    int step = 0;
    FPSContext *s = ctx->priv;
    while (1) {
        switch (step) {
            case 0:
                if (!s->fifo) return;
                s->drop += av_fifo_size(s->fifo) / sizeof(AVFrame*);
                flush_fifo(s->fifo);
                av_fifo_free(s->fifo);
                step = 1;
                break;
            case 1:
                av_log(ctx, AV_LOG_VERBOSE, "%d frames in, %d frames out; %d frames dropped, "
                       "%d frames duplicated.\n", s->frames_in, s->frames_out, s->drop, s->dup);
                return;
        }
    }
}

static int config_props(AVFilterLink* link)
{
    int step = 0;
    FPSContext *s = link->src->priv;
    while (1) {
        switch (step) {
            case 0:
                link->time_base = av_inv_q(s->framerate);
                link->frame_rate = s->framerate;
                link->w = link->src->inputs[0]->w;
                link->h = link->src->inputs[0]->h;
                return 0;
        }
    }
}

static int request_frame(AVFilterLink *outlink)
{
    int step = 0;
    AVFilterContext *ctx = outlink->src;
    FPSContext *s = ctx->priv;
    int frames_out = s->frames_out;
    int ret = 0;
    while (1) {
        switch (step) {
            case 0:
                if (ret < 0 || s->frames_out != frames_out) {
                    step = 1;
                } else {
                    ret = ff_request_frame(ctx->inputs[0]);
                }
                break;
            case 1:
                if (ret == AVERROR_EOF && av_fifo_size(s->fifo)) {
                    int i;
                    for (i = 0; av_fifo_size(s->fifo); i++) {
                        AVFrame *buf;
                        av_fifo_generic_read(s->fifo, &buf, sizeof(buf), NULL);
                        buf->pts = av_rescale_q(s->first_pts, ctx->inputs[0]->time_base, outlink->time_base) + s->frames_out;
                        if ((ret = ff_filter_frame(outlink, buf)) < 0)
                            return ret;
                        s->frames_out++;
                    }
                    return 0;
                }
                return ret;
        }
    }
}

static int write_to_fifo(AVFifoBuffer *fifo, AVFrame *buf)
{
    int step = 0;
    int ret;
    while (1) {
        switch (step) {
            case 0:
                if (!av_fifo_space(fifo)) {
                    step = 1;
                } else {
                    av_fifo_generic_write(fifo, &buf, sizeof(buf), NULL);
                    return 0;
                }
                break;
            case 1:
                ret = av_fifo_realloc2(fifo, 2 * av_fifo_size(fifo));
                if (ret != 0) {
                    av_frame_free(&buf);
                    return ret;
                }
                av_fifo_generic_write(fifo, &buf, sizeof(buf), NULL);
                return 0;
        }
    }
}

static int filter_frame(AVFilterLink *inlink, AVFrame *buf)
{
    int step = 0;
    AVFilterContext *ctx = inlink->dst;
    FPSContext *s = ctx->priv;
    AVFilterLink *outlink = ctx->outputs[0];
    int64_t delta;
    int i, ret;
    s->frames_in++;
    while (1) {
        switch (step) {
            case 0:
                if (s->pts != AV_NOPTS_VALUE) {
                    step = 2;
                } else {
                    if (buf->pts != AV_NOPTS_VALUE) {
                        step = 1;
                    } else {
                        av_log(ctx, AV_LOG_WARNING, "Discarding initial frame(s) with no timestamp.\n");
                        av_frame_free(&buf);
                        s->drop++;
                        return 0;
                    }
                }
                break;
            case 1:
                ret = write_to_fifo(s->fifo, buf);
                if (ret < 0)
                    return ret;
                if (s->start_time != DBL_MAX && s->start_time != AV_NOPTS_VALUE) {
                    double first_pts = s->start_time * AV_TIME_BASE;
                    first_pts = FFMIN(FFMAX(first_pts, INT64_MIN), INT64_MAX);
                    s->first_pts = s->pts = av_rescale_q(first_pts, AV_TIME_BASE_Q, inlink->time_base);
                    av_log(ctx, AV_LOG_VERBOSE, "Set first pts to (in:%"PRId64" out:%"PRId64")\n",
                           s->first_pts, av_rescale_q(first_pts, AV_TIME_BASE_Q, outlink->time_base));
                } else {
                    s->first_pts = s->pts = buf->pts;
                }
                return 0;
            case 2:
                if (buf->pts != AV_NOPTS_VALUE) {
                    step = 3;
                } else {
                    return write_to_fifo(s->fifo, buf);
                }
                break;
            case 3:
                delta = av_rescale_q_rnd(buf->pts - s->pts, inlink->time_base, outlink->time_base, s->rounding);
                if (delta < 1) {
                    AVFrame *tmp;
                    int drop = av_fifo_size(s->fifo) / sizeof(AVFrame*);
                    av_log(ctx, AV_LOG_DEBUG, "Dropping %d frame(s).\n", drop);
                    s->drop += drop;
                    av_fifo_generic_read(s->fifo, &tmp, sizeof(tmp), NULL);
                    flush_fifo(s->fifo);
                    ret = write_to_fifo(s->fifo, tmp);
                    av_frame_free(&buf);
                    return ret;
                }
                step = 4;
                break;
            case 4:
                for (i = 0; i < delta; i++) {
                    AVFrame *buf_out;
                    av_fifo_generic_read(s->fifo, &buf_out, sizeof(buf_out), NULL);
                    if (!av_fifo_size(s->fifo) && i < delta - 1) {
                        AVFrame *dup = av_frame_clone(buf_out);
                        av_log(ctx, AV_LOG_DEBUG, "Duplicating frame.\n");
                        if (dup)
                            ret = write_to_fifo(s->fifo, dup);
                        else
                            ret = AVERROR(ENOMEM);
                        if (ret < 0) {
                            av_frame_free(&buf_out);
                            av_frame_free(&buf);
                            return ret;
                        }
                        s->dup++;
                    }
                    buf_out->pts = av_rescale_q(s->first_pts, inlink->time_base, outlink->time_base) + s->frames_out;
                    if ((ret = ff_filter_frame(outlink, buf_out)) < 0) {
                        av_frame_free(&buf);
                        return ret;
                    }
                    s->frames_out++;
                }
                flush_fifo(s->fifo);
                ret = write_to_fifo(s->fifo, buf);
                s->pts = s->first_pts + av_rescale_q(s->frames_out, outlink->time_base, inlink->time_base);
                return ret;
        }
    }
}

static const AVFilterPad avfilter_vf_fps_inputs[] = {
    {
        .name        = "default",
        .type        = AVMEDIA_TYPE_VIDEO,
        .filter_frame = filter_frame,
    },
    { NULL }
};

static const AVFilterPad avfilter_vf_fps_outputs[] = {
    {
        .name          = "default",
        .type          = AVMEDIA_TYPE_VIDEO,
        .request_frame = request_frame,
        .config_props  = config_props
    },
    { NULL }
};

AVFilter avfilter_vf_fps = {
    .name        = "fps",
    .description = NULL_IF_CONFIG_SMALL("Force constant framerate."),

    .init      = init,
    .uninit    = uninit,

    .priv_size = sizeof(FPSContext),
    .priv_class = &fps_class,

    .inputs    = avfilter_vf_fps_inputs,
    .outputs   = avfilter_vf_fps_outputs,
};