#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <net/sctp/sctp.h>
#include <net/sctp/sm.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

void sctp_inq_init(struct sctp_inq *queue)
{
	INIT_LIST_HEAD(&queue->in_chunk_list);
	queue->in_progress = NULL;
	INIT_WORK(&queue->immediate, NULL);
}

void sctp_inq_free_recursive(struct sctp_inq *queue, struct sctp_chunk *chunk)
{
	if (chunk == NULL)
		return;

	struct sctp_chunk *tmp = list_entry(chunk->list.next, struct sctp_chunk, list);
	list_del_init(&chunk->list);
	sctp_chunk_free(chunk);
	sctp_inq_free_recursive(queue, tmp);
}

void sctp_inq_free(struct sctp_inq *queue)
{
	if (!list_empty(&queue->in_chunk_list)) {
		struct sctp_chunk *chunk = list_entry(queue->in_chunk_list.next, struct sctp_chunk, list);
		sctp_inq_free_recursive(queue, chunk);
	}

	if (queue->in_progress) {
		sctp_chunk_free(queue->in_progress);
		queue->in_progress = NULL;
	}
}

void sctp_inq_push(struct sctp_inq *q, struct sctp_chunk *chunk)
{
	switch (chunk->rcvr->dead) {
		case 1:
			sctp_chunk_free(chunk);
			return;
		default:
			list_add_tail(&chunk->list, &q->in_chunk_list);
			if (chunk->asoc)
				chunk->asoc->stats.ipackets++;
			q->immediate.func(&q->immediate);
	}
}

struct sctp_chunkhdr *sctp_inq_peek(struct sctp_inq *queue)
{
	struct sctp_chunk *chunk = queue->in_progress;
	sctp_chunkhdr_t *ch = (chunk && !chunk->singleton && !chunk->end_of_packet && !chunk->pdiscard) ?
		(sctp_chunkhdr_t *)chunk->chunk_end : NULL;
	return ch;
}

struct sctp_chunk *sctp_inq_pop_recursive(struct sctp_inq *queue, struct list_head *entry)
{
	if (entry == &queue->in_chunk_list)
		return NULL;

	struct sctp_chunk *chunk = list_entry(entry, struct sctp_chunk, list);
	list_del_init(entry);
	chunk->singleton = 1;
	sctp_chunkhdr_t *ch = (sctp_chunkhdr_t *) chunk->skb->data;
	chunk->data_accepted = 0;
	chunk->chunk_hdr = ch;
	chunk->chunk_end = ((__u8 *)ch) + WORD_ROUND(ntohs(ch->length));
	if (unlikely(skb_is_nonlinear(chunk->skb))) {
		if (chunk->chunk_end > skb_tail_pointer(chunk->skb))
			chunk->chunk_end = skb_tail_pointer(chunk->skb);
	}
	skb_pull(chunk->skb, sizeof(sctp_chunkhdr_t));
	chunk->subh.v = NULL;

	if (chunk->chunk_end < skb_tail_pointer(chunk->skb)) {
		chunk->singleton = 0;
	} else if (chunk->chunk_end > skb_tail_pointer(chunk->skb)) {
		sctp_chunk_free(chunk);
		return sctp_inq_pop_recursive(queue, entry->next);
	} else {
		chunk->end_of_packet = 1;
	}
	return chunk;
}

struct sctp_chunk *sctp_inq_pop(struct sctp_inq *queue)
{
	struct sctp_chunk *chunk = queue->in_progress;

	if (chunk) {
		if (chunk->singleton || chunk->end_of_packet || chunk->pdiscard) {
			sctp_chunk_free(chunk);
			chunk = queue->in_progress = NULL;
		} else {
			sctp_chunkhdr_t *ch = (sctp_chunkhdr_t *) chunk->chunk_end;
			skb_pull(chunk->skb, chunk->chunk_end - chunk->skb->data);
			if (skb_headlen(chunk->skb) < sizeof(sctp_chunkhdr_t)) {
				sctp_chunk_free(chunk);
				chunk = queue->in_progress = NULL;
			}
		}
	}

	if (!chunk) {
		if (!list_empty(&queue->in_chunk_list)) {
			chunk = queue->in_progress = sctp_inq_pop_recursive(queue, queue->in_chunk_list.next);
		}
	}

	if (chunk) {
		pr_debug("+++sctp_inq_pop+++ chunk:%p[%s], length:%d, skb->len:%d\n",
		         chunk, sctp_cname(SCTP_ST_CHUNK(chunk->chunk_hdr->type)),
		         ntohs(chunk->chunk_hdr->length), chunk->skb->len);
	}
	return chunk;
}

void sctp_inq_set_th_handler(struct sctp_inq *q, work_func_t callback)
{
	INIT_WORK(&q->immediate, callback);
}