#include <stdio.h>
#include <stdlib.h>

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int *program;
} VM;

void push(VM *vm, int value) {
    vm->stack[vm->sp++] = value;
}

int pop(VM *vm) {
    return vm->stack[--vm->sp];
}

void run(VM *vm) {
    int running = 1;
    while (running) {
        switch (vm->program[vm->pc++]) {
            case PUSH: {
                int value = vm->program[vm->pc++];
                push(vm, value);
                break;
            }
            case POP: {
                pop(vm);
                break;
            }
            case ADD: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a + b);
                break;
            }
            case SUB: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a - b);
                break;
            }
            case JMP: {
                vm->pc = vm->program[vm->pc];
                break;
            }
            case JZ: {
                int addr = vm->program[vm->pc++];
                if (pop(vm) == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case LOAD: {
                int addr = vm->program[vm->pc++];
                push(vm, vm->stack[addr]);
                break;
            }
            case STORE: {
                int addr = vm->program[vm->pc++];
                vm->stack[addr] = pop(vm);
                break;
            }
            case CALL: {
                int addr = vm->program[vm->pc++];
                push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case RET: {
                vm->pc = pop(vm);
                break;
            }
            case HALT: {
                running = 0;
                break;
            }
        }
    }
}

int main() {
    VM vm = { .sp = 0, .pc = 0 };
    int program[] = {
        PUSH, 2,
        PUSH, 3,
        ADD,
        PUSH, 5,
        SUB,
        HALT
    };
    vm.program = program;
    run(&vm);
    printf("Result: %d\n", pop(&vm));
    return 0;
}