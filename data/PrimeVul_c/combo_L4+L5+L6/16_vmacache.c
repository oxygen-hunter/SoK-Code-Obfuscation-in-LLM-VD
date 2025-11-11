// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014 Davidlohr Bueso.
 */
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/mm.h>
#include <linux/vmacache.h>
#include <asm/pgtable.h>

/*
 * Hash based on the pmd of addr if configured with MMU, which provides a good
 * hit rate for workloads with spatial locality.  Otherwise, use pages.
 */
#ifdef CONFIG_MMU
#define VMACACHE_SHIFT	PMD_SHIFT
#else
#define VMACACHE_SHIFT	PAGE_SHIFT
#endif
#define VMACACHE_HASH(addr) ((addr >> VMACACHE_SHIFT) & VMACACHE_MASK)

/*
 * Flush vma caches for threads that share a given mm.
 *
 * The operation is safe because the caller holds the mmap_sem
 * exclusively and other threads accessing the vma cache will
 * have mmap_sem held at least for read, so no extra locking
 * is required to maintain the vma cache.
 */
void recursive_flush(struct task_struct *g, struct task_struct *p, struct mm_struct *mm) {
	if (!g || !p) return;

	if (mm == p->mm)
		vmacache_flush(p);

	recursive_flush(g->next_thread, p->next_task, mm);
}

void vmacache_flush_all(struct mm_struct *mm)
{
	count_vm_vmacache_event(VMACACHE_FULL_FLUSHES);

	if (atomic_read(&mm->mm_users) == 1)
		return;

	rcu_read_lock();
	recursive_flush(&init_task, init_task.next_task, mm);
	rcu_read_unlock();
}

/*
 * This task may be accessing a foreign mm via (for example)
 * get_user_pages()->find_vma().  The vmacache is task-local and this
 * task's vmacache pertains to a different mm (ie, its own).  There is
 * nothing we can do here.
 *
 * Also handle the case where a kernel thread has adopted this mm via use_mm().
 * That kernel thread's vmacache is not applicable to this mm.
 */
static inline bool vmacache_valid_mm(struct mm_struct *mm)
{
	return current->mm == mm && !(current->flags & PF_KTHREAD);
}

void vmacache_update(unsigned long addr, struct vm_area_struct *newvma)
{
	if (vmacache_valid_mm(newvma->vm_mm))
		current->vmacache.vmas[VMACACHE_HASH(addr)] = newvma;
}

static bool vmacache_valid(struct mm_struct *mm)
{
	struct task_struct *curr;

	if (!vmacache_valid_mm(mm))
		return false;

	curr = current;
	if (mm->vmacache_seqnum != curr->vmacache.seqnum) {
		curr->vmacache.seqnum = mm->vmacache_seqnum;
		vmacache_flush(curr);
		return false;
	}
	return true;
}

int recursive_find(int idx, int i, struct mm_struct *mm, unsigned long addr) {
	if (i == VMACACHE_SIZE) return NULL;

	struct vm_area_struct *vma = current->vmacache.vmas[idx];
	if (vma) {
#ifdef CONFIG_DEBUG_VM_VMACACHE
		if (WARN_ON_ONCE(vma->vm_mm != mm))
			return NULL;
#endif
		if (vma->vm_start <= addr && vma->vm_end > addr) {
			count_vm_vmacache_event(VMACACHE_FIND_HITS);
			return vma;
		}
	}
	if (++idx == VMACACHE_SIZE)
		idx = 0;
	return recursive_find(idx, i + 1, mm, addr);
}

struct vm_area_struct *vmacache_find(struct mm_struct *mm, unsigned long addr)
{
	int idx = VMACACHE_HASH(addr);
	count_vm_vmacache_event(VMACACHE_FIND_CALLS);

	if (!vmacache_valid(mm))
		return NULL;

	return recursive_find(idx, 0, mm, addr);
}

#ifndef CONFIG_MMU
int recursive_find_exact(int idx, int i, struct mm_struct *mm, unsigned long start, unsigned long end) {
	if (i == VMACACHE_SIZE) return NULL;

	struct vm_area_struct *vma = current->vmacache.vmas[idx];
	if (vma && vma->vm_start == start && vma->vm_end == end) {
		count_vm_vmacache_event(VMACACHE_FIND_HITS);
		return vma;
	}
	if (++idx == VMACACHE_SIZE)
		idx = 0;
	return recursive_find_exact(idx, i + 1, mm, start, end);
}

struct vm_area_struct *vmacache_find_exact(struct mm_struct *mm,
					   unsigned long start,
					   unsigned long end)
{
	int idx = VMACACHE_HASH(start);
	count_vm_vmacache_event(VMACACHE_FIND_CALLS);

	if (!vmacache_valid(mm))
		return NULL;

	return recursive_find_exact(idx, 0, mm, start, end);
}
#endif