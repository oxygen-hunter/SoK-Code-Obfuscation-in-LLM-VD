#include "gdevprn.h"

#define STACK_SIZE 1024
#define PROGRAM_SIZE 2048

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
    unsigned char program[PROGRAM_SIZE];
} VM;

enum {
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
};

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
    while (1) {
        int op = vm->program[vm->pc++];
        switch (op) {
        case OP_PUSH:
            vm_push(vm, vm->program[vm->pc++]);
            break;
        case OP_POP:
            vm_pop(vm);
            break;
        case OP_ADD: {
            int b = vm_pop(vm);
            int a = vm_pop(vm);
            vm_push(vm, a + b);
        } break;
        case OP_SUB: {
            int b = vm_pop(vm);
            int a = vm_pop(vm);
            vm_push(vm, a - b);
        } break;
        case OP_JMP:
            vm->pc = vm->program[vm->pc];
            break;
        case OP_JZ: {
            int target = vm->program[vm->pc++];
            int condition = vm_pop(vm);
            if (condition == 0) {
                vm->pc = target;
            }
        } break;
        case OP_LOAD:
            vm_push(vm, vm->stack[vm->program[vm->pc++]]);
            break;
        case OP_STORE:
            vm->stack[vm->program[vm->pc++]] = vm_pop(vm);
            break;
        case OP_CALL: {
            int target = vm->program[vm->pc++];
            vm_push(vm, vm->pc);
            vm->pc = target;
        } break;
        case OP_RET:
            vm->pc = vm_pop(vm);
            break;
        case OP_HALT:
            return;
        }
    }
}

static void setup_program(VM *vm) {
    unsigned char program[] = {
        OP_PUSH, BLACK, OP_PUSH, VIOLET, OP_PUSH, GREEN, OP_PUSH, CYAN,
        OP_PUSH, RED, OP_PUSH, MAGENTA, OP_PUSH, YELLOW, OP_PUSH, WHITE,
        OP_HALT
    };
    int program_size = sizeof(program) / sizeof(program[0]);
    for (int i = 0; i < program_size; ++i) {
        vm->program[i] = program[i];
    }
}

int main() {
    VM vm;
    vm_init(&vm);
    setup_program(&vm);
    vm_run(&vm);
    return 0;
}