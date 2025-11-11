#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    char ckID[16];
    int64_t ckSize;
    char formType[16];
} Wave64FileHeader;

typedef struct {
    char ckID[16];
    int64_t ckSize;
} Wave64ChunkHeader;

typedef struct {
    unsigned char *instructions;
    int *stack;
    int pc;
    int sp;
} VM;

#define PUSH 0
#define POP 1
#define ADD 2
#define SUB 3
#define JMP 4
#define JZ 5
#define LOAD 6
#define STORE 7
#define HALT 8

#define STACK_SIZE 256

VM *init_vm(unsigned char *instructions) {
    VM *vm = malloc(sizeof(VM));
    vm->instructions = instructions;
    vm->stack = malloc(STACK_SIZE * sizeof(int));
    vm->pc = 0;
    vm->sp = -1;
    return vm;
}

void push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void execute(VM *vm) {
    while (1) {
        unsigned char instr = vm->instructions[vm->pc++];
        switch (instr) {
            case PUSH: {
                int value = (vm->instructions[vm->pc++] << 8) | vm->instructions[vm->pc++];
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
                int addr = (vm->instructions[vm->pc++] << 8) | vm->instructions[vm->pc++];
                vm->pc = addr;
                break;
            }
            case JZ: {
                int addr = (vm->instructions[vm->pc++] << 8) | vm->instructions[vm->pc++];
                if (pop(vm) == 0)
                    vm->pc = addr;
                break;
            }
            case LOAD: {
                int index = vm->instructions[vm->pc++];
                push(vm, vm->stack[index]);
                break;
            }
            case STORE: {
                int index = vm->instructions[vm->pc++];
                vm->stack[index] = pop(vm);
                break;
            }
            case HALT: {
                return;
            }
            default:
                break;
        }
    }
}

int main() {
    unsigned char program[] = {
        PUSH, 0x00, 0x05, // push 5
        PUSH, 0x00, 0x03, // push 3
        ADD,             // add
        HALT             // halt
    };
    
    VM *vm = init_vm(program);
    execute(vm);
    
    printf("Result: %d\n", pop(vm));
    
    free(vm->stack);
    free(vm);
    return 0;
}