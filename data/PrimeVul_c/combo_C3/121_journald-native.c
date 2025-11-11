#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/statvfs.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

typedef struct {
    int opcode;
    int operand;
} Instruction;

enum {
    I_PUSH,
    I_POP,
    I_ADD,
    I_SUB,
    I_JMP,
    I_JZ,
    I_LOAD,
    I_STORE,
    I_END
};

typedef struct {
    int *stack;
    int stack_size;
    int pc;
    Instruction *instructions;
    int *registers;
} VM;

void vm_init(VM *vm, int stack_size, Instruction *instructions) {
    vm->stack = (int *)malloc(stack_size * sizeof(int));
    vm->stack_size = stack_size;
    vm->pc = 0;
    vm->instructions = instructions;
    vm->registers = (int *)calloc(10, sizeof(int));
}

void vm_free(VM *vm) {
    free(vm->stack);
    free(vm->registers);
}

void vm_run(VM *vm) {
    int sp = -1;
    while (1) {
        Instruction inst = vm->instructions[vm->pc++];
        switch (inst.opcode) {
            case I_PUSH:
                vm->stack[++sp] = inst.operand;
                break;
            case I_POP:
                --sp;
                break;
            case I_ADD:
                vm->stack[sp - 1] += vm->stack[sp];
                --sp;
                break;
            case I_SUB:
                vm->stack[sp - 1] -= vm->stack[sp];
                --sp;
                break;
            case I_JMP:
                vm->pc = inst.operand;
                break;
            case I_JZ:
                if (vm->stack[sp--] == 0)
                    vm->pc = inst.operand;
                break;
            case I_LOAD:
                vm->stack[++sp] = vm->registers[inst.operand];
                break;
            case I_STORE:
                vm->registers[inst.operand] = vm->stack[sp--];
                break;
            case I_END:
                return;
        }
    }
}

Instruction program[] = {
    { I_PUSH, 10 },
    { I_STORE, 0 },
    { I_LOAD, 0 },
    { I_PUSH, 1 },
    { I_SUB, 0 },
    { I_STORE, 0 },
    { I_LOAD, 0 },
    { I_JZ, 9 },
    { I_JMP, 2 },
    { I_END, 0 }
};

int main() {
    VM vm;
    vm_init(&vm, 100, program);
    vm_run(&vm);
    vm_free(&vm);
    return 0;
}