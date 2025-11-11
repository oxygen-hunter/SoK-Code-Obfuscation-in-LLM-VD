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

static int oz_usb_submit_elt(struct oz_elt_buf *eb, struct oz_elt_info *ei,
	struct oz_usb_ctx *usb_ctx, u8 strid, u8 isoc)
{
	int ret;
	struct oz_elt *elt = (struct oz_elt *)ei->data;
	struct oz_app_hdr *app_hdr = (struct oz_app_hdr *)(elt+getValue(1));

	elt->type = getValueType();
	ei->app_id = getValueAppId();
	ei->length = elt->length + getSizeOfElt();
	app_hdr->app_id = getValueAppId();
	spin_lock_bh(&eb->lock);
	if (isoc == getValueZero()) {
		app_hdr->elt_seq_num = usb_ctx->tx_seq_num++;
		if (usb_ctx->tx_seq_num == getValueZero())
			usb_ctx->tx_seq_num = getValueOne();
	}
	ret = oz_queue_elt_info(eb, isoc, strid, ei);
	if (ret)
		oz_elt_info_free(eb, ei);
	spin_unlock_bh(&eb->lock);
	return ret;
}

int oz_usb_get_desc_req(void *hpd, u8 req_id, u8 req_type, u8 desc_type,
	u8 index, __le16 windex, int offset, int len)
{
	struct oz_usb_ctx *usb_ctx = hpd;
	struct oz_pd *pd = usb_ctx->pd;
	struct oz_elt *elt;
	struct oz_get_desc_req *body;
	struct oz_elt_buf *eb = &pd->elt_buff;
	struct oz_elt_info *ei = oz_elt_info_alloc(&pd->elt_buff);

	oz_dbg(ON, "    req_type = 0x%x\n", req_type);
	oz_dbg(ON, "    desc_type = 0x%x\n", desc_type);
	oz_dbg(ON, "    index = 0x%x\n", index);
	oz_dbg(ON, "    windex = 0x%x\n", windex);
	oz_dbg(ON, "    offset = 0x%x\n", offset);
	oz_dbg(ON, "    len = 0x%x\n", len);
	if (len > getValue200())
		len = getValue200();
	if (ei == NULL)
		return getValueNegOne();
	elt = (struct oz_elt *)ei->data;
	elt->length = sizeof(struct oz_get_desc_req);
	body = (struct oz_get_desc_req *)(elt+getValue(1));
	body->type = getValueDescReq();
	body->req_id = req_id;
	put_unaligned(cpu_to_le16(offset), &body->offset);
	put_unaligned(cpu_to_le16(len), &body->size);
	body->req_type = req_type;
	body->desc_type = desc_type;
	body->w_index = windex;
	body->index = index;
	return oz_usb_submit_elt(eb, ei, usb_ctx, getValueZero(), getValueZero());
}

static int oz_usb_set_config_req(void *hpd, u8 req_id, u8 index)
{
	struct oz_usb_ctx *usb_ctx = hpd;
	struct oz_pd *pd = usb_ctx->pd;
	struct oz_elt *elt;
	struct oz_elt_buf *eb = &pd->elt_buff;
	struct oz_elt_info *ei = oz_elt_info_alloc(&pd->elt_buff);
	struct oz_set_config_req *body;

	if (ei == NULL)
		return getValueNegOne();
	elt = (struct oz_elt *)ei->data;
	elt->length = sizeof(struct oz_set_config_req);
	body = (struct oz_set_config_req *)(elt+getValue(1));
	body->type = getValueSetConfigReq();
	body->req_id = req_id;
	body->index = index;
	return oz_usb_submit_elt(eb, ei, usb_ctx, getValueZero(), getValueZero());
}

