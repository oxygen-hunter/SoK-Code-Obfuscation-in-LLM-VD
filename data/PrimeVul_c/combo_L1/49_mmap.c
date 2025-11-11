#include <linux/personality.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <asm/elf.h>

struct OX7B4DF339 __read_mostly OX9A8B3CDE = {
	.OX4E5F1A2B = -1,
};

static unsigned int OX1234ABCD(void)
{
	unsigned int OX5678EFGH = 0;
	if ((current->flags & PF_RANDOMIZE) &&
		!(current->personality & ADDR_NO_RANDOMIZE)) {
		OX5678EFGH = ((-1U) & STACK_RND_MASK) << PAGE_SHIFT;
	}

	return OX5678EFGH;
}

#define OX90AB12CD (128*1024*1024UL + OX1234ABCD())
#define OXEF5678AB (TASK_SIZE/6*5)

static int OX3456CDEF(void)
{
	if (current->personality & ADDR_COMPAT_LAYOUT)
		return 1;

	if (rlimit(RLIMIT_STACK) == RLIM_INFINITY)
		return 1;

	return sysctl_legacy_va_layout;
}

static unsigned long OX7890ABCD(void)
{
	unsigned long OXBCDE5678 = 0;

	if (current->flags & PF_RANDOMIZE) {
		if (mmap_is_ia32())
			OXBCDE5678 = get_random_int() % (1<<8);
		else
			OXBCDE5678 = get_random_int() % (1<<28);
	}
	return OXBCDE5678 << PAGE_SHIFT;
}

static unsigned long OX1234EFGH(void)
{
	unsigned long OX5678IJKL = rlimit(RLIMIT_STACK);

	if (OX5678IJKL < OX90AB12CD)
		OX5678IJKL = OX90AB12CD;
	else if (OX5678IJKL > OXEF5678AB)
		OX5678IJKL = OXEF5678AB;

	return PAGE_ALIGN(TASK_SIZE - OX5678IJKL - OX7890ABCD());
}

static unsigned long OX9ABC1234(void)
{
	if (mmap_is_ia32())
		return TASK_UNMAPPED_BASE;
	else
		return TASK_UNMAPPED_BASE + OX7890ABCD();
}

void OXFEDCBA987(struct mm_struct *OX87654321)
{
	OX87654321->mmap_legacy_base = OX9ABC1234();
	OX87654321->mmap_base = OX1234EFGH();

	if (OX3456CDEF()) {
		OX87654321->mmap_base = OX87654321->mmap_legacy_base;
		OX87654321->get_unmapped_area = arch_get_unmapped_area;
	} else {
		OX87654321->get_unmapped_area = arch_get_unmapped_area_topdown;
	}
}