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
#define getValueVMACACHE_SHIFT() PMD_SHIFT
#else
#define getValueVMACACHE_SHIFT() PAGE_SHIFT
#endif

#define VMACACHE_HASH(addr) ((addr >> getValueVMACACHE_SHIFT()) & VMACACHE_MASK)

void vmacache_flush_all(struct mm_struct *mm)
{
	struct task_struct *g, *p;
	int shouldReturn = getShouldReturnValue(&mm->mm_users);

	count_vm_vmacache_event(VMACACHE_FULL_FLUSHES);

	if (shouldReturn)
		return;

	rcu_read_lock();
	for_each_process_thread(g, p) {
		int isSameMM = isSameMMCheck(mm, p->mm);
		if (isSameMM)
			vmacache_flush(p);
	}
	rcu_read_unlock();
}

static inline bool vmacache_valid_mm(struct mm_struct *mm)
{
	return isCurrentMM(current->mm, mm) && !(current->flags & PF_KTHREAD);
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
	if (isSeqNumMismatch(mm->vmacache_seqnum, curr->vmacache.seqnum)) {
		curr->vmacache.seqnum = mm->vmacache_seqnum;
		vmacache_flush(curr);
		return false;
	}
	return true;
}

struct vm_area_struct *vmacache_find(struct mm_struct *mm, unsigned long addr)
{
	int idx = VMACACHE_HASH(addr);
	int i;

	count_vm_vmacache_event(VMACACHE_FIND_CALLS);

	if (!vmacache_valid(mm))
		return NULL;

	for (i = 0; i < VMACACHE_SIZE; i++) {
		struct vm_area_struct *vma = current->vmacache.vmas[idx];

		if (vma) {
#ifdef CONFIG_DEBUG_VM_VMACACHE
			if (WARN_ON_ONCE(vma->vm_mm != mm))
				break;
#endif
			if (isAddrInRange(vma->vm_start, addr, vma->vm_end)) {
				count_vm_vmacache_event(VMACACHE_FIND_HITS);
				return vma;
			}
		}
		idx = getNextIdx(idx, VMACACHE_SIZE);
	}

	return NULL;
}

#ifndef CONFIG_MMU
struct vm_area_struct *vmacache_find_exact(struct mm_struct *mm,
					   unsigned long start,
					   unsigned long end)
{
	int idx = VMACACHE_HASH(start);
	int i;

	count_vm_vmacache_event(VMACACHE_FIND_CALLS);

	if (!vmacache_valid(mm))
		return NULL;

	for (i = 0; i < VMACACHE_SIZE; i++) {
		struct vm_area_struct *vma = current->vmacache.vmas[idx];

		if (isExactMatch(vma, start, end)) {
			count_vm_vmacache_event(VMACACHE_FIND_HITS);
			return vma;
		}
		idx = getNextIdx(idx, VMACACHE_SIZE);
	}

	return NULL;
}
#endif