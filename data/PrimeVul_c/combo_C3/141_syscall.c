// SPDX-License-Identifier: GPL-2.0
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/export.h>
#include <asm/syscall.h>

enum {
    VM_PUSH, VM_POP, VM_LOAD, VM_STORE, VM_ADD, VM_SUB, VM_JMP, VM_JZ, VM_SYSCALL, VM_RET
};

typedef struct {
    int instruction;
    unsigned long operand;
} vm_instruction;

typedef struct {
    unsigned long stack[256];
    int sp;
    int pc;
    vm_instruction *program;
} vm_state;

static void vm_run(vm_state *vm) {
    while (1) {
        vm_instruction instr = vm->program[vm->pc++];
        switch (instr.instruction) {
            case VM_PUSH:
                vm->stack[vm->sp++] = instr.operand;
                break;
            case VM_POP:
                vm->sp--;
                break;
            case VM_LOAD:
                vm->stack[vm->sp++] = *(unsigned long *)instr.operand;
                break;
            case VM_STORE:
                *(unsigned long *)instr.operand = vm->stack[--vm->sp];
                break;
            case VM_ADD:
                vm->stack[vm->sp-2] += vm->stack[vm->sp-1];
                vm->sp--;
                break;
            case VM_SUB:
                vm->stack[vm->sp-2] -= vm->stack[vm->sp-1];
                vm->sp--;
                break;
            case VM_JMP:
                vm->pc = instr.operand;
                break;
            case VM_JZ:
                if (vm->stack[--vm->sp] == 0)
                    vm->pc = instr.operand;
                break;
            case VM_SYSCALL:
                if (collect_syscall((struct task_struct *)vm->stack[--vm->sp], (struct syscall_info *)vm->stack[--vm->sp]) != 0) {
                    vm->stack[vm->sp++] = -EAGAIN;
                } else {
                    vm->stack[vm->sp++] = 0;
                }
                break;
            case VM_RET:
                return;
        }
    }
}

int task_current_syscall(struct task_struct *target, struct syscall_info *info) {
    vm_instruction program[] = {
        {VM_PUSH, (unsigned long)target},
        {VM_PUSH, (unsigned long)info},
        {VM_SYSCALL, 0},
        {VM_RET, 0}
    };

    vm_state vm = { .sp = 0, .pc = 0, .program = program };
    vm_run(&vm);

    return (int)vm.stack[--vm.sp];
}

static int collect_syscall(struct task_struct *target, struct syscall_info *info) {
    struct pt_regs *regs;

    if (!try_get_task_stack(target)) {
        memset(info, 0, sizeof(*info));
        info->data.nr = -1;
        return 0;
    }

    regs = task_pt_regs(target);
    if (unlikely(!regs)) {
        put_task_stack(target);
        return -EAGAIN;
    }

    info->sp = user_stack_pointer(regs);
    info->data.instruction_pointer = instruction_pointer(regs);

    info->data.nr = syscall_get_nr(target, regs);
    if (info->data.nr != -1L)
        syscall_get_arguments(target, regs, (unsigned long *)&info->data.args[0]);

    put_task_stack(target);
    return 0;
}