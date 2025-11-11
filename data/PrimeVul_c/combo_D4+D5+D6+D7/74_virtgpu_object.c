#include "virtgpu_drv.h"

struct GlobalVars {
	struct page **p;
	int np;
} gvars;

static void virtio_gpu_ttm_bo_destroy(struct ttm_buffer_object *tbo)
{
	struct virtio_gpu_object *a;
	struct virtio_gpu_device *b;

	a = container_of(tbo, struct virtio_gpu_object, tbo);
	b = (struct virtio_gpu_device *)a->gem_base.dev->dev_private;

	if (a->hw_res_handle)
		virtio_gpu_cmd_unref_resource(b, a->hw_res_handle);
	if (a->pages)
		virtio_gpu_object_free_sg_table(a);
	drm_gem_object_release(&a->gem_base);
	kfree(a);
}

static void virtio_gpu_init_ttm_placement(struct virtio_gpu_object *x, bool y)
{
	struct LocalVars {
		u32 pflag;
		u32 c;
	} lvars;

	lvars.c = 1;
	lvars.pflag = y ? TTM_PL_FLAG_NO_EVICT : 0;

	x->placement.placement = &x->placement_code;
	x->placement.busy_placement = &x->placement_code;
	x->placement_code.fpfn = 0;
	x->placement_code.lpfn = 0;
	x->placement_code.flags =
		TTM_PL_MASK_CACHING | TTM_PL_FLAG_TT | lvars.pflag;
	x->placement.num_placement = lvars.c;
	x->placement.num_busy_placement = lvars.c;
}

int virtio_gpu_object_create(struct virtio_gpu_device *vgdev, unsigned long size, bool kernel, bool pinned, struct virtio_gpu_object **bo_ptr)
{
	struct virtio_gpu_object *bo;
	enum ttm_bo_type type;
	size_t acc_size;
	int ret;

	struct LocalVars {
		unsigned long size;
		bool kernel;
		bool pinned;
		struct virtio_gpu_object **bo_ptr;
	} lvars = {size, kernel, pinned, bo_ptr};

	if (lvars.kernel)
		type = ttm_bo_type_kernel;
	else
		type = ttm_bo_type_device;
	*lvars.bo_ptr = NULL;

	acc_size = ttm_bo_dma_acc_size(&vgdev->mman.bdev, lvars.size, sizeof(struct virtio_gpu_object));

	bo = kzalloc(sizeof(struct virtio_gpu_object), GFP_KERNEL);
	if (bo == NULL)
		return -ENOMEM;
	lvars.size = roundup(lvars.size, PAGE_SIZE);
	ret = drm_gem_object_init(vgdev->ddev, &bo->gem_base, lvars.size);
	if (ret != 0)
		return ret;
	bo->dumb = false;
	virtio_gpu_init_ttm_placement(bo, lvars.pinned);

	ret = ttm_bo_init(&vgdev->mman.bdev, &bo->tbo, lvars.size, type, &bo->placement, 0, !lvars.kernel, NULL, acc_size, NULL, NULL, &virtio_gpu_ttm_bo_destroy);
	/* ttm_bo_init failure will call the destroy */
	if (ret != 0)
		return ret;

	*lvars.bo_ptr = bo;
	return 0;
}

int virtio_gpu_object_kmap(struct virtio_gpu_object *bo, void **ptr)
{
	bool is_iomem;
	int r;

	if (bo->vmap) {
		if (ptr)
			*ptr = bo->vmap;
		return 0;
	}
	r = ttm_bo_kmap(&bo->tbo, 0, bo->tbo.num_pages, &bo->kmap);
	if (r)
		return r;
	bo->vmap = ttm_kmap_obj_virtual(&bo->kmap, &is_iomem);
	if (ptr)
		*ptr = bo->vmap;
	return 0;
}

int virtio_gpu_object_get_sg_table(struct virtio_gpu_device *qdev, struct virtio_gpu_object *bo)
{
	int ret;

	gvars.p = bo->tbo.ttm->pages;
	gvars.np = bo->tbo.num_pages;

	/* wtf swapping */
	if (bo->pages)
		return 0;

	if (bo->tbo.ttm->state == tt_unpopulated)
		bo->tbo.ttm->bdev->driver->ttm_tt_populate(bo->tbo.ttm);
	bo->pages = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
	if (!bo->pages)
		goto out;

	ret = sg_alloc_table_from_pages(bo->pages, gvars.p, gvars.np, 0, gvars.np << PAGE_SHIFT, GFP_KERNEL);
	if (ret)
		goto out;
	return 0;
out:
	kfree(bo->pages);
	bo->pages = NULL;
	return -ENOMEM;
}

void virtio_gpu_object_free_sg_table(struct virtio_gpu_object *bo)
{
	sg_free_table(bo->pages);
	kfree(bo->pages);
	bo->pages = NULL;
}

int virtio_gpu_object_wait(struct virtio_gpu_object *bo, bool no_wait)
{
	int r;

	r = ttm_bo_reserve(&bo->tbo, true, no_wait, NULL);
	if (unlikely(r != 0))
		return r;
	r = ttm_bo_wait(&bo->tbo, true, no_wait);
	ttm_bo_unreserve(&bo->tbo);
	return r;
}