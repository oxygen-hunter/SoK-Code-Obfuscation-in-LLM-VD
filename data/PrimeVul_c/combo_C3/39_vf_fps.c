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
    { "zero", "round towards 0", OFFSET(rounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_ZERO }, 0, 5, V|F, "round" },
    { "inf", "round away from 0", OFFSET(rounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_INF }, 0, 5, V|F, "round" },
    { "down", "round towards -infty", OFFSET(rounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_DOWN }, 0, 5, V|F, "round" },
    { "up", "round towards +infty", OFFSET(rounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_UP }, 0, 5, V|F, "round" },
    { "near", "round to nearest", OFFSET(rounding), AV_OPT_TYPE_CONST, { .i64 = AV_ROUND_NEAR_INF }, 0, 5, V|F, "round" },
    { NULL },
};

AVFILTER_DEFINE_CLASS(fps);

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, DUP_FRM, DROP_FRM, OUTPUT_FRM, END
} Instruction;

typedef struct VM {
    int64_t stack[256];
    int sp;
    int pc;
    Instruction code[1024];
    FPSContext *ctx;
} VM;

void vm_init(VM *vm, FPSContext *ctx) {
    vm->sp = -1;
    vm->pc = 0;
    vm->ctx = ctx;
}

void vm_run(VM *vm) {
    while (1) {
        switch (vm->code[vm->pc++]) {
            case PUSH:
                vm->stack[++vm->sp] = vm->code[vm->pc++];
                break;
            case POP:
                vm->sp--;
                break;
            case ADD:
                vm->stack[vm->sp-1] += vm->stack[vm->sp];
                vm->sp--;
                break;
            case SUB:
                vm->stack[vm->sp-1] -= vm->stack[vm->sp];
                vm->sp--;
                break;
            case JMP:
                vm->pc = vm->code[vm->pc];
                break;
            case JZ:
                if (vm->stack[vm->sp--] == 0)
                    vm->pc = vm->code[vm->pc];
                else
                    vm->pc++;
                break;
            case LOAD:
                vm->stack[++vm->sp] = *((int64_t*)vm->code[vm->pc++]);
                break;
            case STORE:
                *((int64_t*)vm->code[vm->pc++]) = vm->stack[vm->sp--];
                break;
            case DUP_FRM:
                vm->ctx->dup++;
                break;
            case DROP_FRM:
                vm->ctx->drop++;
                break;
            case OUTPUT_FRM:
                vm->ctx->frames_out++;
                break;
            case END:
                return;
        }
    }
}

static av_cold int init(AVFilterContext *ctx) {
    FPSContext *s = ctx->priv;
    VM vm;
    if (!(s->fifo = av_fifo_alloc(2*sizeof(AVFrame*))))
        return AVERROR(ENOMEM);
    s->pts = AV_NOPTS_VALUE;
    s->first_pts = AV_NOPTS_VALUE;
    av_log(ctx, AV_LOG_VERBOSE, "fps=%d/%d\n", s->framerate.num, s->framerate.den);

    vm_init(&vm, s);
    vm.code[0] = PUSH; vm.code[1] = (Instruction)s->framerate.num;
    vm.code[2] = PUSH; vm.code[3] = (Instruction)s->framerate.den;
    vm.code[4] = OUTPUT_FRM;
    vm.code[5] = END;
    vm_run(&vm);

    return 0;
}

static void flush_fifo(AVFifoBuffer *fifo) {
    while (av_fifo_size(fifo)) {
        AVFrame *tmp;
        av_fifo_generic_read(fifo, &tmp, sizeof(tmp), NULL);
        av_frame_free(&tmp);
    }
}

static av_cold void uninit(AVFilterContext *ctx) {
    FPSContext *s = ctx->priv;
    if (s->fifo) {
        s->drop += av_fifo_size(s->fifo) / sizeof(AVFrame*);
        flush_fifo(s->fifo);
        av_fifo_free(s->fifo);
    }
    av_log(ctx, AV_LOG_VERBOSE, "%d frames in, %d frames out; %d frames dropped, %d frames duplicated.\n", s->frames_in, s->frames_out, s->drop, s->dup);
}

static int config_props(AVFilterLink* link) {
    FPSContext   *s = link->src->priv;
    link->time_base = av_inv_q(s->framerate);
    link->frame_rate= s->framerate;
    link->w         = link->src->inputs[0]->w;
    link->h         = link->src->inputs[0]->h;
    return 0;
}

