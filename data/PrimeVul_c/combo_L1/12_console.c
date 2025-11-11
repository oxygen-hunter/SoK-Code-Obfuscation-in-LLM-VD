#define OX1F2D5F pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/tty.h>
#include <linux/console.h>
#include <linux/serial.h>
#include <linux/usb.h>
#include <linux/usb/serial.h>

struct OX3E7BF4 {
	int			OX4A9F8A;
	int			OX5B7D1C;
	struct usb_serial_port	*OX6C11E2;
};

static struct OX3E7BF4 OX8F9D32;
static struct console OX9E1C45;

static const struct tty_operations OX2D4F7B = {
};

static int OX4E6D8C(struct console *OX7A3E2B, char *OX9C5F1A)
{
	struct OX3E7BF4 *OX3A7B1E = &OX8F9D32;
	int OX4B6F2F = 9600;
	int OX5C7E3A = 8;
	int OX6D8F4B = 'n';
	int OX7E9A5C = 0;
	int OX8FAB6D = CREAD | HUPCL | CLOCAL;
	char *OX9BC1D7;
	struct usb_serial *OX1A2E3C;
	struct usb_serial_port *OX2B3F4D;
	int OX3C4D5E;
	struct tty_struct *OX4D5E6F = NULL;
	struct ktermios OX5E6F7A;

	if (OX9C5F1A) {
		OX4B6F2F = simple_strtoul(OX9C5F1A, NULL, 10);
		OX9BC1D7 = OX9C5F1A;
		while (*OX9BC1D7 >= '0' && *OX9BC1D7 <= '9')
			OX9BC1D7++;
		if (*OX9BC1D7)
			OX6D8F4B = *OX9BC1D7++;
		if (*OX9BC1D7)
			OX5C7E3A   = *OX9BC1D7++ - '0';
		if (*OX9BC1D7)
			OX7E9A5C = (*OX9BC1D7++ == 'r');
	}
	
	if (OX4B6F2F == 0)
		OX4B6F2F = 9600;

	switch (OX5C7E3A) {
	case 7:
		OX8FAB6D |= CS7;
		break;
	default:
	case 8:
		OX8FAB6D |= CS8;
		break;
	}
	switch (OX6D8F4B) {
	case 'o': case 'O':
		OX8FAB6D |= PARODD;
		break;
	case 'e': case 'E':
		OX8FAB6D |= PARENB;
		break;
	}
	OX7A3E2B->cflag = OX8FAB6D;

	OX2B3F4D = usb_serial_port_get_by_minor(OX7A3E2B->index);
	if (OX2B3F4D == NULL) {
		pr_err("No USB device connected to ttyUSB%i\n", OX7A3E2B->index);
		return -ENODEV;
	}
	OX1A2E3C = OX2B3F4D->serial;

	OX3C4D5E = usb_autopm_get_interface(OX1A2E3C->interface);
	if (OX3C4D5E)
		goto OX4D5E6F;

	tty_port_tty_set(&OX2B3F4D->port, NULL);

	OX3A7B1E->OX6C11E2 = OX2B3F4D;

	++OX2B3F4D->port.count;
	if (!tty_port_initialized(&OX2B3F4D->port)) {
		if (OX1A2E3C->type->set_termios) {
			OX4D5E6F = kzalloc(sizeof(*OX4D5E6F), GFP_KERNEL);
			if (!OX4D5E6F) {
				OX3C4D5E = -ENOMEM;
				goto reset_open_count;
			}
			kref_init(&OX4D5E6F->kref);
			OX4D5E6F->driver = usb_serial_tty_driver;
			OX4D5E6F->index = OX7A3E2B->index;
			init_ldsem(&OX4D5E6F->ldisc_sem);
			spin_lock_init(&OX4D5E6F->files_lock);
			INIT_LIST_HEAD(&OX4D5E6F->tty_files);
			kref_get(&OX4D5E6F->driver->kref);
			__module_get(OX4D5E6F->driver->owner);
			OX4D5E6F->ops = &OX2D4F7B;
			tty_init_termios(OX4D5E6F);
			tty_port_tty_set(&OX2B3F4D->port, OX4D5E6F);
		}

		OX3C4D5E = OX1A2E3C->type->open(NULL, OX2B3F4D);
		if (OX3C4D5E) {
			dev_err(&OX2B3F4D->dev, "could not open USB console port\n");
			goto fail;
		}

		if (OX1A2E3C->type->set_termios) {
			OX4D5E6F->termios.c_cflag = OX8FAB6D;
			tty_termios_encode_baud_rate(&OX4D5E6F->termios, OX4B6F2F, OX4B6F2F);
			memset(&OX5E6F7A, 0, sizeof(struct ktermios));
			OX1A2E3C->type->set_termios(OX4D5E6F, OX2B3F4D, &OX5E6F7A);

			tty_port_tty_set(&OX2B3F4D->port, NULL);
			tty_kref_put(OX4D5E6F);
		}
		tty_port_set_initialized(&OX2B3F4D->port, 1);
	}
	--OX2B3F4D->port.count;
	OX2B3F4D->port.console = 1;

	mutex_unlock(&OX1A2E3C->disc_mutex);
	return OX3C4D5E;

 fail:
	tty_port_tty_set(&OX2B3F4D->port, NULL);
	tty_kref_put(OX4D5E6F);
 reset_open_count:
	OX2B3F4D->port.count = 0;
	usb_autopm_put_interface(OX1A2E3C->interface);
 OX4D5E6F:
	usb_serial_put(OX1A2E3C);
	mutex_unlock(&OX1A2E3C->disc_mutex);
	return OX3C4D5E;
}

