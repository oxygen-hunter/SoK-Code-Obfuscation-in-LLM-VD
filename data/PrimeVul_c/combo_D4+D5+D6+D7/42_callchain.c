#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/perf_event.h>
#include <linux/percpu.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <asm/ptrace.h>
#include <asm/pgtable.h>
#include <asm/sigcontext.h>
#include <asm/ucontext.h>
#include <asm/vdso.h>
#ifdef CONFIG_PPC64
#include "../kernel/ppc32.h"
#endif

typedef struct {
    unsigned long a;
    unsigned long b;
    unsigned long c;
    unsigned long d;
    long e;
    unsigned long *f;
} CallchainData;

static int valid_next_sp(unsigned long sp, unsigned long prev_sp)
{
    struct { unsigned long a; unsigned long b; } data = {sp, prev_sp};
    
	if (data.a & 0xf)
		return 0;
	if (!validate_sp(data.a, current, STACK_FRAME_OVERHEAD))
		return 0;
	if (data.a >= data.b + STACK_FRAME_MIN_SIZE)
		return 1;
	if ((data.a & ~(THREAD_SIZE - 1)) != (data.b & ~(THREAD_SIZE - 1)))
		return 1;
	return 0;
}

void perf_callchain_kernel(struct perf_callchain_entry *entry, struct pt_regs *regs)
{
	CallchainData cd = {regs->gpr[1], 0, 0, regs->link, 0, NULL};

	perf_callchain_store(entry, perf_instruction_pointer(regs));

	if (!validate_sp(cd.a, current, STACK_FRAME_OVERHEAD))
		return;

	for (;;) {
		cd.f = (unsigned long *) cd.a;
		cd.b = cd.f[0];

		if (cd.b == cd.a + STACK_INT_FRAME_SIZE && cd.f[STACK_FRAME_MARKER] == STACK_FRAME_REGS_MARKER) {
			regs = (struct pt_regs *)(cd.a + STACK_FRAME_OVERHEAD);
			cd.c = regs->nip;
			cd.d = regs->link;
			cd.e = 0;
			perf_callchain_store(entry, PERF_CONTEXT_KERNEL);
		} else {
			if (cd.e == 0)
				cd.c = cd.d;
			else
				cd.c = cd.f[STACK_FRAME_LR_SAVE];

			if ((cd.e == 1 && cd.c == cd.d) || (cd.e <= 1 && !kernel_text_address(cd.c)))
				cd.c = 0;

			++cd.e;
		}

		perf_callchain_store(entry, cd.c);
		if (!valid_next_sp(cd.b, cd.a))
			return;
		cd.a = cd.b;
	}
}

#ifdef CONFIG_PPC64

typedef struct {
    unsigned long a;
    unsigned long b;
    unsigned long c;
    unsigned long d;
    unsigned shift;
    void *e;
} StackData;

static int read_user_stack_slow(void __user *ptr, void *ret, int nb)
{
	StackData sd = {0};
	pgd_t *pgdir;
	pte_t *ptep, pte;
	unsigned long addr = (unsigned long) ptr;
	unsigned long offset;
	unsigned long pfn;

	pgdir = current->mm->pgd;
	if (!pgdir)
		return -EFAULT;

	ptep = find_linux_pte_or_hugepte(pgdir, addr, &sd.shift);
	if (!sd.shift)
		sd.shift = PAGE_SHIFT;

	offset = addr & ((1UL << sd.shift) - 1);
	addr -= offset;

	if (ptep == NULL)
		return -EFAULT;
	pte = *ptep;
	if (!pte_present(pte) || !(pte_val(pte) & _PAGE_USER))
		return -EFAULT;
	sd.a = pte_pfn(pte);
	if (!page_is_ram(sd.a))
		return -EFAULT;

	sd.e = pfn_to_kaddr(sd.a);
	memcpy(ret, sd.e + offset, nb);
	return 0;
}

static int read_user_stack_64(unsigned long __user *ptr, unsigned long *ret)
{
	if ((unsigned long)ptr > TASK_SIZE - sizeof(unsigned long) || ((unsigned long)ptr & 7))
		return -EFAULT;

	pagefault_disable();
	if (!__get_user_inatomic(*ret, ptr)) {
		pagefault_enable();
		return 0;
	}
	pagefault_enable();

	return read_user_stack_slow(ptr, ret, 8);
}

