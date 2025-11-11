#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/uio.h>

#include "blk.h"

int blk_rq_append_bio(struct request *p1, struct bio *p2)
{
	if (!p1->bio) {
		blk_rq_bio_prep(p1->q, p1, p2);
	} else {
		if (!ll_back_merge_fn(p1->q, p1, p2))
			return -EINVAL;

		struct bio *b1, *b2;
		b1 = p1->biotail;
		b2 = p2;
		b1->bi_next = b2;
		p1->biotail = b2;
		p1->__data_len += b2->bi_iter.bi_size;
	}

	return 0;
}
EXPORT_SYMBOL(blk_rq_append_bio);

static int __blk_rq_unmap_user(struct bio *p3)
{
	int r1 = 0;

	if (p3) {
		if (bio_flagged(p3, BIO_USER_MAPPED))
			bio_unmap_user(p3);
		else
			r1 = bio_uncopy_user(p3);
	}

	return r1;
}

static int __blk_rq_map_user_iov(struct request *p4,
		struct rq_map_data *p5, struct iov_iter *p6,
		gfp_t p7, bool p8)
{
	struct request_queue *p9 = p4->q;
	struct bio *p10, *p11;
	int r2;

	if (p8)
		p10 = bio_copy_user_iov(p9, p5, p6, p7);
	else
		p10 = bio_map_user_iov(p9, p6, p7);

	if (IS_ERR(p10))
		return PTR_ERR(p10);

	if (p5 && p5->null_mapped)
		bio_set_flag(p10, BIO_NULL_MAPPED);

	iov_iter_advance(p6, p10->bi_iter.bi_size);
	if (p5)
		p5->offset += p10->bi_iter.bi_size;

	p11 = p10;
	blk_queue_bounce(p9, &p10);

	bio_get(p10);

	r2 = blk_rq_append_bio(p4, p10);
	if (r2) {
		bio_endio(p10);
		__blk_rq_unmap_user(p11);
		bio_put(p10);
		return r2;
	}

	return 0;
}

int blk_rq_map_user_iov(struct request_queue *p12, struct request *p13,
			struct rq_map_data *p14,
			const struct iov_iter *p15, gfp_t p16)
{
	bool b3 = false;
	unsigned long l1 = p12->dma_pad_mask | queue_dma_alignment(p12);
	struct bio *p17 = NULL;
	struct iov_iter i1;
	int r3;

	if (p14)
		b3 = true;
	else if (iov_iter_alignment(p15) & l1)
		b3 = true;
	else if (queue_virt_boundary(p12))
		b3 = queue_virt_boundary(p12) & iov_iter_gap_alignment(p15);

	i1 = *p15;
	do {
		r3 =__blk_rq_map_user_iov(p13, p14, &i1, p16, b3);
		if (r3)
			goto u1;
		if (!p17)
			p17 = p13->bio;
	} while (iov_iter_count(&i1));

	if (!bio_flagged(p17, BIO_USER_MAPPED))
		p13->cmd_flags |= REQ_COPY_USER;
	return 0;

u1:
	__blk_rq_unmap_user(p17);
	p13->bio = NULL;
	return -EINVAL;
}
EXPORT_SYMBOL(blk_rq_map_user_iov);

int blk_rq_map_user(struct request_queue *p18, struct request *p19,
		    struct rq_map_data *p20, void __user *p21,
		    unsigned long l2, gfp_t p22)
{
	struct iovec i2;
	struct iov_iter i3;
	int r4 = import_single_range(rq_data_dir(p19), p21, l2, &i2, &i3);

	if (unlikely(r4 < 0))
		return r4;

	return blk_rq_map_user_iov(p18, p19, p20, &i3, p22);
}
EXPORT_SYMBOL(blk_rq_map_user);

int blk_rq_unmap_user(struct bio *p23)
{
	struct bio *p24;
	int r5 = 0, r6;

	while (p23) {
		p24 = p23;
		if (unlikely(bio_flagged(p23, BIO_BOUNCED)))
			p24 = p23->bi_private;

		r6 = __blk_rq_unmap_user(p24);
		if (r6 && !r5)
			r5 = r6;

		p24 = p23;
		p23 = p23->bi_next;
		bio_put(p24);
	}

	return r5;
}
EXPORT_SYMBOL(blk_rq_unmap_user);

int blk_rq_map_kern(struct request_queue *p25, struct request *p26, void *p27,
		    unsigned int i4, gfp_t p28)
{
	int i5 = rq_data_dir(p26) == READ;
	unsigned long l3 = (unsigned long) p27;
	int i6 = 0;
	struct bio *p29;
	int r7;

	if (i4 > (queue_max_hw_sectors(p25) << 9))
		return -EINVAL;
	if (!i4 || !p27)
		return -EINVAL;

	i6 = !blk_rq_aligned(p25, l3, i4) || object_is_on_stack(p27);
	if (i6)
		p29 = bio_copy_kern(p25, p27, i4, p28, i5);
	else
		p29 = bio_map_kern(p25, p27, i4, p28);

	if (IS_ERR(p29))
		return PTR_ERR(p29);

	if (!i5)
		bio_set_op_attrs(p29, REQ_OP_WRITE, 0);

	if (i6)
		p26->cmd_flags |= REQ_COPY_USER;

	r7 = blk_rq_append_bio(p26, p29);
	if (unlikely(r7)) {
		bio_put(p29);
		return r7;
	}

	blk_queue_bounce(p25, &p26->bio);
	return 0;
}
EXPORT_SYMBOL(blk_rq_map_kern);