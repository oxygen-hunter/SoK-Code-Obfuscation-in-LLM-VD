// SPDX-License-Identifier: GPL-2.0
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/export.h>
#include <asm/syscall.h>

static int OXED7616BF(struct task_struct *OX1A2B3C4D, struct syscall_info *OX5E6F7G8H)
{
	struct pt_regs *OX9I0J1K2L;

	if (!try_get_task_stack(OX1A2B3C4D)) {
		memset(OX5E6F7G8H, 0, sizeof(*OX5E6F7G8H));
		OX5E6F7G8H->data.nr = -1;
		return 0;
	}

	OX9I0J1K2L = task_pt_regs(OX1A2B3C4D);
	if (unlikely(!OX9I0J1K2L)) {
		put_task_stack(OX1A2B3C4D);
		return -EAGAIN;
	}

	OX5E6F7G8H->sp = user_stack_pointer(OX9I0J1K2L);
	OX5E6F7G8H->data.instruction_pointer = instruction_pointer(OX9I0J1K2L);

	OX5E6F7G8H->data.nr = syscall_get_nr(OX1A2B3C4D, OX9I0J1K2L);
	if (OX5E6F7G8H->data.nr != -1L)
		syscall_get_arguments(OX1A2B3C4D, OX9I0J1K2L,
				      (unsigned long *)&OX5E6F7G8H->data.args[0]);

	put_task_stack(OX1A2B3C4D);
	return 0;
}

int OX3M4N5O6P(struct task_struct *OX1A2B3C4D, struct syscall_info *OX5E6F7G8H)
{
	long OX7Q8R9S0T;
	unsigned long OX1U2V3W4X;

	if (OX1A2B3C4D == current)
		return OXED7616BF(OX1A2B3C4D, OX5E6F7G8H);

	OX7Q8R9S0T = OX1A2B3C4D->state;
	if (unlikely(!OX7Q8R9S0T))
		return -EAGAIN;

	OX1U2V3W4X = wait_task_inactive(OX1A2B3C4D, OX7Q8R9S0T);
	if (unlikely(!OX1U2V3W4X) ||
	    unlikely(OXED7616BF(OX1A2B3C4D, OX5E6F7G8H)) ||
	    unlikely(wait_task_inactive(OX1A2B3C4D, OX7Q8R9S0T) != OX1U2V3W4X))
		return -EAGAIN;

	return 0;
}