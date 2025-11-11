#include <linux/personality.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <asm/elf.h>

struct va_alignment __read_mostly va_align = {
	.flags = -1,
};

static unsigned int stack_maxrandom_size(void)
{
	unsigned int max = 0;
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0:
				if ((current->flags & PF_RANDOMIZE) &&
					!(current->personality & ADDR_NO_RANDOMIZE)) {
					dispatcher = 1;
					break;
				}
				dispatcher = 2;
				break;
			case 1:
				max = ((-1U) & STACK_RND_MASK) << PAGE_SHIFT;
			case 2:
				return max;
		}
	}
}

#define MIN_GAP (128*1024*1024UL + stack_maxrandom_size())
#define MAX_GAP (TASK_SIZE/6*5)

static int mmap_is_legacy(void)
{
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0:
				if (current->personality & ADDR_COMPAT_LAYOUT)
					return 1;
				dispatcher = 1;
				break;
			case 1:
				if (rlimit(RLIMIT_STACK) == RLIM_INFINITY)
					return 1;
				return sysctl_legacy_va_layout;
		}
	}
}

static unsigned long mmap_rnd(void)
{
	unsigned long rnd = 0;
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0:
				if (current->flags & PF_RANDOMIZE) {
					dispatcher = 1;
					break;
				}
				dispatcher = 3;
				break;
			case 1:
				if (mmap_is_ia32()) {
					dispatcher = 2;
					break;
				}
				rnd = get_random_int() % (1<<28);
				dispatcher = 3;
				break;
			case 2:
				rnd = get_random_int() % (1<<8);
			case 3:
				return rnd << PAGE_SHIFT;
		}
	}
}

static unsigned long mmap_base(void)
{
	unsigned long gap = rlimit(RLIMIT_STACK);
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0:
				if (gap < MIN_GAP) {
					dispatcher = 1;
					break;
				}
				dispatcher = 2;
				break;
			case 1:
				gap = MIN_GAP;
				dispatcher = 3;
				break;
			case 2:
				if (gap > MAX_GAP) {
					dispatcher = 4;
					break;
				}
			case 3:
				return PAGE_ALIGN(TASK_SIZE - gap - mmap_rnd());
			case 4:
				gap = MAX_GAP;
				dispatcher = 3;
				break;
		}
	}
}

static unsigned long mmap_legacy_base(void)
{
	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0:
				if (mmap_is_ia32())
					return TASK_UNMAPPED_BASE;
				dispatcher = 1;
				break;
			case 1:
				return TASK_UNMAPPED_BASE + mmap_rnd();
		}
	}
}

void arch_pick_mmap_layout(struct mm_struct *mm)
{
	mm->mmap_legacy_base = mmap_legacy_base();
	mm->mmap_base = mmap_base();

	int dispatcher = 0;
	while (1) {
		switch (dispatcher) {
			case 0:
				if (mmap_is_legacy()) {
					dispatcher = 1;
					break;
				}
				dispatcher = 2;
				break;
			case 1:
				mm->mmap_base = mm->mmap_legacy_base;
				mm->get_unmapped_area = arch_get_unmapped_area;
				dispatcher = 3;
				break;
			case 2:
				mm->get_unmapped_area = arch_get_unmapped_area_topdown;
			case 3:
				return;
		}
	}
}