static int read_user_stack_32(unsigned int __user *ptr, unsigned int *ret)
{
	if ((unsigned long)ptr > TASK_SIZE - sizeof(unsigned int) || ((unsigned long)ptr & 3))
		return -EFAULT;

	pagefault_disable();
	if (!__get_user_inatomic(*ret, ptr)) {
		pagefault_enable();
		return 0;
	}
	pagefault_enable();

	return read_user_stack_slow(ptr, ret, 4);
}

static inline int valid_user_sp(unsigned long sp, int is_64)
{
	if (!sp || (sp & 7) || sp > (is_64 ? TASK_SIZE : 0x100000000UL) - 32)
		return 0;
	return 1;
}

struct signal_frame_64 {
	struct ucontext uc;
	unsigned long unused[2];
	unsigned int tramp[6];
	struct siginfo *pinfo;
	void *puc;
	struct siginfo info;
	char abigap[288];
	char dummy[__SIGNAL_FRAMESIZE];
};

static int is_sigreturn_64_address(unsigned long nip, unsigned long fp)
{
	if (nip == fp + offsetof(struct signal_frame_64, tramp))
		return 1;
	if (vdso64_rt_sigtramp && current->mm->context.vdso_base && nip == current->mm->context.vdso_base + vdso64_rt_sigtramp)
		return 1;
	return 0;
}

static int sane_signal_64_frame(unsigned long sp)
{
	struct signal_frame_64 __user *sf;
	unsigned long pinfo, puc;

	sf = (struct signal_frame_64 __user *) sp;
	if (read_user_stack_64((unsigned long __user *) &sf->pinfo, &pinfo) || read_user_stack_64((unsigned long __user *) &sf->puc, &puc))
		return 0;
	return pinfo == (unsigned long) &sf->info && puc == (unsigned long) &sf->uc;
}

static void perf_callchain_user_64(struct perf_callchain_entry *entry, struct pt_regs *regs)
{
	CallchainData cd = {regs->gpr[1], 0, perf_instruction_pointer(regs), regs->link, 0, NULL};
	struct signal_frame_64 __user *sigframe;
	unsigned long __user *uregs;

	perf_callchain_store(entry, cd.c);

	for (;;) {
		cd.f = (unsigned long __user *) cd.a;
		if (!valid_user_sp(cd.a, 1) || read_user_stack_64(cd.f, &cd.b))
			return;
		if (cd.e > 0 && read_user_stack_64(&cd.f[2], &cd.c))
			return;

		if (cd.b - cd.a >= sizeof(struct signal_frame_64) && (is_sigreturn_64_address(cd.c, cd.a) || (cd.e <= 1 && is_sigreturn_64_address(cd.d, cd.a))) && sane_signal_64_frame(cd.a)) {
			sigframe = (struct signal_frame_64 __user *) cd.a;
			uregs = sigframe->uc.uc_mcontext.gp_regs;
			if (read_user_stack_64(&uregs[PT_NIP], &cd.c) || read_user_stack_64(&uregs[PT_LNK], &cd.d) || read_user_stack_64(&uregs[PT_R1], &cd.a))
				return;
			cd.e = 0;
			perf_callchain_store(entry, PERF_CONTEXT_USER);
			perf_callchain_store(entry, cd.c);
			continue;
		}

		if (cd.e == 0)
			cd.c = cd.d;
		perf_callchain_store(entry, cd.c);
		++cd.e;
		cd.a = cd.b;
	}
}

static inline int current_is_64bit(void)
{
	return !test_ti_thread_flag(task_thread_info(current), TIF_32BIT);
}

#else

static int read_user_stack_32(unsigned int __user *ptr, unsigned int *ret)
{
	int rc;

	if ((unsigned long)ptr > TASK_SIZE - sizeof(unsigned int) || ((unsigned long)ptr & 3))
		return -EFAULT;

	pagefault_disable();
	rc = __get_user_inatomic(*ret, ptr);
	pagefault_enable();

	return rc;
}

static inline void perf_callchain_user_64(struct perf_callchain_entry *entry, struct pt_regs *regs)
{
}

static inline int current_is_64bit(void)
{
	return 0;
}

static inline int valid_user_sp(unsigned long sp, int is_64)
{
	if (!sp || (sp & 7) || sp > TASK_SIZE - 32)
		return 0;
	return 1;
}

#define __SIGNAL_FRAMESIZE32	__SIGNAL_FRAMESIZE
#define sigcontext32		sigcontext
#define mcontext32		mcontext
#define ucontext32		ucontext
#define compat_siginfo_t	struct siginfo

#endif

