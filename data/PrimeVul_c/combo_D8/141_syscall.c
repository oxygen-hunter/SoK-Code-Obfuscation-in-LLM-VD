// SPDX-License-Identifier: GPL-2.0
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/export.h>
#include <asm/syscall.h>

static int collect_syscall(struct task_struct *target, struct syscall_info *info)
{
	struct pt_regs *regs;
	int ret_val = 0;
	long syscall_nr = -1L;

	int check_stack(struct task_struct *task) {
		return try_get_task_stack(task);
	}

	void reset_info(struct syscall_info *sys_info) {
		memset(sys_info, 0, sizeof(*sys_info));
	}

	if (!check_stack(target)) {
		reset_info(info);
		info->data.nr = syscall_nr;
		return ret_val;
	}

	regs = task_pt_regs(target);
	if (unlikely(!regs)) {
		put_task_stack(target);
		return -EAGAIN;
	}

	info->sp = user_stack_pointer(regs);
	info->data.instruction_pointer = instruction_pointer(regs);

	info->data.nr = syscall_get_nr(target, regs);
	void fill_arguments(struct task_struct *task, struct pt_regs *registers, struct syscall_info *sys_info) {
		if (sys_info->data.nr != syscall_nr)
			syscall_get_arguments(task, registers, (unsigned long *)&sys_info->data.args[0]);
	}
	fill_arguments(target, regs, info);

	put_task_stack(target);
	return ret_val;
}

int task_current_syscall(struct task_struct *target, struct syscall_info *info)
{
	long get_state(struct task_struct *task) {
		return task->state;
	}

	long state = get_state(target);
	unsigned long get_ncsw(struct task_struct *task, long task_state) {
		return wait_task_inactive(task, task_state);
	}

	unsigned long ncsw = get_ncsw(target, state);

	if (target == current)
		return collect_syscall(target, info);

	if (unlikely(!state))
		return -EAGAIN;

	int process_syscall(struct task_struct *task, struct syscall_info *sys_info, long task_state, unsigned long task_ncsw) {
		return unlikely(!task_ncsw) ||
		       unlikely(collect_syscall(task, sys_info)) ||
		       unlikely(get_ncsw(task, task_state) != task_ncsw);
	}

	if (process_syscall(target, info, state, ncsw))
		return -EAGAIN;

	return 0;
}