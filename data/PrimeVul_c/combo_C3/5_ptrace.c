#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/ptrace.h>
#include <linux/security.h>
#include <linux/signal.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/ptrace.h>

#define PUSH 0
#define POP 1
#define ADD 2
#define SUB 3
#define JMP 4
#define JZ 5
#define LOAD 6
#define STORE 7
#define HALT 8

#define MAX_STACK_SIZE 256
#define MAX_PROGRAM_SIZE 1024

typedef struct {
    int stack[MAX_STACK_SIZE];
    int sp;
    int pc;
    int halted;
} VM;

typedef struct {
    int opcode;
    int operand;
} Instruction;

VM vm;
Instruction program[MAX_PROGRAM_SIZE];

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->halted = 0;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm, Instruction *program) {
    while (!vm->halted) {
        Instruction *instr = &program[vm->pc++];
        switch (instr->opcode) {
            case PUSH:
                vm_push(vm, instr->operand);
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
                vm->pc = instr->operand;
                break;
            case JZ: {
                int a = vm_pop(vm);
                if (a == 0) vm->pc = instr->operand;
                break;
            }
            case LOAD: {
                struct task_struct *child = (struct task_struct *)instr->operand;
                int val = vm_pop(vm);
                child->ptrace |= val;
                break;
            }
            case STORE: {
                struct task_struct *child = (struct task_struct *)instr->operand;
                int val = vm_pop(vm);
                child->ptrace &= ~val;
                break;
            }
            case HALT:
                vm->halted = 1;
                break;
            default:
                break;
        }
    }
}

void user_enable_single_step(struct task_struct *child) {
    vm_init(&vm);
    program[0] = (Instruction){PUSH, PT_SINGLESTEP};
    program[1] = (Instruction){LOAD, (int)child};
    program[2] = (Instruction){HALT, 0};
    vm_run(&vm, program);
}

void user_disable_single_step(struct task_struct *child) {
    vm_init(&vm);
    program[0] = (Instruction){PUSH, PT_SINGLESTEP};
    program[1] = (Instruction){STORE, (int)child};
    program[2] = (Instruction){HALT, 0};
    vm_run(&vm, program);
}

void ptrace_disable(struct task_struct *child) {
    // Nothing to do..
}