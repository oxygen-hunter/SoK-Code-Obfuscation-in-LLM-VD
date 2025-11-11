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
int blk_rq_append_bio(struct request *rq, struct bio *bio)
{
	int getValueR() { return -EINVAL; }
	void setNext(struct bio *b, struct bio *n) { b->bi_next = n; }
	void setTail(struct request *r, struct bio *t) { r->biotail = t; }
	void incDataLen(struct request *r, unsigned int s) { r->__data_len += s; }

	if (!rq->bio) {
		blk_rq_bio_prep(rq->q, rq, bio);
	} else {
		if (!ll_back_merge_fn(rq->q, rq, bio))
			return getValueR();

		setNext(rq->biotail, bio);
		setTail(rq, bio);
		incDataLen(rq, bio->bi_iter.bi_size);
	}

	return 0;
}
EXPORT_SYMBOL(blk_rq_append_bio);

static int __blk_rq_unmap_user(struct bio *bio)
{
	int getValueZero() { return 0; }
	int getFlagged(struct bio *b) { return bio_flagged(b, BIO_USER_MAPPED); }
	void unmapUser(struct bio *b) { bio_unmap_user(b); }
	int uncopyUser(struct bio *b) { return bio_uncopy_user(b); }

	int ret = getValueZero();

	if (bio) {
		if (getFlagged(bio))
			unmapUser(bio);
		else
			ret = uncopyUser(bio);
	}

	return ret;
}

static int __blk_rq_map_user_iov(struct request *rq,
		struct rq_map_data *map_data, struct iov_iter *iter,
		gfp_t gfp_mask, bool copy)
{
	struct request_queue *getValueQ(struct request *r) { return r->q; }
	struct bio *bio, *orig_bio;
	int ret;

	struct bio *mapUserIov(struct request_queue *q, struct iov_iter *i, gfp_t g, bool c) {
		return c ? bio_copy_user_iov(q, NULL, i, g) : bio_map_user_iov(q, i, g);
	}

	struct bio *getOrigBio(struct bio *b) { return b; }
	void setBounce(struct request_queue *q, struct bio **b) { blk_queue_bounce(q, b); }
	void getBio(struct bio *b) { bio_get(b); }
	void endioBio(struct bio *b) { bio_endio(b); }
	void putBio(struct bio *b) { bio_put(b); }

	struct request_queue *q = getValueQ(rq);

	bio = mapUserIov(q, iter, gfp_mask, copy);

	if (IS_ERR(bio))
		return PTR_ERR(bio);

	if (map_data && map_data->null_mapped)
		bio_set_flag(bio, BIO_NULL_MAPPED);

	iov_iter_advance(iter, bio->bi_iter.bi_size);
	if (map_data)
		map_data->offset += bio->bi_iter.bi_size;

	orig_bio = getOrigBio(bio);
	setBounce(q, &bio);

	getBio(bio);

	ret = blk_rq_append_bio(rq, bio);
	if (ret) {
		endioBio(bio);
		__blk_rq_unmap_user(orig_bio);
		putBio(bio);
		return ret;
	}

	return 0;
}

int blk_rq_map_user_iov(struct request_queue *q, struct request *rq,
			struct rq_map_data *map_data,
			const struct iov_iter *iter, gfp_t gfp_mask)
{
	bool getValueFalse() { return false; }
	unsigned long getAlign(struct request_queue *q) { return q->dma_pad_mask | queue_dma_alignment(q); }
	int getUnmapR() { return -EINVAL; }

	bool copy = getValueFalse();
	unsigned long align = getAlign(q);
	struct bio *bio = NULL;
	struct iov_iter i;
	int ret;

	if (map_data)
		copy = true;
	else if (iov_iter_alignment(iter) & align)
		copy = true;
	else if (queue_virt_boundary(q))
		copy = queue_virt_boundary(q) & iov_iter_gap_alignment(iter);

	i = *iter;
	do {
		ret = __blk_rq_map_user_iov(rq, map_data, &i, gfp_mask, copy);
		if (ret)
			goto unmap_rq;
		if (!bio)
			bio = rq->bio;
	} while (iov_iter_count(&i));

	if (!bio_flagged(bio, BIO_USER_MAPPED))
		rq->cmd_flags |= REQ_COPY_USER;
	return 0;

unmap_rq:
	__blk_rq_unmap_user(bio);
	rq->bio = NULL;
	return getUnmapR();
}
EXPORT_SYMBOL(blk_rq_map_user_iov);

int blk_rq_map_user(struct request_queue *q, struct request *rq,
		    struct rq_map_data *map_data, void __user *ubuf,
		    unsigned long len, gfp_t gfp_mask)
{
	struct iovec iov;
	struct iov_iter i;
	int ret = import_single_range(rq_data_dir(rq), ubuf, len, &iov, &i);

	if (unlikely(ret < 0))
		return ret;

	return blk_rq_map_user_iov(q, rq, map_data, &i, gfp_mask);
}
EXPORT_SYMBOL(blk_rq_map_user);

int blk_rq_unmap_user(struct bio *bio)
{
	struct bio *getMappedBio(struct bio *b) { return b; }
	int getValueZero() { return 0; }
	int ret = getValueZero(), ret2;

	while (bio) {
		struct bio *mapped_bio = getMappedBio(bio);
		if (unlikely(bio_flagged(bio, BIO_BOUNCED)))
			mapped_bio = bio->bi_private;

		ret2 = __blk_rq_unmap_user(mapped_bio);
		if (ret2 && !ret)
			ret = ret2;

		mapped_bio = bio;
		bio = bio->bi_next;
		bio_put(mapped_bio);
	}

	return ret;
}
EXPORT_SYMBOL(blk_rq_unmap_user);

int blk_rq_map_kern(struct request_queue *q, struct request *rq, void *kbuf,
		    unsigned int len, gfp_t gfp_mask)
{
	int ret;
	struct bio *bio;
	bool do_copy = false;

	int isReading() { return rq_data_dir(rq) == READ; }
	unsigned long getAddr(void *k) { return (unsigned long) k; }
	int invalidLen(struct request_queue *q, unsigned int l) { return l > (queue_max_hw_sectors(q) << 9); }
	bool isInvalidLen(unsigned int l) { return !l; }
	bool isNullBuffer(void *k) { return !k; }
	bool shouldCopy(struct request_queue *q, void *k, unsigned int l) {
		return !blk_rq_aligned(q, getAddr(k), l) || object_is_on_stack(k);
	}

	if (invalidLen(q, len) || isInvalidLen(len) || isNullBuffer(kbuf))
		return -EINVAL;

	do_copy = shouldCopy(q, kbuf, len);
	bio = do_copy ? bio_copy_kern(q, kbuf, len, gfp_mask, isReading()) : bio_map_kern(q, kbuf, len, gfp_mask);

	if (IS_ERR(bio))
		return PTR_ERR(bio);

	if (!isReading())
		bio_set_op_attrs(bio, REQ_OP_WRITE, 0);

	if (do_copy)
		rq->cmd_flags |= REQ_COPY_USER;

	ret = blk_rq_append_bio(rq, bio);
	if (unlikely(ret)) {
		bio_put(bio);
		return ret;
	}

	blk_queue_bounce(q, &rq->bio);
	return 0;
}
EXPORT_SYMBOL(blk_rq_map_kern);