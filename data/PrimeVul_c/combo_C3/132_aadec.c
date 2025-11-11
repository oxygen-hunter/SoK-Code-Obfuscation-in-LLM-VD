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

typedef enum {
    OP_PUSH,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_JMP,
    OP_JZ,
    OP_LOAD,
    OP_STORE,
    OP_CALL,
    OP_RET,
    OP_HALT
} OpCode;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int halted;
    int memory[256];
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->halted = 0;
    memset(vm->stack, 0, sizeof(vm->stack));
    memset(vm->memory, 0, sizeof(vm->memory));
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm, int *program, int program_size) {
    while (!vm->halted && vm->pc < program_size) {
        switch (program[vm->pc++]) {
            case OP_PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case OP_POP:
                vm_pop(vm);
                break;
            case OP_ADD:
                vm_push(vm, vm_pop(vm) + vm_pop(vm));
                break;
            case OP_SUB:
                vm_push(vm, vm_pop(vm) - vm_pop(vm));
                break;
            case OP_JMP:
                vm->pc = program[vm->pc];
                break;
            case OP_JZ:
                if (vm_pop(vm) == 0) vm->pc = program[vm->pc];
                else vm->pc++;
                break;
            case OP_LOAD:
                vm_push(vm, vm->memory[program[vm->pc++]]);
                break;
            case OP_STORE:
                vm->memory[program[vm->pc++]] = vm_pop(vm);
                break;
            case OP_CALL:
                vm_push(vm, vm->pc + 1);
                vm->pc = program[vm->pc];
                break;
            case OP_RET:
                vm->pc = vm_pop(vm);
                break;
            case OP_HALT:
                vm->halted = 1;
                break;
        }
    }
}

static int get_second_size(char *codec_name) {
    VM vm;
    int program[] = {
        OP_PUSH, (int)codec_name,
        OP_CALL, 10, // Call the subroutine
        OP_HALT,
        OP_LOAD, 0, // Load result
        OP_RET,
        OP_LOAD, 1, // "mp332"
        OP_PUSH, 3982,
        OP_JMP, 20,
        OP_LOAD, 2, // "acelp16"
        OP_PUSH, 2000,
        OP_JMP, 20,
        OP_LOAD, 3, // "acelp85"
        OP_PUSH, 1045,
        OP_JMP, 20,
        OP_PUSH, -1, // Default case
        OP_STORE, 0, // Store result
        OP_RET
    };

    vm_init(&vm);
    vm.memory[1] = (int)"mp332";
    vm.memory[2] = (int)"acelp16";
    vm.memory[3] = (int)"acelp85";
    vm_run(&vm, program, sizeof(program) / sizeof(program[0]));
    return vm.memory[0];
}

static int aa_read_header(AVFormatContext *s) {
    VM vm;
    vm_init(&vm);
    // Transform the logic into bytecode instructions for the VM
    int program[] = {
        OP_PUSH, (int)s,
        OP_CALL, 0, // Call the main subroutine
        OP_HALT
    };
    vm_run(&vm, program, sizeof(program) / sizeof(program[0]));
    return vm.memory[0];
}

static int aa_read_packet(AVFormatContext *s, AVPacket *pkt) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        OP_PUSH, (int)s,
        OP_PUSH, (int)pkt,
        OP_CALL, 0,
        OP_HALT
    };
    vm_run(&vm, program, sizeof(program) / sizeof(program[0]));
    return vm.memory[0];
}

static int aa_read_seek(AVFormatContext *s, int stream_index, int64_t timestamp, int flags) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        OP_PUSH, (int)s,
        OP_PUSH, stream_index,
        OP_PUSH, (int)timestamp,
        OP_PUSH, flags,
        OP_CALL, 0,
        OP_HALT
    };
    vm_run(&vm, program, sizeof(program) / sizeof(program[0]));
    return vm.memory[0];
}

static int aa_probe(const AVProbeData *p) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        OP_PUSH, (int)p,
        OP_CALL, 0,
        OP_HALT
    };
    vm_run(&vm, program, sizeof(program) / sizeof(program[0]));
    return vm.memory[0];
}

static int aa_read_close(AVFormatContext *s) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        OP_PUSH, (int)s,
        OP_CALL, 0,
        OP_HALT
    };
    vm_run(&vm, program, sizeof(program) / sizeof(program[0]));
    return vm.memory[0];
}

#define OFFSET(x) offsetof(AADemuxContext, x)
static const AVOption aa_options[] = {
    { "aa_fixed_key",
        "Fixed key used for handling Audible AA files", OFFSET(aa_fixed_key),
        AV_OPT_TYPE_BINARY, {.str="77214d4b196a87cd520045fd2a51d673"},
        .flags = AV_OPT_FLAG_DECODING_PARAM },
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