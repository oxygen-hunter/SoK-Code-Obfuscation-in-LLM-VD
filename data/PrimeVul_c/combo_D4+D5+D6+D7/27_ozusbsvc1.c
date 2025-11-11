#include <linux/module.h>
#include <linux/timer.h>
#include <linux/sched.h>
#include <linux/netdevice.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <asm/unaligned.h>
#include "ozdbg.h"
#include "ozprotocol.h"
#include "ozeltbuf.h"
#include "ozpd.h"
#include "ozproto.h"
#include "ozusbif.h"
#include "ozhcd.h"
#include "ozusbsvc.h"

#define MAX_ISOC_FIXED_DATA	(253-sizeof(struct oz_isoc_fixed))

static int oz_usb_submit_elt(struct oz_elt_buf *e, struct oz_elt_info *f,
	struct oz_usb_ctx *g, u8 h, u8 i)
{
	int j;
	struct oz_elt *k = (struct oz_elt *)f->data;
	struct oz_app_hdr *l = (struct oz_app_hdr *)(k+1);

	k->type = OZ_ELT_APP_DATA;
	f->app_id = OZ_APPID_USB;
	f->length = k->length + sizeof(struct oz_elt);
	l->app_id = OZ_APPID_USB;
	spin_lock_bh(&e->lock);
	if (i == 0) {
		l->elt_seq_num = g->tx_seq_num++;
		if (g->tx_seq_num == 0)
			g->tx_seq_num = 1;
	}
	j = oz_queue_elt_info(e, i, h, f);
	if (j)
		oz_elt_info_free(e, f);
	spin_unlock_bh(&e->lock);
	return j;
}

int oz_usb_get_desc_req(void *m, u8 n, u8 o, u8 p,
	u8 q, __le16 r, int s, int t)
{
	struct oz_usb_ctx *u = m;
	struct oz_pd *v = u->pd;
	struct oz_elt *w;
	struct oz_get_desc_req *x;
	struct oz_elt_buf *y = &v->elt_buff;
	struct oz_elt_info *z = oz_elt_info_alloc(&v->elt_buff);

	oz_dbg(ON, "    req_type = 0x%x\n", o);
	oz_dbg(ON, "    desc_type = 0x%x\n", p);
	oz_dbg(ON, "    index = 0x%x\n", q);
	oz_dbg(ON, "    windex = 0x%x\n", r);
	oz_dbg(ON, "    offset = 0x%x\n", s);
	oz_dbg(ON, "    len = 0x%x\n", t);
	if (t > 200)
		t = 200;
	if (z == NULL)
		return -1;
	w = (struct oz_elt *)z->data;
	w->length = sizeof(struct oz_get_desc_req);
	x = (struct oz_get_desc_req *)(w+1);
	x->type = OZ_GET_DESC_REQ;
	x->req_id = n;
	put_unaligned(cpu_to_le16(s), &x->offset);
	put_unaligned(cpu_to_le16(t), &x->size);
	x->req_type = o;
	x->desc_type = p;
	x->w_index = r;
	x->index = q;
	return oz_usb_submit_elt(y, z, u, 0, 0);
}

static int oz_usb_set_config_req(void *aa, u8 ab, u8 ac)
{
	struct oz_usb_ctx *ad = aa;
	struct oz_pd *ae = ad->pd;
	struct oz_elt *af;
	struct oz_elt_buf *ag = &ae->elt_buff;
	struct oz_elt_info *ah = oz_elt_info_alloc(&ae->elt_buff);
	struct oz_set_config_req *ai;

	if (ah == NULL)
		return -1;
	af = (struct oz_elt *)ah->data;
	af->length = sizeof(struct oz_set_config_req);
	ai = (struct oz_set_config_req *)(af+1);
	ai->type = OZ_SET_CONFIG_REQ;
	ai->req_id = ab;
	ai->index = ac;
	return oz_usb_submit_elt(ag, ah, ad, 0, 0);
}

