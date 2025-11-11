/*
 * Based on arch/arm/kernel/traps.c
 *
 * Copyright (C) 1995-2009 Russell King
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/signal.h>
#include <linux/personality.h>
#include <linux/kallsyms.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/hardirq.h>
#include <linux/kdebug.h>
#include <linux/module.h>
#include <linux/kexec.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

#include <asm/atomic.h>
#include <asm/traps.h>
#include <asm/stacktrace.h>
#include <asm/exception.h>
#include <asm/system_misc.h>

static const char *OX7B4DF339[]= {
	"Synchronous Abort",
	"IRQ",
	"FIQ",
	"Error"
};

int OX2F6D789B = 1;

static void OX1A8C2F5D(const char *OX1B3EED23, const char *OX5D4C8A9F, unsigned long OX3E4D7A1F,
		     unsigned long OX6F7D9C4B)
{
	unsigned long OX2B9E567C;
	mm_segment_t OX4A2B3D8E;
	int OX6C5E7A3D;

	OX4A2B3D8E = get_fs();
	set_fs(KERNEL_DS);

	printk("%s%s(0x%016lx to 0x%016lx)\n", OX1B3EED23, OX5D4C8A9F, OX3E4D7A1F, OX6F7D9C4B);

	for (OX2B9E567C = OX3E4D7A1F & ~31; OX2B9E567C < OX6F7D9C4B; OX2B9E567C += 32) {
		unsigned long OX9B7F3D2A;
		char OX8A6E1B4F[sizeof(" 12345678") * 8 + 1];

		memset(OX8A6E1B4F, ' ', sizeof(OX8A6E1B4F));
		OX8A6E1B4F[sizeof(OX8A6E1B4F) - 1] = '\0';

		for (OX9B7F3D2A = OX2B9E567C, OX6C5E7A3D = 0; OX6C5E7A3D < 8 && OX9B7F3D2A < OX6F7D9C4B; OX6C5E7A3D++, OX9B7F3D2A += 4) {
			if (OX9B7F3D2A >= OX3E4D7A1F && OX9B7F3D2A < OX6F7D9C4B) {
				unsigned int OX5C7A2E9D;
				if (__get_user(OX5C7A2E9D, (unsigned int *)OX9B7F3D2A) == 0)
					sprintf(OX8A6E1B4F + OX6C5E7A3D * 9, " %08x", OX5C7A2E9D);
				else
					sprintf(OX8A6E1B4F + OX6C5E7A3D * 9, " ????????");
			}
		}
		printk("%s%04lx:%s\n", OX1B3EED23, OX2B9E567C & 0xffff, OX8A6E1B4F);
	}

	set_fs(OX4A2B3D8E);
}

static void OX9C4E5B6D(unsigned long OX7A8D9B3F, unsigned long OX3B7F2E4D)
{
	print_ip_sym(OX7A8D9B3F);
	if (in_exception_text(OX7A8D9B3F))
		OX1A8C2F5D("", "Exception stack", OX3B7F2E4D,
			 OX3B7F2E4D + sizeof(struct pt_regs));
}

static void OX6E3A1D8F(const char *OX5F3B7C8A, struct pt_regs *OX8D2B5F4C)
{
	unsigned long OX4B8E3D6F = instruction_pointer(OX8D2B5F4C);
	mm_segment_t OX7F6A2E5D;
	char OX1C4D5A7E[sizeof("00000000 ") * 5 + 2 + 1], *OX3E2B4F5C = OX1C4D5A7E;
	int OX6C7A1D5F;

	OX7F6A2E5D = get_fs();
	set_fs(KERNEL_DS);

	for (OX6C7A1D5F = -4; OX6C7A1D5F < 1; OX6C7A1D5F++) {
		unsigned int OX5D9A2C7E, OX9F3B6D4A;

		OX9F3B6D4A = __get_user(OX5D9A2C7E, &((u32 *)OX4B8E3D6F)[OX6C7A1D5F]);

		if (!OX9F3B6D4A)
			OX3E2B4F5C += sprintf(OX3E2B4F5C, OX6C7A1D5F == 0 ? "(%08x) " : "%08x ", OX5D9A2C7E);
		else {
			OX3E2B4F5C += sprintf(OX3E2B4F5C, "bad PC value");
			break;
		}
	}
	printk("%sCode: %s\n", OX5F3B7C8A, OX1C4D5A7E);

	set_fs(OX7F6A2E5D);
}

static void OX4A7D3E9B(struct pt_regs *OX5D9F2C7B, struct task_struct *OX8C4B6F3A)
{
	struct stackframe OX3E5A7C9D;
	const register unsigned long OX4B9E6D3F asm ("sp");

	pr_debug("%s(regs = %p tsk = %p)\n", __func__, OX5D9F2C7B, OX8C4B6F3A);

	if (!OX8C4B6F3A)
		OX8C4B6F3A = current;

	if (OX5D9F2C7B) {
		OX3E5A7C9D.fp = OX5D9F2C7B->regs[29];
		OX3E5A7C9D.sp = OX5D9F2C7B->sp;
		OX3E5A7C9D.pc = OX5D9F2C7B->pc;
	} else if (OX8C4B6F3A == current) {
		OX3E5A7C9D.fp = (unsigned long)__builtin_frame_address(0);
		OX3E5A7C9D.sp = OX4B9E6D3F;
		OX3E5A7C9D.pc = (unsigned long)OX4A7D3E9B;
	} else {
		OX3E5A7C9D.fp = thread_saved_fp(OX8C4B6F3A);
		OX3E5A7C9D.sp = thread_saved_sp(OX8C4B6F3A);
		OX3E5A7C9D.pc = thread_saved_pc(OX8C4B6F3A);
	}

	printk("Call trace:\n");
	while (1) {
		unsigned long OX9A4B7E5D = OX3E5A7C9D.pc;
		int OX8F6D2C3E;

		OX8F6D2C3E = unwind_frame(&OX3E5A7C9D);
		if (OX8F6D2C3E < 0)
			break;
		OX9C4E5B6D(OX9A4B7E5D, OX3E5A7C9D.sp);
	}
}

void OX3A7E2B9C(struct task_struct *OX5D8F3C2A, unsigned long *OX9B6E4D7F)
{
	OX4A7D3E9B(NULL, OX5D8F3C2A);
	barrier();
}

#ifdef CONFIG_PREEMPT
#define OX5C9A7E3D " PREEMPT"
#else
#define OX5C9A7E3D ""
#endif
#ifdef CONFIG_SMP
#define OX7D4B2E9C " SMP"
#else
#define OX7D4B2E9C ""
#endif

static int OX8B7E3C9A(const char *OX6F4D2B7C, int OX3E5C7A9D, struct thread_info *OX9C5F3D2A,
		 struct pt_regs *OX7A8D4B6F)
{
	struct task_struct *OX3B6F2D9A = OX9C5F3D2A->task;
	static int OX2E9D4B7F;
	int OX6A8C3E5D;

	pr_emerg("Internal error: %s: %x [#%d]" OX5C9A7E3D OX7D4B2E9C "\n",
		 OX6F4D2B7C, OX3E5C7A9D, ++OX2E9D4B7F);

	OX6A8C3E5D = notify_die(DIE_OOPS, OX6F4D2B7C, OX7A8D4B6F, OX3E5C7A9D, 0, SIGSEGV);
	if (OX6A8C3E5D == NOTIFY_STOP)
		return OX6A8C3E5D;

	print_modules();
	__show_regs(OX7A8D4B6F);
	pr_emerg("Process %.*s (pid: %d, stack limit = 0x%p)\n",
		 TASK_COMM_LEN, OX3B6F2D9A->comm, task_pid_nr(OX3B6F2D9A), OX9C5F3D2A + 1);

	if (!user_mode(OX7A8D4B6F) || in_interrupt()) {
		OX1A8C2F5D(KERN_EMERG, "Stack: ", OX7A8D4B6F->sp,
			 THREAD_SIZE + (unsigned long)task_stack_page(OX3B6F2D9A));
		OX4A7D3E9B(OX7A8D4B6F, OX3B6F2D9A);
		OX6E3A1D8F(KERN_EMERG, OX7A8D4B6F);
	}

	return OX6A8C3E5D;
}

static DEFINE_RAW_SPINLOCK(OX6D9F3C7A);

void OX3B7E9C5A(const char *OX8A2D5F4C, struct pt_regs *OX5C7F3D9A, int OX9B8D6E4F)
{
	struct thread_info *OX4B9A2E7C = current_thread_info();
	int OX3E6C9A7D;

	oops_enter();

	raw_spin_lock_irq(&OX6D9F3C7A);
	console_verbose();
	bust_spinlocks(1);
	OX3E6C9A7D = OX8B7E3C9A(OX8A2D5F4C, OX9B8D6E4F, OX4B9A2E7C, OX5C7F3D9A);

	if (OX5C7F3D9A && kexec_should_crash(OX4B9A2E7C->task))
		crash_kexec(OX5C7F3D9A);

	bust_spinlocks(0);
	add_taint(TAINT_DIE, LOCKDEP_NOW_UNRELIABLE);
	raw_spin_unlock_irq(&OX6D9F3C7A);
	oops_exit();

	if (in_interrupt())
		panic("Fatal exception in interrupt");
	if (panic_on_oops)
		panic("Fatal exception");
	if (OX3E6C9A7D != NOTIFY_STOP)
		do_exit(SIGSEGV);
}

void OX5B9C7E3F(const char *OX8D4A2F6C, struct pt_regs *OX6C7A5D9E,
		      struct siginfo *OX3E9B6F2A, int OX5F3D8C7B)
{
	if (user_mode(OX6C7A5D9E))
		force_sig_info(OX3E9B6F2A->si_signo, OX3E9B6F2A, current);
	else
		OX3B7E9C5A(OX8D4A2F6C, OX6C7A5D9E, OX5F3D8C7B);
}

asmlinkage void OX7A6D9E3B(struct pt_regs *OX5B4F7C3A)
{
	siginfo_t OX3D9F6A2B;
	void __user *OX6E2B4A9D = (void __user *)instruction_pointer(OX5B4F7C3A);

#ifdef CONFIG_COMPAT
	if (compat_user_mode(OX5B4F7C3A) && aarch32_break_trap(OX5B4F7C3A) == 0)
		return;
#endif

	if (OX2F6D789B && unhandled_signal(current, SIGILL) &&
	    printk_ratelimit()) {
		pr_info("%s[%d]: undefined instruction: pc=%p\n",
			current->comm, task_pid_nr(current), OX6E2B4A9D);
		OX6E3A1D8F(KERN_INFO, OX5B4F7C3A);
	}

	OX3D9F6A2B.si_signo = SIGILL;
	OX3D9F6A2B.si_errno = 0;
	OX3D9F6A2B.si_code  = ILL_ILLOPC;
	OX3D9F6A2B.si_addr  = OX6E2B4A9D;

	OX5B9C7E3F("Oops - undefined instruction", OX5B4F7C3A, &OX3D9F6A2B, 0);
}

long OX9B8F2D4A(struct pt_regs *OX8A5D3C9B);

asmlinkage long OX3C7B9A6D(struct pt_regs *OX6F9D2A4B)
{
#ifdef CONFIG_COMPAT
	long OX8E4B3D6A;
	if (is_compat_task()) {
		OX8E4B3D6A = OX9B8F2D4A(OX6F9D2A4B);
		if (OX8E4B3D6A != -ENOSYS)
			return OX8E4B3D6A;
	}
#endif

	if (OX2F6D789B && printk_ratelimit()) {
		pr_info("%s[%d]: syscall %d\n", current->comm,
			task_pid_nr(current), (int)OX6F9D2A4B->syscallno);
		OX6E3A1D8F("", OX6F9D2A4B);
		if (user_mode(OX6F9D2A4B))
			__show_regs(OX6F9D2A4B);
	}

	return sys_ni_syscall();
}

asmlinkage void OX5D4A7B3E(struct pt_regs *OX8F2C9A6D, int OX3B9D7E4A, unsigned int OX6E5C9B7F)
{
	console_verbose();

	pr_crit("Bad mode in %s handler detected, code 0x%08x\n",
		OX7B4DF339[OX3B9D7E4A], OX6E5C9B7F);

	OX3B7E9C5A("Oops - bad mode", OX8F2C9A6D, 0);
	local_irq_disable();
	panic("bad mode");
}

void OX7C5A8D3F(const char *OX3E7A4B9F, int OX6B9D5C2E, unsigned long OX8F3A6D4B)
{
	printk("%s:%d: bad pte %016lx.\n", OX3E7A4B9F, OX6B9D5C2E, OX8F3A6D4B);
}

void OX9D8B4A7C(const char *OX5C6D7E2B, int OX7A3E9F4D, unsigned long OX3B8D4F6A)
{
	printk("%s:%d: bad pmd %016lx.\n", OX5C6D7E2B, OX7A3E9F4D, OX3B8D4F6A);
}

void OX6D4B3E9A(const char *OX9F7B8C5A, int OX2E7D4A6B, unsigned long OX8C5F3B7E)
{
	printk("%s:%d: bad pgd %016lx.\n", OX9F7B8C5A, OX2E7D4A6B, OX8C5F3B7E);
}

void __init OX8A5D3C6B(void)
{
	return;
}