#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define STACK_SIZE 1024
#define MEM_SIZE 1024

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT, CALL
};

typedef struct {
    int pc;
    int sp;
    int stack[STACK_SIZE];
    uint8_t memory[MEM_SIZE];
} VM;

int fetch(VM *vm) {
    return vm->memory[vm->pc++];
}

void push(VM *vm, int value) {
    vm->stack[vm->sp++] = value;
}

int pop(VM *vm) {
    return vm->stack[--vm->sp];
}

void execute(VM *vm) {
    int running = 1;
    while (running) {
        int instr = fetch(vm);
        switch (instr) {
            case PUSH: {
                int value = fetch(vm);
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
                int address = fetch(vm);
                vm->pc = address;
                break;
            }
            case JZ: {
                int address = fetch(vm);
                if (pop(vm) == 0) {
                    vm->pc = address;
                }
                break;
            }
            case LOAD: {
                int address = fetch(vm);
                push(vm, vm->memory[address]);
                break;
            }
            case STORE: {
                int address = fetch(vm);
                vm->memory[address] = pop(vm);
                break;
            }
            case HALT: {
                running = 0;
                break;
            }
            case CALL: {
                int address = fetch(vm);
                push(vm, vm->pc);
                vm->pc = address;
                break;
            }
        }
    }
}

void init_vm(VM *vm) {
    vm->pc = 0;
    vm->sp = 0;
    memset(vm->stack, 0, sizeof(vm->stack));
    memset(vm->memory, 0, sizeof(vm->memory));
}

void load_program(VM *vm, uint8_t *program, int size) {
    memcpy(vm->memory, program, size);
}

int main() {
    VM vm;
    init_vm(&vm);

    uint8_t program[] = {
        PUSH, 10, PUSH, 20, ADD, LOAD, 0, STORE, 1, HALT
    };

    load_program(&vm, program, sizeof(program));
    execute(&vm);

    printf("Final result: %d\n", vm.memory[1]);
    return 0;
}