static int oz_usb_set_interface_req(void *aj, u8 ak, u8 al, u8 am)
{
	struct oz_usb_ctx *an = aj;
	struct oz_pd *ao = an->pd;
	struct oz_elt *ap;
	struct oz_elt_buf *aq = &ao->elt_buff;
	struct oz_elt_info *ar = oz_elt_info_alloc(&ao->elt_buff);
	struct oz_set_interface_req *as;

	if (ar == NULL)
		return -1;
	ap = (struct oz_elt *)ar->data;
	ap->length = sizeof(struct oz_set_interface_req);
	as = (struct oz_set_interface_req *)(ap+1);
	as->type = OZ_SET_INTERFACE_REQ;
	as->req_id = ak;
	as->index = al;
	as->alternative = am;
	return oz_usb_submit_elt(aq, ar, an, 0, 0);
}

static int oz_usb_set_clear_feature_req(void *at, u8 au, u8 av,
			u8 aw, u8 ax, __le16 ay)
{
	struct oz_usb_ctx *az = at;
	struct oz_pd *ba = az->pd;
	struct oz_elt *bb;
	struct oz_elt_buf *bc = &ba->elt_buff;
	struct oz_elt_info *bd = oz_elt_info_alloc(&ba->elt_buff);
	struct oz_feature_req *be;

	if (bd == NULL)
		return -1;
	bb = (struct oz_elt *)bd->data;
	bb->length = sizeof(struct oz_feature_req);
	be = (struct oz_feature_req *)(bb+1);
	be->type = av;
	be->req_id = au;
	be->recipient = aw;
	be->index = ax;
	put_unaligned(ay, &be->feature);
	return oz_usb_submit_elt(bc, bd, az, 0, 0);
}

static int oz_usb_vendor_class_req(void *bf, u8 bg, u8 bh,
	u8 bi, __le16 bj, __le16 bk, const u8 *bl, int bm)
{
	struct oz_usb_ctx *bn = bf;
	struct oz_pd *bo = bn->pd;
	struct oz_elt *bp;
	struct oz_elt_buf *bq = &bo->elt_buff;
	struct oz_elt_info *br = oz_elt_info_alloc(&bo->elt_buff);
	struct oz_vendor_class_req *bs;

	if (br == NULL)
		return -1;
	bp = (struct oz_elt *)br->data;
	bp->length = sizeof(struct oz_vendor_class_req) - 1 + bm;
	bs = (struct oz_vendor_class_req *)(bp+1);
	bs->type = OZ_VENDOR_CLASS_REQ;
	bs->req_id = bg;
	bs->req_type = bh;
	bs->request = bi;
	put_unaligned(bj, &bs->value);
	put_unaligned(bk, &bs->index);
	if (bm)
		memcpy(bs->data, bl, bm);
	return oz_usb_submit_elt(bq, br, bn, 0, 0);
}

