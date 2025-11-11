#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STACK_SIZE 1024

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, PRINT
} Instruction;

typedef struct {
    int stack[MAX_STACK_SIZE];
    int sp;
    int pc;
    int running;
    Instruction program[256];
    int program_length;
    int memory[256];
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
    memset(vm->stack, 0, sizeof(vm->stack));
    memset(vm->memory, 0, sizeof(vm->memory));
}

void vm_push(VM *vm, int value) {
    if (vm->sp < MAX_STACK_SIZE - 1) {
        vm->stack[++vm->sp] = value;
    } else {
        fprintf(stderr, "Stack overflow\n");
        exit(1);
    }
}

int vm_pop(VM *vm) {
    if (vm->sp >= 0) {
        return vm->stack[vm->sp--];
    } else {
        fprintf(stderr, "Stack underflow\n");
        exit(1);
    }
}

void vm_run(VM *vm) {
    while (vm->running) {
        switch (vm->program[vm->pc]) {
            case PUSH:
                vm->pc++;
                vm_push(vm, vm->program[vm->pc]);
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
                vm->pc = vm->program[++vm->pc] - 1;
                break;
            case JZ: {
                int addr = vm->program[++vm->pc];
                if (vm_pop(vm) == 0) {
                    vm->pc = addr - 1;
                }
                break;
            }
            case LOAD:
                vm->pc++;
                vm_push(vm, vm->memory[vm->program[vm->pc]]);
                break;
            case STORE:
                vm->pc++;
                vm->memory[vm->program[vm->pc]] = vm_pop(vm);
                break;
            case PRINT:
                printf("Output: %d\n", vm_pop(vm));
                break;
            default:
                vm->running = 0;
                break;
        }
        vm->pc++;
    }
}

void load_program(VM *vm) {
    vm->program_length = 0;

    vm->program[vm->program_length++] = PUSH;
    vm->program[vm->program_length++] = 5;

    vm->program[vm->program_length++] = PUSH;
    vm->program[vm->program_length++] = 3;

    vm->program[vm->program_length++] = ADD;
    
    vm->program[vm->program_length++] = PRINT;

    vm->program[vm->program_length++] = PUSH;
    vm->program[vm->program_length++] = 7;

    vm->program[vm->program_length++] = PUSH;
    vm->program[vm->program_length++] = 2;

    vm->program[vm->program_length++] = SUB;

    vm->program[vm->program_length++] = PRINT;
}

int main() {
    VM vm;
    vm_init(&vm);
    load_program(&vm);
    vm_run(&vm);
    return 0;
}