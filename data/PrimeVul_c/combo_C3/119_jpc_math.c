#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdarg.h>

#define MAX_STACK_SIZE 100

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
} InstructionSet;

typedef struct {
    int stack[MAX_STACK_SIZE];
    int stackPointer;
    int programCounter;
} VM;

void vmInitialize(VM *vm) {
    vm->stackPointer = -1;
    vm->programCounter = 0;
}

void vmPush(VM *vm, int value) {
    if (vm->stackPointer < MAX_STACK_SIZE - 1) {
        vm->stack[++vm->stackPointer] = value;
    }
}

int vmPop(VM *vm) {
    if (vm->stackPointer >= 0) {
        return vm->stack[vm->stackPointer--];
    }
    return 0;
}

void vmExecute(VM *vm, int *program) {
    int running = 1;
    while (running) {
        switch (program[vm->programCounter++]) {
            case PUSH:
                vmPush(vm, program[vm->programCounter++]);
                break;
            case POP:
                vmPop(vm);
                break;
            case ADD: {
                int b = vmPop(vm);
                int a = vmPop(vm);
                vmPush(vm, a + b);
                break;
            }
            case SUB: {
                int b = vmPop(vm);
                int a = vmPop(vm);
                vmPush(vm, a - b);
                break;
            }
            case JMP:
                vm->programCounter = program[vm->programCounter];
                break;
            case JZ: {
                int address = program[vm->programCounter++];
                if (vmPop(vm) == 0) {
                    vm->programCounter = address;
                }
                break;
            }
            case LOAD:
                vmPush(vm, program[program[vm->programCounter++]]);
                break;
            case STORE: {
                int value = vmPop(vm);
                program[program[vm->programCounter++]] = value;
                break;
            }
            case HALT:
                running = 0;
                break;
        }
    }
}

int jpc_floorlog2(int x) {
    int program[] = {
        PUSH, x,
        PUSH, 0,
        LOAD, 1,
        JZ, 16,
        PUSH, 1,
        SUB,
        PUSH, 1,
        ADD,
        JMP, 2,
        POP,
        HALT
    };
    VM vm;
    vmInitialize(&vm);
    vmExecute(&vm, program);
    return vmPop(&vm);
}

int jpc_firstone(int x) {
    int program[] = {
        PUSH, x,
        PUSH, -1,
        LOAD, 1,
        JZ, 16,
        PUSH, 1,
        SUB,
        PUSH, 1,
        ADD,
        JMP, 2,
        POP,
        HALT
    };
    VM vm;
    vmInitialize(&vm);
    vmExecute(&vm, program);
    return vmPop(&vm);
}