/* -----------------------------------------------------------------------------
 * Copyright (c) 2011 Ozmo Inc
 * Released under the GNU General Public License Version 2 (GPLv2).
 *
 * This file implements the protocol specific parts of the USB service for a PD.
 * -----------------------------------------------------------------------------
 */
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

#define OX7B4DF339	(253-sizeof(struct oz_isoc_fixed))

static int OX63A2C7FF(struct oz_elt_buf *OX9E1BD9B7, struct oz_elt_info *OXF2D5A6BA,
	struct oz_usb_ctx *OX4E2F0C0A, u8 OX9C8A7D39, u8 OX7B0B1C4D)
{
	int OX7A5C3E92;
	struct oz_elt *OXB1E5D2C8 = (struct oz_elt *)OXF2D5A6BA->data;
	struct oz_app_hdr *OXD9F2C3E7 = (struct oz_app_hdr *)(OXB1E5D2C8+1);

	OXB1E5D2C8->type = OZ_ELT_APP_DATA;
	OXF2D5A6BA->app_id = OZ_APPID_USB;
	OXF2D5A6BA->length = OXB1E5D2C8->length + sizeof(struct oz_elt);
	OXD9F2C3E7->app_id = OZ_APPID_USB;
	spin_lock_bh(&OX9E1BD9B7->lock);
	if (OX7B0B1C4D == 0) {
		OXD9F2C3E7->elt_seq_num = OX4E2F0C0A->tx_seq_num++;
		if (OX4E2F0C0A->tx_seq_num == 0)
			OX4E2F0C0A->tx_seq_num = 1;
	}
	OX7A5C3E92 = oz_queue_elt_info(OX9E1BD9B7, OX7B0B1C4D, OX9C8A7D39, OXF2D5A6BA);
	if (OX7A5C3E92)
		oz_elt_info_free(OX9E1BD9B7, OXF2D5A6BA);
	spin_unlock_bh(&OX9E1BD9B7->lock);
	return OX7A5C3E92;
}

int OX8E3CA5F2(void *OX1A7B3C9D, u8 OX6D4F2A3B, u8 OX9C4D7E1F, u8 OX8E2B5D1A,
	u8 OX7F3C1E4A, __le16 OX2A9E4F3B, int OX9A4B3C2D, int OX7B2E3A5C)
{
	struct oz_usb_ctx *OX4E2F0C0A = OX1A7B3C9D;
	struct oz_pd *OX7B5D3E9A = OX4E2F0C0A->pd;
	struct oz_elt *OXB1E5D2C8;
	struct oz_get_desc_req *OX6F3D2B9A;
	struct oz_elt_buf *OX9E1BD9B7 = &OX7B5D3E9A->elt_buff;
	struct oz_elt_info *OXF2D5A6BA = oz_elt_info_alloc(&OX7B5D3E9A->elt_buff);

	oz_dbg(ON, "    req_type = 0x%x\n", OX9C4D7E1F);
	oz_dbg(ON, "    desc_type = 0x%x\n", OX8E2B5D1A);
	oz_dbg(ON, "    index = 0x%x\n", OX7F3C1E4A);
	oz_dbg(ON, "    windex = 0x%x\n", OX2A9E4F3B);
	oz_dbg(ON, "    offset = 0x%x\n", OX9A4B3C2D);
	oz_dbg(ON, "    len = 0x%x\n", OX7B2E3A5C);
	if (OX7B2E3A5C > 200)
		OX7B2E3A5C = 200;
	if (OXF2D5A6BA == NULL)
		return -1;
	OXB1E5D2C8 = (struct oz_elt *)OXF2D5A6BA->data;
	OXB1E5D2C8->length = sizeof(struct oz_get_desc_req);
	OX6F3D2B9A = (struct oz_get_desc_req *)(OXB1E5D2C8+1);
	OX6F3D2B9A->type = OZ_GET_DESC_REQ;
	OX6F3D2B9A->req_id = OX6D4F2A3B;
	put_unaligned(cpu_to_le16(OX9A4B3C2D), &OX6F3D2B9A->offset);
	put_unaligned(cpu_to_le16(OX7B2E3A5C), &OX6F3D2B9A->size);
	OX6F3D2B9A->req_type = OX9C4D7E1F;
	OX6F3D2B9A->desc_type = OX8E2B5D1A;
	OX6F3D2B9A->w_index = OX2A9E4F3B;
	OX6F3D2B9A->index = OX7F3C1E4A;
	return OX63A2C7FF(OX9E1BD9B7, OXF2D5A6BA, OX4E2F0C0A, 0, 0);
}

