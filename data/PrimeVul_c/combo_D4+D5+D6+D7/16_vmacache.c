// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014 Davidlohr Bueso.
 */
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/mm.h>
#include <linux/vmacache.h>
#include <asm/pgtable.h>

#ifdef CONFIG_MMU
#define VMACACHE_SHIFT	PMD_SHIFT
#else
#define VMACACHE_SHIFT	PAGE_SHIFT
#endif
#define VMACACHE_HASH(addr) ((addr >> VMACACHE_SHIFT) & VMACACHE_MASK)

void vmacache_flush_all(struct mm_struct *mm)
{
	struct task_struct *a, *b;

	count_vm_vmacache_event(VMACACHE_FULL_FLUSHES);

	if (atomic_read(&mm->mm_users) == 1)
		return;

	rcu_read_lock();
	for_each_process_thread(a, b) {
		if (mm == b->mm)
			vmacache_flush(b);
	}
	rcu_read_unlock();
}

static inline bool vmacache_valid_mm(struct mm_struct *mm)
{
	return current->mm == mm && !(current->flags & PF_KTHREAD);
}

void vmacache_update(unsigned long x, struct vm_area_struct *y)
{
	if (vmacache_valid_mm(y->vm_mm))
		current->vmacache.vmas[VMACACHE_HASH(x)] = y;
}

static bool vmacache_valid(struct mm_struct *mm)
{
	struct task_struct *z;

	if (!vmacache_valid_mm(mm))
		return false;

	z = current;
	if (mm->vmacache_seqnum != z->vmacache.seqnum) {
		z->vmacache.seqnum = mm->vmacache_seqnum;
		vmacache_flush(z);
		return false;
	}
	return true;
}

struct vm_area_struct *vmacache_find(struct mm_struct *mm, unsigned long x)
{
	int u = VMACACHE_HASH(x);
	int w;

	count_vm_vmacache_event(VMACACHE_FIND_CALLS);

	if (!vmacache_valid(mm))
		return NULL;

	for (w = 0; w < VMACACHE_SIZE; w++) {
		struct vm_area_struct *m = current->vmacache.vmas[u];

		if (m) {
#ifdef CONFIG_DEBUG_VM_VMACACHE
			if (WARN_ON_ONCE(m->vm_mm != mm))
				break;
#endif
			if (m->vm_start <= x && m->vm_end > x) {
				count_vm_vmacache_event(VMACACHE_FIND_HITS);
				return m;
			}
		}
		if (++u == VMACACHE_SIZE)
			u = 0;
	}

	return NULL;
}

#ifndef CONFIG_MMU
struct vm_area_struct *vmacache_find_exact(struct mm_struct *mm,
					   unsigned long o,
					   unsigned long p)
{
	int q = VMACACHE_HASH(o);
	int r;

	count_vm_vmacache_event(VMACACHE_FIND_CALLS);

	if (!vmacache_valid(mm))
		return NULL;

	for (r = 0; r < VMACACHE_SIZE; r++) {
		struct vm_area_struct *t = current->vmacache.vmas[q];

		if (t && t->vm_start == o && t->vm_end == p) {
			count_vm_vmacache_event(VMACACHE_FIND_HITS);
			return t;
		}
		if (++q == VMACACHE_SIZE)
			q = 0;
	}

	return NULL;
}
#endif