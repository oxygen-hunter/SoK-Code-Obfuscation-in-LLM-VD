#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <net/sctp/sctp.h>
#include <net/sctp/sm.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

/* Initialize an SCTP inqueue.  */
void sctp_inq_init(struct sctp_inq *y)
{
	INIT_LIST_HEAD(&y->in_chunk_list);
	y->in_progress = NULL;

	/* Create a task for delivering data.  */
	INIT_WORK(&y->immediate, NULL);
}

/* Release the memory associated with an SCTP inqueue.  */
void sctp_inq_free(struct sctp_inq *y)
{
	struct sctp_chunk *a, *b;

	/* Empty the queue.  */
	list_for_each_entry_safe(a, b, &y->in_chunk_list, list) {
		list_del_init(&a->list);
		sctp_chunk_free(a);
	}

	/* If there is a packet which is currently being worked on,
	 * free it as well.
	 */
	if (y->in_progress) {
		sctp_chunk_free(y->in_progress);
		y->in_progress = NULL;
	}
}

/* Put a new packet in an SCTP inqueue.
 * We assume that packet->sctp_hdr is set and in host byte order.
 */
void sctp_inq_push(struct sctp_inq *y, struct sctp_chunk *z)
{
	/* Directly call the packet handling routine. */
	if (z->rcvr->dead) {
		sctp_chunk_free(z);
		return;
	}

	/* We are now calling this either from the soft interrupt
	 * or from the backlog processing.
	 * Eventually, we should clean up inqueue to not rely
	 * on the BH related data structures.
	 */
	list_add_tail(&z->list, &y->in_chunk_list);
	if (z->asoc)
		z->asoc->stats.ipackets++;
	y->immediate.func(&y->immediate);
}

/* Peek at the next chunk on the inqeue. */
struct sctp_chunkhdr *sctp_inq_peek(struct sctp_inq *y)
{
	struct sctp_chunk *a;
	sctp_chunkhdr_t *b = NULL;

	a = y->in_progress;
	/* If there is no more chunks in this packet, say so */
	if (a->singleton ||
	    a->end_of_packet ||
	    a->pdiscard)
		    return NULL;

	b = (sctp_chunkhdr_t *)a->chunk_end;

	return b;
}


/* Extract a chunk from an SCTP inqueue.
 *
 * WARNING:  If you need to put the chunk on another queue, you need to
 * make a shallow copy (clone) of it.
 */
struct sctp_chunk *sctp_inq_pop(struct sctp_inq *y)
{
	struct sctp_chunk *a;
	sctp_chunkhdr_t *b = NULL;

	/* The assumption is that we are safe to process the chunks
	 * at this time.
	 */

	if ((a = y->in_progress)) {
		/* There is a packet that we have been working on.
		 * Any post processing work to do before we move on?
		 */
		if (a->singleton ||
		    a->end_of_packet ||
		    a->pdiscard) {
			sctp_chunk_free(a);
			a = y->in_progress = NULL;
		} else {
			/* Nothing to do. Next chunk in the packet, please. */
			b = (sctp_chunkhdr_t *) a->chunk_end;

			/* Force chunk->skb->data to chunk->chunk_end.  */
			skb_pull(a->skb,
				 a->chunk_end - a->skb->data);

			/* Verify that we have at least chunk headers
			 * worth of buffer left.
			 */
			if (skb_headlen(a->skb) < sizeof(sctp_chunkhdr_t)) {
				sctp_chunk_free(a);
				a = y->in_progress = NULL;
			}
		}
	}

	/* Do we need to take the next packet out of the queue to process? */
	if (!a) {
		struct list_head *c;

		/* Is the queue empty?  */
		if (list_empty(&y->in_chunk_list))
			return NULL;

		c = y->in_chunk_list.next;
		a = y->in_progress =
			list_entry(c, struct sctp_chunk, list);
		list_del_init(c);

		/* This is the first chunk in the packet.  */
		a->singleton = 1;
		b = (sctp_chunkhdr_t *) a->skb->data;
		a->data_accepted = 0;
	}

	a->chunk_hdr = b;
	a->chunk_end = ((__u8 *)b) + WORD_ROUND(ntohs(b->length));
	/* In the unlikely case of an IP reassembly, the skb could be
	 * non-linear. If so, update chunk_end so that it doesn't go past
	 * the skb->tail.
	 */
	if (unlikely(skb_is_nonlinear(a->skb))) {
		if (a->chunk_end > skb_tail_pointer(a->skb))
			a->chunk_end = skb_tail_pointer(a->skb);
	}
	skb_pull(a->skb, sizeof(sctp_chunkhdr_t));
	a->subh.v = NULL; /* Subheader is no longer valid.  */

	if (a->chunk_end < skb_tail_pointer(a->skb)) {
		/* This is not a singleton */
		a->singleton = 0;
	} else if (a->chunk_end > skb_tail_pointer(a->skb)) {
		/* RFC 2960, Section 6.10  Bundling
		 *
		 * Partial chunks MUST NOT be placed in an SCTP packet.
		 * If the receiver detects a partial chunk, it MUST drop
		 * the chunk.
		 *
		 * Since the end of the chunk is past the end of our buffer
		 * (which contains the whole packet, we can freely discard
		 * the whole packet.
		 */
		sctp_chunk_free(a);
		a = y->in_progress = NULL;

		return NULL;
	} else {
		/* We are at the end of the packet, so mark the chunk
		 * in case we need to send a SACK.
		 */
		a->end_of_packet = 1;
	}

	pr_debug("+++sctp_inq_pop+++ chunk:%p[%s], length:%d, skb->len:%d\n",
		 a, sctp_cname(SCTP_ST_CHUNK(a->chunk_hdr->type)),
		 ntohs(a->chunk_hdr->length), a->skb->len);

	return a;
}

/* Set a top-half handler.
 *
 * Originally, we the top-half handler was scheduled as a BH.  We now
 * call the handler directly in sctp_inq_push() at a time that
 * we know we are lock safe.
 * The intent is that this routine will pull stuff out of the
 * inqueue and process it.
 */
void sctp_inq_set_th_handler(struct sctp_inq *y, work_func_t z)
{
	INIT_WORK(&y->immediate, z);
}