#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define STACK_SIZE 1024

typedef enum {
    OP_PUSH,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_JMP,
    OP_JZ,
    OP_LOAD,
    OP_STORE,
    OP_CMP,
    OP_HALT
} Opcode;

typedef struct {
    int64_t stack[STACK_SIZE];
    int sp;
    int pc;
    int halted;
    int64_t reg[256];
} VM;

void vm_init(VM *vm) {
    vm->sp = 0;
    vm->pc = 0;
    vm->halted = 0;
    memset(vm->reg, 0, sizeof(vm->reg));
}

void vm_push(VM *vm, int64_t value) {
    vm->stack[vm->sp++] = value;
}

int64_t vm_pop(VM *vm) {
    return vm->stack[--vm->sp];
}

void vm_run(VM *vm, int64_t *program) {
    while (!vm->halted) {
        switch (program[vm->pc++]) {
            case OP_PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case OP_POP:
                vm_pop(vm);
                break;
            case OP_ADD: {
                int64_t a = vm_pop(vm);
                int64_t b = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case OP_SUB: {
                int64_t a = vm_pop(vm);
                int64_t b = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case OP_JMP:
                vm->pc = program[vm->pc];
                break;
            case OP_JZ:
                if (vm_pop(vm) == 0) {
                    vm->pc = program[vm->pc];
                } else {
                    vm->pc++;
                }
                break;
            case OP_LOAD:
                vm_push(vm, vm->reg[program[vm->pc++]]);
                break;
            case OP_STORE:
                vm->reg[program[vm->pc++]] = vm_pop(vm);
                break;
            case OP_CMP: {
                int64_t a = vm_pop(vm);
                int64_t b = vm_pop(vm);
                vm_push(vm, a == b ? 1 : 0);
                break;
            }
            case OP_HALT:
                vm->halted = 1;
                break;
        }
    }
}

int main() {
    VM vm;
    vm_init(&vm);

    // Example program
    int64_t program[] = {
        OP_PUSH, 5,
        OP_PUSH, 10,
        OP_ADD,
        OP_STORE, 0,
        OP_LOAD, 0,
        OP_PUSH, 15,
        OP_CMP,
        OP_JZ, 15,
        OP_PUSH, 1,
        OP_STORE, 1,
        OP_HALT
    };

    vm_run(&vm, program);

    printf("Result: %lld\n", vm.reg[1]);
    return 0;
}