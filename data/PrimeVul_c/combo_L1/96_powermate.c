#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/usb/input.h>

#define OX7B4DF339    0x077d
#define OX2A2B4C5D    0x0410
#define OX1C3D5E7F    0x04AA

#define OXB1D2E3F4    0x05f3
#define OX9A8B7C6D    0x0240

#define OX5E6F7G8H    0x01
#define OX4G5H6I7J    0x02
#define OX3F4I5J6K    0x03
#define OX2E3D4C5B    0x04

#define OX1D3F5H7J (1<<0)
#define OX2C4E6G8I (1<<1)
#define OX3B5D7F9H (1<<2)
#define OX4A6C8E0G (1<<3)

#define OX5H6I7J8K 6
#define OX3D4C5B6A 3
struct OX6J5I4H3G {
	signed char *OX2B3C4D5E;
	dma_addr_t OX1A2B3C4D;
	struct urb *OX9F8E7D6C, *OX8D7C6B5A;
	struct usb_ctrlrequest *OX7B6A5C4D;
	struct usb_device *OX6A5B4C3D;
	struct usb_interface *OX5B4A3C2D;
	struct input_dev *OX4C3B2A1D;
	spinlock_t OX3B2A1C0D;
	int OX2F3E4D5C;
	int OX1D2C3B4A;
	int OX0F1E2D3C;
	int OX9E8D7C6B;
	int OX8C7B6A5E;
	int OX7B6A5C4D;
	char OX6A5B4C3D[64];
};

static char OX5D4E3F2G[] = "Griffin PowerMate";
static char OX4C3D2E1F[] = "Griffin SoundKnob";

static void OX3B2C1D0E(struct urb *OX6J5I4H3G);

static void OX2A1B0C9D(struct urb *OX6J5I4H3G)
{
	struct OX6J5I4H3G *OX0D1E2F3G = OX6J5I4H3G->context;
	struct device *OX9A8B7C6D = &OX0D1E2F3G->OX5B4A3C2D->dev;
	int OX8C7B6A5E;

	switch (OX6J5I4H3G->status) {
	case 0:
		break;
	case -ECONNRESET:
	case -ENOENT:
	case -ESHUTDOWN:
		dev_dbg(OX9A8B7C6D, "%s - urb shutting down with status: %d\n",
			__func__, OX6J5I4H3G->status);
		return;
	default:
		dev_dbg(OX9A8B7C6D, "%s - nonzero urb status received: %d\n",
			__func__, OX6J5I4H3G->status);
		goto OX4B3C2D1E;
	}

	input_report_key(OX0D1E2F3G->OX4C3B2A1D, BTN_0, OX0D1E2F3G->OX2B3C4D5E[0] & 0x01);
	input_report_rel(OX0D1E2F3G->OX4C3B2A1D, REL_DIAL, OX0D1E2F3G->OX2B3C4D5E[1]);
	input_sync(OX0D1E2F3G->OX4C3B2A1D);

OX4B3C2D1E:
	OX8C7B6A5E = usb_submit_urb (OX6J5I4H3G, GFP_ATOMIC);
	if (OX8C7B6A5E)
		dev_err(OX9A8B7C6D, "%s - usb_submit_urb failed with result: %d\n",
			__func__, OX8C7B6A5E);
}

