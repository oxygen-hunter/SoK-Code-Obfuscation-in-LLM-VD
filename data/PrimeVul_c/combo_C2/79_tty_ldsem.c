#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/tty.h>
#include <linux/sched.h>

#ifdef CONFIG_DEBUG_LOCK_ALLOC
# define __acq(l, s, t, r, c, n, i)		\
				lock_acquire(&(l)->dep_map, s, t, r, c, n, i)
# define __rel(l, n, i)				\
				lock_release(&(l)->dep_map, n, i)
# ifdef CONFIG_PROVE_LOCKING
#  define lockdep_acquire(l, s, t, i)		__acq(l, s, t, 0, 2, NULL, i)
#  define lockdep_acquire_nest(l, s, t, n, i)	__acq(l, s, t, 0, 2, n, i)
#  define lockdep_acquire_read(l, s, t, i)	__acq(l, s, t, 1, 2, NULL, i)
#  define lockdep_release(l, n, i)		__rel(l, n, i)
# else
#  define lockdep_acquire(l, s, t, i)		__acq(l, s, t, 0, 1, NULL, i)
#  define lockdep_acquire_nest(l, s, t, n, i)	__acq(l, s, t, 0, 1, n, i)
#  define lockdep_acquire_read(l, s, t, i)	__acq(l, s, t, 1, 1, NULL, i)
#  define lockdep_release(l, n, i)		__rel(l, n, i)
# endif
#else
# define lockdep_acquire(l, s, t, i)		do { } while (0)
# define lockdep_acquire_nest(l, s, t, n, i)	do { } while (0)
# define lockdep_acquire_read(l, s, t, i)	do { } while (0)
# define lockdep_release(l, n, i)		do { } while (0)
#endif

#ifdef CONFIG_LOCK_STAT
# define lock_stat(_lock, stat)		lock_##stat(&(_lock)->dep_map, _RET_IP_)
#else
# define lock_stat(_lock, stat)		do { } while (0)
#endif

#if BITS_PER_LONG == 64
# define LDSEM_ACTIVE_MASK	0xffffffffL
#else
# define LDSEM_ACTIVE_MASK	0x0000ffffL
#endif

#define LDSEM_UNLOCKED		0L
#define LDSEM_ACTIVE_BIAS	1L
#define LDSEM_WAIT_BIAS		(-LDSEM_ACTIVE_MASK-1)
#define LDSEM_READ_BIAS		LDSEM_ACTIVE_BIAS
#define LDSEM_WRITE_BIAS	(LDSEM_WAIT_BIAS + LDSEM_ACTIVE_BIAS)

struct ldsem_waiter {
	struct list_head list;
	struct task_struct *task;
};

static inline long ldsem_atomic_update(long delta, struct ld_semaphore *sem)
{
	return atomic_long_add_return(delta, (atomic_long_t *)&sem->count);
}

static inline int ldsem_cmpxchg(long *old, long new, struct ld_semaphore *sem)
{
	long tmp = *old;
	*old = atomic_long_cmpxchg(&sem->count, *old, new);
	return *old == tmp;
}

void __init_ldsem(struct ld_semaphore *sem, const char *name,
		  struct lock_class_key *key)
{
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
#ifdef CONFIG_DEBUG_LOCK_ALLOC
				debug_check_no_locks_freed((void *)sem, sizeof(*sem));
				lockdep_init_map(&sem->dep_map, name, key, 0);
#endif
				sem->count = LDSEM_UNLOCKED;
				sem->wait_readers = 0;
				raw_spin_lock_init(&sem->wait_lock);
				INIT_LIST_HEAD(&sem->read_wait);
				INIT_LIST_HEAD(&sem->write_wait);
				state = 1;
				break;
			case 1:
				return;
		}
	}
}

static void __ldsem_wake_readers(struct ld_semaphore *sem)
{
	int state = 0;
	while (1) {
		switch (state) {
			struct ldsem_waiter *waiter, *next;
			struct task_struct *tsk;
			long adjust, count;
			case 0:
				adjust = sem->wait_readers * (LDSEM_ACTIVE_BIAS - LDSEM_WAIT_BIAS);
				count = ldsem_atomic_update(adjust, sem);
				state = 1;
				break;
			case 1:
				if (count > 0)
					state = 3;
				else if (ldsem_cmpxchg(&count, count - adjust, sem))
					return;
				break;
			case 2:
				state = 1;
				break;
			case 3:
				list_for_each_entry_safe(waiter, next, &sem->read_wait, list) {
					tsk = waiter->task;
					smp_mb();
					waiter->task = NULL;
					wake_up_process(tsk);
					put_task_struct(tsk);
				}
				INIT_LIST_HEAD(&sem->read_wait);
				sem->wait_readers = 0;
				return;
		}
	}
}

