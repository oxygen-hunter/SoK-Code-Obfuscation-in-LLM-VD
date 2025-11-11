#include <linux/personality.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <asm/elf.h>

struct alignment_va __read_mostly va_align = {
	.flags = -1,
};

static unsigned int random_stack_max_size(void)
{
	unsigned int v[1] = {0};
	if ((current->flags & PF_RANDOMIZE) &&
		!(current->personality & ADDR_NO_RANDOMIZE)) {
		v[0] = ((-1U) & STACK_RND_MASK) << PAGE_SHIFT;
	}
	return v[0];
}

#define GAP_MIN (128*1024*1024UL + random_stack_max_size())
#define GAP_MAX (TASK_SIZE/6*5)

static int check_mmap_legacy(void)
{
	int a[1];
	if (current->personality & ADDR_COMPAT_LAYOUT)
		a[0] = 1;
	else if (rlimit(RLIMIT_STACK) == RLIM_INFINITY)
		a[0] = 1;
	else
		a[0] = sysctl_legacy_va_layout;
	return a[0];
}

static unsigned long rnd_mmap(void)
{
	unsigned long r[1] = {0};

	if (current->flags & PF_RANDOMIZE) {
		if (mmap_is_ia32())
			r[0] = get_random_int() % (1<<8);
		else
			r[0] = get_random_int() % (1<<28);
	}
	return r[0] << PAGE_SHIFT;
}

static unsigned long base_mmap(void)
{
	unsigned long g[1] = {rlimit(RLIMIT_STACK)};

	if (g[0] < GAP_MIN)
		g[0] = GAP_MIN;
	else if (g[0] > GAP_MAX)
		g[0] = GAP_MAX;

	return PAGE_ALIGN(TASK_SIZE - g[0] - rnd_mmap());
}

static unsigned long legacy_base_mmap(void)
{
	if (mmap_is_ia32())
		return TASK_UNMAPPED_BASE;
	else
		return TASK_UNMAPPED_BASE + rnd_mmap();
}

void pick_mmap_arch_layout(struct mm_struct *mm)
{
	unsigned long base[2];
	base[0] = legacy_base_mmap();
	base[1] = base_mmap();
	mm->mmap_legacy_base = base[0];
	mm->mmap_base = base[1];

	if (check_mmap_legacy()) {
		mm->mmap_base = mm->mmap_legacy_base;
		mm->get_unmapped_area = arch_get_unmapped_area;
	} else {
		mm->get_unmapped_area = arch_get_unmapped_area_topdown;
	}
}