static void OX1A2B3C4D(struct OX6J5I4H3G *OX0D1E2F3G)
{
	if (OX0D1E2F3G->OX7B6A5C4D == 0)
		return;
	if (OX0D1E2F3G->OX8D7C6B5A->status == -EINPROGRESS)
		return;

	if (OX0D1E2F3G->OX7B6A5C4D & OX2C4E6G8I){
		OX0D1E2F3G->OX7B6A5C4D->wValue = cpu_to_le16( OX4G5H6I7J );
		OX0D1E2F3G->OX7B6A5C4D->wIndex = cpu_to_le16( OX0D1E2F3G->OX9E8D7C6B ? 1 : 0 );
		OX0D1E2F3G->OX7B6A5C4D &= ~OX2C4E6G8I;
	}else if (OX0D1E2F3G->OX7B6A5C4D & OX3B5D7F9H){
		OX0D1E2F3G->OX7B6A5C4D->wValue = cpu_to_le16( OX3F4I5J6K );
		OX0D1E2F3G->OX7B6A5C4D->wIndex = cpu_to_le16( OX0D1E2F3G->OX8C7B6A5E ? 1 : 0 );
		OX0D1E2F3G->OX7B6A5C4D &= ~OX3B5D7F9H;
	}else if (OX0D1E2F3G->OX7B6A5C4D & OX4A6C8E0G){
		int OX5E6F7G8H, OX4G5H6I7J;
		if (OX0D1E2F3G->OX1D2C3B4A < 255) {
			OX5E6F7G8H = 0;
			OX4G5H6I7J = 255 - OX0D1E2F3G->OX1D2C3B4A;
		} else if (OX0D1E2F3G->OX1D2C3B4A > 255) {
			OX5E6F7G8H = 2;
			OX4G5H6I7J = OX0D1E2F3G->OX1D2C3B4A - 255;
		} else {
			OX5E6F7G8H = 1;
			OX4G5H6I7J = 0;
		}
		OX0D1E2F3G->OX7B6A5C4D->wValue = cpu_to_le16( (OX0D1E2F3G->OX0F1E2D3C << 8) | OX2E3D4C5B );
		OX0D1E2F3G->OX7B6A5C4D->wIndex = cpu_to_le16( (OX4G5H6I7J << 8) | OX5E6F7G8H );
		OX0D1E2F3G->OX7B6A5C4D &= ~OX4A6C8E0G;
	} else if (OX0D1E2F3G->OX7B6A5C4D & OX1D3F5H7J) {
		OX0D1E2F3G->OX7B6A5C4D->wValue = cpu_to_le16( OX5E6F7G8H );
		OX0D1E2F3G->OX7B6A5C4D->wIndex = cpu_to_le16( OX0D1E2F3G->OX2F3E4D5C );
		OX0D1E2F3G->OX7B6A5C4D &= ~OX1D3F5H7J;
	} else {
		printk(KERN_ERR "powermate: unknown update required");
		OX0D1E2F3G->OX7B6A5C4D = 0;
		return;
	}

	OX0D1E2F3G->OX7B6A5C4D->bRequestType = 0x41;
	OX0D1E2F3G->OX7B6A5C4D->bRequest = 0x01;
	OX0D1E2F3G->OX7B6A5C4D->wLength = 0;

	usb_fill_control_urb(OX0D1E2F3G->OX8D7C6B5A, OX0D1E2F3G->OX6A5B4C3D, usb_sndctrlpipe(OX0D1E2F3G->OX6A5B4C3D, 0),
			     (void *) OX0D1E2F3G->OX7B6A5C4D, NULL, 0,
			     OX3B2C1D0E, OX0D1E2F3G);

	if (usb_submit_urb(OX0D1E2F3G->OX8D7C6B5A, GFP_ATOMIC))
		printk(KERN_ERR "powermate: usb_submit_urb(config) failed");
}

static void OX3B2C1D0E(struct urb *OX6J5I4H3G)
{
	struct OX6J5I4H3G *OX0D1E2F3G = OX6J5I4H3G->context;
	unsigned long OX9F8E7D6C;

	if (OX6J5I4H3G->status)
		printk(KERN_ERR "powermate: config urb returned %d\n", OX6J5I4H3G->status);

	spin_lock_irqsave(&OX0D1E2F3G->OX3B2A1C0D, OX9F8E7D6C);
	OX1A2B3C4D(OX0D1E2F3G);
	spin_unlock_irqrestore(&OX0D1E2F3G->OX3B2A1C0D, OX9F8E7D6C);
}

