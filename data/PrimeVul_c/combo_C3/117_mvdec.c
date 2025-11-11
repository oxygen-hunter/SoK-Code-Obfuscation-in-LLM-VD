#include "libavutil/channel_layout.h"
#include "libavutil/eval.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/rational.h"

#include "avformat.h"
#include "internal.h"

typedef struct MvContext {
    int nb_video_tracks;
    int nb_audio_tracks;

    int eof_count;
    int stream_index;
    int frame[2];

    int acompression;
    int aformat;
} MvContext;

#define AUDIO_FORMAT_SIGNED 401

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
} Instruction;

typedef struct VM {
    int stack[256];
    int sp;
    int ip;
    int program[1024];
} VM;

static void vm_init(VM *vm) {
    vm->sp = -1;
    vm->ip = 0;
}

static void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

static int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

static void vm_run(VM *vm) {
    int running = 1;
    while (running) {
        int instr = vm->program[vm->ip++];
        switch (instr) {
            case PUSH: {
                int value = vm->program[vm->ip++];
                vm_push(vm, value);
                break;
            }
            case POP: {
                vm_pop(vm);
                break;
            }
            case ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP: {
                int addr = vm->program[vm->ip++];
                vm->ip = addr;
                break;
            }
            case JZ: {
                int addr = vm->program[vm->ip++];
                if (vm_pop(vm) == 0) {
                    vm->ip = addr;
                }
                break;
            }
            case LOAD: {
                int index = vm->program[vm->ip++];
                vm_push(vm, vm->stack[index]);
                break;
            }
            case STORE: {
                int index = vm->program[vm->ip++];
                vm->stack[index] = vm_pop(vm);
                break;
            }
            case CALL: {
                int addr = vm->program[vm->ip++];
                vm_push(vm, vm->ip);
                vm->ip = addr;
                break;
            }
            case RET: {
                vm->ip = vm_pop(vm);
                break;
            }
            default: {
                running = 0;
                break;
            }
        }
    }
}

static int mv_probe(AVProbeData *p) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = AV_RB32(p->buf);
    vm.program[2] = PUSH;
    vm.program[3] = MKBETAG('M', 'O', 'V', 'I');
    vm.program[4] = SUB;
    vm.program[5] = JZ;
    vm.program[6] = 10;
    vm.program[7] = PUSH;
    vm.program[8] = AV_RB16(p->buf + 4);
    vm.program[9] = PUSH;
    vm.program[10] = 3;
    vm.program[11] = SUB;
    vm.program[12] = JZ;
    vm.program[13] = 17;
    vm.program[14] = PUSH;
    vm.program[15] = 0;
    vm.program[16] = RET;
    vm.program[17] = PUSH;
    vm.program[18] = AVPROBE_SCORE_MAX;
    vm.program[19] = RET;
    vm_run(&vm);
    return vm_pop(&vm);
}

static char *var_read_string(AVIOContext *pb, int size) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = size;
    vm.program[2] = PUSH;
    vm.program[3] = 0;
    vm.program[4] = SUB;
    vm.program[5] = JZ;
    vm.program[6] = 12;
    vm.program[7] = PUSH;
    vm.program[8] = size;
    vm.program[9] = PUSH;
    vm.program[10] = INT_MAX;
    vm.program[11] = SUB;
    vm.program[12] = JZ;
    vm.program[13] = 17;
    vm.program[14] = PUSH;
    vm.program[15] = 0;
    vm.program[16] = RET;
    vm.program[17] = CALL;
    vm.program[18] = 20;
    vm.program[19] = RET;
    vm.program[20] = PUSH;
    vm.program[21] = size;
    vm.program[22] = PUSH;
    vm.program[23] = 1;
    vm.program[24] = ADD;
    vm.program[25] = STORE;
    vm.program[26] = vm.sp;
    vm.program[27] = PUSH;
    vm.program[28] = 0;
    vm.program[29] = RET;
    vm_run(&vm);
    int n = vm_pop(&vm);
    if (n < size) {
        avio_skip(pb, size - n);
    }
    return (char *)vm_pop(&vm);
}

