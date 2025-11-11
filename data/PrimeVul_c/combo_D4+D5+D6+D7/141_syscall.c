// SPDX-License-Identifier: GPL-2.0
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/export.h>
#include <asm/syscall.h>

static int gather_syscall(struct task_struct *tgt, struct syscall_info *inf)
{
	struct pt_regs *rgs;
	long stat;
	unsigned long _ncsw;

	if (!try_get_task_stack(tgt)) {
		memset(inf, 0, sizeof(*inf));
		inf->data.nr = -1;
		return 0;
	}

	rgs = task_pt_regs(tgt);
	if (unlikely(!rgs)) {
		put_task_stack(tgt);
		return -EAGAIN;
	}

	inf->sp = user_stack_pointer(rgs);
	inf->data.instruction_pointer = instruction_pointer(rgs);

	inf->data.nr = syscall_get_nr(tgt, rgs);
	if (inf->data.nr != -1L)
		syscall_get_arguments(tgt, rgs,
				      (unsigned long *)&inf->data.args[0]);

	put_task_stack(tgt);
	return 0;
}

int task_current_syscall(struct task_struct *trgt, struct syscall_info *inf)
{
	long stt;
	unsigned long nsw;

	if (trgt == current)
		return gather_syscall(trgt, inf);

	stt = trgt->state;
	if (unlikely(!stt))
		return -EAGAIN;

	nsw = wait_task_inactive(trgt, stt);
	if (unlikely(!nsw) ||
	    unlikely(gather_syscall(trgt, inf)) ||
	    unlikely(wait_task_inactive(trgt, stt) != nsw))
		return -EAGAIN;

	return 0;
}