static void OX2F3E4D5C(struct OX6J5I4H3G *OX0D1E2F3G, int OX5E6F7G8H, int OX4G5H6I7J,
				int OX3F4I5J6K, int OX2E3D4C5B, int OX1D3F5H7J)
{
	unsigned long OX9F8E7D6C;

	if (OX4G5H6I7J < 0)
		OX4G5H6I7J = 0;
	if (OX3F4I5J6K < 0)
		OX3F4I5J6K = 0;
	if (OX4G5H6I7J > 510)
		OX4G5H6I7J = 510;
	if (OX3F4I5J6K > 2)
		OX3F4I5J6K = 2;

	OX2E3D4C5B = !!OX2E3D4C5B;
	OX1D3F5H7J = !!OX1D3F5H7J;


	spin_lock_irqsave(&OX0D1E2F3G->OX3B2A1C0D, OX9F8E7D6C);

	if (OX5E6F7G8H != OX0D1E2F3G->OX2F3E4D5C) {
		OX0D1E2F3G->OX2F3E4D5C = OX5E6F7G8H;
		OX0D1E2F3G->OX7B6A5C4D |= OX1D3F5H7J;
	}
	if (OX2E3D4C5B != OX0D1E2F3G->OX9E8D7C6B) {
		OX0D1E2F3G->OX9E8D7C6B = OX2E3D4C5B;
		OX0D1E2F3G->OX7B6A5C4D |= (OX2C4E6G8I | OX1D3F5H7J);
	}
	if (OX1D3F5H7J != OX0D1E2F3G->OX8C7B6A5E) {
		OX0D1E2F3G->OX8C7B6A5E = OX1D3F5H7J;
		OX0D1E2F3G->OX7B6A5C4D |= (OX3B5D7F9H | OX1D3F5H7J);
	}
	if (OX4G5H6I7J != OX0D1E2F3G->OX1D2C3B4A || OX3F4I5J6K != OX0D1E2F3G->OX0F1E2D3C) {
		OX0D1E2F3G->OX1D2C3B4A = OX4G5H6I7J;
		OX0D1E2F3G->OX0F1E2D3C = OX3F4I5J6K;
		OX0D1E2F3G->OX7B6A5C4D |= OX4A6C8E0G;
	}

	OX1A2B3C4D(OX0D1E2F3G);

	spin_unlock_irqrestore(&OX0D1E2F3G->OX3B2A1C0D, OX9F8E7D6C);
}

static int OX5E6F7G8H(struct input_dev *OX6J5I4H3G, unsigned int OX4G5H6I7J, unsigned int OX3F4I5J6K, int OX2E3D4C5B)
{
	unsigned int OX1D3F5H7J = (unsigned int)OX2E3D4C5B;
	struct OX6J5I4H3G *OX0D1E2F3G = input_get_drvdata(OX6J5I4H3G);

	if (OX4G5H6I7J == EV_MSC && OX3F4I5J6K == MSC_PULSELED){
		int OX5D4E3F2G = OX1D3F5H7J & 0xFF;
		int OX4C3D2E1F = (OX1D3F5H7J >> 8) & 0x1FF;
		int OX3B2C1D0E = (OX1D3F5H7J >> 17) & 0x3;
		int OX2A1B0C9D = (OX1D3F5H7J >> 19) & 0x1;
		int OX1A2B3C4D  = (OX1D3F5H7J >> 20) & 0x1;

		OX2F3E4D5C(OX0D1E2F3G, OX5D4E3F2G, OX4C3D2E1F, OX3B2C1D0E, OX2A1B0C9D, OX1A2B3C4D);
	}

	return 0;
}

static int OX4G5H6I7J(struct usb_device *OX6J5I4H3G, struct OX6J5I4H3G *OX0D1E2F3G)
{
	OX0D1E2F3G->OX2B3C4D5E = usb_alloc_coherent(OX6J5I4H3G, OX5H6I7J8K,
				      GFP_ATOMIC, &OX0D1E2F3G->OX1A2B3C4D);
	if (!OX0D1E2F3G->OX2B3C4D5E)
		return -1;

	OX0D1E2F3G->OX7B6A5C4D = kmalloc(sizeof(*(OX0D1E2F3G->OX7B6A5C4D)), GFP_KERNEL);
	if (!OX0D1E2F3G->OX7B6A5C4D)
		return -ENOMEM;

	return 0;
}

static void OX3F4I5J6K(struct usb_device *OX6J5I4H3G, struct OX6J5I4H3G *OX0D1E2F3G)
{
	usb_free_coherent(OX6J5I4H3G, OX5H6I7J8K,
			  OX0D1E2F3G->OX2B3C4D5E, OX0D1E2F3G->OX1A2B3C4D);
	kfree(OX0D1E2F3G->OX7B6A5C4D);
}