static int var_read_int(AVIOContext *pb, int size) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = CALL;
    vm.program[1] = 3;
    vm.program[2] = PUSH;
    vm.program[3] = 0;
    vm.program[4] = RET;
    vm.program[5] = PUSH;
    vm.program[6] = 10;
    vm.program[7] = RET;
    vm.program[8] = CALL;
    vm.program[9] = 20;
    vm.program[10] = RET;
    vm_run(&vm);
    return vm_pop(&vm);
}

static AVRational var_read_float(AVIOContext *pb, int size) {
    AVRational v = {0, 0};
    VM vm;
    vm_init(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = size;
    vm.program[2] = CALL;
    vm.program[3] = 4;
    vm.program[4] = PUSH;
    vm.program[5] = 0;
    vm.program[6] = RET;
    vm.program[7] = CALL;
    vm.program[8] = 20;
    vm.program[9] = RET;
    vm.program[10] = vm.sp;
    vm.program[11] = PUSH;
    vm.program[12] = 0;
    vm.program[13] = RET;
    vm.program[14] = CALL;
    vm.program[15] = 20;
    vm.program[16] = RET;
    vm_run(&vm);
    return (AVRational)vm_pop(&vm);
}

static void var_read_metadata(AVFormatContext *avctx, const char *tag, int size) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = size;
    vm.program[2] = CALL;
    vm.program[3] = 4;
    vm.program[4] = PUSH;
    vm.program[5] = 0;
    vm.program[6] = RET;
    vm.program[7] = CALL;
    vm.program[8] = 20;
    vm.program[9] = RET;
    vm.program[10] = vm.sp;
    vm.program[11] = PUSH;
    vm.program[12] = 0;
    vm.program[13] = RET;
    vm.program[14] = CALL;
    vm.program[15] = 20;
    vm.program[16] = RET;
    vm_run(&vm);
}

static int set_channels(AVFormatContext *avctx, AVStream *st, int channels) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = channels;
    vm.program[2] = PUSH;
    vm.program[3] = 0;
    vm.program[4] = SUB;
    vm.program[5] = JZ;
    vm.program[6] = 10;
    vm.program[7] = CALL;
    vm.program[8] = 3;
    vm.program[9] = RET;
    vm.program[10] = PUSH;
    vm.program[11] = 0;
    vm.program[12] = RET;
    vm_run(&vm);
    return vm_pop(&vm);
}

static int parse_global_var(AVFormatContext *avctx, AVStream *st, const char *name, int size) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = name;
    vm.program[2] = CALL;
    vm.program[3] = 4;
    vm.program[4] = PUSH;
    vm.program[5] = 0;
    vm.program[6] = RET;
    vm.program[7] = CALL;
    vm.program[8] = 20;
    vm.program[9] = RET;
    vm.program[10] = vm.sp;
    vm.program[11] = PUSH;
    vm.program[12] = 0;
    vm.program[13] = RET;
    vm.program[14] = CALL;
    vm.program[15] = 20;
    vm.program[16] = RET;
    vm_run(&vm);
    return vm_pop(&vm);
}

static int parse_audio_var(AVFormatContext *avctx, AVStream *st, const char *name, int size) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = name;
    vm.program[2] = CALL;
    vm.program[3] = 4;
    vm.program[4] = PUSH;
    vm.program[5] = 0;
    vm.program[6] = RET;
    vm.program[7] = CALL;
    vm.program[8] = 20;
    vm.program[9] = RET;
    vm.program[10] = vm.sp;
    vm.program[11] = PUSH;
    vm.program[12] = 0;
    vm.program[13] = RET;
    vm.program[14] = CALL;
    vm.program[15] = 20;
    vm.program[16] = RET;
    vm_run(&vm);
    return vm_pop(&vm);
}

