#include <linux/device.h>
#include <linux/hid.h>
#include <linux/input.h>
#include <linux/module.h>

#include "hid-ids.h"

#define CP_RDESC_SWAPPED_MIN_MAX	0x01
#define CP_2WHEEL_MOUSE_HACK		0x02
#define CP_2WHEEL_MOUSE_HACK_ON		0x04

static __u8 *cp_report_fixup(struct hid_device *hdev, __u8 *rdesc, unsigned int *rsize) {
	unsigned long quirks = (unsigned long)hid_get_drvdata(hdev);
	unsigned int i;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (!(quirks & CP_RDESC_SWAPPED_MIN_MAX)) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				return rdesc;
			case 2:
				i = 0;
				state = 3;
				break;
			case 3:
				if (i >= *rsize - 4) {
					state = 4;
					break;
				}
				if (rdesc[i] == 0x29 && rdesc[i + 2] == 0x19) {
					rdesc[i] = 0x19;
					rdesc[i + 2] = 0x29;
					swap(rdesc[i + 3], rdesc[i + 1]);
				}
				i++;
				break;
			case 4:
				return rdesc;
		}
	}
}

static int cp_input_mapped(struct hid_device *hdev, struct hid_input *hi, struct hid_field *field, struct hid_usage *usage, unsigned long **bit, int *max) {
	unsigned long quirks = (unsigned long)hid_get_drvdata(hdev);
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (!(quirks & CP_2WHEEL_MOUSE_HACK)) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				return 0;
			case 2:
				if (usage->type == EV_REL && usage->code == REL_WHEEL) {
					set_bit(REL_HWHEEL, *bit);
				}
				if (usage->hid == 0x00090005) {
					state = 3;
					break;
				}
				state = 1;
				break;
			case 3:
				return -1;
		}
	}
}

static int cp_event(struct hid_device *hdev, struct hid_field *field, struct hid_usage *usage, __s32 value) {
	unsigned long quirks = (unsigned long)hid_get_drvdata(hdev);
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				if (!(hdev->claimed & HID_CLAIMED_INPUT) || !field->hidinput || !usage->type || !(quirks & CP_2WHEEL_MOUSE_HACK)) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				return 0;
			case 2:
				if (usage->hid == 0x00090005) {
					state = 3;
					break;
				}
				state = 5;
				break;
			case 3:
				if (value) {
					quirks |= CP_2WHEEL_MOUSE_HACK_ON;
				} else {
					quirks &= ~CP_2WHEEL_MOUSE_HACK_ON;
				}
				hid_set_drvdata(hdev, (void *)quirks);
				state = 4;
				break;
			case 4:
				return 1;
			case 5:
				if (usage->code == REL_WHEEL && (quirks & CP_2WHEEL_MOUSE_HACK_ON)) {
					struct input_dev *input = field->hidinput->input;
					input_event(input, usage->type, REL_HWHEEL, value);
					state = 4;
					break;
				}
				state = 1;
				break;
		}
	}
}

static int cp_probe(struct hid_device *hdev, const struct hid_device_id *id) {
	unsigned long quirks = id->driver_data;
	int ret;
	int state = 0;

	while (1) {
		switch (state) {
			case 0:
				hid_set_drvdata(hdev, (void *)quirks);
				ret = hid_parse(hdev);
				if (ret) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				hid_err(hdev, "parse failed\n");
				state = 3;
				break;
			case 2:
				ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
				if (ret) {
					state = 4;
					break;
				}
				state = 5;
				break;
			case 3:
				return ret;
			case 4:
				hid_err(hdev, "hw start failed\n");
				state = 3;
				break;
			case 5:
				return 0;
		}
	}
}

static const struct hid_device_id cp_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_1), .driver_data = CP_RDESC_SWAPPED_MIN_MAX },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_2), .driver_data = CP_RDESC_SWAPPED_MIN_MAX },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_3), .driver_data = CP_RDESC_SWAPPED_MIN_MAX },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_4), .driver_data = CP_RDESC_SWAPPED_MIN_MAX },
	{ HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_MOUSE), .driver_data = CP_2WHEEL_MOUSE_HACK },
	{ }
};
MODULE_DEVICE_TABLE(hid, cp_devices);

static struct hid_driver cp_driver = {
	.name = "cypress",
	.id_table = cp_devices,
	.report_fixup = cp_report_fixup,
	.input_mapped = cp_input_mapped,
	.event = cp_event,
	.probe = cp_probe,
};
module_hid_driver(cp_driver);

MODULE_LICENSE("GPL");