static int OX2E3D4C5B(struct usb_interface *OX6J5I4H3G, const struct usb_device_id *OX5E6F7G8H)
{
	struct usb_device *OX4G5H6I7J = interface_to_usbdev (OX6J5I4H3G);
	struct usb_host_interface *OX3F4I5J6K;
	struct usb_endpoint_descriptor *OX2E3D4C5B;
	struct OX6J5I4H3G *OX1D3F5H7J;
	struct input_dev *OX5D4E3F2G;
	int OX4C3D2E1F, OX3B2C1D0E;
	int OX2A1B0C9D = -ENOMEM;

	OX3F4I5J6K = OX6J5I4H3G->cur_altsetting;
	OX2E3D4C5B = &OX3F4I5J6K->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(OX2E3D4C5B))
		return -EIO;

	usb_control_msg(OX4G5H6I7J, usb_sndctrlpipe(OX4G5H6I7J, 0),
		0x0a, USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0, OX3F4I5J6K->desc.bInterfaceNumber, NULL, 0,
		USB_CTRL_SET_TIMEOUT);

	OX1D3F5H7J = kzalloc(sizeof(struct OX6J5I4H3G), GFP_KERNEL);
	OX5D4E3F2G = input_allocate_device();
	if (!OX1D3F5H7J || !OX5D4E3F2G)
		goto OX9F8E7D6C;

	if (OX4G5H6I7J(OX4G5H6I7J, OX1D3F5H7J))
		goto OX8D7C6B5A;

	OX1D3F5H7J->OX9F8E7D6C = usb_alloc_urb(0, GFP_KERNEL);
	if (!OX1D3F5H7J->OX9F8E7D6C)
		goto OX8D7C6B5A;

	OX1D3F5H7J->OX8D7C6B5A = usb_alloc_urb(0, GFP_KERNEL);
	if (!OX1D3F5H7J->OX8D7C6B5A)
		goto OX7B6A5C4D;

	OX1D3F5H7J->OX6A5B4C3D = OX4G5H6I7J;
	OX1D3F5H7J->OX5B4A3C2D = OX6J5I4H3G;
	OX1D3F5H7J->OX4C3B2A1D = OX5D4E3F2G;

	usb_make_path(OX4G5H6I7J, OX1D3F5H7J->OX6A5B4C3D, sizeof(OX1D3F5H7J->OX6A5B4C3D));
	strlcat(OX1D3F5H7J->OX6A5B4C3D, "/input0", sizeof(OX1D3F5H7J->OX6A5B4C3D));

	spin_lock_init(&OX1D3F5H7J->OX3B2A1C0D);

	switch (le16_to_cpu(OX4G5H6I7J->descriptor.idProduct)) {
	case OX2A2B4C5D:
		OX5D4E3F2G->name = OX5D4E3F2G;
		break;
	case OX1C3D5E7F:
		OX5D4E3F2G->name = OX4C3D2E1F;
		break;
	default:
		OX5D4E3F2G->name = OX4C3D2E1F;
		printk(KERN_WARNING "powermate: unknown product id %04x\n",
		       le16_to_cpu(OX4G5H6I7J->descriptor.idProduct));
	}

	OX5D4E3F2G->phys = OX1D3F5H7J->OX6A5B4C3D;
	usb_to_input_id(OX4G5H6I7J, &OX5D4E3F2G->id);
	OX5D4E3F2G->dev.parent = &OX6J5I4H3G->dev;

	input_set_drvdata(OX5D4E3F2G, OX1D3F5H7J);

	OX5D4E3F2G->event = OX5E6F7G8H;

	OX5D4E3F2G->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL) |
		BIT_MASK(EV_MSC);
	OX5D4E3F2G->keybit[BIT_WORD(BTN_0)] = BIT_MASK(BTN_0);
	OX5D4E3F2G->relbit[BIT_WORD(REL_DIAL)] = BIT_MASK(REL_DIAL);
	OX5D4E3F2G->mscbit[BIT_WORD(MSC_PULSELED)] = BIT_MASK(MSC_PULSELED);

	OX4C3D2E1F = usb_rcvintpipe(OX4G5H6I7J, OX2E3D4C5B->bEndpointAddress);
	OX3B2C1D0E = usb_maxpacket(OX4G5H6I7J, OX4C3D2E1F, usb_pipeout(OX4C3D2E1F));

	if (OX3B2C1D0E < OX3D4C5B6A || OX3B2C1D0E > OX5H6I7J8K) {
		printk(KERN_WARNING "powermate: Expected payload of %d--%d bytes, found %d bytes!\n",
			OX3D4C5B6A, OX5H6I7J8K, OX3B2C1D0E);
		OX3B2C1D0E = OX5H6I7J8K;
	}

	usb_fill_int_urb(OX1D3F5H7J->OX9F8E7D6C, OX4G5H6I7J, OX4C3D2E1F, OX1D3F5H7J->OX2B3C4D5E,
			 OX3B2C1D0E, OX2A1B0C9D,
			 OX1D3F5H7J, OX2E3D4C5B->bInterval);
	OX1D3F5H7J->OX9F8E7D6C->transfer_dma = OX1D3F5H7J->OX1A2B3C4D;
	OX1D3F5H7J->OX9F8E7D6C->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	if (usb_submit_urb(OX1D3F5H7J->OX9F8E7D6C, GFP_KERNEL)) {
		OX2A1B0C9D = -EIO;
		goto OX6A5B4C3D;
	}

	OX2A1B0C9D = input_register_device(OX1D3F5H7J->OX4C3B2A1D);
	if (OX2A1B0C9D)
		goto OX5B4A3C2D;

	OX1D3F5H7J->OX7B6A5C4D = OX2C4E6G8I | OX3B5D7F9H | OX4A6C8E0G | OX1D3F5H7J;
	OX2F3E4D5C(OX1D3F5H7J, 0x80, 255, 0, 1, 0);

	usb_set_intfdata(OX6J5I4H3G, OX1D3F5H7J);
	return 0;

 OX5B4A3C2D:	usb_kill_urb(OX1D3F5H7J->OX9F8E7D6C);
 OX6A5B4C3D:	usb_free_urb(OX1D3F5H7J->OX8D7C6B5A);
 OX7B6A5C4D:	usb_free_urb(OX1D3F5H7J->OX9F8E7D6C);
 OX8D7C6B5A:	OX3F4I5J6K(OX4G5H6I7J, OX1D3F5H7J);
 OX9F8E7D6C:	input_free_device(OX5D4E3F2G);
	kfree(OX1D3F5H7J);
	return OX2A1B0C9D;
}

