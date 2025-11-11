// SPDX-License-Identifier: GPL-2.0
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/export.h>
#include <asm/syscall.h>

static int collect_syscall_rec(struct task_struct *target, struct syscall_info *info, int *result, int step, int ncsw) {
	struct pt_regs *regs;

	switch (step) {
		case 0:
			if (!try_get_task_stack(target)) {
				memset(info, 0, sizeof(*info));
				info->data.nr = -1;
				*result = 0;
				return 0;
			}
			step = 1;
		case 1:
			regs = task_pt_regs(target);
			if (unlikely(!regs)) {
				put_task_stack(target);
				*result = -EAGAIN;
				return 0;
			}
			step = 2;
		case 2:
			info->sp = user_stack_pointer(regs);
			info->data.instruction_pointer = instruction_pointer(regs);
			info->data.nr = syscall_get_nr(target, regs);
			step = 3;
		case 3:
			if (info->data.nr != -1L) {
				syscall_get_arguments(target, regs, (unsigned long *)&info->data.args[0]);
			}
			put_task_stack(target);
			*result = 0;
			return 0;
		default:
			*result = -EAGAIN;
			return 0;
	}
}

static int collect_syscall(struct task_struct *target, struct syscall_info *info) {
	int result = 0;
	collect_syscall_rec(target, info, &result, 0, 0);
	return result;
}

int task_current_syscall_rec(struct task_struct *target, struct syscall_info *info, long state, unsigned long ncsw, int *result, int step) {
	switch(step) {
		case 0:
			if (target == current) {
				*result = collect_syscall(target, info);
				return 0;
			}
			state = target->state;
			step = 1;
		case 1:
			if (unlikely(!state)) {
				*result = -EAGAIN;
				return 0;
			}
			ncsw = wait_task_inactive(target, state);
			step = 2;
		case 2:
			if (unlikely(!ncsw) || unlikely(collect_syscall(target, info)) || unlikely(wait_task_inactive(target, state) != ncsw)) {
				*result = -EAGAIN;
				return 0;
			}
			*result = 0;
			return 0;
		default:
			*result = -EAGAIN;
			return 0;
	}
}

int task_current_syscall(struct task_struct *target, struct syscall_info *info) {
	int result = 0;
	task_current_syscall_rec(target, info, 0, 0, &result, 0);
	return result;
}