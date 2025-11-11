#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include "jasper/jas_malloc.h"
#include "jasper/jas_debug.h"
#include "jpc_bs.h"

enum {
    VM_PUSH, VM_POP, VM_ADD, VM_SUB, VM_JMP, VM_JZ, VM_LOAD, VM_STORE, VM_HALT
};

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int memory[256];
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_execute(VM *vm, int *program) {
    int running = 1;
    while (running) {
        int instr = program[vm->pc++];
        switch (instr) {
            case VM_PUSH:
                vm->stack[++vm->sp] = program[vm->pc++];
                break;
            case VM_POP:
                vm->sp--;
                break;
            case VM_ADD:
                vm->stack[vm->sp - 1] += vm->stack[vm->sp];
                vm->sp--;
                break;
            case VM_SUB:
                vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
                vm->sp--;
                break;
            case VM_JMP:
                vm->pc = program[vm->pc];
                break;
            case VM_JZ:
                if (vm->stack[vm->sp--] == 0) {
                    vm->pc = program[vm->pc];
                } else {
                    vm->pc++;
                }
                break;
            case VM_LOAD:
                vm->stack[++vm->sp] = vm->memory[program[vm->pc++]];
                break;
            case VM_STORE:
                vm->memory[program[vm->pc++]] = vm->stack[vm->sp--];
                break;
            case VM_HALT:
                running = 0;
                break;
        }
    }
}

jpc_bitstream_t *jpc_bitstream_sopen(jas_stream_t *stream, char *mode) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        VM_PUSH, (int) stream,
        VM_PUSH, (int) mode,
        VM_PUSH, (int) jpc_bitstream_alloc,
        VM_CALL,
        VM_HALT
    };
    vm_execute(&vm, program);
    return (jpc_bitstream_t *) vm.stack[vm.sp];
}

int jpc_bitstream_close(jpc_bitstream_t *bitstream) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        VM_PUSH, (int) bitstream,
        VM_PUSH, (int) jpc_bitstream_align,
        VM_CALL,
        VM_PUSH, (int) bitstream->flags_,
        VM_PUSH, JPC_BITSTREAM_NOCLOSE,
        VM_AND,
        VM_PUSH, (int) bitstream->stream_,
        VM_PUSH, 0,
        VM_JZ, 14,
        VM_PUSH, (int) jas_stream_close,
        VM_CALL,
        VM_POP,
        VM_PUSH, (int) jas_free,
        VM_CALL,
        VM_PUSH, 0,
        VM_HALT
    };
    vm_execute(&vm, program);
    return vm.stack[vm.sp];
}

static jpc_bitstream_t *jpc_bitstream_alloc() {
    jpc_bitstream_t *bitstream = jas_malloc(sizeof(jpc_bitstream_t));
    if (bitstream) {
        bitstream->stream_ = 0;
        bitstream->cnt_ = 0;
        bitstream->flags_ = 0;
        bitstream->openmode_ = 0;
    }
    return bitstream;
}

int jpc_bitstream_getbit_func(jpc_bitstream_t *bitstream) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        VM_PUSH, (int) bitstream,
        VM_PUSH, (int) jpc_bitstream_getbit_macro,
        VM_CALL,
        VM_HALT
    };
    vm_execute(&vm, program);
    return vm.stack[vm.sp];
}

int jpc_bitstream_putbit_func(jpc_bitstream_t *bitstream, int b) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        VM_PUSH, (int) bitstream,
        VM_PUSH, b,
        VM_PUSH, (int) jpc_bitstream_putbit_macro,
        VM_CALL,
        VM_HALT
    };
    vm_execute(&vm, program);
    return vm.stack[vm.sp];
}

long jpc_bitstream_getbits(jpc_bitstream_t *bitstream, int n) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        VM_PUSH, n,
        VM_PUSH, 0,
        VM_LT,
        VM_JZ, 10,
        VM_PUSH, n,
        VM_PUSH, 32,
        VM_GE,
        VM_JZ, 17,
        VM_ABORT,
        VM_PUSH, (int) bitstream,
        VM_PUSH, (int) jpc_bitstream_getbit,
        VM_CALL,
        VM_PUSH, 0,
        VM_LT,
        VM_JZ, 29,
        VM_PUSH, -1,
        VM_HALT,
        VM_PUSH, (int) bitstream,
        VM_PUSH, n,
        VM_PUSH, (int) jpc_bitstream_getbit,
        VM_CALL,
        VM_PUSH, -1,
        VM_LT,
        VM_JZ, 40,
        VM_PUSH, -1,
        VM_HALT,
        VM_PUSH, 1,
        VM_SHL,
        VM_PUSH, (int) bitstream,
        VM_PUSH, (int) jpc_bitstream_getbit,
        VM_CALL,
        VM_OR,
        VM_HALT
    };
    vm_execute(&vm, program);
    return vm.stack[vm.sp];
}

int jpc_bitstream_putbits(jpc_bitstream_t *bitstream, int n, long v) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        VM_PUSH, n,
        VM_PUSH, 0,
        VM_LT,
        VM_JZ, 10,
        VM_PUSH, n,
        VM_PUSH, 32,
        VM_GE,
        VM_JZ, 17,
        VM_ABORT,
        VM_PUSH, v,
        VM_PUSH, (~JAS_ONES(n)),
        VM_AND,
        VM_JZ, 25,
        VM_ABORT,
        VM_PUSH, n,
        VM_PUSH, 1,
        VM_SUB,
        VM_PUSH, n,
        VM_PUSH, 0,
        VM_LT,
        VM_JZ, 45,
        VM_PUSH, (int) bitstream,
        VM_PUSH, v,
        VM_PUSH, (int) jpc_bitstream_putbit,
        VM_CALL,
        VM_PUSH, EOF,
        VM_EQ,
        VM_JZ, 65,
        VM_PUSH, EOF,
        VM_HALT,
        VM_PUSH, 1,
        VM_SHL,
        VM_HALT
    };
    vm_execute(&vm, program);
    return vm.stack[vm.sp];
}

int jpc_bitstream_fillbuf(jpc_bitstream_t *bitstream) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        VM_PUSH, (int) bitstream,
        VM_PUSH, (int) jpc_bitstream_fillbuf,
        VM_CALL,
        VM_HALT
    };
    vm_execute(&vm, program);
    return vm.stack[vm.sp];
}