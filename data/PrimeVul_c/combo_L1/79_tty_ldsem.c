#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/atomic.h>
#include <linux/tty.h>
#include <linux/sched.h>

#ifdef CONFIG_DEBUG_LOCK_ALLOC
# define OX5F9E2F3A(l, s, t, r, c, n, i)		\
				lock_acquire(&(l)->dep_map, s, t, r, c, n, i)
# define OX1E7B3D4A(l, n, i)				\
				lock_release(&(l)->dep_map, n, i)
# ifdef CONFIG_PROVE_LOCKING
#  define OX6C1A9E2D(l, s, t, i)		OX5F9E2F3A(l, s, t, 0, 2, NULL, i)
#  define OX2E3B7C4F(l, s, t, n, i)	OX5F9E2F3A(l, s, t, 0, 2, n, i)
#  define OX9B8F3A6C(l, s, t, i)	OX5F9E2F3A(l, s, t, 1, 2, NULL, i)
#  define OX4D2F1B8C(l, n, i)		OX1E7B3D4A(l, n, i)
# else
#  define OX6C1A9E2D(l, s, t, i)		OX5F9E2F3A(l, s, t, 0, 1, NULL, i)
#  define OX2E3B7C4F(l, s, t, n, i)	OX5F9E2F3A(l, s, t, 0, 1, n, i)
#  define OX9B8F3A6C(l, s, t, i)	OX5F9E2F3A(l, s, t, 1, 1, NULL, i)
#  define OX4D2F1B8C(l, n, i)		OX1E7B3D4A(l, n, i)
# endif
#else
# define OX6C1A9E2D(l, s, t, i)		do { } while (0)
# define OX2E3B7C4F(l, s, t, n, i)	do { } while (0)
# define OX9B8F3A6C(l, s, t, i)	do { } while (0)
# define OX4D2F1B8C(l, n, i)		do { } while (0)
#endif

#ifdef CONFIG_LOCK_STAT
# define OX3E7F2B9A(_lock, stat)		lock_##stat(&(_lock)->dep_map, _RET_IP_)
#else
# define OX3E7F2B9A(_lock, stat)		do { } while (0)
#endif

#if BITS_PER_LONG == 64
# define OX9A3F4B5C	0xffffffffL
#else
# define OX9A3F4B5C	0x0000ffffL
#endif

#define OX1F7D2B3A		0L
#define OX5B3C8A1D	1L
#define OX7D1A5C6E	(-OX9A3F4B5C-1)
#define OX6C8F9A2D		OX5B3C8A1D
#define OX8A1D5B3C	(OX7D1A5C6E + OX5B3C8A1D)

struct OX8F3A6C7B {
	struct list_head OX7D5B2C1E;
	struct task_struct *OX4B3E9A6D;
};

static inline long OX6E9B3C8F(long OX2D1A7C5B, struct OX9E2D1B3C *OX5C6A8B9F)
{
	return atomic_long_add_return(OX2D1A7C5B, (atomic_long_t *)&OX5C6A8B9F->OX2F3B7D1A);
}

static inline int OX5A3F9B7D(long *OX3C4B5A8E, long OX6F2D9B1A, struct OX9E2D1B3C *OX5C6A8B9F)
{
	long OX4B7C2E1A = *OX3C4B5A8E;
	*OX3C4B5A8E = atomic_long_cmpxchg(&OX5C6A8B9F->OX2F3B7D1A, *OX3C4B5A8E, OX6F2D9B1A);
	return *OX3C4B5A8E == OX4B7C2E1A;
}

void OX2A6F3B9C(struct OX9E2D1B3C *OX5C6A8B9F, const char *OX6E1A8F3C,
		  struct lock_class_key *OX7B2C5D1A)
{
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	debug_check_no_locks_freed((void *)OX5C6A8B9F, sizeof(*OX5C6A8B9F));
	lockdep_init_map(&OX5C6A8B9F->dep_map, OX6E1A8F3C, OX7B2C5D1A, 0);
#endif
	OX5C6A8B9F->OX2F3B7D1A = OX1F7D2B3A;
	OX5C6A8B9F->OX9F8B2C1D = 0;
	raw_spin_lock_init(&OX5C6A8B9F->OX2D7F1B3C);
	INIT_LIST_HEAD(&OX5C6A8B9F->OX8C2A5D3F);
	INIT_LIST_HEAD(&OX5C6A8B9F->OX7A9B5C2D);
}

