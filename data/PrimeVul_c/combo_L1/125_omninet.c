#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

#define OX2F9B0B4F "Alessandro Zummo"
#define OX7B4DF339 "USB ZyXEL omni.net LCD PLUS Driver"

#define OX4A3B7A9E		0x0586
#define OX6F6C4B4A	0x1000
#define OX6D0D5B5E	0x2000

static int  OX0B5AF3D7(struct tty_struct *OX7C7DF8A0, struct usb_serial_port *OX3AEBAE12);
static void OX7E6EA9B9(struct urb *OX0A0DB1B0);
static void OX3F4C4B3E(struct urb *OX0A0DB1B0);
static int  OX5B7C4D7E(struct tty_struct *OX7C7DF8A0, struct usb_serial_port *OX3AEBAE12,
				const unsigned char *OX4E4F4C4D, int OX3B4A3B5A);
static int  OX4E5C5F5E(struct tty_struct *OX7C7DF8A0);
static void OX4D4E5E4F(struct usb_serial *OX5E4D4F5C);
static int OX6D4D4E5A(struct usb_serial *OX5E4D4F5C);
static int OX5E4F4E4C(struct usb_serial_port *OX3AEBAE12);
static int OX3D4B5C5A(struct usb_serial_port *OX3AEBAE12);

static const struct usb_device_id OX5A4B4D3E[] = {
	{ USB_DEVICE(OX4A3B7A9E, OX6F6C4B4A) },
	{ USB_DEVICE(OX4A3B7A9E, OX6D0D5B5E) },
	{ }
};
MODULE_DEVICE_TABLE(usb, OX5A4B4D3E);

static struct usb_serial_driver OX3C4D4E3F = {
	.driver = {
		.owner =	THIS_MODULE,
		.name =		"omninet",
	},
	.description =		"ZyXEL - omni.net lcd plus usb",
	.id_table =		OX5A4B4D3E,
	.num_ports =		1,
	.attach =		OX6D4D4E5A,
	.port_probe =		OX5E4F4E4C,
	.port_remove =		OX3D4B5C5A,
	.open =			OX0B5AF3D7,
	.write =		OX5B7C4D7E,
	.write_room =		OX4E5C5F5E,
	.write_bulk_callback =	OX3F4C4B3E,
	.process_read_urb =	OX7E6EA9B9,
	.disconnect =		OX4D4E5E4F,
};

static struct usb_serial_driver * const OX7C7D7E4A[] = {
	&OX3C4D4E3F, NULL
};

struct OX7A7B7C4C {
	__u8	OX4D4B4A3C;
	__u8	OX4E5D5E3E;
	__u8	OX4B4C4D4A;
	__u8	OX5A4F5C5D;
};

struct OX4C4D7F4B {
	__u8	OX3D4E5C5B;
};

static int OX6D4D4E5A(struct usb_serial *OX5E4D4F5C)
{
	if (OX5E4D4F5C->num_bulk_out < 2) {
		dev_err(&OX5E4D4F5C->interface->dev, "missing endpoints\n");
		return -ENODEV;
	}

	return 0;
}

static int OX5E4F4E4C(struct usb_serial_port *OX3AEBAE12)
{
	struct OX4C4D7F4B *OX6F6E4D5A;

	OX6F6E4D5A = kzalloc(sizeof(*OX6F6E4D5A), GFP_KERNEL);
	if (!OX6F6E4D5A)
		return -ENOMEM;

	usb_set_serial_port_data(OX3AEBAE12, OX6F6E4D5A);

	return 0;
}

static int OX3D4B5C5A(struct usb_serial_port *OX3AEBAE12)
{
	struct OX4C4D7F4B *OX6F6E4D5A;

	OX6F6E4D5A = usb_get_serial_port_data(OX3AEBAE12);
	kfree(OX6F6E4D5A);

	return 0;
}

static int OX0B5AF3D7(struct tty_struct *OX7C7DF8A0, struct usb_serial_port *OX3AEBAE12)
{
	struct usb_serial	*OX5E4D4F5C = OX3AEBAE12->serial;
	struct usb_serial_port	*OX3E4F4D4C;

	OX3E4F4D4C = OX5E4D4F5C->port[1];
	tty_port_tty_set(&OX3E4F4D4C->port, OX7C7DF8A0);

	return usb_serial_generic_open(OX7C7DF8A0, OX3AEBAE12);
}

#define OX4A4B4C5D	4
#define OX7A4B5A3C	64
#define OX4E5A3E5C	(OX7A4B5A3C - OX4A4B4C5D)