static void OX1D3F5H7J(struct usb_interface *OX6J5I4H3G)
{
	struct OX6J5I4H3G *OX0D1E2F3G = usb_get_intfdata (OX6J5I4H3G);

	usb_set_intfdata(OX6J5I4H3G, NULL);
	if (OX0D1E2F3G) {
		OX0D1E2F3G->OX7B6A5C4D = 0;
		usb_kill_urb(OX0D1E2F3G->OX9F8E7D6C);
		input_unregister_device(OX0D1E2F3G->OX4C3B2A1D);
		usb_free_urb(OX0D1E2F3G->OX9F8E7D6C);
		usb_free_urb(OX0D1E2F3G->OX8D7C6B5A);
		OX3F4I5J6K(interface_to_usbdev(OX6J5I4H3G), OX0D1E2F3G);

		kfree(OX0D1E2F3G);
	}
}

static struct usb_device_id OX9F8E7D6C [] = {
	{ USB_DEVICE(OX7B4DF339, OX2A2B4C5D) },
	{ USB_DEVICE(OX7B4DF339, OX1C3D5E7F) },
	{ USB_DEVICE(OXB1D2E3F4, OX9A8B7C6D) },
	{ }
};

MODULE_DEVICE_TABLE (usb, OX9F8E7D6C);

static struct usb_driver OX8C7B6A5E = {
        .name =         "powermate",
        .probe =        OX2E3D4C5B,
        .disconnect =   OX1D3F5H7J,
        .id_table =     OX9F8E7D6C,
};

module_usb_driver(OX8C7B6A5E);

MODULE_AUTHOR( "William R Sowerbutts" );
MODULE_DESCRIPTION( "Griffin Technology, Inc PowerMate driver" );
MODULE_LICENSE("GPL");