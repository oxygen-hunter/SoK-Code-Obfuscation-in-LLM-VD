#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define STACK_SIZE 1024

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
} OpCode;

typedef struct {
    uint8_t code[STACK_SIZE];
    int pc;
    int stack[STACK_SIZE];
    int sp;
    int memory[STACK_SIZE];
} VM;

void push(VM* vm, int value) {
    vm->stack[++vm->sp] = value;
}

int pop(VM* vm) {
    return vm->stack[vm->sp--];
}

void run(VM* vm) {
    vm->pc = 0;
    vm->sp = -1;

    while (1) {
        OpCode op = vm->code[vm->pc++];
        switch (op) {
            case PUSH: {
                int value = vm->code[vm->pc++];
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
                vm->pc = vm->code[vm->pc];
                break;
            }
            case JZ: {
                int addr = vm->code[vm->pc++];
                if (pop(vm) == 0)
                    vm->pc = addr;
                break;
            }
            case LOAD: {
                int addr = vm->code[vm->pc++];
                push(vm, vm->memory[addr]);
                break;
            }
            case STORE: {
                int addr = vm->code[vm->pc++];
                vm->memory[addr] = pop(vm);
                break;
            }
            case HALT: {
                return;
            }
        }
    }
}

void nsc_decode_vm(VM* vm) {
    // Example bytecode for a simple operation
    vm->code[0] = PUSH;
    vm->code[1] = 10; // Push 10
    vm->code[2] = PUSH;
    vm->code[3] = 20; // Push 20
    vm->code[4] = ADD; // Add
    vm->code[5] = HALT; // Halt

    run(vm);
    printf("Result: %d\n", pop(vm)); // Expect 30
}

int main() {
    VM vm;
    nsc_decode_vm(&vm);
    return 0;
}