#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 256
#define MEMORY_SIZE 256

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT, CALL_FUNC
} Instruction;

typedef struct {
    int stack[STACK_SIZE];
    int memory[MEMORY_SIZE];
    int sp; // stack pointer
    int pc; // program counter
    int running; // VM running state
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
    memset(vm->stack, 0, STACK_SIZE * sizeof(int));
    memset(vm->memory, 0, MEMORY_SIZE * sizeof(int));
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_execute(VM *vm, int *program) {
    while (vm->running) {
        switch (program[vm->pc++]) {
            case PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD:
                vm_push(vm, vm_pop(vm) + vm_pop(vm));
                break;
            case SUB:
                vm_push(vm, vm_pop(vm) - vm_pop(vm));
                break;
            case JMP:
                vm->pc = program[vm->pc];
                break;
            case JZ:
                if (vm_pop(vm) == 0) {
                    vm->pc = program[vm->pc];
                } else {
                    vm->pc++;
                }
                break;
            case LOAD:
                vm_push(vm, vm->memory[program[vm->pc++]]);
                break;
            case STORE:
                vm->memory[program[vm->pc++]] = vm_pop(vm);
                break;
            case HALT:
                vm->running = 0;
                break;
            case CALL_FUNC:
                // call specific functions based on function ID
                break;
            default:
                vm->running = 0;
                break;
        }
    }
}

void example_function() {
    printf("Example function executed\n");
}

int main() {
    VM vm;
    vm_init(&vm);

    int program[] = {
        PUSH, 10,
        PUSH, 20,
        ADD,
        CALL_FUNC, 0, // example_function
        HALT
    };

    void (*functions[])(void) = {example_function};

    vm_execute(&vm, program);
    return 0;
}