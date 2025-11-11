#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 1024
#define PROGRAM_SIZE 1024
#define EXT4_NOJOURNAL_MAX_REF_COUNT 100000

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT,
    INC_REF, DEC_REF, CHECK_REF, GET_NOJOURNAL, PUT_NOJOURNAL
};

typedef struct {
    int pc;
    int sp;
    int stack[STACK_SIZE];
    int program[PROGRAM_SIZE];
} VM;

void vm_init(VM *vm, int *program, int program_size) {
    vm->pc = 0;
    vm->sp = -1;
    memcpy(vm->program, program, program_size * sizeof(int));
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm) {
    int running = 1;
    while (running) {
        int instr = vm->program[vm->pc++];
        switch (instr) {
            case PUSH: {
                int value = vm->program[vm->pc++];
                vm_push(vm, value);
                break;
            }
            case POP: {
                vm_pop(vm);
                break;
            }
            case INC_REF: {
                unsigned long ref_cnt = (unsigned long)vm_pop(vm);
                if (ref_cnt >= EXT4_NOJOURNAL_MAX_REF_COUNT) {
                    fprintf(stderr, "BUG_ON: ref_cnt >= EXT4_NOJOURNAL_MAX_REF_COUNT\n");
                    exit(1);
                }
                ref_cnt++;
                vm_push(vm, (int)ref_cnt);
                break;
            }
            case DEC_REF: {
                unsigned long ref_cnt = (unsigned long)vm_pop(vm);
                if (ref_cnt == 0) {
                    fprintf(stderr, "BUG_ON: ref_cnt == 0\n");
                    exit(1);
                }
                ref_cnt--;
                vm_push(vm, (int)ref_cnt);
                break;
            }
            case GET_NOJOURNAL: {
                vm_push(vm, vm_pop(vm) + 1);
                break;
            }
            case PUT_NOJOURNAL: {
                vm_push(vm, vm_pop(vm) - 1);
                break;
            }
            case HALT:
                running = 0;
                break;
        }
    }
}

int main() {
    int program[] = {
        PUSH, 0, // Initialize handle
        GET_NOJOURNAL,
        PUSH, EXT4_NOJOURNAL_MAX_REF_COUNT,
        INC_REF,
        DEC_REF,
        HALT
    };
    
    VM vm;
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_run(&vm);

    return 0;
}