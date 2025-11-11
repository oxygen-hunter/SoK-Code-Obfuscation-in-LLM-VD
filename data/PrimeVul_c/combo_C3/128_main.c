#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"

enum {
    OP_PUSH, OP_POP, OP_ADD, OP_SUB, OP_JMP, OP_JZ, OP_LOAD, OP_STORE, OP_PRINT, OP_EXEC, OP_HALT
};

#define STACK_SIZE 256
#define PROGRAM_SIZE 1024

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
    int program[PROGRAM_SIZE];
} VM;

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
    while(1) {
        int opcode = vm->program[vm->pc++];
        switch(opcode) {
            case OP_PUSH:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case OP_POP:
                vm_pop(vm);
                break;
            case OP_ADD:
                vm_push(vm, vm_pop(vm) + vm_pop(vm));
                break;
            case OP_SUB:
                {
                    int b = vm_pop(vm);
                    int a = vm_pop(vm);
                    vm_push(vm, a - b);
                }
                break;
            case OP_JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case OP_JZ:
                if(vm_pop(vm) == 0) {
                    vm->pc = vm->program[vm->pc];
                } else {
                    vm->pc++;
                }
                break;
            case OP_LOAD:
                vm_push(vm, vm->program[vm->program[vm->pc++]]);
                break;
            case OP_STORE:
                vm->program[vm->program[vm->pc++]] = vm_pop(vm);
                break;
            case OP_PRINT:
                fprintf(stderr, "%d\n", vm_pop(vm));
                break;
            case OP_EXEC:
                {
                    char *args[] = { (char *)vm_pop(vm), NULL };
                    execvp(args[0], args);
                }
                break;
            case OP_HALT:
                return;
        }
    }
}

int main(int argc, char *argv[]) {
    VM vm;
    vm_init(&vm);

    int program[] = {
        OP_PUSH, (int)argv[0],
        OP_STORE, 0,
        OP_PUSH, (int)argv,
        OP_STORE, 1,
        OP_PUSH, 1, 
        OP_PUSH, argc,
        OP_SUB,
        OP_PUSH, 23,
        OP_JZ,
        OP_PUSH, 2,
        OP_ADD,
        OP_LOAD, 1,
        OP_LOAD, 2,
        OP_PUSH, '-',
        OP_ADD,
        OP_PUSH, 'q',
        OP_ADD,
        OP_PUSH, 17,
        OP_JZ,
        OP_PUSH, 'f',
        OP_ADD,
        OP_PUSH, 17,
        OP_JZ,
        OP_HALT,
        OP_LOAD, 0,
        OP_EXEC,
        OP_HALT
    };

    memcpy(vm.program, program, sizeof(program));
    vm_run(&vm);
    
    return EXIT_FAILURE;
}