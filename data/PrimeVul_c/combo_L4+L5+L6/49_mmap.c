#include <linux/personality.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <asm/elf.h>

struct va_alignment __read_mostly va_align = {
	.flags = -1,
};

static unsigned int stack_maxrandom_size_rec(unsigned int max) {
	return ((current->flags & PF_RANDOMIZE) && !(current->personality & ADDR_NO_RANDOMIZE)) ? 
		(((-1U) & STACK_RND_MASK) << PAGE_SHIFT) : max;
}

static unsigned int stack_maxrandom_size(void) {
	return stack_maxrandom_size_rec(0);
}

#define MIN_GAP (128*1024*1024UL + stack_maxrandom_size())
#define MAX_GAP (TASK_SIZE/6*5)

static int rlimit_rec(int r) {
	return (r == RLIM_INFINITY) ? 1 : sysctl_legacy_va_layout;
}

static int mmap_is_legacy_rec(int r) {
	return (current->personality & ADDR_COMPAT_LAYOUT) ? 1 : rlimit_rec(rlimit(RLIMIT_STACK));
}

static int mmap_is_legacy(void) {
	return mmap_is_legacy_rec(0);
}

static unsigned long mmap_rnd_rec(unsigned long rnd) {
	if (current->flags & PF_RANDOMIZE) {
		if (mmap_is_ia32())
			rnd = get_random_int() % (1 << 8);
		else
			rnd = get_random_int() % (1 << 28);
	}
	return rnd << PAGE_SHIFT;
}

static unsigned long mmap_rnd(void) {
	return mmap_rnd_rec(0);
}

static unsigned long mmap_base_rec(unsigned long gap) {
	if (gap < MIN_GAP)
		return MIN_GAP;
	else if (gap > MAX_GAP)
		return MAX_GAP;
	return PAGE_ALIGN(TASK_SIZE - gap - mmap_rnd());
}

static unsigned long mmap_base(void) {
	return mmap_base_rec(rlimit(RLIMIT_STACK));
}

static unsigned long mmap_legacy_base_rec(unsigned long base) {
	return mmap_is_ia32() ? TASK_UNMAPPED_BASE : TASK_UNMAPPED_BASE + mmap_rnd();
}

static unsigned long mmap_legacy_base(void) {
	return mmap_legacy_base_rec(0);
}

void arch_pick_mmap_layout(struct mm_struct *mm) {
	mm->mmap_legacy_base = mmap_legacy_base();
	mm->mmap_base = mmap_base();
	mm->get_unmapped_area = mmap_is_legacy() ? arch_get_unmapped_area : arch_get_unmapped_area_topdown;
}