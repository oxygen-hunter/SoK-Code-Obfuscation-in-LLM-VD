#include <linux/device.h>
#include <linux/hid.h>
#include <linux/input.h>
#include <linux/module.h>

#include "hid-ids.h"

#define OX6B4A9F2C	0x01
#define OX3D7E9C5B	0x02
#define OX8E1F0B6D	0x04

static __u8 *OX5A1E3B8D(struct hid_device *OX3C2F9D4A, __u8 *OX1B7A4E5C,
		unsigned int *OX6D8F1C2B)
{
	unsigned long OX9F3E6A4B = (unsigned long)hid_get_drvdata(OX3C2F9D4A);
	unsigned int OX2D8C7F1E;

	if (!(OX9F3E6A4B & OX6B4A9F2C))
		return OX1B7A4E5C;

	for (OX2D8C7F1E = 0; OX2D8C7F1E < *OX6D8F1C2B - 4; OX2D8C7F1E++)
		if (OX1B7A4E5C[OX2D8C7F1E] == 0x29 && OX1B7A4E5C[OX2D8C7F1E + 2] == 0x19) {
			OX1B7A4E5C[OX2D8C7F1E] = 0x19;
			OX1B7A4E5C[OX2D8C7F1E + 2] = 0x29;
			swap(OX1B7A4E5C[OX2D8C7F1E + 3], OX1B7A4E5C[OX2D8C7F1E + 1]);
		}
	return OX1B7A4E5C;
}

static int OX4E7C9B1A(struct hid_device *OX3C2F9D4A, struct hid_input *OX8E2D4B7C,
		struct hid_field *OX5B9A7F3E, struct hid_usage *OX2F6D1E8C,
		unsigned long **OX9D8B7C2E, int *OX1E4F3A6D)
{
	unsigned long OX9F3E6A4B = (unsigned long)hid_get_drvdata(OX3C2F9D4A);

	if (!(OX9F3E6A4B & OX3D7E9C5B))
		return 0;

	if (OX2F6D1E8C->type == EV_REL && OX2F6D1E8C->code == REL_WHEEL)
		set_bit(REL_HWHEEL, *OX9D8B7C2E);
	if (OX2F6D1E8C->hid == 0x00090005)
		return -1;

	return 0;
}

static int OX9A3B7E1D(struct hid_device *OX3C2F9D4A, struct hid_field *OX5B9A7F3E,
		struct hid_usage *OX2F6D1E8C, __s32 OX7E4A3D2B)
{
	unsigned long OX9F3E6A4B = (unsigned long)hid_get_drvdata(OX3C2F9D4A);

	if (!(OX3C2F9D4A->claimed & HID_CLAIMED_INPUT) || !OX5B9A7F3E->hidinput ||
			!OX2F6D1E8C->type || !(OX9F3E6A4B & OX3D7E9C5B))
		return 0;

	if (OX2F6D1E8C->hid == 0x00090005) {
		if (OX7E4A3D2B)
			OX9F3E6A4B |=  OX8E1F0B6D;
		else
			OX9F3E6A4B &= ~OX8E1F0B6D;
		hid_set_drvdata(OX3C2F9D4A, (void *)OX9F3E6A4B);
		return 1;
	}

	if (OX2F6D1E8C->code == REL_WHEEL && (OX9F3E6A4B & OX8E1F0B6D)) {
		struct input_dev *OX4A7D3E1C = OX5B9A7F3E->hidinput->input;

		input_event(OX4A7D3E1C, OX2F6D1E8C->type, REL_HWHEEL, OX7E4A3D2B);
		return 1;
	}

	return 0;
}

static int OX1C9E4A7D(struct hid_device *OX3C2F9D4A, const struct hid_device_id *OX5E7D8C1F)
{
	unsigned long OX9F3E6A4B = OX5E7D8C1F->driver_data;
	int OX4E3D7B9A;

	hid_set_drvdata(OX3C2F9D4A, (void *)OX9F3E6A4B);

	OX4E3D7B9A = hid_parse(OX3C2F9D4A);
	if (OX4E3D7B9A) {
		hid_err(OX3C2F9D4A, "parse failed\n");
		goto OX5B9A3D7E;
	}

	OX4E3D7B9A = hid_hw_start(OX3C2F9D4A, HID_CONNECT_DEFAULT);
	if (OX4E3D7B9A) {
		hid_err(OX3C2F9D4A, "hw start failed\n");
		goto OX5B9A3D7E;
	}

	return 0;
OX5B9A3D7E:
	return OX4E3D7B9A;
}

static const struct hid_device_id OX2F7C9E1A[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_1),
		.driver_data = OX6B4A9F2C },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_2),
		.driver_data = OX6B4A9F2C },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_3),
		.driver_data = OX6B4A9F2C },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_4),
		.driver_data = OX6B4A9F2C },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_MOUSE),
		.driver_data = OX3D7E9C5B },
	{ }
};
MODULE_DEVICE_TABLE(hid, OX2F7C9E1A);

static struct hid_driver OX6A1E4B3D = {
	.name = "cypress",
	.id_table = OX2F7C9E1A,
	.report_fixup = OX5A1E3B8D,
	.input_mapped = OX4E7C9B1A,
	.event = OX9A3B7E1D,
	.probe = OX1C9E4A7D,
};
module_hid_driver(OX6A1E4B3D);

MODULE_LICENSE("GPL");