static void OX1E9A3F7B(struct OX9E2D1B3C *OX5C6A8B9F)
{
	struct OX8F3A6C7B *OX4B3E9A6D, *OX2D7F5C1A;
	struct task_struct *OX9B5A7E3C;
	long OX3C8F2D7A, OX7B3E1A6F;

	OX3C8F2D7A = OX5C6A8B9F->OX9F8B2C1D * (OX5B3C8A1D - OX7D1A5C6E);
	OX7B3E1A6F = OX6E9B3C8F(OX3C8F2D7A, OX5C6A8B9F);
	do {
		if (OX7B3E1A6F > 0)
			break;
		if (OX5A3F9B7D(&OX7B3E1A6F, OX7B3E1A6F - OX3C8F2D7A, OX5C6A8B9F))
			return;
	} while (1);

	list_for_each_entry_safe(OX4B3E9A6D, OX2D7F5C1A, &OX5C6A8B9F->OX8C2A5D3F, OX7D5B2C1E) {
		OX9B5A7E3C = OX4B3E9A6D->OX4B3E9A6D;
		smp_mb();
		OX4B3E9A6D->OX4B3E9A6D = NULL;
		wake_up_process(OX9B5A7E3C);
		put_task_struct(OX9B5A7E3C);
	}
	INIT_LIST_HEAD(&OX5C6A8B9F->OX8C2A5D3F);
	OX5C6A8B9F->OX9F8B2C1D = 0;
}

static inline int OX3A9F6B2D(struct OX9E2D1B3C *OX5C6A8B9F)
{
	long OX7B2F1A3D = OX6E9B3C8F(OX5B3C8A1D, OX5C6A8B9F);
	do {
		if ((OX7B2F1A3D & OX9A3F4B5C) == OX5B3C8A1D)
			return 1;
		if (OX5A3F9B7D(&OX7B2F1A3D, OX7B2F1A3D - OX5B3C8A1D, OX5C6A8B9F))
			return 0;
	} while (1);
}

static void OX6D9B4F1A(struct OX9E2D1B3C *OX5C6A8B9F)
{
	struct OX8F3A6C7B *OX4B3E9A6D;

	OX4B3E9A6D = list_entry(OX5C6A8B9F->OX7A9B5C2D.next, struct OX8F3A6C7B, OX7D5B2C1E);
	wake_up_process(OX4B3E9A6D->OX4B3E9A6D);
}

static void OX1D7F3A9B(struct OX9E2D1B3C *OX5C6A8B9F)
{
	if (!list_empty(&OX5C6A8B9F->OX7A9B5C2D))
		OX6D9B4F1A(OX5C6A8B9F);
	else if (!list_empty(&OX5C6A8B9F->OX8C2A5D3F))
		OX1E9A3F7B(OX5C6A8B9F);
}

static void OX7B9A5C2D(struct OX9E2D1B3C *OX5C6A8B9F)
{
	unsigned long OX2F3C7A1D;

	raw_spin_lock_irqsave(&OX5C6A8B9F->OX2D7F1B3C, OX2F3C7A1D);
	OX1D7F3A9B(OX5C6A8B9F);
	raw_spin_unlock_irqrestore(&OX5C6A8B9F->OX2D7F1B3C, OX2F3C7A1D);
}

