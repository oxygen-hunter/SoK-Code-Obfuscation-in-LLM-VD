#include "libavutil/intreadwrite.h"
#include "libavcodec/bmp.h"
#include "libavutil/intfloat.h"
#include "avformat.h"
#include "internal.h"

typedef struct {
    uint64_t pts;
} CineDemuxContext;

enum {
    CC_RGB   = 0,
    CC_LEAD  = 1,
    CC_UNINT = 2
};

enum {
    CFA_NONE      = 0,
    CFA_VRI       = 1,
    CFA_VRIV6     = 2,
    CFA_BAYER     = 3,
    CFA_BAYERFLIP = 4,
};

#define CFA_TLGRAY  0x80000000U
#define CFA_TRGRAY  0x40000000U
#define CFA_BLGRAY  0x20000000U
#define CFA_BRGRAY  0x10000000U

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
} InstructionSet;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int* program;
} VM;

static int execute_vm(VM* vm) {
    for (;;) {
        switch (vm->program[vm->pc]) {
            case PUSH:
                vm->stack[vm->sp++] = vm->program[++vm->pc];
                break;
            case POP:
                --vm->sp;
                break;
            case ADD:
                vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1];
                --vm->sp;
                break;
            case SUB:
                vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1];
                --vm->sp;
                break;
            case JMP:
                vm->pc = vm->program[++vm->pc] - 1;
                break;
            case JZ:
                if (vm->stack[--vm->sp] == 0)
                    vm->pc = vm->program[++vm->pc] - 1;
                else
                    vm->pc++;
                break;
            case LOAD:
                vm->stack[vm->sp++] = vm->program[++vm->pc];
                break;
            case STORE:
                vm->program[vm->program[++vm->pc]] = vm->stack[--vm->sp];
                break;
            case HALT:
                return vm->stack[--vm->sp];
        }
        vm->pc++;
    }
}

static int cine_read_probe(AVProbeData *p) {
    int program[] = {
        LOAD, 'C', PUSH, 0, ADD, JZ, 26,
        LOAD, 'I', PUSH, 1, ADD, JZ, 26,
        LOAD, 2, LOAD, 0x2C, SUB, JZ, 26,
        LOAD, 4, PUSH, CC_UNINT, SUB, JZ, 26,
        LOAD, 6, PUSH, 1, SUB, JZ, 26,
        LOAD, 20, PUSH, 0, ADD, JZ, 26,
        LOAD, 24, PUSH, 0, ADD, JZ, 26,
        LOAD, 28, PUSH, 0, ADD, JZ, 26,
        LOAD, 32, PUSH, 0, ADD, JZ, 26,
        PUSH, AVPROBE_SCORE_MAX, HALT,
        PUSH, 0, HALT
    };
    VM vm = { .sp = 0, .pc = 0, .program = program };
    return execute_vm(&vm);
}

static int set_metadata_int(AVDictionary **dict, const char *key, int value, int allow_zero) {
    int program[] = {
        LOAD, value, JZ, 7,
        LOAD, allow_zero, JZ, 7,
        LOAD, dict, LOAD, key, LOAD, value, PUSH, 0, ADD, HALT,
        PUSH, 0, HALT
    };
    VM vm = { .sp = 0, .pc = 0, .program = program };
    return execute_vm(&vm);
}

static int set_metadata_float(AVDictionary **dict, const char *key, float value, int allow_zero) {
    int program[] = {
        LOAD, *(int*)&value, JZ, 7,
        LOAD, allow_zero, JZ, 7,
        LOAD, dict, LOAD, key, LOAD, value, PUSH, 0, ADD, HALT,
        PUSH, 0, HALT
    };
    VM vm = { .sp = 0, .pc = 0, .program = program };
    return execute_vm(&vm);
}

static int cine_read_header(AVFormatContext *avctx) {
    int program[] = {
        LOAD, avctx, LOAD, sizeof(CineDemuxContext), PUSH, sizeof(AVStream), ADD, HALT
    };
    VM vm = { .sp = 0, .pc = 0, .program = program };
    return execute_vm(&vm);
}

static int cine_read_packet(AVFormatContext *avctx, AVPacket *pkt) {
    CineDemuxContext *cine = avctx->priv_data;
    AVStream *st = avctx->streams[0];
    AVIOContext *pb = avctx->pb;
    int n, size, ret;

    if (cine->pts >= st->duration)
        return AVERROR_EOF;

    avio_seek(pb, st->index_entries[cine->pts].pos, SEEK_SET);
    n = avio_rl32(pb);
    if (n < 8)
        return AVERROR_INVALIDDATA;
    avio_skip(pb, n - 8);
    size = avio_rl32(pb);

    ret = av_get_packet(pb, pkt, size);
    if (ret < 0)
        return ret;

    pkt->pts = cine->pts++;
    pkt->stream_index = 0;
    pkt->flags |= AV_PKT_FLAG_KEY;
    return 0;
}

static int cine_read_seek(AVFormatContext *avctx, int stream_index, int64_t timestamp, int flags) {
    CineDemuxContext *cine = avctx->priv_data;

    if ((flags & AVSEEK_FLAG_FRAME) || (flags & AVSEEK_FLAG_BYTE))
        return AVERROR(ENOSYS);

    if (!(avctx->pb->seekable & AVIO_SEEKABLE_NORMAL))
        return AVERROR(EIO);

    cine->pts = timestamp;
    return 0;
}

AVInputFormat ff_cine_demuxer = {
    .name           = "cine",
    .long_name      = NULL_IF_CONFIG_SMALL("Phantom Cine"),
    .priv_data_size = sizeof(CineDemuxContext),
    .read_probe     = cine_read_probe,
    .read_header    = cine_read_header,
    .read_packet    = cine_read_packet,
    .read_seek      = cine_read_seek,
};