static inline int writer_trylock(struct ld_semaphore *sem)
{
	int state = 0;
	while (1) {
		switch (state) {
			long count;
			case 0:
				count = ldsem_atomic_update(LDSEM_ACTIVE_BIAS, sem);
				state = 1;
				break;
			case 1:
				if ((count & LDSEM_ACTIVE_MASK) == LDSEM_ACTIVE_BIAS)
					return 1;
				else if (ldsem_cmpxchg(&count, count - LDSEM_ACTIVE_BIAS, sem))
					return 0;
				break;
			case 2:
				state = 1;
				break;
		}
	}
}

static void __ldsem_wake_writer(struct ld_semaphore *sem)
{
	int state = 0;
	while (1) {
		switch (state) {
			struct ldsem_waiter *waiter;
			case 0:
				waiter = list_entry(sem->write_wait.next, struct ldsem_waiter, list);
				wake_up_process(waiter->task);
				state = 1;
				break;
			case 1:
				return;
		}
	}
}

static void __ldsem_wake(struct ld_semaphore *sem)
{
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				if (!list_empty(&sem->write_wait))
					__ldsem_wake_writer(sem);
				else if (!list_empty(&sem->read_wait))
					__ldsem_wake_readers(sem);
				state = 1;
				break;
			case 1:
				return;
		}
	}
}

static void ldsem_wake(struct ld_semaphore *sem)
{
	int state = 0;
	while (1) {
		switch (state) {
			unsigned long flags;
			case 0:
				raw_spin_lock_irqsave(&sem->wait_lock, flags);
				__ldsem_wake(sem);
				raw_spin_unlock_irqrestore(&sem->wait_lock, flags);
				state = 1;
				break;
			case 1:
				return;
		}
	}
}

static struct ld_semaphore __sched *
down_read_failed(struct ld_semaphore *sem, long count, long timeout)
{
	int state = 0;
	while (1) {
		switch (state) {
			struct ldsem_waiter waiter;
			struct task_struct *tsk = current;
			long adjust = -LDSEM_ACTIVE_BIAS + LDSEM_WAIT_BIAS;
			case 0:
				raw_spin_lock_irq(&sem->wait_lock);
				state = 1;
				break;
			case 1:
				if (ldsem_cmpxchg(&count, count + adjust, sem))
					state = 2;
				else if (count > 0) {
					raw_spin_unlock_irq(&sem->wait_lock);
					return sem;
				}
				break;
			case 2:
				list_add_tail(&waiter.list, &sem->read_wait);
				sem->wait_readers++;
				waiter.task = tsk;
				get_task_struct(tsk);
				if ((count & LDSEM_ACTIVE_MASK) == 0)
					__ldsem_wake(sem);
				raw_spin_unlock_irq(&sem->wait_lock);
				state = 3;
				break;
			case 3:
				set_task_state(tsk, TASK_UNINTERRUPTIBLE);
				if (!waiter.task)
					return sem;
				else if (!timeout)
					return NULL;
				timeout = schedule_timeout(timeout);
				break;
		}
	}
}

static struct ld_semaphore __sched *
down_write_failed(struct ld_semaphore *sem, long count, long timeout)
{
	int state = 0;
	while (1) {
		switch (state) {
			struct ldsem_waiter waiter;
			struct task_struct *tsk = current;
			long adjust = -LDSEM_ACTIVE_BIAS;
			int locked = 0;
			case 0:
				raw_spin_lock_irq(&sem->wait_lock);
				state = 1;
				break;
			case 1:
				if (ldsem_cmpxchg(&count, count + adjust, sem))
					state = 2;
				else if ((count & LDSEM_ACTIVE_MASK) == LDSEM_ACTIVE_BIAS) {
					raw_spin_unlock_irq(&sem->wait_lock);
					return sem;
				}
				break;
			case 2:
				list_add_tail(&waiter.list, &sem->write_wait);
				waiter.task = tsk;
				set_task_state(tsk, TASK_UNINTERRUPTIBLE);
				state = 3;
				break;
			case 3:
				if (!timeout)
					return NULL;
				raw_spin_unlock_irq(&sem->wait_lock);
				timeout = schedule_timeout(timeout);
				raw_spin_lock_irq(&sem->wait_lock);
				set_task_state(tsk, TASK_UNINTERRUPTIBLE);
				if ((locked = writer_trylock(sem)))
					return sem;
				break;
			case 4:
				ldsem_atomic_update(-LDSEM_WAIT_BIAS, sem);
				list_del(&waiter.list);
				raw_spin_unlock_irq(&sem->wait_lock);
				__set_task_state(tsk, TASK_RUNNING);
				return NULL;
		}
	}
}

