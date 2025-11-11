#include "parser.h"
#include "aac_ac3_parser.h"
#include "aacadtsdec.h"
#include "get_bits.h"
#include "mpeg4audio.h"

// Define VM instruction set
typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
} Instruction;

// Define VM structure
typedef struct {
    int stack[256];
    int sp;
    int pc;
    int program[1024];
    int program_size;
} VM;

// VM functions
void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm) {
    while (vm->pc < vm->program_size) {
        switch (vm->program[vm->pc++]) {
            case PUSH:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
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
            case JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case JZ: {
                int addr = vm->program[vm->pc++];
                if (vm_pop(vm) == 0) vm->pc = addr;
                break;
            }
            case LOAD: {
                int addr = vm->program[vm->pc++];
                vm_push(vm, vm->stack[addr]);
                break;
            }
            case STORE: {
                int addr = vm->program[vm->pc++];
                vm->stack[addr] = vm_pop(vm);
                break;
            }
            case CALL: {
                int addr = vm->program[vm->pc++];
                vm_push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case RET:
                vm->pc = vm_pop(vm);
                break;
        }
    }
}

// Converted bytecode logic
void aac_sync_bytecode(VM *vm, uint64_t state, AACAC3ParseContext *hdr_info, int *need_next_header, int *new_frame_start) {
    vm->program_size = 0;
    vm->program[vm->program_size++] = PUSH;
    vm->program[vm->program_size++] = av_be2ne64(state);
    vm->program[vm->program_size++] = LOAD;
    vm->program[vm->program_size++] = 0;
    vm->program[vm->program_size++] = CALL;
    vm->program[vm->program_size++] = 10;
    vm->program[vm->program_size++] = JZ;
    vm->program[vm->program_size++] = 14;
    vm->program[vm->program_size++] = PUSH;
    vm->program[vm->program_size++] = 0;
    vm->program[vm->program_size++] = STORE;
    vm->program[vm->program_size++] = 1;
    vm->program[vm->program_size++] = PUSH;
    vm->program[vm->program_size++] = 1;
    vm->program[vm->program_size++] = STORE;
    vm->program[vm->program_size++] = 2;
    vm->program[vm->program_size++] = RET;
}

// Original logic encapsulated in VM
static int aac_sync(uint64_t state, AACAC3ParseContext *hdr_info, int *need_next_header, int *new_frame_start) {
    VM vm;
    vm_init(&vm);
    aac_sync_bytecode(&vm, state, hdr_info, need_next_header, new_frame_start);
    vm_run(&vm);
    return vm.stack[vm.sp];  // Return the size
}

static av_cold int aac_parse_init(AVCodecParserContext *s1) {
    AACAC3ParseContext *s = s1->priv_data;
    s->header_size = AAC_ADTS_HEADER_SIZE;
    s->sync = aac_sync;
    return 0;
}

AVCodecParser ff_aac_parser = {
    .codec_ids      = { AV_CODEC_ID_AAC },
    .priv_data_size = sizeof(AACAC3ParseContext),
    .parser_init    = aac_parse_init,
    .parser_parse   = ff_aac_ac3_parse,
    .parser_close   = ff_parse_close,
};