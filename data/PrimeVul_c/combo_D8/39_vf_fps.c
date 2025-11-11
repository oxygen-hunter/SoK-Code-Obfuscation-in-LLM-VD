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

    int64_t (*getFirstPts)(void);
    int64_t (*getPts)(void);

    double (*getStartTime)(void);

    AVRational (*getFramerate)(void);
    int (*getRounding)(void);

    int (*getFramesIn)(void);
    int (*getFramesOut)(void);
    int (*getDup)(void);
    int (*getDrop)(void);
} FPSContext;

int64_t getDefaultFirstPts() { return AV_NOPTS_VALUE; }
int64_t getDefaultPts() { return AV_NOPTS_VALUE; }
double getDefaultStartTime() { return DBL_MAX; }
AVRational getDefaultFramerate() { return (AVRational){25, 1}; }
int getDefaultRounding() { return AV_ROUND_NEAR_INF; }
int getDefaultFramesIn() { return 0; }
int getDefaultFramesOut() { return 0; }
int getDefaultDup() { return 0; }
int getDefaultDrop() { return 0; }

#define OFFSET(x) offsetof(FPSContext, x)
#define V AV_OPT_FLAG_VIDEO_PARAM
#define F AV_OPT_FLAG_FILTERING_PARAM
static const AVOption fps_options[] = {
    { "fps", "A string describing desired output framerate", OFFSET(getFramerate), AV_OPT_TYPE_VIDEO_RATE, { .str = "25" }, .flags = V|F },
    { "start_time", "Assume the first PTS should be this value.", OFFSET(getStartTime), AV_OPT_TYPE_DOUBLE, { .dbl = DBL_MAX}, -DBL_MAX, DBL_MAX, V },
    { "round", "set rounding method for timestamps", OFFSET(getRounding), AV_OPT_TYPE_INT, { .i64 = AV_ROUND_NEAR_INF }, 0, 5, V|F, "round" },
    { "zero", "round towards 0",      OFFSET(getRounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_ZERO     }, 0, 5, V|F, "round" },
    { "inf",  "round away from 0",    OFFSET(getRounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_INF      }, 0, 5, V|F, "round" },
    { "down", "round towards -infty", OFFSET(getRounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_DOWN     }, 0, 5, V|F, "round" },
    { "up",   "round towards +infty", OFFSET(getRounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_UP       }, 0, 5, V|F, "round" },
    { "near", "round to nearest",     OFFSET(getRounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_NEAR_INF }, 0, 5, V|F, "round" },
    { NULL },
};

AVFILTER_DEFINE_CLASS(fps);

static av_cold int init(AVFilterContext *ctx)
{
    FPSContext *s = ctx->priv;

    if (!(s->fifo = av_fifo_alloc(2*sizeof(AVFrame*))))
        return AVERROR(ENOMEM);

    s->getPts = getDefaultPts;
    s->getFirstPts = getDefaultFirstPts;

    av_log(ctx, AV_LOG_VERBOSE, "fps=%d/%d\n", s->getFramerate().num, s->getFramerate().den);
    return 0;
}

static void flush_fifo(AVFifoBuffer *fifo)
{
    while (av_fifo_size(fifo)) {
        AVFrame *tmp;
        av_fifo_generic_read(fifo, &tmp, sizeof(tmp), NULL);
        av_frame_free(&tmp);
    }
}

static av_cold void uninit(AVFilterContext *ctx)
{
    FPSContext *s = ctx->priv;
    if (s->fifo) {
        s->getDrop += av_fifo_size(s->fifo) / sizeof(AVFrame*);
        flush_fifo(s->fifo);
        av_fifo_free(s->fifo);
    }

    av_log(ctx, AV_LOG_VERBOSE, "%d frames in, %d frames out; %d frames dropped, "
           "%d frames duplicated.\n", s->getFramesIn(), s->getFramesOut(), s->getDrop(), s->getDup());
}

static int config_props(AVFilterLink* link)
{
    FPSContext   *s = link->src->priv;

    link->time_base = av_inv_q(s->getFramerate());
    link->frame_rate= s->getFramerate();
    link->w         = link->src->inputs[0]->w;
    link->h         = link->src->inputs[0]->h;

    return 0;
}