static int parse_video_var(AVFormatContext *avctx, AVStream *st, const char *name, int size) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = name;
    vm.program[2] = CALL;
    vm.program[3] = 4;
    vm.program[4] = PUSH;
    vm.program[5] = 0;
    vm.program[6] = RET;
    vm.program[7] = CALL;
    vm.program[8] = 20;
    vm.program[9] = RET;
    vm.program[10] = vm.sp;
    vm.program[11] = PUSH;
    vm.program[12] = 0;
    vm.program[13] = RET;
    vm.program[14] = CALL;
    vm.program[15] = 20;
    vm.program[16] = RET;
    vm_run(&vm);
    return vm_pop(&vm);
}

static int read_table(AVFormatContext *avctx, AVStream *st, int (*parse)(AVFormatContext *avctx, AVStream *st, const char *name, int size)) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = CALL;
    vm.program[1] = 4;
    vm.program[2] = PUSH;
    vm.program[3] = 0;
    vm.program[4] = RET;
    vm.program[5] = CALL;
    vm.program[6] = 20;
    vm.program[7] = RET;
    vm.program[8] = vm.sp;
    vm.program[9] = PUSH;
    vm.program[10] = 0;
    vm.program[11] = RET;
    vm.program[12] = CALL;
    vm.program[13] = 20;
    vm.program[14] = RET;
    vm_run(&vm);
    return vm_pop(&vm);
}

static void read_index(AVIOContext *pb, AVStream *st) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = CALL;
    vm.program[1] = 4;
    vm.program[2] = PUSH;
    vm.program[3] = 0;
    vm.program[4] = RET;
    vm.program[5] = CALL;
    vm.program[6] = 20;
    vm.program[7] = RET;
    vm.program[8] = vm.sp;
    vm.program[9] = PUSH;
    vm.program[10] = 0;
    vm.program[11] = RET;
    vm.program[12] = CALL;
    vm.program[13] = 20;
    vm.program[14] = RET;
    vm_run(&vm);
}

static int mv_read_header(AVFormatContext *avctx) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = CALL;
    vm.program[1] = 4;
    vm.program[2] = PUSH;
    vm.program[3] = 0;
    vm.program[4] = RET;
    vm.program[5] = CALL;
    vm.program[6] = 20;
    vm.program[7] = RET;
    vm.program[8] = vm.sp;
    vm.program[9] = PUSH;
    vm.program[10] = 0;
    vm.program[11] = RET;
    vm.program[12] = CALL;
    vm.program[13] = 20;
    vm.program[14] = RET;
    vm_run(&vm);
    return vm_pop(&vm);
}

static int mv_read_packet(AVFormatContext *avctx, AVPacket *pkt) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = CALL;
    vm.program[1] = 4;
    vm.program[2] = PUSH;
    vm.program[3] = 0;
    vm.program[4] = RET;
    vm.program[5] = CALL;
    vm.program[6] = 20;
    vm.program[7] = RET;
    vm.program[8] = vm.sp;
    vm.program[9] = PUSH;
    vm.program[10] = 0;
    vm.program[11] = RET;
    vm.program[12] = CALL;
    vm.program[13] = 20;
    vm.program[14] = RET;
    vm_run(&vm);
    return vm_pop(&vm);
}

static int mv_read_seek(AVFormatContext *avctx, int stream_index, int64_t timestamp, int flags) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = CALL;
    vm.program[1] = 4;
    vm.program[2] = PUSH;
    vm.program[3] = 0;
    vm.program[4] = RET;
    vm.program[5] = CALL;
    vm.program[6] = 20;
    vm.program[7] = RET;
    vm.program[8] = vm.sp;
    vm.program[9] = PUSH;
    vm.program[10] = 0;
    vm.program[11] = RET;
    vm.program[12] = CALL;
    vm.program[13] = 20;
    vm.program[14] = RET;
    vm_run(&vm);
    return vm_pop(&vm);
}

AVInputFormat ff_mv_demuxer = {
    .name           = "mv",
    .long_name      = NULL_IF_CONFIG_SMALL("Silicon Graphics Movie"),
    .priv_data_size = sizeof(MvContext),
    .read_probe     = mv_probe,
    .read_header    = mv_read_header,
    .read_packet    = mv_read_packet,
    .read_seek      = mv_read_seek,
};