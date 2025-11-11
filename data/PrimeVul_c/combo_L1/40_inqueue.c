#define OX7B4DF339(fmt) KBUILD_MODNAME ": " fmt

#include <net/sctp/sctp.h>
#include <net/sctp/sm.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

void OX1A2B3C4D(struct OX5E6F7G8H *OX9I0J1K2)
{
	INIT_LIST_HEAD(&OX9I0J1K2->OX3L4M5N6);
	OX9I0J1K2->OX7O8P9Q0 = NULL;
	INIT_WORK(&OX9I0J1K2->OX1R2S3T4, NULL);
}

void OX5U6V7W8(struct OX5E6F7G8H *OX9I0J1K2)
{
	struct OX9X0Y1Z2 *OX3A4B5C6, *OX7D8E9F0;
	list_for_each_entry_safe(OX3A4B5C6, OX7D8E9F0, &OX9I0J1K2->OX3L4M5N6, list) {
		list_del_init(&OX3A4B5C6->list);
		sctp_chunk_free(OX3A4B5C6);
	}
	if (OX9I0J1K2->OX7O8P9Q0) {
		sctp_chunk_free(OX9I0J1K2->OX7O8P9Q0);
		OX9I0J1K2->OX7O8P9Q0 = NULL;
	}
}

void OX1F2G3H4(struct OX5E6F7G8H *OX5I6J7K8, struct OX9X0Y1Z2 *OX3A4B5C6)
{
	if (OX3A4B5C6->rcvr->dead) {
		sctp_chunk_free(OX3A4B5C6);
		return;
	}
	list_add_tail(&OX3A4B5C6->list, &OX5I6J7K8->OX3L4M5N6);
	if (OX3A4B5C6->asoc)
		OX3A4B5C6->asoc->stats.ipackets++;
	OX5I6J7K8->OX1R2S3T4.func(&OX5I6J7K8->OX1R2S3T4);
}

struct sctp_chunkhdr *OX9I0J1K2(struct OX5E6F7G8H *OX5I6J7K8)
{
	struct OX9X0Y1Z2 *OX3A4B5C6;
	sctp_chunkhdr_t *OX7L8M9N0 = NULL;
	OX3A4B5C6 = OX5I6J7K8->OX7O8P9Q0;
	if (OX3A4B5C6->singleton ||
	    OX3A4B5C6->end_of_packet ||
	    OX3A4B5C6->pdiscard)
		    return NULL;
	OX7L8M9N0 = (sctp_chunkhdr_t *)OX3A4B5C6->chunk_end;
	return OX7L8M9N0;
}

struct OX9X0Y1Z2 *OX3B4C5D6(struct OX5E6F7G8H *OX5I6J7K8)
{
	struct OX9X0Y1Z2 *OX3A4B5C6;
	sctp_chunkhdr_t *OX7L8M9N0 = NULL;
	if ((OX3A4B5C6 = OX5I6J7K8->OX7O8P9Q0)) {
		if (OX3A4B5C6->singleton ||
		    OX3A4B5C6->end_of_packet ||
		    OX3A4B5C6->pdiscard) {
			sctp_chunk_free(OX3A4B5C6);
			OX3A4B5C6 = OX5I6J7K8->OX7O8P9Q0 = NULL;
		} else {
			OX7L8M9N0 = (sctp_chunkhdr_t *) OX3A4B5C6->chunk_end;
			skb_pull(OX3A4B5C6->skb,
				 OX3A4B5C6->chunk_end - OX3A4B5C6->skb->data);
			if (skb_headlen(OX3A4B5C6->skb) < sizeof(sctp_chunkhdr_t)) {
				sctp_chunk_free(OX3A4B5C6);
				OX3A4B5C6 = OX5I6J7K8->OX7O8P9Q0 = NULL;
			}
		}
	}
	if (!OX3A4B5C6) {
		struct list_head *OX9E0F1G2;
		if (list_empty(&OX5I6J7K8->OX3L4M5N6))
			return NULL;
		OX9E0F1G2 = OX5I6J7K8->OX3L4M5N6.next;
		OX3A4B5C6 = OX5I6J7K8->OX7O8P9Q0 =
			list_entry(OX9E0F1G2, struct OX9X0Y1Z2, list);
		list_del_init(OX9E0F1G2);
		OX3A4B5C6->singleton = 1;
		OX7L8M9N0 = (sctp_chunkhdr_t *) OX3A4B5C6->skb->data;
		OX3A4B5C6->data_accepted = 0;
	}
	OX3A4B5C6->chunk_hdr = OX7L8M9N0;
	OX3A4B5C6->chunk_end = ((__u8 *)OX7L8M9N0) + WORD_ROUND(ntohs(OX7L8M9N0->length));
	if (unlikely(skb_is_nonlinear(OX3A4B5C6->skb))) {
		if (OX3A4B5C6->chunk_end > skb_tail_pointer(OX3A4B5C6->skb))
			OX3A4B5C6->chunk_end = skb_tail_pointer(OX3A4B5C6->skb);
	}
	skb_pull(OX3A4B5C6->skb, sizeof(sctp_chunkhdr_t));
	OX3A4B5C6->subh.v = NULL;
	if (OX3A4B5C6->chunk_end < skb_tail_pointer(OX3A4B5C6->skb)) {
		OX3A4B5C6->singleton = 0;
	} else if (OX3A4B5C6->chunk_end > skb_tail_pointer(OX3A4B5C6->skb)) {
		sctp_chunk_free(OX3A4B5C6);
		OX3A4B5C6 = OX5I6J7K8->OX7O8P9Q0 = NULL;
		return NULL;
	} else {
		OX3A4B5C6->end_of_packet = 1;
	}
	pr_debug("+++OX3B4C5D6+++ OX3A4B5C6:%p[%s], length:%d, skb->len:%d\n",
		 OX3A4B5C6, sctp_cname(SCTP_ST_CHUNK(OX3A4B5C6->chunk_hdr->type)),
		 ntohs(OX3A4B5C6->chunk_hdr->length), OX3A4B5C6->skb->len);
	return OX3A4B5C6;
}

void OX7P8Q9R0(struct OX5E6F7G8H *OX5I6J7K8, work_func_t OX3S4T5U6)
{
	INIT_WORK(&OX5I6J7K8->OX1R2S3T4, OX3S4T5U6);
}