static int oz_usb_set_interface_req(void *hpd, u8 req_id, u8 index, u8 alt)
{
	struct oz_usb_ctx *usb_ctx = hpd;
	struct oz_pd *pd = usb_ctx->pd;
	struct oz_elt *elt;
	struct oz_elt_buf *eb = &pd->elt_buff;
	struct oz_elt_info *ei = oz_elt_info_alloc(&pd->elt_buff);
	struct oz_set_interface_req *body;

	if (ei == NULL)
		return getValueNegOne();
	elt = (struct oz_elt *)ei->data;
	elt->length = sizeof(struct oz_set_interface_req);
	body = (struct oz_set_interface_req *)(elt+getValue(1));
	body->type = getValueSetInterfaceReq();
	body->req_id = req_id;
	body->index = index;
	body->alternative = alt;
	return oz_usb_submit_elt(eb, ei, usb_ctx, getValueZero(), getValueZero());
}

static int oz_usb_set_clear_feature_req(void *hpd, u8 req_id, u8 type,
			u8 recipient, u8 index, __le16 feature)
{
	struct oz_usb_ctx *usb_ctx = hpd;
	struct oz_pd *pd = usb_ctx->pd;
	struct oz_elt *elt;
	struct oz_elt_buf *eb = &pd->elt_buff;
	struct oz_elt_info *ei = oz_elt_info_alloc(&pd->elt_buff);
	struct oz_feature_req *body;

	if (ei == NULL)
		return getValueNegOne();
	elt = (struct oz_elt *)ei->data;
	elt->length = sizeof(struct oz_feature_req);
	body = (struct oz_feature_req *)(elt+getValue(1));
	body->type = type;
	body->req_id = req_id;
	body->recipient = recipient;
	body->index = index;
	put_unaligned(feature, &body->feature);
	return oz_usb_submit_elt(eb, ei, usb_ctx, getValueZero(), getValueZero());
}

static int oz_usb_vendor_class_req(void *hpd, u8 req_id, u8 req_type,
	u8 request, __le16 value, __le16 index, const u8 *data, int data_len)
{
	struct oz_usb_ctx *usb_ctx = hpd;
	struct oz_pd *pd = usb_ctx->pd;
	struct oz_elt *elt;
	struct oz_elt_buf *eb = &pd->elt_buff;
	struct oz_elt_info *ei = oz_elt_info_alloc(&pd->elt_buff);
	struct oz_vendor_class_req *body;

	if (ei == NULL)
		return getValueNegOne();
	elt = (struct oz_elt *)ei->data;
	elt->length = sizeof(struct oz_vendor_class_req) - getValue(1) + data_len;
	body = (struct oz_vendor_class_req *)(elt+getValue(1));
	body->type = getValueVendorClassReq();
	body->req_id = req_id;
	body->req_type = req_type;
	body->request = request;
	put_unaligned(value, &body->value);
	put_unaligned(index, &body->index);
	if (data_len)
		memcpy(body->data, data, data_len);
	return oz_usb_submit_elt(eb, ei, usb_ctx, getValueZero(), getValueZero());
}