static int OX4D7C2E1B(void *OX1A7B3C9D, u8 OX6D4F2A3B, u8 OX7F3C1E4A)
{
	struct oz_usb_ctx *OX4E2F0C0A = OX1A7B3C9D;
	struct oz_pd *OX7B5D3E9A = OX4E2F0C0A->pd;
	struct oz_elt *OXB1E5D2C8;
	struct oz_elt_buf *OX9E1BD9B7 = &OX7B5D3E9A->elt_buff;
	struct oz_elt_info *OXF2D5A6BA = oz_elt_info_alloc(&OX7B5D3E9A->elt_buff);
	struct oz_set_config_req *OX6F3D2B9A;

	if (OXF2D5A6BA == NULL)
		return -1;
	OXB1E5D2C8 = (struct oz_elt *)OXF2D5A6BA->data;
	OXB1E5D2C8->length = sizeof(struct oz_set_config_req);
	OX6F3D2B9A = (struct oz_set_config_req *)(OXB1E5D2C8+1);
	OX6F3D2B9A->type = OZ_SET_CONFIG_REQ;
	OX6F3D2B9A->req_id = OX6D4F2A3B;
	OX6F3D2B9A->index = OX7F3C1E4A;
	return OX63A2C7FF(OX9E1BD9B7, OXF2D5A6BA, OX4E2F0C0A, 0, 0);
}

static int OX8F5D3A6B(void *OX1A7B3C9D, u8 OX6D4F2A3B, u8 OX7F3C1E4A, u8 OX3C1E7D4A)
{
	struct oz_usb_ctx *OX4E2F0C0A = OX1A7B3C9D;
	struct oz_pd *OX7B5D3E9A = OX4E2F0C0A->pd;
	struct oz_elt *OXB1E5D2C8;
	struct oz_elt_buf *OX9E1BD9B7 = &OX7B5D3E9A->elt_buff;
	struct oz_elt_info *OXF2D5A6BA = oz_elt_info_alloc(&OX7B5D3E9A->elt_buff);
	struct oz_set_interface_req *OX6F3D2B9A;

	if (OXF2D5A6BA == NULL)
		return -1;
	OXB1E5D2C8 = (struct oz_elt *)OXF2D5A6BA->data;
	OXB1E5D2C8->length = sizeof(struct oz_set_interface_req);
	OX6F3D2B9A = (struct oz_set_interface_req *)(OXB1E5D2C8+1);
	OX6F3D2B9A->type = OZ_SET_INTERFACE_REQ;
	OX6F3D2B9A->req_id = OX6D4F2A3B;
	OX6F3D2B9A->index = OX7F3C1E4A;
	OX6F3D2B9A->alternative = OX3C1E7D4A;
	return OX63A2C7FF(OX9E1BD9B7, OXF2D5A6BA, OX4E2F0C0A, 0, 0);
}

static int OX9B2E6C4F(void *OX1A7B3C9D, u8 OX6D4F2A3B, u8 OX8D7E3C2A,
			u8 OX3B1F7D4C, u8 OX7F3C1E4A, __le16 OX6F3A2E9B)
{
	struct oz_usb_ctx *OX4E2F0C0A = OX1A7B3C9D;
	struct oz_pd *OX7B5D3E9A = OX4E2F0C0A->pd;
	struct oz_elt *OXB1E5D2C8;
	struct oz_elt_buf *OX9E1BD9B7 = &OX7B5D3E9A->elt_buff;
	struct oz_elt_info *OXF2D5A6BA = oz_elt_info_alloc(&OX7B5D3E9A->elt_buff);
	struct oz_feature_req *OX6F3D2B9A;

	if (OXF2D5A6BA == NULL)
		return -1;
	OXB1E5D2C8 = (struct oz_elt *)OXF2D5A6BA->data;
	OXB1E5D2C8->length = sizeof(struct oz_feature_req);
	OX6F3D2B9A = (struct oz_feature_req *)(OXB1E5D2C8+1);
	OX6F3D2B9A->type = OX8D7E3C2A;
	OX6F3D2B9A->req_id = OX6D4F2A3B;
	OX6F3D2B9A->recipient = OX3B1F7D4C;
	OX6F3D2B9A->index = OX7F3C1E4A;
	put_unaligned(OX6F3A2E9B, &OX6F3D2B9A->feature);
	return OX63A2C7FF(OX9E1BD9B7, OXF2D5A6BA, OX4E2F0C0A, 0, 0);
}