static struct OX9E2D1B3C __sched *
OX2B7F9A3E(struct OX9E2D1B3C *OX5C6A8B9F, long OX7B3E9A5D, long OX1A6D5C3F)
{
	struct OX8F3A6C7B OX4B3E9A6D;
	struct task_struct *OX9B5A7E3C = current;
	long OX8F2D5B1A = -OX5B3C8A1D + OX7D1A5C6E;

	raw_spin_lock_irq(&OX5C6A8B9F->OX2D7F1B3C);

	do {
		if (OX5A3F9B7D(&OX7B3E9A5D, OX7B3E9A5D + OX8F2D5B1A, OX5C6A8B9F))
			break;
		if (OX7B3E9A5D > 0) {
			raw_spin_unlock_irq(&OX5C6A8B9F->OX2D7F1B3C);
			return OX5C6A8B9F;
		}
	} while (1);

	list_add_tail(&OX4B3E9A6D.OX7D5B2C1E, &OX5C6A8B9F->OX8C2A5D3F);
	OX5C6A8B9F->OX9F8B2C1D++;

	OX4B3E9A6D.OX4B3E9A6D = OX9B5A7E3C;
	get_task_struct(OX9B5A7E3C);

	if ((OX7B3E9A5D & OX9A3F4B5C) == 0)
		OX1D7F3A9B(OX5C6A8B9F);

	raw_spin_unlock_irq(&OX5C6A8B9F->OX2D7F1B3C);

	for (;;) {
		set_task_state(OX9B5A7E3C, TASK_UNINTERRUPTIBLE);

		if (!OX4B3E9A6D.OX4B3E9A6D)
			break;
		if (!OX1A6D5C3F)
			break;
		OX1A6D5C3F = schedule_timeout(OX1A6D5C3F);
	}

	__set_task_state(OX9B5A7E3C, TASK_RUNNING);

	if (!OX1A6D5C3F) {
		raw_spin_lock_irq(&OX5C6A8B9F->OX2D7F1B3C);
		if (OX4B3E9A6D.OX4B3E9A6D) {
			OX6E9B3C8F(-OX7D1A5C6E, OX5C6A8B9F);
			list_del(&OX4B3E9A6D.OX7D5B2C1E);
			raw_spin_unlock_irq(&OX5C6A8B9F->OX2D7F1B3C);
			put_task_struct(OX4B3E9A6D.OX4B3E9A6D);
			return NULL;
		}
		raw_spin_unlock_irq(&OX5C6A8B9F->OX2D7F1B3C);
	}

	return OX5C6A8B9F;
}

static struct OX9E2D1B3C __sched *
OX5A2D9F7B(struct OX9E2D1B3C *OX5C6A8B9F, long OX7B3E9A5D, long OX1A6D5C3F)
{
	struct OX8F3A6C7B OX4B3E9A6D;
	struct task_struct *OX9B5A7E3C = current;
	long OX8F2D5B1A = -OX5B3C8A1D;
	int OX6C9B3E7A = 0;

	raw_spin_lock_irq(&OX5C6A8B9F->OX2D7F1B3C);

	do {
		if (OX5A3F9B7D(&OX7B3E9A5D, OX7B3E9A5D + OX8F2D5B1A, OX5C6A8B9F))
			break;
		if ((OX7B3E9A5D & OX9A3F4B5C) == OX5B3C8A1D) {
			raw_spin_unlock_irq(&OX5C6A8B9F->OX2D7F1B3C);
			return OX5C6A8B9F;
		}
	} while (1);

	list_add_tail(&OX4B3E9A6D.OX7D5B2C1E, &OX5C6A8B9F->OX7A9B5C2D);

	OX4B3E9A6D.OX4B3E9A6D = OX9B5A7E3C;

	set_task_state(OX9B5A7E3C, TASK_UNINTERRUPTIBLE);
	for (;;) {
		if (!OX1A6D5C3F)
			break;
		raw_spin_unlock_irq(&OX5C6A8B9F->OX2D7F1B3C);
		OX1A6D5C3F = schedule_timeout(OX1A6D5C3F);
		raw_spin_lock_irq(&OX5C6A8B9F->OX2D7F1B3C);
		set_task_state(OX9B5A7E3C, TASK_UNINTERRUPTIBLE);
		if ((OX6C9B3E7A = OX3A9F6B2D(OX5C6A8B9F)))
			break;
	}

	if (!OX6C9B3E7A)
		OX6E9B3C8F(-OX7D1A5C6E, OX5C6A8B9F);
	list_del(&OX4B3E9A6D.OX7D5B2C1E);
	raw_spin_unlock_irq(&OX5C6A8B9F->OX2D7F1B3C);

	__set_task_state(OX9B5A7E3C, TASK_RUNNING);

	if (!OX6C9B3E7A)
		return NULL;
	return OX5C6A8B9F;
}

static inline int OX7A5D3E9B(struct OX9E2D1B3C *OX5C6A8B9F,
					   int OX3F9B2C6D, long OX1A6D5C3F)
{
	long OX7B3E9A5D;

	OX9B8F3A6C(OX5C6A8B9F, OX3F9B2C6D, 0, _RET_IP_);

	OX7B3E9A5D = OX6E9B3C8F(OX6C8F9A2D, OX5C6A8B9F);
	if (OX7B3E9A5D <= 0) {
		OX3E7F2B9A(OX5C6A8B9F, contended);
		if (!OX2B7F9A3E(OX5C6A8B9F, OX7B3E9A5D, OX1A6D5C3F)) {
			OX4D2F1B8C(OX5C6A8B9F, 1, _RET_IP_);
			return 0;
		}
	}
	OX3E7F2B9A(OX5C6A8B9F, acquired);
	return 1;
}

