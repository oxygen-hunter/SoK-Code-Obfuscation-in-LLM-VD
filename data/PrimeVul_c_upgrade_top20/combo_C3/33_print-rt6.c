#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 256
#define MEMORY_SIZE 1024

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
};

typedef struct {
    int stack[STACK_SIZE];
    int sp;  // Stack pointer
    int memory[MEMORY_SIZE];
    int pc;  // Program counter
} VM;

void execute(VM *vm, int *program) {
    vm->sp = -1;
    vm->pc = 0;
    int running = 1;

    while (running) {
        int opcode = program[vm->pc++];
        switch (opcode) {
            case PUSH:
                vm->stack[++vm->sp] = program[vm->pc++];
                break;
            case POP:
                vm->sp--;
                break;
            case ADD:
                vm->stack[vm->sp - 1] += vm->stack[vm->sp];
                vm->sp--;
                break;
            case SUB:
                vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
                vm->sp--;
                break;
            case JMP:
                vm->pc = program[vm->pc];
                break;
            case JZ:
                if (vm->stack[vm->sp--] == 0)
                    vm->pc = program[vm->pc];
                else
                    vm->pc++;
                break;
            case LOAD:
                vm->stack[++vm->sp] = vm->memory[program[vm->pc++]];
                break;
            case STORE:
                vm->memory[program[vm->pc++]] = vm->stack[vm->sp--];
                break;
            case HALT:
                running = 0;
                break;
        }
    }
}

int main() {
    VM vm;
    memset(&vm, 0, sizeof(VM));

    // Compiled instruction sequence for the original logic
    int program[] = {
        PUSH, 10,       // Example initialization
        STORE, 0,       // Store in memory
        LOAD, 0,        // Load from memory
        PUSH, 1,
        ADD,
        STORE, 0,       // Store result
        LOAD, 0,
        PUSH, 11,
        SUB,
        JZ, 16,         // Skip if zero
        JMP, 2,         // Loop back
        HALT
    };

    execute(&vm, program);
    printf("Final result in memory[0]: %d\n", vm.memory[0]);

    return 0;
}