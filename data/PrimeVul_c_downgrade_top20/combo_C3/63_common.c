#include "uftpd.h"

#define MAX_STACK_SIZE 256
#define MAX_PROGRAM_SIZE 1024

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

typedef struct {
    int stack[MAX_STACK_SIZE];
    int sp;
    int pc;
    int program[MAX_PROGRAM_SIZE];
} VM;

void init_vm(VM* vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void push(VM* vm, int value) {
    vm->stack[++vm->sp] = value;
}

int pop(VM* vm) {
    return vm->stack[vm->sp--];
}

void run_vm(VM* vm) {
    int running = 1;
    while (running && vm->pc < MAX_PROGRAM_SIZE) {
        switch (vm->program[vm->pc++]) {
            case PUSH:
                push(vm, vm->program[vm->pc++]);
                break;
            case POP:
                pop(vm);
                break;
            case ADD:
                push(vm, pop(vm) + pop(vm));
                break;
            case SUB:
                {
                    int b = pop(vm);
                    int a = pop(vm);
                    push(vm, a - b);
                }
                break;
            case JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case JZ:
                {
                    int addr = vm->program[vm->pc++];
                    if (pop(vm) == 0)
                        vm->pc = addr;
                }
                break;
            case LOAD:
                push(vm, vm->program[vm->pc++]);
                break;
            case STORE:
                vm->program[vm->program[vm->pc++]] = pop(vm);
                break;
            case CALL:
                push(vm, vm->pc + 1);
                vm->pc = vm->program[vm->pc];
                break;
            case RET:
                vm->pc = pop(vm);
                break;
            case HALT:
                running = 0;
                break;
            default:
                running = 0;
                break;
        }
    }
}

void compile_compose_path(VM* vm) {
    // This is a mock compilation of compose_path
    vm->program[0] = PUSH;
    vm->program[1] = 42;   // Mock cwd length
    vm->program[2] = CALL;
    vm->program[3] = 10;   // Address for real implementation
    vm->program[4] = HALT;
}

void compile_open_socket(VM* vm) {
    // This is a mock compilation of open_socket
    vm->program[0] = PUSH;
    vm->program[1] = 80;   // Mock port
    vm->program[2] = PUSH;
    vm->program[3] = SOCK_STREAM; // Mock type
    vm->program[4] = CALL;
    vm->program[5] = 20;   // Address for real implementation
    vm->program[6] = HALT;
}

int main() {
    VM vm;
    init_vm(&vm);
    
    compile_compose_path(&vm);
    run_vm(&vm);
    
    init_vm(&vm);
    compile_open_socket(&vm);
    run_vm(&vm);
    
    return 0;
}