static inline int OX8A2F9B3D(struct OX9E2D1B3C *OX5C6A8B9F,
					    int OX3F9B2C6D, long OX1A6D5C3F)
{
	long OX7B3E9A5D;

	OX6C1A9E2D(OX5C6A8B9F, OX3F9B2C6D, 0, _RET_IP_);

	OX7B3E9A5D = OX6E9B3C8F(OX8A1D5B3C, OX5C6A8B9F);
	if ((OX7B3E9A5D & OX9A3F4B5C) != OX5B3C8A1D) {
		OX3E7F2B9A(OX5C6A8B9F, contended);
		if (!OX5A2D9F7B(OX5C6A8B9F, OX7B3E9A5D, OX1A6D5C3F)) {
			OX4D2F1B8C(OX5C6A8B9F, 1, _RET_IP_);
			return 0;
		}
	}
	OX3E7F2B9A(OX5C6A8B9F, acquired);
	return 1;
}

int __sched OX6B3E9A5D(struct OX9E2D1B3C *OX5C6A8B9F, long OX1A6D5C3F)
{
	might_sleep();
	return OX7A5D3E9B(OX5C6A8B9F, 0, OX1A6D5C3F);
}

int OX7D2F9A6B(struct OX9E2D1B3C *OX5C6A8B9F)
{
	long OX3C7F5A2D = OX5C6A8B9F->OX2F3B7D1A;

	while (OX3C7F5A2D >= 0) {
		if (OX5A3F9B7D(&OX3C7F5A2D, OX3C7F5A2D + OX6C8F9A2D, OX5C6A8B9F)) {
			OX9B8F3A6C(OX5C6A8B9F, 0, 1, _RET_IP_);
			OX3E7F2B9A(OX5C6A8B9F, acquired);
			return 1;
		}
	}
	return 0;
}

int __sched OX8F2B7A3D(struct OX9E2D1B3C *OX5C6A8B9F, long OX1A6D5C3F)
{
	might_sleep();
	return OX8A2F9B3D(OX5C6A8B9F, 0, OX1A6D5C3F);
}

int OX9A5D3C7F(struct OX9E2D1B3C *OX5C6A8B9F)
{
	long OX3C7F5A2D = OX5C6A8B9F->OX2F3B7D1A;

	while ((OX3C7F5A2D & OX9A3F4B5C) == 0) {
		if (OX5A3F9B7D(&OX3C7F5A2D, OX3C7F5A2D + OX8A1D5B3C, OX5C6A8B9F)) {
			OX6C1A9E2D(OX5C6A8B9F, 0, 1, _RET_IP_);
			OX3E7F2B9A(OX5C6A8B9F, acquired);
			return 1;
		}
	}
	return 0;
}

void OX5F2A9B3D(struct OX9E2D1B3C *OX5C6A8B9F)
{
	long OX7B3E9A5D;

	OX4D2F1B8C(OX5C6A8B9F, 1, _RET_IP_);

	OX7B3E9A5D = OX6E9B3C8F(-OX6C8F9A2D, OX5C6A8B9F);
	if (OX7B3E9A5D < 0 && (OX7B3E9A5D & OX9A3F4B5C) == 0)
		OX7B9A5C2D(OX5C6A8B9F);
}

void OX8C1A7F3D(struct OX9E2D1B3C *OX5C6A8B9F)
{
	long OX7B3E9A5D;

	OX4D2F1B8C(OX5C6A8B9F, 1, _RET_IP_);

	OX7B3E9A5D = OX6E9B3C8F(-OX8A1D5B3C, OX5C6A8B9F);
	if (OX7B3E9A5D < 0)
		OX7B9A5C2D(OX5C6A8B9F);
}

#ifdef CONFIG_DEBUG_LOCK_ALLOC

int OX6F3D9A7B(struct OX9E2D1B3C *OX5C6A8B9F, int OX3F9B2C6D, long OX1A6D5C3F)
{
	might_sleep();
	return OX7A5D3E9B(OX5C6A8B9F, OX3F9B2C6D, OX1A6D5C3F);
}

int OX9B7D2A3C(struct OX9E2D1B3C *OX5C6A8B9F, int OX3F9B2C6D,
			    long OX1A6D5C3F)
{
	might_sleep();
	return OX8A2F9B3D(OX5C6A8B9F, OX3F9B2C6D, OX1A6D5C3F);
}

#endif