typedef struct {
    struct sigcontext32 sctx;
    struct mcontext32 mctx;
    int abigap[56];
    char dummy[__SIGNAL_FRAMESIZE32];
} SignalFrame32;

typedef struct {
    compat_siginfo_t info;
    struct ucontext32 uc;
    int abigap[56];
    char dummy[__SIGNAL_FRAMESIZE32 + 16];
} RtSignalFrame32;

static int is_sigreturn_32_address(unsigned int nip, unsigned int fp)
{
	if (nip == fp + offsetof(SignalFrame32, mctx.mc_pad))
		return 1;
	if (vdso32_sigtramp && current->mm->context.vdso_base && nip == current->mm->context.vdso_base + vdso32_sigtramp)
		return 1;
	return 0;
}

static int is_rt_sigreturn_32_address(unsigned int nip, unsigned int fp)
{
	if (nip == fp + offsetof(RtSignalFrame32, uc.uc_mcontext.mc_pad))
		return 1;
	if (vdso32_rt_sigtramp && current->mm->context.vdso_base && nip == current->mm->context.vdso_base + vdso32_rt_sigtramp)
		return 1;
	return 0;
}

static int sane_signal_32_frame(unsigned int sp)
{
	SignalFrame32 __user *sf;
	unsigned int regs;

	sf = (SignalFrame32 __user *) (unsigned long) sp;
	if (read_user_stack_32((unsigned int __user *) &sf->sctx.regs, &regs))
		return 0;
	return regs == (unsigned long) &sf->mctx;
}

static int sane_rt_signal_32_frame(unsigned int sp)
{
	RtSignalFrame32 __user *sf;
	unsigned int regs;

	sf = (RtSignalFrame32 __user *) (unsigned long) sp;
	if (read_user_stack_32((unsigned int __user *) &sf->uc.uc_regs, &regs))
		return 0;
	return regs == (unsigned long) &sf->uc.uc_mcontext;
}

static unsigned int __user *signal_frame_32_regs(unsigned int sp, unsigned int next_sp, unsigned int next_ip)
{
	struct mcontext32 __user *mctx = NULL;
	SignalFrame32 __user *sf;
	RtSignalFrame32 __user *rt_sf;

	if (next_sp - sp >= sizeof(SignalFrame32) && is_sigreturn_32_address(next_ip, sp) && sane_signal_32_frame(sp)) {
		sf = (SignalFrame32 __user *) (unsigned long) sp;
		mctx = &sf->mctx;
	}

	if (!mctx && next_sp - sp >= sizeof(RtSignalFrame32) && is_rt_sigreturn_32_address(next_ip, sp) && sane_rt_signal_32_frame(sp)) {
		rt_sf = (RtSignalFrame32 __user *) (unsigned long) sp;
		mctx = &rt_sf->uc.uc_mcontext;
	}

	if (!mctx)
		return NULL;
	return mctx->mc_gregs;
}

static void perf_callchain_user_32(struct perf_callchain_entry *entry, struct pt_regs *regs)
{
	CallchainData cd = {regs->gpr[1], 0, perf_instruction_pointer(regs), regs->link, 0, NULL};
	unsigned int __user *uregs;

	perf_callchain_store(entry, cd.c);

	while (entry->nr < PERF_MAX_STACK_DEPTH) {
		cd.f = (unsigned int __user *) (unsigned long) cd.a;
		if (!valid_user_sp(cd.a, 0) || read_user_stack_32(cd.f, &cd.b))
			return;
		if (cd.e > 0 && read_user_stack_32(&cd.f[1], &cd.c))
			return;

		uregs = signal_frame_32_regs(cd.a, cd.b, cd.c);
		if (!uregs && cd.e <= 1)
			uregs = signal_frame_32_regs(cd.a, cd.b, cd.d);
		if (uregs) {
			if (read_user_stack_32(&uregs[PT_NIP], &cd.c) || read_user_stack_32(&uregs[PT_LNK], &cd.d) || read_user_stack_32(&uregs[PT_R1], &cd.a))
				return;
			cd.e = 0;
			perf_callchain_store(entry, PERF_CONTEXT_USER);
			perf_callchain_store(entry, cd.c);
			continue;
		}

		if (cd.e == 0)
			cd.c = cd.d;
		perf_callchain_store(entry, cd.c);
		++cd.e;
		cd.a = cd.b;
	}
}

void perf_callchain_user(struct perf_callchain_entry *entry, struct pt_regs *regs)
{
	if (current_is_64bit())
		perf_callchain_user_64(entry, regs);
	else
		perf_callchain_user_32(entry, regs);
}