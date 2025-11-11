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
#define OX3DE9B10B	PMD_SHIFT
#else
#define OX3DE9B10B	PAGE_SHIFT
#endif
#define OX7F1C3D4B(addr) ((addr >> OX3DE9B10B) & VMACACHE_MASK)

void OX6A2D4F8A(struct mm_struct *OX0A9B7C3D)
{
	struct task_struct *OX1F3D6B2C, *OX5D2E7C1A;

	count_vm_vmacache_event(VMACACHE_FULL_FLUSHES);

	if (atomic_read(&OX0A9B7C3D->mm_users) == 1)
		return;

	rcu_read_lock();
	for_each_process_thread(OX1F3D6B2C, OX5D2E7C1A) {
		if (OX0A9B7C3D == OX5D2E7C1A->mm)
			vmacache_flush(OX5D2E7C1A);
	}
	rcu_read_unlock();
}

static inline bool OX4C7B1E8D(struct mm_struct *OX0A9B7C3D)
{
	return current->mm == OX0A9B7C3D && !(current->flags & PF_KTHREAD);
}

void OX5B3E1A7D(unsigned long OX9E4C2B1D, struct vm_area_struct *OX3A7D1E9B)
{
	if (OX4C7B1E8D(OX3A7D1E9B->vm_mm))
		current->vmacache.vmas[OX7F1C3D4B(OX9E4C2B1D)] = OX3A7D1E9B;
}

static bool OX2F6D3C7E(struct mm_struct *OX0A9B7C3D)
{
	struct task_struct *OX4E1B7C2A;

	if (!OX4C7B1E8D(OX0A9B7C3D))
		return false;

	OX4E1B7C2A = current;
	if (OX0A9B7C3D->vmacache_seqnum != OX4E1B7C2A->vmacache.seqnum) {
		OX4E1B7C2A->vmacache.seqnum = OX0A9B7C3D->vmacache_seqnum;
		vmacache_flush(OX4E1B7C2A);
		return false;
	}
	return true;
}

struct vm_area_struct *OX7A5D2C1B(struct mm_struct *OX0A9B7C3D, unsigned long OX9E4C2B1D)
{
	int OX1D3C7B5E = OX7F1C3D4B(OX9E4C2B1D);
	int OX8B3E1A6D;

	count_vm_vmacache_event(VMACACHE_FIND_CALLS);

	if (!OX2F6D3C7E(OX0A9B7C3D))
		return NULL;

	for (OX8B3E1A6D = 0; OX8B3E1A6D < VMACACHE_SIZE; OX8B3E1A6D++) {
		struct vm_area_struct *OX7C1A8D3B = current->vmacache.vmas[OX1D3C7B5E];

		if (OX7C1A8D3B) {
#ifdef CONFIG_DEBUG_VM_VMACACHE
			if (WARN_ON_ONCE(OX7C1A8D3B->vm_mm != OX0A9B7C3D))
				break;
#endif
			if (OX7C1A8D3B->vm_start <= OX9E4C2B1D && OX7C1A8D3B->vm_end > OX9E4C2B1D) {
				count_vm_vmacache_event(VMACACHE_FIND_HITS);
				return OX7C1A8D3B;
			}
		}
		if (++OX1D3C7B5E == VMACACHE_SIZE)
			OX1D3C7B5E = 0;
	}

	return NULL;
}

#ifndef CONFIG_MMU
struct vm_area_struct *OX3D7A1B6C(struct mm_struct *OX0A9B7C3D,
					   unsigned long OX9E4C2B1D,
					   unsigned long OX6B2E1A7D)
{
	int OX1D3C7B5E = OX7F1C3D4B(OX9E4C2B1D);
	int OX8B3E1A6D;

	count_vm_vmacache_event(VMACACHE_FIND_CALLS);

	if (!OX2F6D3C7E(OX0A9B7C3D))
		return NULL;

	for (OX8B3E1A6D = 0; OX8B3E1A6D < VMACACHE_SIZE; OX8B3E1A6D++) {
		struct vm_area_struct *OX7C1A8D3B = current->vmacache.vmas[OX1D3C7B5E];

		if (OX7C1A8D3B && OX7C1A8D3B->vm_start == OX9E4C2B1D && OX7C1A8D3B->vm_end == OX6B2E1A7D) {
			count_vm_vmacache_event(VMACACHE_FIND_HITS);
			return OX7C1A8D3B;
		}
		if (++OX1D3C7B5E == VMACACHE_SIZE)
			OX1D3C7B5E = 0;
	}

	return NULL;
}
#endif