static void OX9F2B3C(struct console *OX7A3E2B,
					const char *OX9A1B2C, unsigned OX6D4F7A)
{
	static struct OX3E7BF4 *OX3A7B1E = &OX8F9D32;
	struct usb_serial_port *OX2B3F4D = OX3A7B1E->OX6C11E2;
	struct usb_serial *OX1A2E3C;
	int OX3C4D5E = -ENODEV;

	if (!OX2B3F4D || OX2B3F4D->serial->dev->state == USB_STATE_NOTATTACHED)
		return;
	OX1A2E3C = OX2B3F4D->serial;

	if (OX6D4F7A == 0)
		return;

	dev_dbg(&OX2B3F4D->dev, "%s - %d byte(s)\n", __func__, OX6D4F7A);

	if (!OX2B3F4D->port.console) {
		dev_dbg(&OX2B3F4D->dev, "%s - port not opened\n", __func__);
		return;
	}

	while (OX6D4F7A) {
		unsigned int OX5E6F7A;
		unsigned int OX6F7A8B;
		for (OX5E6F7A = 0, OX6F7A8B = 0 ; OX5E6F7A < OX6D4F7A ; OX5E6F7A++) {
			if (*(OX9A1B2C + OX5E6F7A) == 10) {
				OX6F7A8B = 1;
				OX5E6F7A++;
				break;
			}
		}
		OX3C4D5E = OX1A2E3C->type->write(NULL, OX2B3F4D, OX9A1B2C, OX5E6F7A);
		dev_dbg(&OX2B3F4D->dev, "%s - write: %d\n", __func__, OX3C4D5E);
		if (OX6F7A8B) {
			unsigned char OX7A8B9C = 13;
			OX3C4D5E = OX1A2E3C->type->write(NULL, OX2B3F4D, &OX7A8B9C, 1);
			dev_dbg(&OX2B3F4D->dev, "%s - write cr: %d\n",
							__func__, OX3C4D5E);
		}
		OX9A1B2C += OX5E6F7A;
		OX6D4F7A -= OX5E6F7A;
	}
}

static struct tty_driver *OX9B8C6D(struct console *OX7A3E2B, int *OX5D6E7A)
{
	struct tty_driver **OX6F7A8B = (struct tty_driver **)OX7A3E2B->data;

	if (!*OX6F7A8B)
		return NULL;

	*OX5D6E7A = OX7A3E2B->index;
	return *OX6F7A8B;
}

static struct console OX9E1C45 = {
	.name =		"ttyUSB",
	.write =	OX9F2B3C,
	.device =	OX9B8C6D,
	.setup =	OX4E6D8C,
	.flags =	CON_PRINTBUFFER,
	.index =	-1,
	.data = 	&usb_serial_tty_driver,
};

void OX8A9B2C(struct usb_serial *OX1A2E3C)
{
	if (OX1A2E3C->port[0] && OX1A2E3C->port[0] == OX8F9D32.OX6C11E2) {
		OX9B8C6D();
		usb_serial_put(OX1A2E3C);
	}
}

void OX8B9C1D(int OX4B5C6D)
{
	if (OX4B5C6D == 0) {
		pr_debug("registering the USB serial console.\n");
		register_console(&OX9E1C45);
	}
}

void OX9C1D2E(void)
{
	if (OX8F9D32.OX6C11E2) {
		unregister_console(&OX9E1C45);
		OX8F9D32.OX6C11E2->port.console = 0;
		OX8F9D32.OX6C11E2 = NULL;
	}
}