#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common/compat_strlcpy.h"
#include "common/compat_strlcat.h"
#include "internal.h"
#include "opensc.h"
#include "pkcs15.h"
#include "esteid.h"

#define VM_STACK_SIZE 512
#define VM_PROG_SIZE 1024

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Opcode;

typedef struct {
    int stack[VM_STACK_SIZE];
    int sp;
    int pc;
    int running;
    int program[VM_PROG_SIZE];
} VM;

VM vm;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void vm_push(VM *vm, int val) {
    if (vm->sp < VM_STACK_SIZE - 1) {
        vm->stack[++vm->sp] = val;
    }
}

int vm_pop(VM *vm) {
    if (vm->sp >= 0) {
        return vm->stack[vm->sp--];
    }
    return 0;
}

void vm_execute(VM *vm) {
    while (vm->running) {
        switch (vm->program[vm->pc++]) {
            case PUSH:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case JZ: {
                int addr = vm->program[vm->pc++];
                if (vm_pop(vm) == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case LOAD:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case STORE: {
                int addr = vm->program[vm->pc++];
                vm->program[addr] = vm_pop(vm);
                break;
            }
            case CALL: {
                int addr = vm->program[vm->pc++];
                vm_push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case RET:
                vm->pc = vm_pop(vm);
                break;
            case HALT:
                vm->running = 0;
                break;
            default:
                break;
        }
    }
}

void compile_to_vm(VM *vm) {
    int idx = 0;
    vm->program[idx++] = PUSH;
    vm->program[idx++] = (int) "ID-kaart";
    vm->program[idx++] = CALL;
    vm->program[idx++] = 100; // set_string call
    vm->program[idx++] = PUSH;
    vm->program[idx++] = (int) "AS Sertifitseerimiskeskus";
    vm->program[idx++] = CALL;
    vm->program[idx++] = 100; // set_string call
    vm->program[idx++] = HALT;

    vm->program[100] = LOAD;
    vm->program[101] = 0; // Load string
    vm->program[102] = STORE;
    vm->program[103] = 1; // Store to strp
    vm->program[104] = RET;
}

void set_string(char **strp, const char *value) {
    if (*strp) free(*strp);
    *strp = value ? strdup(value) : NULL;
}

int main() {
    vm_init(&vm);
    compile_to_vm(&vm);
    vm_execute(&vm);
    return 0;
}