#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <ldap.h>

#define STACK_SIZE 256
#define MEMORY_SIZE 256

#define ENTRIES "validator,ldap"
#define LDAP_DEFAULT_PORT 389

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT, NOP
} InstructionSet;

typedef struct {
    int stack[STACK_SIZE];
    int sp; // Stack pointer
    int pc; // Program counter
    int memory[MEMORY_SIZE];
    InstructionSet program[MEMORY_SIZE];
} VM;

void execute_vm(VM *vm) {
    while (vm->program[vm->pc] != HALT) {
        switch (vm->program[vm->pc]) {
            case PUSH:
                vm->stack[++vm->sp] = vm->program[++vm->pc];
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
                vm->pc = vm->program[++vm->pc] - 1;
                break;
            case JZ:
                if (vm->stack[vm->sp--] == 0)
                    vm->pc = vm->program[++vm->pc] - 1;
                else
                    vm->pc++;
                break;
            case LOAD:
                vm->stack[++vm->sp] = vm->memory[vm->program[++vm->pc]];
                break;
            case STORE:
                vm->memory[vm->program[++vm->pc]] = vm->stack[vm->sp--];
                break;
            case NOP:
                break;
            default:
                fprintf(stderr, "Unknown instruction %d\n", vm->program[vm->pc]);
                exit(EXIT_FAILURE);
        }
        vm->pc++;
    }
}

void setup_program(VM *vm) {
    vm->program[0] = PUSH;
    vm->program[1] = LDAP_DEFAULT_PORT;
    vm->program[2] = STORE;
    vm->program[3] = 0;
    vm->program[4] = LOAD;
    vm->program[5] = 0;
    vm->program[6] = PUSH;
    vm->program[7] = LDAP_OPT_SUCCESS;
    vm->program[8] = ADD;
    vm->program[9] = HALT;
}

int main() {
    VM vm = { .sp = -1, .pc = 0 };
    setup_program(&vm);
    execute_vm(&vm);
    printf("Execution finished. Stack top: %d\n", vm.stack[vm.sp]);
    return 0;
}