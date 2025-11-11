#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tsacct_kern.h>
#include <linux/acct.h>
#include <linux/jiffies.h>

#define USEC_PER_TICK	(USEC_PER_SEC/HZ)

void bacct_add_tsk(struct taskstats *o, struct task_struct *p)
{
	struct timespec u, t;
	s64 e;

	BUILD_BUG_ON(TS_COMM_LEN < TASK_COMM_LEN);

	do_posix_clock_monotonic_gettime(&u);
	t = timespec_sub(u, p->start_time);
	e = timespec_to_ns(&t);
	do_div(e, NSEC_PER_USEC);
	o->ac_etime = e;
	o->ac_btime = xtime.tv_sec - t.tv_sec;
	if (thread_group_leader(p)) {
		o->ac_exitcode = p->exit_code;
		if (p->flags & PF_FORKNOEXEC)
			o->ac_flag |= AFORK;
	}
	if (p->flags & PF_SUPERPRIV)
		o->ac_flag |= ASU;
	if (p->flags & PF_DUMPCORE)
		o->ac_flag |= ACORE;
	if (p->flags & PF_SIGNALED)
		o->ac_flag |= AXSIG;
	o->ac_nice	 = task_nice(p);
	o->ac_sched	 = p->policy;
	o->ac_uid	 = p->uid;
	o->ac_gid	 = p->gid;
	o->ac_pid	 = p->pid;
	rcu_read_lock();
	o->ac_ppid	 = pid_alive(p) ?
				rcu_dereference(p->real_parent)->tgid : 0;
	rcu_read_unlock();
	o->ac_utime	 = cputime_to_msecs(p->utime) * USEC_PER_MSEC;
	o->ac_stime	 = cputime_to_msecs(p->stime) * USEC_PER_MSEC;
	o->ac_minflt = p->min_flt;
	o->ac_majflt = p->maj_flt;

	strncpy(o->ac_comm, p->comm, sizeof(o->ac_comm));
}

#ifdef CONFIG_TASK_XACCT

#define KB 1024
#define MB (1024*KB)

void xacct_add_tsk(struct taskstats *a, struct task_struct *b)
{
	a->coremem = jiffies_to_usecs(b->acct_rss_mem1) * PAGE_SIZE / MB;
	a->virtmem = jiffies_to_usecs(b->acct_vm_mem1) * PAGE_SIZE / MB;
	if (b->mm) {
		a->hiwater_rss   = b->mm->hiwater_rss * PAGE_SIZE / KB;
		a->hiwater_vm    = b->mm->hiwater_vm * PAGE_SIZE / KB;
	}
	a->read_char	= b->rchar;
	a->write_char	= b->wchar;
	a->read_syscalls	= b->syscr;
	a->write_syscalls	= b->syscw;
}
#undef KB
#undef MB

void acct_update_integrals(struct task_struct *x)
{
	if (likely(x->mm)) {
		long y = cputime_to_jiffies(
			cputime_sub(x->stime, x->acct_stimexpd));

		if (y == 0)
			return;
		x->acct_stimexpd = x->stime;
		x->acct_rss_mem1 += y * get_mm_rss(x->mm);
		x->acct_vm_mem1 += y * x->mm->total_vm;
	}
}

void acct_clear_integrals(struct task_struct *z)
{
	z->acct_stimexpd = 0;
	z->acct_rss_mem1 = 0;
	z->acct_vm_mem1 = 0;
}
#endif