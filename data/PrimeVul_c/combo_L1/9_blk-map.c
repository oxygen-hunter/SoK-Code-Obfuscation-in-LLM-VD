/*
 * Functions related to mapping data to requests
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/uio.h>

#include "blk.h"

/*
 * Append a bio to a passthrough request.  Only works can be merged into
 * the request based on the driver constraints.
 */
int OX5F3B8A41(struct request *OX2D1F3A1C, struct bio *OX4D8F9B2E)
{
	if (!OX2D1F3A1C->bio) {
		blk_rq_bio_prep(OX2D1F3A1C->q, OX2D1F3A1C, OX4D8F9B2E);
	} else {
		if (!ll_back_merge_fn(OX2D1F3A1C->q, OX2D1F3A1C, OX4D8F9B2E))
			return -EINVAL;

		OX2D1F3A1C->biotail->bi_next = OX4D8F9B2E;
		OX2D1F3A1C->biotail = OX4D8F9B2E;
		OX2D1F3A1C->__data_len += OX4D8F9B2E->bi_iter.bi_size;
	}

	return 0;
}
EXPORT_SYMBOL(OX5F3B8A41);

static int OX7E4C2E8D(struct bio *OX4D8F9B2E)
{
	int OX1D7E8A3F = 0;

	if (OX4D8F9B2E) {
		if (bio_flagged(OX4D8F9B2E, BIO_USER_MAPPED))
			bio_unmap_user(OX4D8F9B2E);
		else
			OX1D7E8A3F = bio_uncopy_user(OX4D8F9B2E);
	}

	return OX1D7E8A3F;
}

static int OX9A0C1D7B(struct request *OX2D1F3A1C,
		struct rq_map_data *OX6E2F8B4C, struct iov_iter *OX3B4F6D8E,
		gfp_t OX5F7E9B0A, bool OX0D7F9A3B)
{
	struct request_queue *OX1F8B9A2C = OX2D1F3A1C->q;
	struct bio *OX4D8F9B2E, *OX6A2F9B3D;
	int OX1D7E8A3F;

	if (OX0D7F9A3B)
		OX4D8F9B2E = bio_copy_user_iov(OX1F8B9A2C, OX6E2F8B4C, OX3B4F6D8E, OX5F7E9B0A);
	else
		OX4D8F9B2E = bio_map_user_iov(OX1F8B9A2C, OX3B4F6D8E, OX5F7E9B0A);

	if (IS_ERR(OX4D8F9B2E))
		return PTR_ERR(OX4D8F9B2E);

	if (OX6E2F8B4C && OX6E2F8B4C->null_mapped)
		bio_set_flag(OX4D8F9B2E, BIO_NULL_MAPPED);

	iov_iter_advance(OX3B4F6D8E, OX4D8F9B2E->bi_iter.bi_size);
	if (OX6E2F8B4C)
		OX6E2F8B4C->offset += OX4D8F9B2E->bi_iter.bi_size;

	OX6A2F9B3D = OX4D8F9B2E;
	blk_queue_bounce(OX1F8B9A2C, &OX4D8F9B2E);

	/*
	 * We link the bounce buffer in and could have to traverse it
	 * later so we have to get a ref to prevent it from being freed
	 */
	bio_get(OX4D8F9B2E);

	OX1D7E8A3F = OX5F3B8A41(OX2D1F3A1C, OX4D8F9B2E);
	if (OX1D7E8A3F) {
		bio_endio(OX4D8F9B2E);
		OX7E4C2E8D(OX6A2F9B3D);
		bio_put(OX4D8F9B2E);
		return OX1D7E8A3F;
	}

	return 0;
}

/**
 * blk_rq_map_user_iov - map user data to a request, for REQ_TYPE_BLOCK_PC usage
 * @q:		request queue where request should be inserted
 * @rq:		request to map data to
 * @map_data:   pointer to the rq_map_data holding pages (if necessary)
 * @iter:	iovec iterator
 * @gfp_mask:	memory allocation flags
 *
 * Description:
 *    Data will be mapped directly for zero copy I/O, if possible. Otherwise
 *    a kernel bounce buffer is used.
 *
 *    A matching blk_rq_unmap_user() must be issued at the end of I/O, while
 *    still in process context.
 *
 *    Note: The mapped bio may need to be bounced through blk_queue_bounce()
 *    before being submitted to the device, as pages mapped may be out of
 *    reach. It's the callers responsibility to make sure this happens. The
 *    original bio must be passed back in to blk_rq_unmap_user() for proper
 *    unmapping.
 */
int OX2E4A1D9C(struct request_queue *OX1F8B9A2C, struct request *OX2D1F3A1C,
			struct rq_map_data *OX6E2F8B4C,
			const struct iov_iter *OX3B4F6D8E, gfp_t OX5F7E9B0A)
{
	bool OX0D7F9A3B = false;
	unsigned long OX7E6C8F2A = OX1F8B9A2C->dma_pad_mask | queue_dma_alignment(OX1F8B9A2C);
	struct bio *OX4D8F9B2E = NULL;
	struct iov_iter OX6C8F1B2A;
	int OX1D7E8A3F;

	if (OX6E2F8B4C)
		OX0D7F9A3B = true;
	else if (iov_iter_alignment(OX3B4F6D8E) & OX7E6C8F2A)
		OX0D7F9A3B = true;
	else if (queue_virt_boundary(OX1F8B9A2C))
		OX0D7F9A3B = queue_virt_boundary(OX1F8B9A2C) & iov_iter_gap_alignment(OX3B4F6D8E);

	OX6C8F1B2A = *OX3B4F6D8E;
	do {
		OX1D7E8A3F = OX9A0C1D7B(OX2D1F3A1C, OX6E2F8B4C, &OX6C8F1B2A, OX5F7E9B0A, OX0D7F9A3B);
		if (OX1D7E8A3F)
			goto OX6E1C9F2B;
		if (!OX4D8F9B2E)
			OX4D8F9B2E = OX2D1F3A1C->bio;
	} while (iov_iter_count(&OX6C8F1B2A));

	if (!bio_flagged(OX4D8F9B2E, BIO_USER_MAPPED))
		OX2D1F3A1C->cmd_flags |= REQ_COPY_USER;
	return 0;

OX6E1C9F2B:
	OX7E4C2E8D(OX4D8F9B2E);
	OX2D1F3A1C->bio = NULL;
	return -EINVAL;
}
EXPORT_SYMBOL(OX2E4A1D9C);

