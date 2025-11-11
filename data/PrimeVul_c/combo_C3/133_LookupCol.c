#include <stdio.h>
#include <string.h>

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} OpCode;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int running;
    int memory[256];
} VM;

void initVM(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void execute(VM *vm, int *program) {
    while (vm->running) {
        int opcode = program[vm->pc++];
        switch (opcode) {
            case PUSH: {
                int value = program[vm->pc++];
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
                int addr = program[vm->pc++];
                vm->pc = addr;
                break;
            }
            case JZ: {
                int addr = program[vm->pc++];
                if (pop(vm) == 0)
                    vm->pc = addr;
                break;
            }
            case LOAD: {
                int addr = program[vm->pc++];
                push(vm, vm->memory[addr]);
                break;
            }
            case STORE: {
                int addr = program[vm->pc++];
                vm->memory[addr] = pop(vm);
                break;
            }
            case CALL: {
                int addr = program[vm->pc++];
                push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case RET: {
                vm->pc = pop(vm);
                break;
            }
            case HALT: {
                vm->running = 0;
                break;
            }
        }
    }
}

int main() {
    int program[] = {
        PUSH, 10,
        PUSH, 20,
        ADD,
        PUSH, 30,
        SUB,
        HALT
    };

    VM vm;
    initVM(&vm);
    execute(&vm, program);
    printf("Result: %d\n", pop(&vm));

    return 0;
}