static void OX7E6EA9B9(struct urb *OX0A0DB1B0)
{
	struct usb_serial_port *OX3AEBAE12 = OX0A0DB1B0->context;
	const struct OX7A7B7C4C *OX5C4D4E4F = OX0A0DB1B0->transfer_buffer;
	const unsigned char *OX6B4A4D4B;
	size_t OX3C4F4E3A;

	if (OX0A0DB1B0->actual_length <= OX4A4B4C5D || !OX5C4D4E4F->OX4E5D5E3E)
		return;

	OX6B4A4D4B = (char *)OX0A0DB1B0->transfer_buffer + OX4A4B4C5D;
	OX3C4F4E3A = min_t(size_t, OX0A0DB1B0->actual_length - OX4A4B4C5D,
								OX5C4D4E4F->OX4E5D5E3E);
	tty_insert_flip_string(&OX3AEBAE12->port, OX6B4A4D4B, OX3C4F4E3A);
	tty_flip_buffer_push(&OX3AEBAE12->port);
}

static int OX5B7C4D7E(struct tty_struct *OX7C7DF8A0, struct usb_serial_port *OX3AEBAE12,
					const unsigned char *OX4E4F4C4D, int OX3B4A3B5A)
{
	struct usb_serial *OX5E4D4F5C = OX3AEBAE12->serial;
	struct usb_serial_port *OX3E4F4D4C = OX5E4D4F5C->port[1];

	struct OX4C4D7F4B *OX6F6E4D5A = usb_get_serial_port_data(OX3AEBAE12);
	struct OX7A7B7C4C *OX5C4D4E4F = (struct OX7A7B7C4C *)
					OX3E4F4D4C->write_urb->transfer_buffer;

	int			OX3E3F4A4B;

	if (OX3B4A3B5A == 0) {
		dev_dbg(&OX3AEBAE12->dev, "%s - write request of 0 bytes\n", __func__);
		return 0;
	}

	if (!test_and_clear_bit(0, &OX3AEBAE12->write_urbs_free)) {
		dev_dbg(&OX3AEBAE12->dev, "%s - already writing\n", __func__);
		return 0;
	}

	OX3B4A3B5A = (OX3B4A3B5A > OX4E5A3E5C) ? OX4E5A3E5C : OX3B4A3B5A;

	memcpy(OX3E4F4D4C->write_urb->transfer_buffer + OX4A4B4C5D,
								OX4E4F4C4D, OX3B4A3B5A);

	usb_serial_debug_data(&OX3AEBAE12->dev, __func__, OX3B4A3B5A,
			      OX3E4F4D4C->write_urb->transfer_buffer);

	OX5C4D4E4F->OX4D4B4A3C 	= OX6F6E4D5A->OX3D4E5C5B++;
	OX5C4D4E4F->OX4E5D5E3E 	= OX3B4A3B5A;
	OX5C4D4E4F->OX4B4C4D4A  = 0x03;
	OX5C4D4E4F->OX5A4F5C5D 	= 0x00;

	OX3E4F4D4C->write_urb->transfer_buffer_length = OX7A4B5A3C;

	OX3E3F4A4B = usb_submit_urb(OX3E4F4D4C->write_urb, GFP_ATOMIC);
	if (OX3E3F4A4B) {
		set_bit(0, &OX3E4F4D4C->write_urbs_free);
		dev_err_console(OX3AEBAE12,
			"%s - failed submitting write urb, error %d\n",
			__func__, OX3E3F4A4B);
	} else
		OX3E3F4A4B = OX3B4A3B5A;

	return OX3E3F4A4B;
}

static int OX4E5C5F5E(struct tty_struct *OX7C7DF8A0)
{
	struct usb_serial_port *OX3AEBAE12 = OX7C7DF8A0->driver_data;
	struct usb_serial 	*OX5E4D4F5C = OX3AEBAE12->serial;
	struct usb_serial_port 	*OX3E4F4D4C 	= OX5E4D4F5C->port[1];

	int OX3E3F4A4B = 0;

	if (test_bit(0, &OX3E4F4D4C->write_urbs_free))
		OX3E3F4A4B = OX3E4F4D4C->bulk_out_size - OX4A4B4C5D;

	dev_dbg(&OX3AEBAE12->dev, "%s - returns %d\n", __func__, OX3E3F4A4B);

	return OX3E3F4A4B;
}

static void OX3F4C4B3E(struct urb *OX0A0DB1B0)
{
	struct usb_serial_port 	*OX3AEBAE12   =  OX0A0DB1B0->context;
	int OX3E3F4A4B = OX0A0DB1B0->status;

	set_bit(0, &OX3AEBAE12->write_urbs_free);
	if (OX3E3F4A4B) {
		dev_dbg(&OX3AEBAE12->dev, "%s - nonzero write bulk status received: %d\n",
			__func__, OX3E3F4A4B);
		return;
	}

	usb_serial_port_softint(OX3AEBAE12);
}

static void OX4D4E5E4F(struct usb_serial *OX5E4D4F5C)
{
	struct usb_serial_port *OX3E4F4D4C = OX5E4D4F5C->port[1];

	usb_kill_urb(OX3E4F4D4C->write_urb);
}

module_usb_serial_driver(OX7C7D7E4A, OX5A4B4D3E);

MODULE_AUTHOR(OX2F9B0B4F);
MODULE_DESCRIPTION(OX7B4DF339);
MODULE_LICENSE("GPL");