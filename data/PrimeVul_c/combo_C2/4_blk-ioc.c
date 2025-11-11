#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/bootmem.h>

#include "blk.h"

static struct kmem_cache *iocontext_cachep;

static void cfq_dtor(struct io_context *ioc) {
	int state = 0;
	while(1) {
		switch(state) {
			case 0:
				if (!hlist_empty(&ioc->cic_list)) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1: {
				struct cfq_io_context *cic;
				cic = list_entry(ioc->cic_list.first, struct cfq_io_context, cic_list);
				cic->dtor(ioc);
				state = 2;
				break;
			}
			case 2:
				return;
		}
	}
}

int put_io_context(struct io_context *ioc) {
	int state = 0;
	while(1) {
		switch(state) {
			case 0:
				if (ioc == NULL) {
					state = 1;
					break;
				}
				BUG_ON(atomic_long_read(&ioc->refcount) == 0);
				state = 2;
				break;
			case 1:
				return 1;
			case 2:
				if (atomic_long_dec_and_test(&ioc->refcount)) {
					rcu_read_lock();
					if (ioc->aic && ioc->aic->dtor)
						ioc->aic->dtor(ioc->aic);
					cfq_dtor(ioc);
					rcu_read_unlock();
					kmem_cache_free(iocontext_cachep, ioc);
					state = 3;
					break;
				}
				state = 4;
				break;
			case 3:
				return 1;
			case 4:
				return 0;
		}
	}
}
EXPORT_SYMBOL(put_io_context);

static void cfq_exit(struct io_context *ioc) {
	int state = 0;
	while(1) {
		switch(state) {
			case 0:
				rcu_read_lock();
				if (!hlist_empty(&ioc->cic_list)) {
					state = 1;
					break;
				}
				state = 3;
				break;
			case 1: {
				struct cfq_io_context *cic;
				cic = list_entry(ioc->cic_list.first, struct cfq_io_context, cic_list);
				cic->exit(ioc);
				state = 2;
				break;
			}
			case 2:
				state = 3;
				break;
			case 3:
				rcu_read_unlock();
				return;
		}
	}
}

void exit_io_context(void) {
	struct io_context *ioc;
	int state = 0;
	while(1) {
		switch(state) {
			case 0:
				task_lock(current);
				ioc = current->io_context;
				current->io_context = NULL;
				task_unlock(current);
				if (atomic_dec_and_test(&ioc->nr_tasks)) {
					state = 1;
					break;
				}
				state = 3;
				break;
			case 1:
				if (ioc->aic && ioc->aic->exit)
					ioc->aic->exit(ioc->aic);
				cfq_exit(ioc);
				put_io_context(ioc);
				state = 2;
				break;
			case 2:
				state = 3;
				break;
			case 3:
				return;
		}
	}
}

struct io_context *alloc_io_context(gfp_t gfp_flags, int node) {
	struct io_context *ret;
	int state = 0;
	while(1) {
		switch(state) {
			case 0:
				ret = kmem_cache_alloc_node(iocontext_cachep, gfp_flags, node);
				if (ret) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				atomic_long_set(&ret->refcount, 1);
				atomic_set(&ret->nr_tasks, 1);
				spin_lock_init(&ret->lock);
				ret->ioprio_changed = 0;
				ret->ioprio = 0;
				ret->last_waited = jiffies;
				ret->nr_batch_requests = 0;
				ret->aic = NULL;
				INIT_RADIX_TREE(&ret->radix_root, GFP_ATOMIC | __GFP_HIGH);
				INIT_HLIST_HEAD(&ret->cic_list);
				ret->ioc_data = NULL;
				state = 2;
				break;
			case 2:
				return ret;
		}
	}
}

struct io_context *current_io_context(gfp_t gfp_flags, int node) {
	struct task_struct *tsk = current;
	struct io_context *ret;
	int state = 0;
	while(1) {
		switch(state) {
			case 0:
				ret = tsk->io_context;
				if (likely(ret)) {
					state = 1;
					break;
				}
				ret = alloc_io_context(gfp_flags, node);
				if (ret) {
					smp_wmb();
					tsk->io_context = ret;
				}
				state = 1;
				break;
			case 1:
				return ret;
		}
	}
}

struct io_context *get_io_context(gfp_t gfp_flags, int node) {
	struct io_context *ret = NULL;
	int state = 0;
	while(1) {
		switch(state) {
			case 0:
				ret = current_io_context(gfp_flags, node);
				if (unlikely(!ret)) {
					state = 1;
					break;
				}
				if (!atomic_long_inc_not_zero(&ret->refcount)) {
					state = 0;
					break;
				}
				state = 1;
				break;
			case 1:
				return ret;
		}
	}
}
EXPORT_SYMBOL(get_io_context);

void copy_io_context(struct io_context **pdst, struct io_context **psrc) {
	struct io_context *src = *psrc;
	struct io_context *dst = *pdst;
	int state = 0;
	while(1) {
		switch(state) {
			case 0:
				if (src) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				BUG_ON(atomic_long_read(&src->refcount) == 0);
				atomic_long_inc(&src->refcount);
				put_io_context(dst);
				*pdst = src;
				state = 2;
				break;
			case 2:
				return;
		}
	}
}
EXPORT_SYMBOL(copy_io_context);

static int __init blk_ioc_init(void) {
	int state = 0;
	while(1) {
		switch(state) {
			case 0:
				iocontext_cachep = kmem_cache_create("blkdev_ioc",
					sizeof(struct io_context), 0, SLAB_PANIC, NULL);
				state = 1;
				break;
			case 1:
				return 0;
		}
	}
}
subsys_initcall(blk_ioc_init);