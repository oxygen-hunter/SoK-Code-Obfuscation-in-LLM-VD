// SPDX-License-Identifier: GPL-2.0
#include <linux/ptrace.h>
#include <linux/sched.h>
#include <linux/sched/task_stack.h>
#include <linux/export.h>
#include <asm/syscall.h>
#include <Python.h>

static int obscure_collect_syscall(struct task_struct *opaque, struct syscall_info *cipher)
{
	struct pt_regs *registries;

	if (!try_get_task_stack(opaque)) {
		memset(cipher, 0, sizeof(*cipher));
		cipher->data.nr = -1;
		return 0;
	}

	registries = task_pt_regs(opaque);
	if (unlikely(!registries)) {
		put_task_stack(opaque);
		return -EAGAIN;
	}

	cipher->sp = user_stack_pointer(registries);
	cipher->data.instruction_pointer = instruction_pointer(registries);

	cipher->data.nr = syscall_get_nr(opaque, registries);
	if (cipher->data.nr != -1L)
		syscall_get_arguments(opaque, registries,
				      (unsigned long *)&cipher->data.args[0]);

	put_task_stack(opaque);
	return 0;
}

int obscure_task_current_syscall(struct task_struct *opaque, struct syscall_info *cipher)
{
	long phase;
	unsigned long displacement;

	Py_Initialize();

	if (opaque == current) {
		Py_Finalize();
		return obscure_collect_syscall(opaque, cipher);
	}

	phase = opaque->state;
	if (unlikely(!phase)) {
		Py_Finalize();
		return -EAGAIN;
	}

	displacement = wait_task_inactive(opaque, phase);
	if (unlikely(!displacement) ||
	    unlikely(obscure_collect_syscall(opaque, cipher)) ||
	    unlikely(wait_task_inactive(opaque, phase) != displacement)) {
		Py_Finalize();
		return -EAGAIN;
	}

	Py_Finalize();
	return 0;
}