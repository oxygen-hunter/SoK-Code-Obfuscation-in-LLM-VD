// SPDX-License-Identifier: GPL-2.0
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/export.h>
#include <asm/syscall.h>

static int collect_syscall(struct task_struct *target, struct syscall_info *info) {
    struct pt_regs *regs;
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                if (!try_get_task_stack(target)) {
                    memset(info, 0, sizeof(*info));
                    info->data.nr = -1;
                    return 0;
                }
                dispatcher = 1;
                break;
            case 1:
                regs = task_pt_regs(target);
                if (unlikely(!regs)) {
                    put_task_stack(target);
                    return -EAGAIN;
                }
                dispatcher = 2;
                break;
            case 2:
                info->sp = user_stack_pointer(regs);
                info->data.instruction_pointer = instruction_pointer(regs);
                dispatcher = 3;
                break;
            case 3:
                info->data.nr = syscall_get_nr(target, regs);
                if (info->data.nr != -1L) {
                    dispatcher = 4;
                } else {
                    dispatcher = 5;
                }
                break;
            case 4:
                syscall_get_arguments(target, regs, (unsigned long *)&info->data.args[0]);
                dispatcher = 5;
                break;
            case 5:
                put_task_stack(target);
                return 0;
        }
    }
}

int task_current_syscall(struct task_struct *target, struct syscall_info *info) {
    long state;
    unsigned long ncsw;
    int dispatcher = 0;

    while (1) {
        switch (dispatcher) {
            case 0:
                if (target == current) {
                    return collect_syscall(target, info);
                }
                dispatcher = 1;
                break;
            case 1:
                state = target->state;
                if (unlikely(!state)) {
                    return -EAGAIN;
                }
                dispatcher = 2;
                break;
            case 2:
                ncsw = wait_task_inactive(target, state);
                if (unlikely(!ncsw) || unlikely(collect_syscall(target, info)) ||
                    unlikely(wait_task_inactive(target, state) != ncsw)) {
                    return -EAGAIN;
                }
                return 0;
        }
    }
}