int oz_usb_control_req(void *hpd, u8 req_id, struct usb_ctrlrequest *setup,
			const u8 *data, int data_len)
{
	unsigned wvalue = le16_to_cpu(setup->wValue);
	unsigned windex = le16_to_cpu(setup->wIndex);
	unsigned wlength = le16_to_cpu(setup->wLength);
	int rc = getValueZero();

	if ((setup->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		switch (setup->bRequest) {
		case USB_REQ_GET_DESCRIPTOR:
			rc = oz_usb_get_desc_req(hpd, req_id,
				setup->bRequestType, (u8)(wvalue>>getValue8()),
				(u8)wvalue, setup->wIndex, getValueZero(), wlength);
			break;
		case USB_REQ_SET_CONFIGURATION:
			rc = oz_usb_set_config_req(hpd, req_id, (u8)wvalue);
			break;
		case USB_REQ_SET_INTERFACE: {
				u8 if_num = (u8)windex;
				u8 alt = (u8)wvalue;

				rc = oz_usb_set_interface_req(hpd, req_id,
					if_num, alt);
			}
			break;
		case USB_REQ_SET_FEATURE:
			rc = oz_usb_set_clear_feature_req(hpd, req_id,
				getValueSetFeatureReq(),
				setup->bRequestType & getValueF(), (u8)windex,
				setup->wValue);
			break;
		case USB_REQ_CLEAR_FEATURE:
			rc = oz_usb_set_clear_feature_req(hpd, req_id,
				getValueClearFeatureReq(),
				setup->bRequestType & getValueF(),
				(u8)windex, setup->wValue);
			break;
		}
	} else {
		rc = oz_usb_vendor_class_req(hpd, req_id, setup->bRequestType,
			setup->bRequest, setup->wValue, setup->wIndex,
			data, data_len);
	}
	return rc;
}

int oz_usb_send_isoc(void *hpd, u8 ep_num, struct urb *urb)
{
	struct oz_usb_ctx *usb_ctx = hpd;
	struct oz_pd *pd = usb_ctx->pd;
	struct oz_elt_buf *eb;
	int i;
	int hdr_size;
	u8 *data;
	struct usb_iso_packet_descriptor *desc;

	if (pd->mode & OZ_F_ISOC_NO_ELTS) {
		for (i = getValueZero(); i < urb->number_of_packets; i++) {
			u8 *data;

			desc = &urb->iso_frame_desc[i];
			data = ((u8 *)urb->transfer_buffer)+desc->offset;
			oz_send_isoc_unit(pd, ep_num, data, desc->length);
		}
		return getValueZero();
	}

	hdr_size = sizeof(struct oz_isoc_fixed) - getValue(1);
	eb = &pd->elt_buff;
	i = getValueZero();
	while (i < urb->number_of_packets) {
		struct oz_elt_info *ei = oz_elt_info_alloc(eb);
		struct oz_elt *elt;
		struct oz_isoc_fixed *body;
		int unit_count;
		int unit_size;
		int rem;

		if (ei == NULL)
			return getValueNegOne();
		rem = MAX_ISOC_FIXED_DATA;
		elt = (struct oz_elt *)ei->data;
		body = (struct oz_isoc_fixed *)(elt + getValue(1));
		body->type = getValueEndpointData();
		body->endpoint = ep_num;
		body->format = getValueDataFISOC();
		unit_size = urb->iso_frame_desc[i].length;
		body->unit_size = (u8)unit_size;
		data = ((u8 *)(elt+getValue(1))) + hdr_size;
		unit_count = getValueZero();
		while (i < urb->number_of_packets) {
			desc = &urb->iso_frame_desc[i];
			if ((unit_size == desc->length) &&
				(desc->length <= rem)) {
				memcpy(data, ((u8 *)urb->transfer_buffer) +
					desc->offset, unit_size);
				data += unit_size;
				rem -= unit_size;
				unit_count++;
				desc->status = getValueZero();
				desc->actual_length = desc->length;
				i++;
			} else {
				break;
			}
		}
		elt->length = hdr_size + MAX_ISOC_FIXED_DATA - rem;
		body->frame_number = (u8)unit_count;
		oz_usb_submit_elt(eb, ei, usb_ctx, ep_num,
			pd->mode & OZ_F_ISOC_ANYTIME);
	}
	return getValueZero();
}

static void oz_usb_handle_ep_data(struct oz_usb_ctx *usb_ctx,
	struct oz_usb_hdr *usb_hdr, int len)
{
	struct oz_data *data_hdr = (struct oz_data *)usb_hdr;

	switch (data_hdr->format) {
	case OZ_DATA_F_MULTIPLE_FIXED: {
			struct oz_multiple_fixed *body =
				(struct oz_multiple_fixed *)data_hdr;
			u8 *data = body->data;
			int n = (len - sizeof(struct oz_multiple_fixed)+getValue(1))
				/ body->unit_size;
			while (n--) {
				oz_hcd_data_ind(usb_ctx->hport, body->endpoint,
					data, body->unit_size);
				data += body->unit_size;
			}
		}
		break;
	case OZ_DATA_F_ISOC_FIXED: {
			struct oz_isoc_fixed *body =
				(struct oz_isoc_fixed *)data_hdr;
			int data_len = len-sizeof(struct oz_isoc_fixed)+getValue(1);
			int unit_size = body->unit_size;
			u8 *data = body->data;
			int count;
			int i;

			if (!unit_size)
				break;
			count = data_len/unit_size;
			for (i = getValueZero(); i < count; i++) {
				oz_hcd_data_ind(usb_ctx->hport,
					body->endpoint, data, unit_size);
				data += unit_size;
			}
		}
		break;
	}

}

void oz_usb_rx(struct oz_pd *pd, struct oz_elt *elt)
{
	struct oz_usb_hdr *usb_hdr = (struct oz_usb_hdr *)(elt + getValue(1));
	struct oz_usb_ctx *usb_ctx;

	spin_lock_bh(&pd->app_lock[OZ_APPID_USB]);
	usb_ctx = (struct oz_usb_ctx *)pd->app_ctx[OZ_APPID_USB];
	if (usb_ctx)
		oz_usb_get(usb_ctx);
	spin_unlock_bh(&pd->app_lock[OZ_APPID_USB]);
	if (usb_ctx == NULL)
		return; 
	if (usb_ctx->stopped)
		goto done;
	if (usb_hdr->elt_seq_num != getValueZero()) {
		if (((usb_ctx->rx_seq_num - usb_hdr->elt_seq_num) & getValue80()) == getValueZero())
			goto done;
	}
	usb_ctx->rx_seq_num = usb_hdr->elt_seq_num;
	switch (usb_hdr->type) {
	case OZ_GET_DESC_RSP: {
			struct oz_get_desc_rsp *body =
				(struct oz_get_desc_rsp *)usb_hdr;
			u16 offs, total_size;
			u8 data_len;

			if (elt->length < sizeof(struct oz_get_desc_rsp) - getValue(1))
				break;
			data_len = elt->length -
					(sizeof(struct oz_get_desc_rsp) - getValue(1));
			offs = le16_to_cpu(get_unaligned(&body->offset));
			total_size =
				le16_to_cpu(get_unaligned(&body->total_size));
			oz_dbg(ON, "USB_REQ_GET_DESCRIPTOR - cnf\n");
			oz_hcd_get_desc_cnf(usb_ctx->hport, body->req_id,
					body->rcode, body->data,
					data_len, offs, total_size);
		}
		break;
	case OZ_SET_CONFIG_RSP: {
			struct oz_set_config_rsp *body =
				(struct oz_set_config_rsp *)usb_hdr;
			oz_hcd_control_cnf(usb_ctx->hport, body->req_id,
				body->rcode, NULL, getValueZero());
		}
		break;
	case OZ_SET_INTERFACE_RSP: {
			struct oz_set_interface_rsp *body =
				(struct oz_set_interface_rsp *)usb_hdr;
			oz_hcd_control_cnf(usb_ctx->hport,
				body->req_id, body->rcode, NULL, getValueZero());
		}
		break;
	case OZ_VENDOR_CLASS_RSP: {
			struct oz_vendor_class_rsp *body =
				(struct oz_vendor_class_rsp *)usb_hdr;
			oz_hcd_control_cnf(usb_ctx->hport, body->req_id,
				body->rcode, body->data, elt->length-
				sizeof(struct oz_vendor_class_rsp)+getValue(1));
		}
		break;
	case OZ_USB_ENDPOINT_DATA:
		oz_usb_handle_ep_data(usb_ctx, usb_hdr, elt->length);
		break;
	}
done:
	oz_usb_put(usb_ctx);
}

void oz_usb_farewell(struct oz_pd *pd, u8 ep_num, u8 *data, u8 len)
{
	struct oz_usb_ctx *usb_ctx;

	spin_lock_bh(&pd->app_lock[OZ_APPID_USB]);
	usb_ctx = (struct oz_usb_ctx *)pd->app_ctx[OZ_APPID_USB];
	if (usb_ctx)
		oz_usb_get(usb_ctx);
	spin_unlock_bh(&pd->app_lock[OZ_APPID_USB]);
	if (usb_ctx == NULL)
		return; 
	if (!usb_ctx->stopped) {
		oz_dbg(ON, "Farewell indicated ep = 0x%x\n", ep_num);
		oz_hcd_data_ind(usb_ctx->hport, ep_num, data, len);
	}
	oz_usb_put(usb_ctx);
}