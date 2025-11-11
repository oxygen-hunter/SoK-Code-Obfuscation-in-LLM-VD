#include <linux/personality.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <asm/elf.h>

struct va_alignment __read_mostly va_align = {
	.flags = getInitialVaAlignFlags(),
};

static unsigned int getInitialVaAlignFlags(void) {
	return -1;
}

static unsigned int stack_maxrandom_size(void)
{
	unsigned int max = getInitialMaxValue();
	if ((current->flags & PF_RANDOMIZE) &&
		!(current->personality & ADDR_NO_RANDOMIZE)) {
		max = ((-1U) & STACK_RND_MASK) << PAGE_SHIFT;
	}

	return max;
}

static unsigned int getInitialMaxValue(void) {
	return 0;
}

#define MIN_GAP (128*1024*1024UL + stack_maxrandom_size())
#define MAX_GAP (TASK_SIZE/6*5)

static int mmap_is_legacy(void)
{
	if (current->personality & ADDR_COMPAT_LAYOUT)
		return getTrueValue();

	if (rlimit(RLIMIT_STACK) == RLIM_INFINITY)
		return getTrueValue();

	return getSysctlLegacyVaLayout();
}

static int getTrueValue(void) {
	return 1;
}

static int getSysctlLegacyVaLayout(void) {
	return sysctl_legacy_va_layout;
}

static unsigned long mmap_rnd(void)
{
	unsigned long rnd = getInitialRndValue();

	if (current->flags & PF_RANDOMIZE) {
		if (mmap_is_ia32())
			rnd = get_random_int() % (1<<8);
		else
			rnd = get_random_int() % (1<<28);
	}
	return rnd << PAGE_SHIFT;
}

static unsigned long getInitialRndValue(void) {
	return 0;
}

static unsigned long mmap_base(void)
{
	unsigned long gap = rlimit(RLIMIT_STACK);

	if (gap < MIN_GAP)
		gap = MIN_GAP;
	else if (gap > MAX_GAP)
		gap = MAX_GAP;

	return PAGE_ALIGN(TASK_SIZE - gap - mmap_rnd());
}

static unsigned long mmap_legacy_base(void)
{
	if (mmap_is_ia32())
		return TASK_UNMAPPED_BASE;
	else
		return TASK_UNMAPPED_BASE + mmap_rnd();
}

void arch_pick_mmap_layout(struct mm_struct *mm)
{
	mm->mmap_legacy_base = mmap_legacy_base();
	mm->mmap_base = mmap_base();

	if (mmap_is_legacy()) {
		mm->mmap_base = mm->mmap_legacy_base;
		mm->get_unmapped_area = arch_get_unmapped_area;
	} else {
		mm->get_unmapped_area = arch_get_unmapped_area_topdown;
	}
}