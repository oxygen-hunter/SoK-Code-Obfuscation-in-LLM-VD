#include "tif_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef NEED_LIBPORT
# include "libport.h"
#endif

#include "tiffio.h"

#define VM_STACK_SIZE 256
#define VM_PROG_SIZE 1024

typedef enum {
    VM_NOP,
    VM_PUSH,
    VM_POP,
    VM_ADD,
    VM_SUB,
    VM_JMP,
    VM_JZ,
    VM_LOAD,
    VM_STORE,
    VM_HALT
} VMInstruction;

typedef struct {
    int stack[VM_STACK_SIZE];
    int sp;
    int pc;
    int program[VM_PROG_SIZE];
} VM;

void vm_init(VM* vm) {
    vm->sp = -1;
    vm->pc = 0;
    memset(vm->program, 0, sizeof(vm->program));
}

void vm_push(VM* vm, int value) {
    if (vm->sp < VM_STACK_SIZE - 1) {
        vm->stack[++vm->sp] = value;
    }
}

int vm_pop(VM* vm) {
    return (vm->sp >= 0) ? vm->stack[vm->sp--] : 0;
}

void vm_execute(VM* vm) {
    int running = 1;
    while (running) {
        switch (vm->program[vm->pc++]) {
            case VM_NOP:
                break;
            case VM_PUSH:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case VM_POP:
                vm_pop(vm);
                break;
            case VM_ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case VM_SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case VM_JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case VM_JZ:
                if (vm_pop(vm) == 0)
                    vm->pc = vm->program[vm->pc];
                else
                    vm->pc++;
                break;
            case VM_LOAD:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case VM_STORE:
                vm->program[vm->program[vm->pc++]] = vm_pop(vm);
                break;
            case VM_HALT:
                running = 0;
                break;
            default:
                running = 0;
                break;
        }
    }
}

void load_program(VM* vm) {
    int i = 0;
    vm->program[i++] = VM_PUSH;
    vm->program[i++] = 1;
    vm->program[i++] = VM_PUSH;
    vm->program[i++] = 2;
    vm->program[i++] = VM_ADD;
    vm->program[i++] = VM_PUSH;
    vm->program[i++] = 3;
    vm->program[i++] = VM_SUB;
    vm->program[i++] = VM_HALT;
}

int main(int argc, char* argv[]) {
    VM vm;
    vm_init(&vm);
    load_program(&vm);
    vm_execute(&vm);

    return 0;
}