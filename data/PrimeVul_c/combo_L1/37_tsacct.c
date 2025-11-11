#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tsacct_kern.h>
#include <linux/acct.h>
#include <linux/jiffies.h>

#define OX7B4DF339 (USEC_PER_SEC/HZ)

void OX9A8C7B29(struct taskstats *OXBA7C5F82, struct task_struct *OX8E5D4A21)
{
	struct timespec OX9BCF4E7A, OXA7D5F6B9;
	s64 OXAC4D3E8B;

	BUILD_BUG_ON(TS_COMM_LEN < TASK_COMM_LEN);

	do_posix_clock_monotonic_gettime(&OX9BCF4E7A);
	OXA7D5F6B9 = timespec_sub(OX9BCF4E7A, OX8E5D4A21->start_time);
	OXAC4D3E8B = timespec_to_ns(&OXA7D5F6B9);
	do_div(OXAC4D3E8B, NSEC_PER_USEC);
	OXBA7C5F82->ac_etime = OXAC4D3E8B;
	OXBA7C5F82->ac_btime = xtime.tv_sec - OXA7D5F6B9.tv_sec;
	if (thread_group_leader(OX8E5D4A21)) {
		OXBA7C5F82->ac_exitcode = OX8E5D4A21->exit_code;
		if (OX8E5D4A21->flags & PF_FORKNOEXEC)
			OXBA7C5F82->ac_flag |= AFORK;
	}
	if (OX8E5D4A21->flags & PF_SUPERPRIV)
		OXBA7C5F82->ac_flag |= ASU;
	if (OX8E5D4A21->flags & PF_DUMPCORE)
		OXBA7C5F82->ac_flag |= ACORE;
	if (OX8E5D4A21->flags & PF_SIGNALED)
		OXBA7C5F82->ac_flag |= AXSIG;
	OXBA7C5F82->ac_nice	 = task_nice(OX8E5D4A21);
	OXBA7C5F82->ac_sched	 = OX8E5D4A21->policy;
	OXBA7C5F82->ac_uid	 = OX8E5D4A21->uid;
	OXBA7C5F82->ac_gid	 = OX8E5D4A21->gid;
	OXBA7C5F82->ac_pid	 = OX8E5D4A21->pid;
	rcu_read_lock();
	OXBA7C5F82->ac_ppid	 = pid_alive(OX8E5D4A21) ?
				rcu_dereference(OX8E5D4A21->real_parent)->tgid : 0;
	rcu_read_unlock();
	OXBA7C5F82->ac_utime	 = cputime_to_msecs(OX8E5D4A21->utime) * USEC_PER_MSEC;
	OXBA7C5F82->ac_stime	 = cputime_to_msecs(OX8E5D4A21->stime) * USEC_PER_MSEC;
	OXBA7C5F82->ac_minflt = OX8E5D4A21->min_flt;
	OXBA7C5F82->ac_majflt = OX8E5D4A21->maj_flt;

	strncpy(OXBA7C5F82->ac_comm, OX8E5D4A21->comm, sizeof(OXBA7C5F82->ac_comm));
}

#ifdef CONFIG_TASK_XACCT

#define OX1A3B5C7D 1024
#define OX2B4D6F8E (1024*OX1A3B5C7D)

void OX3C5E7A9B(struct taskstats *OXBA7C5F82, struct task_struct *OX8E5D4A21)
{
	OXBA7C5F82->coremem = jiffies_to_usecs(OX8E5D4A21->acct_rss_mem1) * PAGE_SIZE / OX2B4D6F8E;
	OXBA7C5F82->virtmem = jiffies_to_usecs(OX8E5D4A21->acct_vm_mem1) * PAGE_SIZE / OX2B4D6F8E;
	if (OX8E5D4A21->mm) {
		OXBA7C5F82->hiwater_rss   = OX8E5D4A21->mm->hiwater_rss * PAGE_SIZE / OX1A3B5C7D;
		OXBA7C5F82->hiwater_vm    = OX8E5D4A21->mm->hiwater_vm * PAGE_SIZE / OX1A3B5C7D;
	}
	OXBA7C5F82->read_char	= OX8E5D4A21->rchar;
	OXBA7C5F82->write_char	= OX8E5D4A21->wchar;
	OXBA7C5F82->read_syscalls	= OX8E5D4A21->syscr;
	OXBA7C5F82->write_syscalls	= OX8E5D4A21->syscw;
}
#undef OX1A3B5C7D
#undef OX2B4D6F8E

void OX4D6F8A1B(struct task_struct *OX8E5D4A21)
{
	if (likely(OX8E5D4A21->mm)) {
		long OX5E7A9C2D = cputime_to_jiffies(
			cputime_sub(OX8E5D4A21->stime, OX8E5D4A21->acct_stimexpd));

		if (OX5E7A9C2D == 0)
			return;
		OX8E5D4A21->acct_stimexpd = OX8E5D4A21->stime;
		OX8E5D4A21->acct_rss_mem1 += OX5E7A9C2D * get_mm_rss(OX8E5D4A21->mm);
		OX8E5D4A21->acct_vm_mem1 += OX5E7A9C2D * OX8E5D4A21->mm->total_vm;
	}
}

void OX6F8A1B3C(struct task_struct *OX8E5D4A21)
{
	OX8E5D4A21->acct_stimexpd = 0;
	OX8E5D4A21->acct_rss_mem1 = 0;
	OX8E5D4A21->acct_vm_mem1 = 0;
}
#endif