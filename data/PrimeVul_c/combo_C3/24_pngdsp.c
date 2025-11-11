#include "libavutil/attributes.h"
#include "libavutil/common.h"
#include "png.h"
#include "pngdsp.h"

#define pb_7f (~0UL/255 * 0x7f)
#define pb_80 (~0UL/255 * 0x80)

typedef enum {
    VM_NOP,
    VM_PUSH, 
    VM_POP,  
    VM_ADD,  
    VM_SUB,  
    VM_LOAD, 
    VM_STORE,
    VM_JMP,  
    VM_JZ,   
    VM_HALT  
} VM_Instruction;

typedef struct {
    uint8_t* stack;
    size_t sp;
    size_t pc;
} VM;

void vm_execute(VM* vm, const uint8_t* program, size_t program_size) {
    while (vm->pc < program_size) {
        switch (program[vm->pc++]) {
            case VM_NOP: break;

            case VM_PUSH:
                vm->stack[vm->sp++] = program[vm->pc++];
                break;

            case VM_POP:
                vm->sp--;
                break;

            case VM_ADD:
                vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1];
                vm->sp--;
                break;

            case VM_SUB:
                vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1];
                vm->sp--;
                break;

            case VM_LOAD:
                vm->stack[vm->sp++] = program[vm->pc++];
                break;

            case VM_STORE: {
                uint8_t value = vm->stack[--vm->sp];
                program[program[vm->pc++]] = value;
                break;
            }

            case VM_JMP:
                vm->pc = program[vm->pc];
                break;

            case VM_JZ:
                if (vm->stack[--vm->sp] == 0)
                    vm->pc = program[vm->pc];
                else
                    vm->pc++;
                break;

            case VM_HALT:
                return;
        }
    }
}

void add_bytes_l2_vm(uint8_t *dst, uint8_t *src1, uint8_t *src2, int w) {
    VM vm = { .stack = (uint8_t*)malloc(w * sizeof(uint8_t)), .sp = 0, .pc = 0 };
    uint8_t program[] = {
        VM_NOP,
        VM_LOAD, 0,
        VM_LOAD, 1,
        VM_LOAD, 2,
        VM_NOP,
        VM_HALT
    };

    for (long i = 0; i <= w - sizeof(long); i += sizeof(long)) {
        *(long *)(dst + i) = ((*(long *)(src1 + i) & pb_7f) + (*(long *)(src2 + i) & pb_7f)) ^ ((*(long *)(src1 + i) ^ *(long *)(src2 + i)) & pb_80);
    }
    for (long i = 0; i < w; i++) {
        program[1] = src1[i];
        program[2] = src2[i];
        program[3] = dst[i];
        vm_execute(&vm, program, sizeof(program));
    }
    free(vm.stack);
}

av_cold void ff_pngdsp_init(PNGDSPContext *dsp) {
    dsp->add_bytes_l2         = add_bytes_l2_vm;
    dsp->add_paeth_prediction = ff_add_png_paeth_prediction;

    if (ARCH_X86) ff_pngdsp_init_x86(dsp);
}