static inline int __ldsem_down_read_nested(struct ld_semaphore *sem,
					   int subclass, long timeout)
{
	int state = 0;
	while (1) {
		switch (state) {
			long count;
			case 0:
				lockdep_acquire_read(sem, subclass, 0, _RET_IP_);
				count = ldsem_atomic_update(LDSEM_READ_BIAS, sem);
				if (count <= 0) {
					lock_stat(sem, contended);
					if (!down_read_failed(sem, count, timeout)) {
						lockdep_release(sem, 1, _RET_IP_);
						return 0;
					}
				}
				lock_stat(sem, acquired);
				return 1;
		}
	}
}

static inline int __ldsem_down_write_nested(struct ld_semaphore *sem,
					    int subclass, long timeout)
{
	int state = 0;
	while (1) {
		switch (state) {
			long count;
			case 0:
				lockdep_acquire(sem, subclass, 0, _RET_IP_);
				count = ldsem_atomic_update(LDSEM_WRITE_BIAS, sem);
				if ((count & LDSEM_ACTIVE_MASK) != LDSEM_ACTIVE_BIAS) {
					lock_stat(sem, contended);
					if (!down_write_failed(sem, count, timeout)) {
						lockdep_release(sem, 1, _RET_IP_);
						return 0;
					}
				}
				lock_stat(sem, acquired);
				return 1;
		}
	}
}

int __sched ldsem_down_read(struct ld_semaphore *sem, long timeout)
{
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				might_sleep();
				return __ldsem_down_read_nested(sem, 0, timeout);
		}
	}
}

int ldsem_down_read_trylock(struct ld_semaphore *sem)
{
	int state = 0;
	while (1) {
		switch (state) {
			long count;
			case 0:
				count = sem->count;
				state = 1;
				break;
			case 1:
				if (count >= 0 && ldsem_cmpxchg(&count, count + LDSEM_READ_BIAS, sem)) {
					lockdep_acquire_read(sem, 0, 1, _RET_IP_);
					lock_stat(sem, acquired);
					return 1;
				}
				break;
		}
	}
}

int __sched ldsem_down_write(struct ld_semaphore *sem, long timeout)
{
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				might_sleep();
				return __ldsem_down_write_nested(sem, 0, timeout);
		}
	}
}

int ldsem_down_write_trylock(struct ld_semaphore *sem)
{
	int state = 0;
	while (1) {
		switch (state) {
			long count;
			case 0:
				count = sem->count;
				state = 1;
				break;
			case 1:
				if ((count & LDSEM_ACTIVE_MASK) == 0 &&
					ldsem_cmpxchg(&count, count + LDSEM_WRITE_BIAS, sem)) {
					lockdep_acquire(sem, 0, 1, _RET_IP_);
					lock_stat(sem, acquired);
					return 1;
				}
				break;
		}
	}
}

void ldsem_up_read(struct ld_semaphore *sem)
{
	int state = 0;
	while (1) {
		switch (state) {
			long count;
			case 0:
				lockdep_release(sem, 1, _RET_IP_);
				count = ldsem_atomic_update(-LDSEM_READ_BIAS, sem);
				if (count < 0 && (count & LDSEM_ACTIVE_MASK) == 0)
					ldsem_wake(sem);
				state = 1;
				break;
			case 1:
				return;
		}
	}
}

void ldsem_up_write(struct ld_semaphore *sem)
{
	int state = 0;
	while (1) {
		switch (state) {
			long count;
			case 0:
				lockdep_release(sem, 1, _RET_IP_);
				count = ldsem_atomic_update(-LDSEM_WRITE_BIAS, sem);
				if (count < 0)
					ldsem_wake(sem);
				state = 1;
				break;
			case 1:
				return;
		}
	}
}

#ifdef CONFIG_DEBUG_LOCK_ALLOC

int ldsem_down_read_nested(struct ld_semaphore *sem, int subclass, long timeout)
{
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				might_sleep();
				return __ldsem_down_read_nested(sem, subclass, timeout);
		}
	}
}

int ldsem_down_write_nested(struct ld_semaphore *sem, int subclass,
			    long timeout)
{
	int state = 0;
	while (1) {
		switch (state) {
			case 0:
				might_sleep();
				return __ldsem_down_write_nested(sem, subclass, timeout);
		}
	}
}

#endif