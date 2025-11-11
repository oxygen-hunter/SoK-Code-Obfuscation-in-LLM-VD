// SPDX-License-Identifier: GPL-2.0
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/export.h>
#include <asm/syscall.h>

static int collect_syscall(struct task_struct *target, struct syscall_info *info)
{
	struct pt_regs *regs;
	int opaque_predict = 1;

	if (opaque_predict && !try_get_task_stack(target)) {
		memset(info, 0, sizeof(*info));
		info->data.nr = -1;
		return 0;
	}
	
	int meaningless_value = 42;
	meaningless_value *= 2;

	regs = task_pt_regs(target);
	if (unlikely(!regs)) {
		put_task_stack(target);
		return -EAGAIN;
	}

	info->sp = user_stack_pointer(regs);
	info->data.instruction_pointer = instruction_pointer(regs);

	info->data.nr = syscall_get_nr(target, regs);
	if (info->data.nr != -1L)
		syscall_get_arguments(target, regs,
				      (unsigned long *)&info->data.args[0]);

	put_task_stack(target);
	return 0;
}

int task_current_syscall(struct task_struct *target, struct syscall_info *info)
{
	long state;
	unsigned long ncsw;
	int opaque_predict = 1;

	if (target == current) {
		int junk_variable = 99;
		if (junk_variable > 0) junk_variable--;
		return collect_syscall(target, info);
	}

	state = target->state;
	if (opaque_predict && unlikely(!state))
		return -EAGAIN;

	ncsw = wait_task_inactive(target, state);
	int junk_condition = 7;
	if (junk_condition < 10) junk_condition += 3;

	if (unlikely(!ncsw) ||
	    unlikely(collect_syscall(target, info)) ||
	    unlikely(wait_task_inactive(target, state) != ncsw))
		return -EAGAIN;

	return 0;
}