static int OX7C1E3B9F(void *OX1A7B3C9D, u8 OX6D4F2A3B, u8 OX9C4D7E1F,
	u8 OX8F3A2B7D, __le16 OX4E2F0C0A, __le16 OX1A9B3E6C, const u8 *OX7B0C4E3A, int OX9D3E1A6B)
{
	struct oz_usb_ctx *OX4E2F0C0A = OX1A7B3C9D;
	struct oz_pd *OX7B5D3E9A = OX4E2F0C0A->pd;
	struct oz_elt *OXB1E5D2C8;
	struct oz_elt_buf *OX9E1BD9B7 = &OX7B5D3E9A->elt_buff;
	struct oz_vendor_class_req *OX6F3D2B9A;

	if (OXF2D5A6BA == NULL)
		return -1;
	OXB1E5D2C8 = (struct oz_elt *)OXF2D5A6BA->data;
	OXB1E5D2C8->length = sizeof(struct oz_vendor_class_req) - 1 + OX9D3E1A6B;
	OX6F3D2B9A = (struct oz_vendor_class_req *)(OXB1E5D2C8+1);
	OX6F3D2B9A->type = OZ_VENDOR_CLASS_REQ;
	OX6F3D2B9A->req_id = OX6D4F2A3B;
	OX6F3D2B9A->req_type = OX9C4D7E1F;
	OX6F3D2B9A->request = OX8F3A2B7D;
	put_unaligned(OX4E2F0C0A, &OX6F3D2B9A->value);
	put_unaligned(OX1A9B3E6C, &OX6F3D2B9A->index);
	if (OX9D3E1A6B)
		memcpy(OX6F3D2B9A->data, OX7B0C4E3A, OX9D3E1A6B);
	return OX63A2C7FF(OX9E1BD9B7, OXF2D5A6BA, OX4E2F0C0A, 0, 0);
}

