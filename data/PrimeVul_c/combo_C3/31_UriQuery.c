#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    OP_PUSH, OP_POP, OP_ADD, OP_SUB, OP_JZ, OP_JMP, OP_LOAD, OP_STORE, OP_HALT
} Instruction;

typedef struct {
    int *stack;
    int sp;
    int pc;
    int *program;
    int *memory;
    int halted;
} VM;

VM *create_vm(int *program) {
    VM *vm = malloc(sizeof(VM));
    vm->stack = malloc(256 * sizeof(int));
    vm->sp = -1;
    vm->pc = 0;
    vm->program = program;
    vm->memory = malloc(256 * sizeof(int));
    vm->halted = 0;
    return vm;
}

void destroy_vm(VM *vm) {
    free(vm->stack);
    free(vm->memory);
    free(vm);
}

void push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void execute(VM *vm) {
    while (!vm->halted) {
        int instr = vm->program[vm->pc++];
        switch (instr) {
            case OP_PUSH:
                push(vm, vm->program[vm->pc++]);
                break;
            case OP_POP:
                pop(vm);
                break;
            case OP_ADD: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a + b);
                break;
            }
            case OP_SUB: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a - b);
                break;
            }
            case OP_JZ: {
                int addr = vm->program[vm->pc++];
                if (pop(vm) == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case OP_JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case OP_LOAD:
                push(vm, vm->memory[vm->program[vm->pc++]]);
                break;
            case OP_STORE:
                vm->memory[vm->program[vm->pc++]] = pop(vm);
                break;
            case OP_HALT:
                vm->halted = 1;
                break;
        }
    }
}

int main() {
    int program[] = {
        OP_PUSH, 5,
        OP_PUSH, 10,
        OP_ADD,
        OP_PUSH, 1,
        OP_SUB,
        OP_HALT
    };

    VM *vm = create_vm(program);
    execute(vm);
    printf("Result: %d\n", pop(vm));
    destroy_vm(vm);
    return 0;
}