static int request_frame(AVFilterLink *outlink) {
    AVFilterContext *ctx = outlink->src;
    FPSContext        *s = ctx->priv;
    int frames_out = s->frames_out;
    int ret = 0;
    while (ret >= 0 && s->frames_out == frames_out)
        ret = ff_request_frame(ctx->inputs[0]);
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

static int write_to_fifo(AVFifoBuffer *fifo, AVFrame *buf) {
    int ret;
    if (!av_fifo_space(fifo) &&
        (ret = av_fifo_realloc2(fifo, 2*av_fifo_size(fifo)))) {
        av_frame_free(&buf);
        return ret;
    }
    av_fifo_generic_write(fifo, &buf, sizeof(buf), NULL);
    return 0;
}

static int filter_frame(AVFilterLink *inlink, AVFrame *buf) {
    AVFilterContext    *ctx = inlink->dst;
    FPSContext           *s = ctx->priv;
    AVFilterLink   *outlink = ctx->outputs[0];
    int64_t delta;
    int i, ret;
    VM vm;
    vm_init(&vm, s);

    vm.code[0] = PUSH; vm.code[1] = (Instruction)s->frames_in++;
    vm.code[2] = LOAD; vm.code[3] = (Instruction)&s->pts;
    vm.code[4] = JZ; vm.code[5] = 10;
    vm.code[6] = LOAD; vm.code[7] = (Instruction)&buf->pts;
    vm.code[8] = PUSH; vm.code[9] = AV_NOPTS_VALUE;
    vm.code[10] = SUB;
    vm.code[11] = JZ; vm.code[12] = 27;
    vm.code[13] = PUSH; vm.code[14] = (Instruction)&s->fifo;
    vm.code[15] = PUSH; vm.code[16] = (Instruction)buf;
    vm.code[17] = STORE; vm.code[18] = (Instruction)&ret;
    vm.code[19] = POP; vm.code[20] = JZ; vm.code[21] = 48;
    vm.code[22] = LOAD; vm.code[23] = (Instruction)&s->start_time;
    vm.code[24] = PUSH; vm.code[25] = DBL_MAX;
    vm.code[26] = SUB;
    vm.code[27] = JZ; vm.code[28] = 47;
    vm.code[29] = PUSH; vm.code[30] = AV_TIME_BASE;
    vm.code[31] = STORE; vm.code[32] = (Instruction)&vm.stack[vm.sp];
    vm.code[33] = PUSH; vm.code[34] = INT64_MIN;
    vm.code[35] = PUSH; vm.code[36] = INT64_MAX;
    vm.code[37] = PUSH; vm.code[38] = (Instruction)&s->first_pts;
    vm.code[39] = STORE; vm.code[40] = (Instruction)&s->pts;
    vm.code[41] = LOAD; vm.code[42] = (Instruction)&buf->pts;
    vm.code[43] = STORE; vm.code[44] = (Instruction)&s->first_pts;
    vm.code[45] = STORE; vm.code[46] = (Instruction)&s->pts;
    vm.code[47] = JMP; vm.code[48] = 0;
    vm.code[49] = LOAD; vm.code[50] = (Instruction)&s->pts;
    vm.code[51] = PUSH; vm.code[52] = AV_NOPTS_VALUE;
    vm.code[53] = SUB; vm.code[54] = JZ; vm.code[55] = 63;
    vm.code[56] = PUSH; vm.code[57] = (Instruction)&s->fifo;
    vm.code[58] = PUSH; vm.code[59] = (Instruction)buf;
    vm.code[60] = STORE; vm.code[61] = (Instruction)&ret;
    vm.code[62] = END;
    vm.code[63] = LOAD; vm.code[64] = (Instruction)&buf->pts;
    vm.code[65] = PUSH; vm.code[66] = AV_NOPTS_VALUE;
    vm.code[67] = SUB; vm.code[68] = JZ; vm.code[69] = 83;
    vm.code[70] = PUSH; vm.code[71] = (Instruction)&s->pts;
    vm.code[72] = PUSH; vm.code[73] = (Instruction)&buf->pts;
    vm.code[74] = SUB; vm.code[75] = STORE; vm.code[76] = (Instruction)&delta;
    vm.code[77] = PUSH; vm.code[78] = 1;
    vm.code[79] = SUB; vm.code[80] = JZ; vm.code[81] = 95;
    vm.code[82] = END;
    vm.code[83] = PUSH; vm.code[84] = (Instruction)&s->fifo;
    vm.code[85] = STORE; vm.code[86] = (Instruction)&ret;
    vm.code[87] = PUSH; vm.code[88] = (Instruction)buf;
    vm.code[89] = DROP_FRM;
    vm.code[90] = END;
    vm_run(&vm);

    if (delta < 1) {
        AVFrame *tmp;
        int drop = av_fifo_size(s->fifo)/sizeof(AVFrame*);
        av_log(ctx, AV_LOG_DEBUG, "Dropping %d frame(s).\n", drop);
        s->drop += drop;
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