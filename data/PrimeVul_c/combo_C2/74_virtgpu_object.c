#include "virtgpu_drv.h"

static void virtio_gpu_ttm_bo_destroy(struct ttm_buffer_object *tbo)
{
	int _state = 0;
	while (1) {
		switch (_state) {
		case 0: {
			struct virtio_gpu_object *bo;
			struct virtio_gpu_device *vgdev;
			bo = container_of(tbo, struct virtio_gpu_object, tbo);
			vgdev = (struct virtio_gpu_device *)bo->gem_base.dev->dev_private;
			if (bo->hw_res_handle) {
				_state = 1;
				break;
			}
			_state = 2;
			break;
		}
		case 1: {
			virtio_gpu_cmd_unref_resource(vgdev, bo->hw_res_handle);
			_state = 2;
			break;
		}
		case 2: {
			if (bo->pages) {
				_state = 3;
				break;
			}
			_state = 4;
			break;
		}
		case 3: {
			virtio_gpu_object_free_sg_table(bo);
			_state = 4;
			break;
		}
		case 4: {
			drm_gem_object_release(&bo->gem_base);
			kfree(bo);
			return;
		}
		}
	}
}

static void virtio_gpu_init_ttm_placement(struct virtio_gpu_object *vgbo,
					  bool pinned)
{
	int _state = 0;
	while (1) {
		switch (_state) {
		case 0: {
			u32 c = 1;
			u32 pflag = pinned ? TTM_PL_FLAG_NO_EVICT : 0;
			vgbo->placement.placement = &vgbo->placement_code;
			vgbo->placement.busy_placement = &vgbo->placement_code;
			vgbo->placement_code.fpfn = 0;
			vgbo->placement_code.lpfn = 0;
			vgbo->placement_code.flags =
				TTM_PL_MASK_CACHING | TTM_PL_FLAG_TT | pflag;
			vgbo->placement.num_placement = c;
			vgbo->placement.num_busy_placement = c;
			return;
		}
		}
	}
}

int virtio_gpu_object_create(struct virtio_gpu_device *vgdev,
			     unsigned long size, bool kernel, bool pinned,
			     struct virtio_gpu_object **bo_ptr)
{
	int _state = 0;
	while (1) {
		switch (_state) {
		case 0: {
			struct virtio_gpu_object *bo;
			enum ttm_bo_type type;
			size_t acc_size;
			int ret;
			if (kernel)
				type = ttm_bo_type_kernel;
			else
				type = ttm_bo_type_device;
			*bo_ptr = NULL;
			acc_size = ttm_bo_dma_acc_size(&vgdev->mman.bdev, size,
						       sizeof(struct virtio_gpu_object));
			bo = kzalloc(sizeof(struct virtio_gpu_object), GFP_KERNEL);
			if (bo == NULL) {
				_state = 1;
				break;
			}
			size = roundup(size, PAGE_SIZE);
			ret = drm_gem_object_init(vgdev->ddev, &bo->gem_base, size);
			if (ret != 0) {
				_state = 2;
				break;
			}
			bo->dumb = false;
			virtio_gpu_init_ttm_placement(bo, pinned);
			ret = ttm_bo_init(&vgdev->mman.bdev, &bo->tbo, size, type,
					  &bo->placement, 0, !kernel, NULL, acc_size,
					  NULL, NULL, &virtio_gpu_ttm_bo_destroy);
			if (ret != 0) {
				_state = 2;
				break;
			}
			*bo_ptr = bo;
			return 0;
		}
		case 1: {
			return -ENOMEM;
		}
		case 2: {
			return ret;
		}
		}
	}
}

int virtio_gpu_object_kmap(struct virtio_gpu_object *bo, void **ptr)
{
	int _state = 0;
	while (1) {
		switch (_state) {
		case 0: {
			bool is_iomem;
			int r;
			if (bo->vmap) {
				_state = 1;
				break;
			}
			r = ttm_bo_kmap(&bo->tbo, 0, bo->tbo.num_pages, &bo->kmap);
			if (r) {
				_state = 2;
				break;
			}
			bo->vmap = ttm_kmap_obj_virtual(&bo->kmap, &is_iomem);
			if (ptr)
				*ptr = bo->vmap;
			return 0;
		}
		case 1: {
			if (ptr)
				*ptr = bo->vmap;
			return 0;
		}
		case 2: {
			return r;
		}
		}
	}
}

int virtio_gpu_object_get_sg_table(struct virtio_gpu_device *qdev,
				   struct virtio_gpu_object *bo)
{
	int _state = 0;
	while (1) {
		switch (_state) {
		case 0: {
			int ret;
			struct page **pages = bo->tbo.ttm->pages;
			int nr_pages = bo->tbo.num_pages;
			if (bo->pages) {
				_state = 1;
				break;
			}
			if (bo->tbo.ttm->state == tt_unpopulated)
				bo->tbo.ttm->bdev->driver->ttm_tt_populate(bo->tbo.ttm);
			bo->pages = kmalloc(sizeof(struct sg_table), GFP_KERNEL);
			if (!bo->pages) {
				_state = 2;
				break;
			}
			ret = sg_alloc_table_from_pages(bo->pages, pages, nr_pages, 0,
							nr_pages << PAGE_SHIFT, GFP_KERNEL);
			if (ret) {
				_state = 2;
				break;
			}
			return 0;
		}
		case 1: {
			return 0;
		}
		case 2: {
			kfree(bo->pages);
			bo->pages = NULL;
			return -ENOMEM;
		}
		}
	}
}

void virtio_gpu_object_free_sg_table(struct virtio_gpu_object *bo)
{
	int _state = 0;
	while (1) {
		switch (_state) {
		case 0: {
			sg_free_table(bo->pages);
			kfree(bo->pages);
			bo->pages = NULL;
			return;
		}
		}
	}
}

int virtio_gpu_object_wait(struct virtio_gpu_object *bo, bool no_wait)
{
	int _state = 0;
	while (1) {
		switch (_state) {
		case 0: {
			int r;
			r = ttm_bo_reserve(&bo->tbo, true, no_wait, NULL);
			if (unlikely(r != 0)) {
				_state = 1;
				break;
			}
			r = ttm_bo_wait(&bo->tbo, true, no_wait);
			ttm_bo_unreserve(&bo->tbo);
			return r;
		}
		case 1: {
			return r;
		}
		}
	}
}