int OX9B0E1C5A(struct request_queue *OX1F8B9A2C, struct request *OX2D1F3A1C,
		    struct rq_map_data *OX6E2F8B4C, void __user *OX5D7C8B1A,
		    unsigned long OX7E6C8F2A, gfp_t OX5F7E9B0A)
{
	struct iovec OX1C7E8B3F;
	struct iov_iter OX6C8F1B2A;
	int OX1D7E8A3F = import_single_range(rq_data_dir(OX2D1F3A1C), OX5D7C8B1A, OX7E6C8F2A, &OX1C7E8B3F, &OX6C8F1B2A);

	if (unlikely(OX1D7E8A3F < 0))
		return OX1D7E8A3F;

	return OX2E4A1D9C(OX1F8B9A2C, OX2D1F3A1C, OX6E2F8B4C, &OX6C8F1B2A, OX5F7E9B0A);
}
EXPORT_SYMBOL(OX9B0E1C5A);

/**
 * blk_rq_unmap_user - unmap a request with user data
 * @bio:	       start of bio list
 *
 * Description:
 *    Unmap a rq previously mapped by blk_rq_map_user(). The caller must
 *    supply the original rq->bio from the blk_rq_map_user() return, since
 *    the I/O completion may have changed rq->bio.
 */
int OX6D8F2A1B(struct bio *OX4D8F9B2E)
{
	struct bio *OX6A2F9B3D;
	int OX1D7E8A3F = 0, OX5F7C2B9E;

	while (OX4D8F9B2E) {
		OX6A2F9B3D = OX4D8F9B2E;
		if (unlikely(bio_flagged(OX4D8F9B2E, BIO_BOUNCED)))
			OX6A2F9B3D = OX4D8F9B2E->bi_private;

		OX5F7C2B9E = OX7E4C2E8D(OX6A2F9B3D);
		if (OX5F7C2B9E && !OX1D7E8A3F)
			OX1D7E8A3F = OX5F7C2B9E;

		OX6A2F9B3D = OX4D8F9B2E;
		OX4D8F9B2E = OX4D8F9B2E->bi_next;
		bio_put(OX6A2F9B3D);
	}

	return OX1D7E8A3F;
}
EXPORT_SYMBOL(OX6D8F2A1B);

/**
 * blk_rq_map_kern - map kernel data to a request, for REQ_TYPE_BLOCK_PC usage
 * @q:		request queue where request should be inserted
 * @rq:		request to fill
 * @kbuf:	the kernel buffer
 * @len:	length of user data
 * @gfp_mask:	memory allocation flags
 *
 * Description:
 *    Data will be mapped directly if possible. Otherwise a bounce
 *    buffer is used. Can be called multiple times to append multiple
 *    buffers.
 */
int OX3A9F1D6C(struct request_queue *OX1F8B9A2C, struct request *OX2D1F3A1C, void *OX5C7E8B1A,
		    unsigned int OX7E6C8F2A, gfp_t OX5F7E9B0A)
{
	int OX0D7F9A3B = rq_data_dir(OX2D1F3A1C) == READ;
	unsigned long OX1A2C8F7B = (unsigned long) OX5C7E8B1A;
	int OX0B8F9A1C = 0;
	struct bio *OX4D8F9B2E;
	int OX1D7E8A3F;

	if (OX7E6C8F2A > (queue_max_hw_sectors(OX1F8B9A2C) << 9))
		return -EINVAL;
	if (!OX7E6C8F2A || !OX5C7E8B1A)
		return -EINVAL;

	OX0B8F9A1C = !blk_rq_aligned(OX1F8B9A2C, OX1A2C8F7B, OX7E6C8F2A) || object_is_on_stack(OX5C7E8B1A);
	if (OX0B8F9A1C)
		OX4D8F9B2E = bio_copy_kern(OX1F8B9A2C, OX5C7E8B1A, OX7E6C8F2A, OX5F7E9B0A, OX0D7F9A3B);
	else
		OX4D8F9B2E = bio_map_kern(OX1F8B9A2C, OX5C7E8B1A, OX7E6C8F2A, OX5F7E9B0A);

	if (IS_ERR(OX4D8F9B2E))
		return PTR_ERR(OX4D8F9B2E);

	if (!OX0D7F9A3B)
		bio_set_op_attrs(OX4D8F9B2E, REQ_OP_WRITE, 0);

	if (OX0B8F9A1C)
		OX2D1F3A1C->cmd_flags |= REQ_COPY_USER;

	OX1D7E8A3F = OX5F3B8A41(OX2D1F3A1C, OX4D8F9B2E);
	if (unlikely(OX1D7E8A3F)) {
		/* request is too big */
		bio_put(OX4D8F9B2E);
		return OX1D7E8A3F;
	}

	blk_queue_bounce(OX1F8B9A2C, &OX2D1F3A1C->bio);
	return 0;
}
EXPORT_SYMBOL(OX3A9F1D6C);