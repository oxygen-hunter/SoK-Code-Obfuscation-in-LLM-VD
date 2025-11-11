#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/bootmem.h>

#include "blk.h"

static struct kmem_cache *ioCtxCache;

static void cfq_dtor(struct io_context *ctx)
{
	if (!hlist_empty(&ctx->cic_list)) {
		struct cfq_io_context *c;

		c = list_entry(ctx->cic_list.first, struct cfq_io_context,
								cic_list);
		c->dtor(ctx);
	}
}

int put_io_context(struct io_context *ctx)
{
	if (ctx == NULL)
		return 1;

	BUG_ON(atomic_long_read(&ctx->refcount) == 0);

	if (atomic_long_dec_and_test(&ctx->refcount)) {
		rcu_read_lock();
		if (ctx->aic && ctx->aic->dtor)
			ctx->aic->dtor(ctx->aic);
		cfq_dtor(ctx);
		rcu_read_unlock();

		kmem_cache_free(ioCtxCache, ctx);
		return 1;
	}
	return 0;
}
EXPORT_SYMBOL(put_io_context);

static void cfq_exit(struct io_context *ctx)
{
	rcu_read_lock();

	if (!hlist_empty(&ctx->cic_list)) {
		struct cfq_io_context *c;

		c = list_entry(ctx->cic_list.first, struct cfq_io_context,
								cic_list);
		c->exit(ctx);
	}
	rcu_read_unlock();
}

void exit_io_context(void)
{
	struct io_context *ctx;

	task_lock(current);
	ctx = current->io_context;
	current->io_context = NULL;
	task_unlock(current);

	if (atomic_dec_and_test(&ctx->nr_tasks)) {
		if (ctx->aic && ctx->aic->exit)
			ctx->aic->exit(ctx->aic);
		cfq_exit(ctx);

		put_io_context(ctx);
	}
}

struct io_context *alloc_io_context(gfp_t flags, int n)
{
	struct io_context *result;

	result = kmem_cache_alloc_node(ioCtxCache, flags, n);
	if (result) {
		atomic_long_set(&result->refcount, 1);
		atomic_set(&result->nr_tasks, 1);
		spin_lock_init(&result->lock);
		result->ioprio_changed = 0;
		result->ioprio = 0;
		result->last_waited = jiffies;
		result->nr_batch_requests = 0;
		result->aic = NULL;
		INIT_RADIX_TREE(&result->radix_root, GFP_ATOMIC | __GFP_HIGH);
		INIT_HLIST_HEAD(&result->cic_list);
		result->ioc_data = NULL;
	}

	return result;
}

struct io_context *current_io_context(gfp_t flags, int n)
{
	struct task_struct *task = current;
	struct io_context *result;

	result = task->io_context;
	if (likely(result))
		return result;

	result = alloc_io_context(flags, n);
	if (result) {
		smp_wmb();
		task->io_context = result;
	}

	return result;
}

struct io_context *get_io_context(gfp_t flags, int n)
{
	struct io_context *result = NULL;

	do {
		result = current_io_context(flags, n);
		if (unlikely(!result))
			break;
	} while (!atomic_long_inc_not_zero(&result->refcount));

	return result;
}
EXPORT_SYMBOL(get_io_context);

void copy_io_context(struct io_context **destination, struct io_context **source)
{
	struct io_context *src = *source;
	struct io_context *dst = *destination;

	if (src) {
		BUG_ON(atomic_long_read(&src->refcount) == 0);
		atomic_long_inc(&src->refcount);
		put_io_context(dst);
		*destination = src;
	}
}
EXPORT_SYMBOL(copy_io_context);

static int __init blk_ioc_init(void)
{
	ioCtxCache = kmem_cache_create("blkdev_ioc",
			sizeof(struct io_context), 0, SLAB_PANIC, NULL);
	return 0;
}
subsys_initcall(blk_ioc_init);