/*
 * tsacct.c - System accounting over taskstats interface
 *
 * Copyright (C) Jay Lan,	<jlan@sgi.com>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tsacct_kern.h>
#include <linux/acct.h>
#include <linux/jiffies.h>

#define USEC_PER_TICK	(USEC_PER_SEC/HZ)

void bacct_add_tsk(struct taskstats *stats, struct task_struct *tsk)
{
	struct timespec uptime, ts;
	s64 ac_etime;

	BUILD_BUG_ON(TS_COMM_LEN < TASK_COMM_LEN);

	do_posix_clock_monotonic_gettime(&uptime);
	ts = timespec_sub(uptime, tsk->start_time);
	ac_etime = timespec_to_ns(&ts);
	do_div(ac_etime, NSEC_PER_USEC);
	stats->ac_etime = ac_etime;
	stats->ac_btime = xtime.tv_sec - ts.tv_sec;
	if (thread_group_leader(tsk)) {
		stats->ac_exitcode = tsk->exit_code;
		if (tsk->flags & PF_FORKNOEXEC)
			stats->ac_flag |= AFORK;
	}
	if (tsk->flags & PF_SUPERPRIV)
		stats->ac_flag |= ASU;
	if (tsk->flags & PF_DUMPCORE)
		stats->ac_flag |= ACORE;
	if (tsk->flags & PF_SIGNALED)
		stats->ac_flag |= AXSIG;
	stats->ac_nice	 = task_nice(tsk);
	stats->ac_sched	 = tsk->policy;
	stats->ac_uid	 = tsk->uid;
	stats->ac_gid	 = tsk->gid;
	stats->ac_pid	 = tsk->pid;
	rcu_read_lock();
	stats->ac_ppid	 = pid_alive(tsk) ?
				rcu_dereference(tsk->real_parent)->tgid : 0;
	rcu_read_unlock();
	stats->ac_utime	 = cputime_to_msecs(tsk->utime) * USEC_PER_MSEC;
	stats->ac_stime	 = cputime_to_msecs(tsk->stime) * USEC_PER_MSEC;
	stats->ac_minflt = tsk->min_flt;
	stats->ac_majflt = tsk->maj_flt;

	strncpy(stats->ac_comm, tsk->comm, sizeof(stats->ac_comm));
}

#ifdef CONFIG_TASK_XACCT

#define KB 1024
#define MB (1024*KB)

void xacct_add_tsk(struct taskstats *stats, struct task_struct *p)
{
	stats->coremem = jiffies_to_usecs(p->acct_rss_mem1) * PAGE_SIZE / MB;
	stats->virtmem = jiffies_to_usecs(p->acct_vm_mem1) * PAGE_SIZE / MB;
	if (p->mm) {
		stats->hiwater_rss   = p->mm->hiwater_rss * PAGE_SIZE / KB;
		stats->hiwater_vm    = p->mm->hiwater_vm * PAGE_SIZE / KB;
	}
	stats->read_char	= p->rchar;
	stats->write_char	= p->wchar;
	stats->read_syscalls	= p->syscr;
	stats->write_syscalls	= p->syscw;
}
#undef KB
#undef MB

void acct_update_integrals(struct task_struct *tsk)
{
	if (likely(tsk->mm)) {
		long delta = cputime_to_jiffies(
			cputime_sub(tsk->stime, tsk->acct_stimexpd));

		if (delta == 0)
			return;
		tsk->acct_stimexpd = tsk->stime;
		tsk->acct_rss_mem1 += delta * get_mm_rss(tsk->mm);
		tsk->acct_vm_mem1 += delta * tsk->mm->total_vm;
	}
}

void acct_clear_integrals(struct task_struct *tsk)
{
	asm volatile (
		"movl $0, %[stimexpd]\n\t"
		"movl $0, %[rss_mem1]\n\t"
		"movl $0, %[vm_mem1]\n\t"
		: [stimexpd] "=m" (tsk->acct_stimexpd),
		  [rss_mem1] "=m" (tsk->acct_rss_mem1),
		  [vm_mem1] "=m" (tsk->acct_vm_mem1)
	);
}
#endif