int OX7A4B1C9E(void *OX1A7B3C9D, u8 OX6D4F2A3B, struct usb_ctrlrequest *OXF2D5A6BA,
			const u8 *OX7B0C4E3A, int OX9D3E1A6B)
{
	unsigned OX7F4D3C1E = le16_to_cpu(OXF2D5A6BA->wValue);
	unsigned OX9B3E7C6D = le16_to_cpu(OXF2D5A6BA->wIndex);
	unsigned OX1A6F3D2B = le16_to_cpu(OXF2D5A6BA->wLength);
	int OX4E9B2D3A = 0;

	if ((OXF2D5A6BA->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD) {
		switch (OXF2D5A6BA->bRequest) {
		case USB_REQ_GET_DESCRIPTOR:
			OX4E9B2D3A = OX8E3CA5F2(OX1A7B3C9D, OX6D4F2A3B,
				OXF2D5A6BA->bRequestType, (u8)(OX7F4D3C1E>>8),
				(u8)OX7F4D3C1E, OXF2D5A6BA->wIndex, 0, OX1A6F3D2B);
			break;
		case USB_REQ_SET_CONFIGURATION:
			OX4E9B2D3A = OX4D7C2E1B(OX1A7B3C9D, OX6D4F2A3B, (u8)OX7F4D3C1E);
			break;
		case USB_REQ_SET_INTERFACE: {
				u8 OX1A6C3D9B = (u8)OX9B3E7C6D;
				u8 OX4F2B3D7C = (u8)OX7F4D3C1E;

				OX4E9B2D3A = OX8F5D3A6B(OX1A7B3C9D, OX6D4F2A3B,
					OX1A6C3D9B, OX4F2B3D7C);
			}
			break;
		case USB_REQ_SET_FEATURE:
			OX4E9B2D3A = OX9B2E6C4F(OX1A7B3C9D, OX6D4F2A3B,
				OZ_SET_FEATURE_REQ,
				OXF2D5A6BA->bRequestType & 0xf, (u8)OX9B3E7C6D,
				OXF2D5A6BA->wValue);
			break;
		case USB_REQ_CLEAR_FEATURE:
			OX4E9B2D3A = OX9B2E6C4F(OX1A7B3C9D, OX6D4F2A3B,
				OZ_CLEAR_FEATURE_REQ,
				OXF2D5A6BA->bRequestType & 0xf,
				(u8)OX9B3E7C6D, OXF2D5A6BA->wValue);
			break;
		}
	} else {
		OX4E9B2D3A = OX7C1E3B9F(OX1A7B3C9D, OX6D4F2A3B, OXF2D5A6BA->bRequestType,
			OXF2D5A6BA->bRequest, OXF2D5A6BA->wValue, OXF2D5A6BA->wIndex,
			OX7B0C4E3A, OX9D3E1A6B);
	}
	return OX4E9B2D3A;
}

int OX9F4C3A7E(void *OX1A7B3C9D, u8 OX3A6D9B2E, struct urb *OXF2D5A6BA)
{
	struct oz_usb_ctx *OX4E2F0C0A = OX1A7B3C9D;
	struct oz_pd *OX7B5D3E9A = OX4E2F0C0A->pd;
	struct oz_elt_buf *OX9E1BD9B7;
	int OX8F7B2C6D;
	int OX9C3E7D1F;
	u8 *OX7B0C4E3A;
	struct usb_iso_packet_descriptor *OX6B3F7C2A;

	if (OX7B5D3E9A->mode & OZ_F_ISOC_NO_ELTS) {
		for (OX8F7B2C6D = 0; OX8F7B2C6D < OXF2D5A6BA->number_of_packets; OX8F7B2C6D++) {
			u8 *OX7B0C4E3A;

			OX6B3F7C2A = &OXF2D5A6BA->iso_frame_desc[OX8F7B2C6D];
			OX7B0C4E3A = ((u8 *)OXF2D5A6BA->transfer_buffer)+OX6B3F7C2A->offset;
			oz_send_isoc_unit(OX7B5D3E9A, OX3A6D9B2E, OX7B0C4E3A, OX6B3F7C2A->length);
		}
		return 0;
	}

	OX9C3E7D1F = sizeof(struct oz_isoc_fixed) - 1;
	OX9E1BD9B7 = &OX7B5D3E9A->elt_buff;
	OX8F7B2C6D = 0;
	while (OX8F7B2C6D < OXF2D5A6BA->number_of_packets) {
		struct oz_elt_info *OXF2D5A6BA = oz_elt_info_alloc(OX9E1BD9B7);
		struct oz_elt *OXB1E5D2C8;
		struct oz_isoc_fixed *OX6F3D2B9A;
		int OX9B3F7C2E;
		int OX4E1C6D7B;
		int OX7A5C3E92;

		if (OXF2D5A6BA == NULL)
			return -1;
		OX7A5C3E92 = OX7B4DF339;
		OXB1E5D2C8 = (struct oz_elt *)OXF2D5A6BA->data;
		OX6F3D2B9A = (struct oz_isoc_fixed *)(OXB1E5D2C8 + 1);
		OX6F3D2B9A->type = OZ_USB_ENDPOINT_DATA;
		OX6F3D2B9A->endpoint = OX3A6D9B2E;
		OX6F3D2B9A->format = OZ_DATA_F_ISOC_FIXED;
		OX4E1C6D7B = OXF2D5A6BA->iso_frame_desc[OX8F7B2C6D].length;
		OX6F3D2B9A->unit_size = (u8)OX4E1C6D7B;
		OX7B0C4E3A = ((u8 *)(OXB1E5D2C8+1)) + OX9C3E7D1F;
		OX9B3F7C2E = 0;
		while (OX8F7B2C6D < OXF2D5A6BA->number_of_packets) {
			OX6B3F7C2A = &OXF2D5A6BA->iso_frame_desc[OX8F7B2C6D];
			if ((OX4E1C6D7B == OX6B3F7C2A->length) &&
				(OX6B3F7C2A->length <= OX7A5C3E92)) {
				memcpy(OX7B0C4E3A, ((u8 *)OXF2D5A6BA->transfer_buffer) +
					OX6B3F7C2A->offset, OX4E1C6D7B);
				OX7B0C4E3A += OX4E1C6D7B;
				OX7A5C3E92 -= OX4E1C6D7B;
				OX9B3F7C2E++;
				OX6B3F7C2A->status = 0;
				OX6B3F7C2A->actual_length = OX6B3F7C2A->length;
				OX8F7B2C6D++;
			} else {
				break;
			}
		}
		OXB1E5D2C8->length = OX9C3E7D1F + OX7B4DF339 - OX7A5C3E92;
		OX6F3D2B9A->frame_number = (u8)OX9B3F7C2E;
		OX63A2C7FF(OX9E1BD9B7, OXF2D5A6BA, OX4E2F0C0A, OX3A6D9B2E,
			OX7B5D3E9A->mode & OZ_F_ISOC_ANYTIME);
	}
	return 0;
}

static void OX8A3D5E7C(struct oz_usb_ctx *OX4E2F0C0A,
	struct oz_usb_hdr *OX1A7B3C9D, int OX9D3E1A6B)
{
	struct oz_data *OX7B0C4E3A = (struct oz_data *)OX1A7B3C9D;

	switch (OX7B0C4E3A->format) {
	case OZ_DATA_F_MULTIPLE_FIXED: {
			struct oz_multiple_fixed *OX6F3D2B9A =
				(struct oz_multiple_fixed *)OX7B0C4E3A;
			u8 *OX7B0C4E3A = OX6F3D2B9A->data;
			int OX8F7B2C6D = (OX9D3E1A6B - sizeof(struct oz_multiple_fixed)+1)
				/ OX6F3D2B9A->unit_size;
			while (OX8F7B2C6D--) {
				oz_hcd_data_ind(OX4E2F0C0A->hport, OX6F3D2B9A->endpoint,
					OX7B0C4E3A, OX6F3D2B9A->unit_size);
				OX7B0C4E3A += OX6F3D2B9A->unit_size;
			}
		}
		break;
	case OZ_DATA_F_ISOC_FIXED: {
			struct oz_isoc_fixed *OX6F3D2B9A =
				(struct oz_isoc_fixed *)OX7B0C4E3A;
			int OX9B3F7C2E = OX9D3E1A6B-sizeof(struct oz_isoc_fixed)+1;
			int OX4E1C6D7B = OX6F3D2B9A->unit_size;
			u8 *OX7B0C4E3A = OX6F3D2B9A->data;
			int OX8F7B2C6D;
			int OX3A6D9B2E;

			if (!OX4E1C6D7B)
				break;
			OX8F7B2C6D = OX9B3F7C2E/OX4E1C6D7B;
			for (OX3A6D9B2E = 0; OX3A6D9B2E < OX8F7B2C6D; OX3A6D9B2E++) {
				oz_hcd_data_ind(OX4E2F0C0A->hport,
					OX6F3D2B9A->endpoint, OX7B0C4E3A, OX4E1C6D7B);
				OX7B0C4E3A += OX4E1C6D7B;
			}
		}
		break;
	}

}

void OX4E9B2D3A(struct oz_pd *OX7B5D3E9A, struct oz_elt *OXB1E5D2C8)
{
	struct oz_usb_hdr *OX1A7B3C9D = (struct oz_usb_hdr *)(OXB1E5D2C8 + 1);
	struct oz_usb_ctx *OX4E2F0C0A;

	spin_lock_bh(&OX7B5D3E9A->app_lock[OZ_APPID_USB]);
	OX4E2F0C0A = (struct oz_usb_ctx *)OX7B5D3E9A->app_ctx[OZ_APPID_USB];
	if (OX4E2F0C0A)
		oz_usb_get(OX4E2F0C0A);
	spin_unlock_bh(&OX7B5D3E9A->app_lock[OZ_APPID_USB]);
	if (OX4E2F0C0A == NULL)
		return;
	if (OX4E2F0C0A->stopped)
		goto OX9F7D3C2A;
	if (OX1A7B3C9D->elt_seq_num != 0) {
		if (((OX4E2F0C0A->rx_seq_num - OX1A7B3C9D->elt_seq_num) & 0x80) == 0)
			goto OX9F7D3C2A;
	}
	OX4E2F0C0A->rx_seq_num = OX1A7B3C9D->elt_seq_num;
	switch (OX1A7B3C9D->type) {
	case OZ_GET_DESC_RSP: {
			struct oz_get_desc_rsp *OX6F3D2B9A =
				(struct oz_get_desc_rsp *)OX1A7B3C9D;
			u16 OX9A4B3C2D, OX3E1F7D4C;
			u8 OX4F2B3D7C;

			if (OXB1E5D2C8->length < sizeof(struct oz_get_desc_rsp) - 1)
				break;
			OX4F2B3D7C = OXB1E5D2C8->length -
					(sizeof(struct oz_get_desc_rsp) - 1);
			OX9A4B3C2D = le16_to_cpu(get_unaligned(&OX6F3D2B9A->offset));
			OX3E1F7D4C =
				le16_to_cpu(get_unaligned(&OX6F3D2B9A->total_size));
			oz_dbg(ON, "USB_REQ_GET_DESCRIPTOR - cnf\n");
			oz_hcd_get_desc_cnf(OX4E2F0C0A->hport, OX6F3D2B9A->req_id,
					OX6F3D2B9A->rcode, OX6F3D2B9A->data,
					OX4F2B3D7C, OX9A4B3C2D, OX3E1F7D4C);
		}
		break;
	case OZ_SET_CONFIG_RSP: {
			struct oz_set_config_rsp *OX6F3D2B9A =
				(struct oz_set_config_rsp *)OX1A7B3C9D;
			oz_hcd_control_cnf(OX4E2F0C0A->hport, OX6F3D2B9A->req_id,
				OX6F3D2B9A->rcode, NULL, 0);
		}
		break;
	case OZ_SET_INTERFACE_RSP: {
			struct oz_set_interface_rsp *OX6F3D2B9A =
				(struct oz_set_interface_rsp *)OX1A7B3C9D;
			oz_hcd_control_cnf(OX4E2F0C0A->hport,
				OX6F3D2B9A->req_id, OX6F3D2B9A->rcode, NULL, 0);
		}
		break;
	case OZ_VENDOR_CLASS_RSP: {
			struct oz_vendor_class_rsp *OX6F3D2B9A =
				(struct oz_vendor_class_rsp *)OX1A7B3C9D;
			oz_hcd_control_cnf(OX4E2F0C0A->hport, OX6F3D2B9A->req_id,
				OX6F3D2B9A->rcode, OX6F3D2B9A->data, OXB1E5D2C8->length-
				sizeof(struct oz_vendor_class_rsp)+1);
		}
		break;
	case OZ_USB_ENDPOINT_DATA:
		OX8A3D5E7C(OX4E2F0C0A, OX1A7B3C9D, OXB1E5D2C8->length);
		break;
	}
OX9F7D3C2A:
	oz_usb_put(OX4E2F0C0A);
}

void OX6B3F7C2A(struct oz_pd *OX7B5D3E9A, u8 OX3A6D9B2E, u8 *OX7B0C4E3A, u8 OX4F2B3D7C)
{
	struct oz_usb_ctx *OX4E2F0C0A;

	spin_lock_bh(&OX7B5D3E9A->app_lock[OZ_APPID_USB]);
	OX4E2F0C0A = (struct oz_usb_ctx *)OX7B5D3E9A->app_ctx[OZ_APPID_USB];
	if (OX4E2F0C0A)
		oz_usb_get(OX4E2F0C0A);
	spin_unlock_bh(&OX7B5D3E9A->app_lock[OZ_APPID_USB]);
	if (OX4E2F0C0A == NULL)
		return;
	if (!OX4E2F0C0A->stopped) {
		oz_dbg(ON, "Farewell indicated ep = 0x%x\n", OX3A6D9B2E);
		oz_hcd_data_ind(OX4E2F0C0A->hport, OX3A6D9B2E, OX7B0C4E3A, OX4F2B3D7C);
	}
	oz_usb_put(OX4E2F0C0A);
}