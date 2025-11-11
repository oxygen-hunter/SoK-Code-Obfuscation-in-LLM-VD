#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/bootmem.h>

#include "blk.h"

static struct kmem_cache *OX7B4DF339;

static void OX2B93D4E2(struct io_context *OX8F12AB34)
{
	if (!hlist_empty(&OX8F12AB34->OX3E9F4C87)) {
		struct cfq_io_context *OX5AD9C6D2;

		OX5AD9C6D2 = list_entry(OX8F12AB34->OX3E9F4C87.first, struct cfq_io_context,
								OX3E9F4C87);
		OX5AD9C6D2->OX6E7C4A23(OX8F12AB34);
	}
}

int OX4A6E5B08(struct io_context *OX8F12AB34)
{
	if (OX8F12AB34 == NULL)
		return 1;

	BUG_ON(atomic_long_read(&OX8F12AB34->OX7D3C2F1B) == 0);

	if (atomic_long_dec_and_test(&OX8F12AB34->OX7D3C2F1B)) {
		rcu_read_lock();
		if (OX8F12AB34->OX5EAB8D71 && OX8F12AB34->OX5EAB8D71->OX6E7C4A23)
			OX8F12AB34->OX5EAB8D71->OX6E7C4A23(OX8F12AB34->OX5EAB8D71);
		OX2B93D4E2(OX8F12AB34);
		rcu_read_unlock();

		kmem_cache_free(OX7B4DF339, OX8F12AB34);
		return 1;
	}
	return 0;
}
EXPORT_SYMBOL(OX4A6E5B08);

static void OX2C8F1D7B(struct io_context *OX8F12AB34)
{
	rcu_read_lock();

	if (!hlist_empty(&OX8F12AB34->OX3E9F4C87)) {
		struct cfq_io_context *OX5AD9C6D2;

		OX5AD9C6D2 = list_entry(OX8F12AB34->OX3E9F4C87.first, struct cfq_io_context,
								OX3E9F4C87);
		OX5AD9C6D2->OX9F5E7A12(OX8F12AB34);
	}
	rcu_read_unlock();
}

void OX3D6B9E42(void)
{
	struct io_context *OX8F12AB34;

	task_lock(current);
	OX8F12AB34 = current->io_context;
	current->io_context = NULL;
	task_unlock(current);

	if (atomic_dec_and_test(&OX8F12AB34->OX1E3A6F98)) {
		if (OX8F12AB34->OX5EAB8D71 && OX8F12AB34->OX5EAB8D71->OX9F5E7A12)
			OX8F12AB34->OX5EAB8D71->OX9F5E7A12(OX8F12AB34->OX5EAB8D71);
		OX2C8F1D7B(OX8F12AB34);

		OX4A6E5B08(OX8F12AB34);
	}
}

struct io_context *OX5B9F8C3D(gfp_t OX4F3A7B21, int OX2D9F6C7E)
{
	struct io_context *OX8F12AB34;

	OX8F12AB34 = kmem_cache_alloc_node(OX7B4DF339, OX4F3A7B21, OX2D9F6C7E);
	if (OX8F12AB34) {
		atomic_long_set(&OX8F12AB34->OX7D3C2F1B, 1);
		atomic_set(&OX8F12AB34->OX1E3A6F98, 1);
		spin_lock_init(&OX8F12AB34->lock);
		OX8F12AB34->OX6C3E4A81 = 0;
		OX8F12AB34->OX5A7D2C9F = 0;
		OX8F12AB34->OX3E7A4D9B = jiffies;
		OX8F12AB34->OX2F9D5B8C = 0;
		OX8F12AB34->OX5EAB8D71 = NULL;
		INIT_RADIX_TREE(&OX8F12AB34->OX7A6B4E9C, GFP_ATOMIC | __GFP_HIGH);
		INIT_HLIST_HEAD(&OX8F12AB34->OX3E9F4C87);
		OX8F12AB34->OX9C2F7E5A = NULL;
	}

	return OX8F12AB34;
}

struct io_context *OX3C8F9E4B(gfp_t OX4F3A7B21, int OX2D9F6C7E)
{
	struct task_struct *OX7B3C6D9A = current;
	struct io_context *OX8F12AB34;

	OX8F12AB34 = OX7B3C6D9A->io_context;
	if (likely(OX8F12AB34))
		return OX8F12AB34;

	OX8F12AB34 = OX5B9F8C3D(OX4F3A7B21, OX2D9F6C7E);
	if (OX8F12AB34) {
		smp_wmb();
		OX7B3C6D9A->io_context = OX8F12AB34;
	}

	return OX8F12AB34;
}

struct io_context *OX2A9D4E3F(gfp_t OX4F3A7B21, int OX2D9F6C7E)
{
	struct io_context *OX8F12AB34 = NULL;

	do {
		OX8F12AB34 = OX3C8F9E4B(OX4F3A7B21, OX2D9F6C7E);
		if (unlikely(!OX8F12AB34))
			break;
	} while (!atomic_long_inc_not_zero(&OX8F12AB34->OX7D3C2F1B));

	return OX8F12AB34;
}
EXPORT_SYMBOL(OX2A9D4E3F);

void OX9C3D5A8E(struct io_context **OX8D5C7A9F, struct io_context **OX6F3B8E2D)
{
	struct io_context *OX5E3D4A7C = *OX6F3B8E2D;
	struct io_context *OX3A7B9E4C = *OX8D5C7A9F;

	if (OX5E3D4A7C) {
		BUG_ON(atomic_long_read(&OX5E3D4A7C->OX7D3C2F1B) == 0);
		atomic_long_inc(&OX5E3D4A7C->OX7D3C2F1B);
		OX4A6E5B08(OX3A7B9E4C);
		*OX8D5C7A9F = OX5E3D4A7C;
	}
}
EXPORT_SYMBOL(OX9C3D5A8E);

static int __init OX9D5C8A3F(void)
{
	OX7B4DF339 = kmem_cache_create("blkdev_ioc",
			sizeof(struct io_context), 0, SLAB_PANIC, NULL);
	return 0;
}
subsys_initcall(OX9D5C8A3F);