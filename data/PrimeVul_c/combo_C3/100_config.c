#include "util.h"
#include "cache.h"
#include "exec_cmd.h"

#define MAXNAME (256)
#define DEBUG_CACHE_DIR ".debug"
#define MAXPATHLEN (1024)

// Define the instruction set
typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

// Virtual Machine Structure
typedef struct {
    int stack[1024];
    int sp; // stack pointer
    int pc; // program counter
    int running;
} VM;

// Initialize the VM
void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void vm_run(VM *vm, Instruction *program) {
    while (vm->running) {
        switch (program[vm->pc]) {
            case PUSH:
                vm->sp++;
                vm->stack[vm->sp] = program[++vm->pc];
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
                vm->pc = program[++vm->pc] - 1;
                break;
            case JZ:
                if (vm->stack[vm->sp--] == 0)
                    vm->pc = program[++vm->pc] - 1;
                else
                    vm->pc++;
                break;
            case LOAD:
                vm->sp++;
                vm->stack[vm->sp] = vm->stack[program[++vm->pc]];
                break;
            case STORE:
                vm->stack[program[++vm->pc]] = vm->stack[vm->sp--];
                break;
            case CALL:
                vm->stack[++vm->sp] = vm->pc + 2;
                vm->pc = program[++vm->pc] - 1;
                break;
            case RET:
                vm->pc = vm->stack[vm->sp--] - 1;
                break;
            case HALT:
                vm->running = 0;
                break;
        }
        vm->pc++;
    }
}

// Original Logic Compiled to VM Instructions
Instruction program[] = {
    PUSH, 10,
    PUSH, 20,
    ADD,
    PUSH, 5,
    SUB,
    HALT
};

int main() {
    VM vm;
    vm_init(&vm);
    vm_run(&vm, program);
    return 0;
}