#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <net/sctp/sctp.h>
#include <net/sctp/sm.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

static inline struct list_head *getListHead(struct sctp_inq *queue) {
    return &queue->in_chunk_list;
}

static inline struct sctp_chunk **getInProgress(struct sctp_inq *queue) {
    return &queue->in_progress;
}

static inline void *getImmediate(struct sctp_inq *queue) {
    return &queue->immediate;
}

static inline struct sctp_chunk **getChunkInProgress(struct sctp_inq *queue) {
    return &queue->in_progress;
}

void sctp_inq_init(struct sctp_inq *queue)
{
    INIT_LIST_HEAD(getListHead(queue));
    *getInProgress(queue) = NULL;
    INIT_WORK(getImmediate(queue), NULL);
}

void sctp_inq_free(struct sctp_inq *queue)
{
    struct sctp_chunk *chunk, *tmp;

    list_for_each_entry_safe(chunk, tmp, getListHead(queue), list) {
        list_del_init(&chunk->list);
        sctp_chunk_free(chunk);
    }

    if (*getInProgress(queue)) {
        sctp_chunk_free(*getInProgress(queue));
        *getInProgress(queue) = NULL;
    }
}

void sctp_inq_push(struct sctp_inq *q, struct sctp_chunk *chunk)
{
    if (chunk->rcvr->dead) {
        sctp_chunk_free(chunk);
        return;
    }

    list_add_tail(&chunk->list, getListHead(q));
    if (chunk->asoc)
        chunk->asoc->stats.ipackets++;
    q->immediate.func(getImmediate(q));
}

struct sctp_chunkhdr *sctp_inq_peek(struct sctp_inq *queue)
{
    struct sctp_chunk *chunk;
    sctp_chunkhdr_t *ch = NULL;

    chunk = *getChunkInProgress(queue);
    if (chunk->singleton || chunk->end_of_packet || chunk->pdiscard)
        return NULL;

    ch = (sctp_chunkhdr_t *)chunk->chunk_end;

    return ch;
}

struct sctp_chunk *sctp_inq_pop(struct sctp_inq *queue)
{
    struct sctp_chunk *chunk;
    sctp_chunkhdr_t *ch = NULL;

    if ((chunk = *getChunkInProgress(queue))) {
        if (chunk->singleton || chunk->end_of_packet || chunk->pdiscard) {
            sctp_chunk_free(chunk);
            chunk = *getChunkInProgress(queue) = NULL;
        } else {
            ch = (sctp_chunkhdr_t *) chunk->chunk_end;
            skb_pull(chunk->skb, chunk->chunk_end - chunk->skb->data);

            if (skb_headlen(chunk->skb) < sizeof(sctp_chunkhdr_t)) {
                sctp_chunk_free(chunk);
                chunk = *getChunkInProgress(queue) = NULL;
            }
        }
    }

    if (!chunk) {
        struct list_head *entry;

        if (list_empty(getListHead(queue)))
            return NULL;

        entry = getListHead(queue)->next;
        chunk = *getChunkInProgress(queue) = list_entry(entry, struct sctp_chunk, list);
        list_del_init(entry);

        chunk->singleton = 1;
        ch = (sctp_chunkhdr_t *) chunk->skb->data;
        chunk->data_accepted = 0;
    }

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
        chunk = *getChunkInProgress(queue) = NULL;
        return NULL;
    } else {
        chunk->end_of_packet = 1;
    }

    pr_debug("+++sctp_inq_pop+++ chunk:%p[%s], length:%d, skb->len:%d\n",
             chunk, sctp_cname(SCTP_ST_CHUNK(chunk->chunk_hdr->type)),
             ntohs(chunk->chunk_hdr->length), chunk->skb->len);

    return chunk;
}

void sctp_inq_set_th_handler(struct sctp_inq *q, work_func_t callback)
{
    INIT_WORK(getImmediate(q), callback);
}