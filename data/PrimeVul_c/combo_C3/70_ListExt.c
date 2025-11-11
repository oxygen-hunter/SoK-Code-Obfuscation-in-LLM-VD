#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#include <limits.h>

typedef enum {
    VM_NOP,
    VM_PUSH,
    VM_POP,
    VM_LOAD,
    VM_STORE,
    VM_ADD,
    VM_SUB,
    VM_JMP,
    VM_JZ,
    VM_CALL,
    VM_RET,
    VM_HALT
} VM_Opcode;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int running;
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_exec(VM *vm, int *program) {
    while (vm->running) {
        switch (program[vm->pc++]) {
            case VM_NOP:
                break;
            case VM_PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case VM_POP:
                vm_pop(vm);
                break;
            case VM_LOAD: {
                int addr = program[vm->pc++];
                vm_push(vm, vm->stack[addr]);
                break;
            }
            case VM_STORE: {
                int addr = program[vm->pc++];
                vm->stack[addr] = vm_pop(vm);
                break;
            }
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
                vm->pc = program[vm->pc];
                break;
            case VM_JZ: {
                int addr = program[vm->pc++];
                if (vm_pop(vm) == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case VM_CALL: {
                int addr = program[vm->pc++];
                vm_push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case VM_RET:
                vm->pc = vm_pop(vm);
                break;
            case VM_HALT:
                vm->running = 0;
                break;
            default:
                vm->running = 0;
                break;
        }
    }
}

char **vm_XListExtensions(Display *dpy, int *nextensions) {
    VM vm;
    vm_init(&vm);
    
    static int program[] = {
        VM_PUSH, 0,
        VM_STORE, 0,
        VM_CALL, 10,
        VM_HALT,
        VM_PUSH, (int)dpy,
        VM_PUSH, (int)nextensions,
        VM_CALL, 20,
        VM_RET,
        VM_PUSH, (int)dpy,
        VM_CALL, 30,
        VM_RET
    };
    
    vm_exec(&vm, program);
    return (char **)vm_pop(&vm);
}

int vm_XFreeExtensionList(char **list) {
    VM vm;
    vm_init(&vm);

    static int program[] = {
        VM_PUSH, (int)list,
        VM_CALL, 10,
        VM_HALT,
        VM_PUSH, (int)list,
        VM_CALL, 20,
        VM_RET
    };

    vm_exec(&vm, program);
    return vm_pop(&vm);
}