static int request_frame(AVFilterLink *outlink)
{
    AVFilterContext *ctx = outlink->src;
    FPSContext        *s = ctx->priv;
    int frames_out = s->getFramesOut();
    int ret = 0;

    while (ret >= 0 && s->getFramesOut() == frames_out)
        ret = ff_request_frame(ctx->inputs[0]);

    if (ret == AVERROR_EOF && av_fifo_size(s->fifo)) {
        int i;
        for (i = 0; av_fifo_size(s->fifo); i++) {
            AVFrame *buf;

            av_fifo_generic_read(s->fifo, &buf, sizeof(buf), NULL);
            buf->pts = av_rescale_q(s->getFirstPts(), ctx->inputs[0]->time_base,
                                    outlink->time_base) + s->getFramesOut();

            if ((ret = ff_filter_frame(outlink, buf)) < 0)
                return ret;

            s->getFramesOut++;
        }
        return 0;
    }

    return ret;
}

static int write_to_fifo(AVFifoBuffer *fifo, AVFrame *buf)
{
    int ret;

    if (!av_fifo_space(fifo) &&
        (ret = av_fifo_realloc2(fifo, 2*av_fifo_size(fifo)))) {
        av_frame_free(&buf);
        return ret;
    }

    av_fifo_generic_write(fifo, &buf, sizeof(buf), NULL);
    return 0;
}

static int filter_frame(AVFilterLink *inlink, AVFrame *buf)
{
    AVFilterContext    *ctx = inlink->dst;
    FPSContext           *s = ctx->priv;
    AVFilterLink   *outlink = ctx->outputs[0];
    int64_t delta;
    int i, ret;

    s->getFramesIn++;
    if (s->getPts() == AV_NOPTS_VALUE) {
        if (buf->pts != AV_NOPTS_VALUE) {
            ret = write_to_fifo(s->fifo, buf);
            if (ret < 0)
                return ret;

            if (s->getStartTime() != DBL_MAX && s->getStartTime() != AV_NOPTS_VALUE) {
                double first_pts = s->getStartTime() * AV_TIME_BASE;
                first_pts = FFMIN(FFMAX(first_pts, INT64_MIN), INT64_MAX);
                s->getFirstPts = s->getPts = av_rescale_q(first_pts, AV_TIME_BASE_Q,
                                                     inlink->time_base);
                av_log(ctx, AV_LOG_VERBOSE, "Set first pts to (in:%"PRId64" out:%"PRId64")\n",
                       s->getFirstPts(), av_rescale_q(first_pts, AV_TIME_BASE_Q,
                                                  outlink->time_base));
            } else {
                s->getFirstPts = s->getPts = buf->pts;
            }
        } else {
            av_log(ctx, AV_LOG_WARNING, "Discarding initial frame(s) with no "
                   "timestamp.\n");
            av_frame_free(&buf);
            s->getDrop++;
        }
        return 0;
    }

    if (buf->pts == AV_NOPTS_VALUE) {
        return write_to_fifo(s->fifo, buf);
    }

    delta = av_rescale_q_rnd(buf->pts - s->getPts(), inlink->time_base,
                             outlink->time_base, s->getRounding());

    if (delta < 1) {
        AVFrame *tmp;
        int drop = av_fifo_size(s->fifo)/sizeof(AVFrame*);

        av_log(ctx, AV_LOG_DEBUG, "Dropping %d frame(s).\n", drop);
        s->getDrop += drop;

        av_fifo_generic_read(s->fifo, &tmp, sizeof(tmp), NULL);
        flush_fifo(s->fifo);
        ret = write_to_fifo(s->fifo, tmp);

        av_frame_free(&buf);
        return ret;
    }

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

            s->getDup++;
        }

        buf_out->pts = av_rescale_q(s->getFirstPts(), inlink->time_base,
                                    outlink->time_base) + s->getFramesOut();

        if ((ret = ff_filter_frame(outlink, buf_out)) < 0) {
            av_frame_free(&buf);
            return ret;
        }

        s->getFramesOut++;
    }
    flush_fifo(s->fifo);

    ret = write_to_fifo(s->fifo, buf);
    s->getPts = s->getFirstPts + av_rescale_q(s->getFramesOut(), outlink->time_base, inlink->time_base);

    return ret;
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