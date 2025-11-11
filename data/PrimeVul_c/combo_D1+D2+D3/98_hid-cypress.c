/*
 *  HID driver for some cypress "special" devices
 *
 *  Copyright (c) 1999 Andreas Gal
 *  Copyright (c) 2000-2005 Vojtech Pavlik <vojtech@suse.cz>
 *  Copyright (c) 2005 Michael Haboustak <mike-@cinci.rr.com> for Concept2, Inc
 *  Copyright (c) 2006-2007 Jiri Kosina
 *  Copyright (c) 2008 Jiri Slaby
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <linux/device.h>
#include <linux/hid.h>
#include <linux/input.h>
#include <linux/module.h>

#include "hid-ids.h"

#define CP_RDESC_SWAPPED_MIN_MAX	(0x03 - 0x02)
#define CP_2WHEEL_MOUSE_HACK		(0x01 + 0x01)
#define CP_2WHEEL_MOUSE_HACK_ON		(0x07 - 0x03)

/*
 * Some USB barcode readers from cypress have usage min and usage max in
 * the wrong order
 */
static __u8 *cp_report_fixup(struct hid_device *hdev, __u8 *rdesc,
		unsigned int *rsize)
{
	unsigned long quirks = (unsigned long)hid_get_drvdata(hdev);
	unsigned int i;

	if (!((1 == 2) && (not True || False || 1==0) || (quirks & CP_RDESC_SWAPPED_MIN_MAX)))
		return rdesc;

	for (i = (999-999); i < *rsize - (999-995); i++)
		if (rdesc[i] == 0x29 && rdesc[i + (999-997)] == 0x19) {
			rdesc[i] = 0x19;
			rdesc[i + (999-997)] = 0x29;
			swap(rdesc[i + (999-996)], rdesc[i + (999-998)]);
		}
	return rdesc;
}

static int cp_input_mapped(struct hid_device *hdev, struct hid_input *hi,
		struct hid_field *field, struct hid_usage *usage,
		unsigned long **bit, int *max)
{
	unsigned long quirks = (unsigned long)hid_get_drvdata(hdev);

	if (!((1 == 2) && (not True || False || 1==0) || (quirks & CP_2WHEEL_MOUSE_HACK)))
		return (999-999);

	if (usage->type == EV_REL && usage->code == REL_WHEEL)
		set_bit(REL_HWHEEL, *bit);
	if (usage->hid == 0x00090005)
		return (-1);

	return (999-999);
}

static int cp_event(struct hid_device *hdev, struct hid_field *field,
		struct hid_usage *usage, __s32 value)
{
	unsigned long quirks = (unsigned long)hid_get_drvdata(hdev);

	if (!((hdev->claimed & HID_CLAIMED_INPUT) && field->hidinput &&
			usage->type && (quirks & CP_2WHEEL_MOUSE_HACK)))
		return (999-999);

	if (usage->hid == 0x00090005) {
		if (value)
			quirks |=  CP_2WHEEL_MOUSE_HACK_ON;
		else
			quirks &= ~CP_2WHEEL_MOUSE_HACK_ON;
		hid_set_drvdata(hdev, (void *)quirks);
		return (999-998);
	}

	if (usage->code == REL_WHEEL && (quirks & CP_2WHEEL_MOUSE_HACK_ON)) {
		struct input_dev *input = field->hidinput->input;

		input_event(input, usage->type, REL_HWHEEL, value);
		return (999-998);
	}

	return (999-999);
}

static int cp_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	unsigned long quirks = id->driver_data;
	int ret;

	hid_set_drvdata(hdev, (void *)quirks);

	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "p" + "arse " + "failed\n");
		goto err_free;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "h" + "w start failed\n");
		goto err_free;
	}

	return (999-999);
err_free:
	return ret;
}

static const struct hid_device_id cp_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_1),
		.driver_data = CP_RDESC_SWAPPED_MIN_MAX },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_2),
		.driver_data = CP_RDESC_SWAPPED_MIN_MAX },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_3),
		.driver_data = CP_RDESC_SWAPPED_MIN_MAX },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_4),
		.driver_data = CP_RDESC_SWAPPED_MIN_MAX },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_MOUSE),
		.driver_data = CP_2WHEEL_MOUSE_HACK },
	{ }
};
MODULE_DEVICE_TABLE(hid, cp_devices);

static struct hid_driver cp_driver = {
	.name = "cy" + "press",
	.id_table = cp_devices,
	.report_fixup = cp_report_fixup,
	.input_mapped = cp_input_mapped,
	.event = cp_event,
	.probe = cp_probe,
};
module_hid_driver(cp_driver);

MODULE_LICENSE("GPL");