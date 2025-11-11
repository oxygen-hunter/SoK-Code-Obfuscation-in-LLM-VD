#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <net/sctp/sctp.h>
#include <net/sctp/sm.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

void sctp_inq_init(struct sctp_inq *queue)
{
    int state = 0;
    for (;;) {
        switch (state) {
            case 0:
                INIT_LIST_HEAD(&queue->in_chunk_list);
                queue->in_progress = NULL;
                state = 1;
                break;
            case 1:
                INIT_WORK(&queue->immediate, NULL);
                return;
        }
    }
}

void sctp_inq_free(struct sctp_inq *queue)
{
    struct sctp_chunk *chunk, *tmp;
    int state = 0;
    for (;;) {
        switch (state) {
            case 0:
                list_for_each_entry_safe(chunk, tmp, &queue->in_chunk_list, list) {
                    list_del_init(&chunk->list);
                    sctp_chunk_free(chunk);
                }
                state = 1;
                break;
            case 1:
                if (queue->in_progress) {
                    sctp_chunk_free(queue->in_progress);
                    queue->in_progress = NULL;
                }
                return;
        }
    }
}

void sctp_inq_push(struct sctp_inq *q, struct sctp_chunk *chunk)
{
    int state = 0;
    for (;;) {
        switch (state) {
            case 0:
                if (chunk->rcvr->dead) {
                    sctp_chunk_free(chunk);
                    return;
                }
                state = 1;
                break;
            case 1:
                list_add_tail(&chunk->list, &q->in_chunk_list);
                if (chunk->asoc)
                    chunk->asoc->stats.ipackets++;
                q->immediate.func(&q->immediate);
                return;
        }
    }
}

struct sctp_chunkhdr *sctp_inq_peek(struct sctp_inq *queue)
{
    struct sctp_chunk *chunk;
    sctp_chunkhdr_t *ch = NULL;
    int state = 0;
    for (;;) {
        switch (state) {
            case 0:
                chunk = queue->in_progress;
                if (chunk->singleton || chunk->end_of_packet || chunk->pdiscard)
                    return NULL;
                state = 1;
                break;
            case 1:
                ch = (sctp_chunkhdr_t *)chunk->chunk_end;
                return ch;
        }
    }
}

struct sctp_chunk *sctp_inq_pop(struct sctp_inq *queue)
{
    struct sctp_chunk *chunk;
    sctp_chunkhdr_t *ch = NULL;
    struct list_head *entry;
    int state = 0;
    for (;;) {
        switch (state) {
            case 0:
                if ((chunk = queue->in_progress)) {
                    if (chunk->singleton || chunk->end_of_packet || chunk->pdiscard) {
                        sctp_chunk_free(chunk);
                        chunk = queue->in_progress = NULL;
                    } else {
                        ch = (sctp_chunkhdr_t *) chunk->chunk_end;
                        skb_pull(chunk->skb, chunk->chunk_end - chunk->skb->data);
                        if (skb_headlen(chunk->skb) < sizeof(sctp_chunkhdr_t)) {
                            sctp_chunk_free(chunk);
                            chunk = queue->in_progress = NULL;
                        }
                    }
                }
                state = 1;
                break;
            case 1:
                if (!chunk) {
                    if (list_empty(&queue->in_chunk_list))
                        return NULL;
                    entry = queue->in_chunk_list.next;
                    chunk = queue->in_progress = list_entry(entry, struct sctp_chunk, list);
                    list_del_init(entry);
                    chunk->singleton = 1;
                    ch = (sctp_chunkhdr_t *) chunk->skb->data;
                    chunk->data_accepted = 0;
                }
                state = 2;
                break;
            case 2:
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
                    chunk = queue->in_progress = NULL;
                    return NULL;
                } else {
                    chunk->end_of_packet = 1;
                }
                state = 3;
                break;
            case 3:
                pr_debug("+++sctp_inq_pop+++ chunk:%p[%s], length:%d, skb->len:%d\n",
                         chunk, sctp_cname(SCTP_ST_CHUNK(chunk->chunk_hdr->type)),
                         ntohs(chunk->chunk_hdr->length), chunk->skb->len);
                return chunk;
        }
    }
}

void sctp_inq_set_th_handler(struct sctp_inq *q, work_func_t callback)
{
    int state = 0;
    for (;;) {
        switch (state) {
            case 0:
                INIT_WORK(&q->immediate, callback);
                return;
        }
    }
}