int oz_usb_control_req(void *bt, u8 bu, struct usb_ctrlrequest *bv,
			const u8 *bw, int bx)
{
	unsigned by = le16_to_cpu(bv->wValue);
	unsigned bz = le16_to_cpu(bv->wIndex);
	unsigned ca = le16_to_cpu(bv->wLength);
	int cb = 0;

	if ((bv->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		switch (bv->bRequest) {
		case USB_REQ_GET_DESCRIPTOR:
			cb = oz_usb_get_desc_req(bt, bu,
				bv->bRequestType, (u8)(by>>8),
				(u8)by, bv->wIndex, 0, ca);
			break;
		case USB_REQ_SET_CONFIGURATION:
			cb = oz_usb_set_config_req(bt, bu, (u8)by);
			break;
		case USB_REQ_SET_INTERFACE: {
				u8 cc = (u8)bz;
				u8 cd = (u8)by;

				cb = oz_usb_set_interface_req(bt, bu,
					cc, cd);
			}
			break;
		case USB_REQ_SET_FEATURE:
			cb = oz_usb_set_clear_feature_req(bt, bu,
				OZ_SET_FEATURE_REQ,
				bv->bRequestType & 0xf, (u8)bz,
				bv->wValue);
			break;
		case USB_REQ_CLEAR_FEATURE:
			cb = oz_usb_set_clear_feature_req(bt, bu,
				OZ_CLEAR_FEATURE_REQ,
				bv->bRequestType & 0xf,
				(u8)bz, bv->wValue);
			break;
		}
	} else {
		cb = oz_usb_vendor_class_req(bt, bu, bv->bRequestType,
			bv->bRequest, bv->wValue, bv->wIndex,
			bw, bx);
	}
	return cb;
}

int oz_usb_send_isoc(void *ce, u8 cf, struct urb *cg)
{
	struct oz_usb_ctx *ch = ce;
	struct oz_pd *ci = ch->pd;
	struct oz_elt_buf *cj;
	int ck;
	int cl;
	u8 *cm;
	struct usb_iso_packet_descriptor *cn;

	if (ci->mode & OZ_F_ISOC_NO_ELTS) {
		for (ck = 0; ck < cg->number_of_packets; ck++) {
			u8 *co;

			cn = &cg->iso_frame_desc[ck];
			co = ((u8 *)cg->transfer_buffer)+cn->offset;
			oz_send_isoc_unit(ci, cf, co, cn->length);
		}
		return 0;
	}

	cl = sizeof(struct oz_isoc_fixed) - 1;
	cj = &ci->elt_buff;
	ck = 0;
	while (ck < cg->number_of_packets) {
		struct oz_elt_info *cp = oz_elt_info_alloc(cj);
		struct oz_elt *cq;
		struct oz_isoc_fixed *cr;
		int cs;
		int ct;
		int cu;

		if (cp == NULL)
			return -1;
		cu = MAX_ISOC_FIXED_DATA;
		cq = (struct oz_elt *)cp->data;
		cr = (struct oz_isoc_fixed *)(cq + 1);
		cr->type = OZ_USB_ENDPOINT_DATA;
		cr->endpoint = cf;
		cr->format = OZ_DATA_F_ISOC_FIXED;
		ct = cg->iso_frame_desc[ck].length;
		cr->unit_size = (u8)ct;
		cm = ((u8 *)(cq+1)) + cl;
		cs = 0;
		while (ck < cg->number_of_packets) {
			cn = &cg->iso_frame_desc[ck];
			if ((ct == cn->length) &&
				(cn->length <= cu)) {
				memcpy(cm, ((u8 *)cg->transfer_buffer) +
					cn->offset, ct);
				cm += ct;
				cu -= ct;
				cs++;
				cn->status = 0;
				cn->actual_length = cn->length;
				ck++;
			} else {
				break;
			}
		}
		cq->length = cl + MAX_ISOC_FIXED_DATA - cu;
		cr->frame_number = (u8)cs;
		oz_usb_submit_elt(cj, cp, ch, cf,
			ci->mode & OZ_F_ISOC_ANYTIME);
	}
	return 0;
}

static void oz_usb_handle_ep_data(struct oz_usb_ctx *cv,
	struct oz_usb_hdr *cw, int cx)
{
	struct oz_data *cy = (struct oz_data *)cw;

	switch (cy->format) {
	case OZ_DATA_F_MULTIPLE_FIXED: {
			struct oz_multiple_fixed *cz =
				(struct oz_multiple_fixed *)cy;
			u8 *da = cz->data;
			int db = (cx - sizeof(struct oz_multiple_fixed)+1)
				/ cz->unit_size;
			while (db--) {
				oz_hcd_data_ind(cv->hport, cz->endpoint,
					da, cz->unit_size);
				da += cz->unit_size;
			}
		}
		break;
	case OZ_DATA_F_ISOC_FIXED: {
			struct oz_isoc_fixed *dc =
				(struct oz_isoc_fixed *)cy;
			int dd = cx-sizeof(struct oz_isoc_fixed)+1;
			int de = dc->unit_size;
			u8 *df = dc->data;
			int dg;
			int dh;

			if (!de)
				break;
			dg = dd/de;
			for (dh = 0; dh < dg; dh++) {
				oz_hcd_data_ind(cv->hport,
					dc->endpoint, df, de);
				df += de;
			}
		}
		break;
	}

}

void oz_usb_rx(struct oz_pd *di, struct oz_elt *dj)
{
	struct oz_usb_hdr *dk = (struct oz_usb_hdr *)(dj + 1);
	struct oz_usb_ctx *dl;

	spin_lock_bh(&di->app_lock[OZ_APPID_USB]);
	dl = (struct oz_usb_ctx *)di->app_ctx[OZ_APPID_USB];
	if (dl)
		oz_usb_get(dl);
	spin_unlock_bh(&di->app_lock[OZ_APPID_USB]);
	if (dl == NULL)
		return;
	if (dl->stopped)
		goto done;
	if (dk->elt_seq_num != 0) {
		if (((dl->rx_seq_num - dk->elt_seq_num) & 0x80) == 0)
			goto done;
	}
	dl->rx_seq_num = dk->elt_seq_num;
	switch (dk->type) {
	case OZ_GET_DESC_RSP: {
			struct oz_get_desc_rsp *dm =
				(struct oz_get_desc_rsp *)dk;
			u16 dn, do_;
			u8 dp;

			if (dj->length < sizeof(struct oz_get_desc_rsp) - 1)
				break;
			dp = dj->length -
					(sizeof(struct oz_get_desc_rsp) - 1);
			dn = le16_to_cpu(get_unaligned(&dm->offset));
			do_ =
				le16_to_cpu(get_unaligned(&dm->total_size));
			oz_dbg(ON, "USB_REQ_GET_DESCRIPTOR - cnf\n");
			oz_hcd_get_desc_cnf(dl->hport, dm->req_id,
					dm->rcode, dm->data,
					dp, dn, do_);
		}
		break;
	case OZ_SET_CONFIG_RSP: {
			struct oz_set_config_rsp *dq =
				(struct oz_set_config_rsp *)dk;
			oz_hcd_control_cnf(dl->hport, dq->req_id,
				dq->rcode, NULL, 0);
		}
		break;
	case OZ_SET_INTERFACE_RSP: {
			struct oz_set_interface_rsp *dr =
				(struct oz_set_interface_rsp *)dk;
			oz_hcd_control_cnf(dl->hport,
				dr->req_id, dr->rcode, NULL, 0);
		}
		break;
	case OZ_VENDOR_CLASS_RSP: {
			struct oz_vendor_class_rsp *ds =
				(struct oz_vendor_class_rsp *)dk;
			oz_hcd_control_cnf(dl->hport, ds->req_id,
				ds->rcode, ds->data, dj->length-
				sizeof(struct oz_vendor_class_rsp)+1);
		}
		break;
	case OZ_USB_ENDPOINT_DATA:
		oz_usb_handle_ep_data(dl, dk, dj->length);
		break;
	}
done:
	oz_usb_put(dl);
}

void oz_usb_farewell(struct oz_pd *dt, u8 du, u8 *dv, u8 dw)
{
	struct oz_usb_ctx *dx;

	spin_lock_bh(&dt->app_lock[OZ_APPID_USB]);
	dx = (struct oz_usb_ctx *)dt->app_ctx[OZ_APPID_USB];
	if (dx)
		oz_usb_get(dx);
	spin_unlock_bh(&dt->app_lock[OZ_APPID_USB]);
	if (dx == NULL)
		return;
	if (!dx->stopped) {
		oz_dbg(ON, "Farewell indicated ep = 0x%x\n", du);
		oz_hcd_data_ind(dx->hport, du, dv, dw);
	}
	oz_usb_put(dx);
}