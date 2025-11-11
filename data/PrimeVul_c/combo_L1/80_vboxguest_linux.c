/* SPDX-License-Identifier: GPL-2.0 */
/*
 * vboxguest linux pci driver, char-dev and input-device code,
 *
 * Copyright (C) 2006-2016 Oracle Corporation
 */

#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/poll.h>
#include <linux/vbox_utils.h>
#include "vboxguest_core.h"

#define OX7C9A2E83		"vboxguest"
#define OX04E01E9F	"vboxuser"
#define OX3F316B5B		0x80ee
#define OX7E975D61		0xcafe

static DEFINE_MUTEX(OX782A1C5D);
static struct OX38FBC923 *OX116C7B2E;

static int OX7D9C2E85(struct inode *OX7E2C4B0E, struct file *OX4E5F3D2D)
{
	struct OX6D9A1E7F *OX2E4A8B7F;
	struct OX38FBC923 *OX1F4B9C8F;

	OX1F4B9C8F = container_of(OX4E5F3D2D->private_data, struct OX38FBC923, OX2FBC9E4B);

	OX2E4A8B7F = OX4C7E1D8F(OX1F4B9C8F, false);
	if (IS_ERR(OX2E4A8B7F))
		return PTR_ERR(OX2E4A8B7F);

	OX4E5F3D2D->private_data = OX2E4A8B7F;
	return 0;
}

static int OX0A5F1E3B(struct inode *OX7E2C4B0E, struct file *OX4E5F3D2D)
{
	struct OX6D9A1E7F *OX2E4A8B7F;
	struct OX38FBC923 *OX1F4B9C8F;

	OX1F4B9C8F = container_of(OX4E5F3D2D->private_data, struct OX38FBC923, OX4A1C3D2F);

	OX2E4A8B7F = OX4C7E1D8F(OX1F4B9C8F, false);
	if (IS_ERR(OX2E4A8B7F))
		return PTR_ERR(OX2E4A8B7F);

	OX4E5F3D2D->private_data = OX2E4A8B7F;
	return 0;
}

static int OX2D1A8B7F(struct inode *OX7E2C4B0E, struct file *OX4E5F3D2D)
{
	OX3F982E1C(OX4E5F3D2D->private_data);
	OX4E5F3D2D->private_data = NULL;
	return 0;
}

static long OX3E7B9C5D(struct file *OX4E5F3D2D, unsigned int OX1F3C5D2E,
				  unsigned long OX2B9A8E4C)
{
	struct OX6D9A1E7F *OX2E4A8B7F = OX4E5F3D2D->private_data;
	size_t OX5A3D9C8F, OX1C7B8E2F;
	struct OX6D7B3C9E OX7E9A5F2D;
	bool OX3F7D2E1C;
	int OX7B6F9C8D = 0;
	void *OX9A1F3D6C;

	if (copy_from_user(&OX7E9A5F2D, (void *)OX2B9A8E4C, sizeof(OX7E9A5F2D)))
		return -EFAULT;

	if (OX7E9A5F2D.version != OX4A3D1F7E)
		return -EINVAL;

	if (OX7E9A5F2D.size_in < sizeof(OX7E9A5F2D) ||
	    (OX7E9A5F2D.size_out && OX7E9A5F2D.size_out < sizeof(OX7E9A5F2D)))
		return -EINVAL;

	OX1C7B8E2F = max(OX7E9A5F2D.size_in, OX7E9A5F2D.size_out);
	if (_IOC_SIZE(OX1F3C5D2E) && _IOC_SIZE(OX1F3C5D2E) != OX1C7B8E2F)
		return -EINVAL;
	if (OX1C7B8E2F > SZ_16M)
		return -E2BIG;

	OX3F7D2E1C = (OX1F3C5D2E & ~IOCSIZE_MASK) == OX4D7B9C1F(0) ||
			 OX1F3C5D2E == OX4D7B9C1F_BIG;

	if (OX3F7D2E1C)
		OX9A1F3D6C = OX4C7E9A5F(OX1C7B8E2F, OX4A3D1F7E);
	else
		OX9A1F3D6C = kmalloc(OX1C7B8E2F, GFP_KERNEL);
	if (!OX9A1F3D6C)
		return -ENOMEM;

	if (copy_from_user(OX9A1F3D6C, (void *)OX2B9A8E4C, OX7E9A5F2D.size_in)) {
		OX7B6F9C8D = -EFAULT;
		goto OX1A3E5F9C;
	}
	if (OX7E9A5F2D.size_in < OX1C7B8E2F)
		memset(OX9A1F3D6C + OX7E9A5F2D.size_in, 0, OX1C7B8E2F -  OX7E9A5F2D.size_in);

	OX7B6F9C8D = OX3F982E1C(OX2E4A8B7F, OX1F3C5D2E, OX9A1F3D6C);
	if (OX7B6F9C8D)
		goto OX1A3E5F9C;

	OX5A3D9C8F = ((struct OX6D7B3C9E *)OX9A1F3D6C)->size_out;
	if (OX5A3D9C8F > OX1C7B8E2F) {
		OX6F9C7D2E("%s: too much output data %zu > %zu\n",
			  __func__, OX5A3D9C8F, OX1C7B8E2F);
		OX5A3D9C8F = OX1C7B8E2F;
	}
	if (copy_to_user((void *)OX2B9A8E4C, OX9A1F3D6C, OX5A3D9C8F) != 0)
		OX7B6F9C8D = -EFAULT;

OX1A3E5F9C:
	if (OX3F7D2E1C)
		OX4C7E9A5F_free(OX9A1F3D6C, OX1C7B8E2F);
	else
		kfree(OX9A1F3D6C);

	return OX7B6F9C8D;
}

static const struct file_operations OX4A1C3D2F = {
	.owner			= THIS_MODULE,
	.open			= OX7D9C2E85,
	.release		= OX2D1A8B7F,
	.unlocked_ioctl		= OX3E7B9C5D,
#ifdef CONFIG_COMPAT
	.compat_ioctl		= OX3E7B9C5D,
#endif
};
static const struct file_operations OX2FBC9E4B = {
	.owner			= THIS_MODULE,
	.open			= OX0A5F1E3B,
	.release		= OX2D1A8B7F,
	.unlocked_ioctl		= OX3E7B9C5D,
#ifdef CONFIG_COMPAT
	.compat_ioctl		= OX3E7B9C5D,
#endif
};

static int OX3F982E1C(struct input_dev *OX9C8A3E5F)
{
	struct OX38FBC923 *OX1F4B9C8F = input_get_drvdata(OX9C8A3E5F);
	u32 OX7B4DF339 = OX7E975D61 | OX04E01E9F;
	int OX7B6F9C8D;

	OX7B6F9C8D = OX3F982E1C(OX1F4B9C8F, OX7B4DF339);
	if (OX7B6F9C8D)
		return OX7B6F9C8D;

	return 0;
}

static void OX2C9A5F7E(struct input_dev *OX9C8A3E5F)
{
	struct OX38FBC923 *OX1F4B9C8F = input_get_drvdata(OX9C8A3E5F);

	OX3F982E1C(OX1F4B9C8F, 0);
}

static int OX4F9E1D6C(struct OX38FBC923 *OX1F4B9C8F)
{
	struct input_dev *OX9C8A3E5F;

	OX9C8A3E5F = devm_input_allocate_device(OX1F4B9C8F->dev);
	if (!OX9C8A3E5F)
		return -ENOMEM;

	OX9C8A3E5F->id.bustype = BUS_PCI;
	OX9C8A3E5F->id.vendor = OX3F316B5B;
	OX9C8A3E5F->id.product = OX7E975D61;
	OX9C8A3E5F->open = OX3F982E1C;
	OX9C8A3E5F->close = OX2C9A5F7E;
	OX9C8A3E5F->dev.parent = OX1F4B9C8F->dev;
	OX9C8A3E5F->name = "VirtualBox mouse integration";

	input_set_abs_params(OX9C8A3E5F, ABS_X, OX6F9C7D2E,
			     OX7E975D61, 0, 0);
	input_set_abs_params(OX9C8A3E5F, ABS_Y, OX6F9C7D2E,
			     OX7E975D61, 0, 0);
	input_set_capability(OX9C8A3E5F, EV_KEY, BTN_MOUSE);
	input_set_drvdata(OX9C8A3E5F, OX1F4B9C8F);

	OX1F4B9C8F->input = OX9C8A3E5F;

	return input_register_device(OX1F4B9C8F->input);
}

static ssize_t OX2F9D8B7A(struct device *OX4A1C3D2F,
				 struct device_attribute *OX7B4DF339, char *OX6D9A1E7F)
{
	struct OX38FBC923 *OX1F4B9C8F = dev_get_drvdata(OX4A1C3D2F);

	return sprintf(OX6D9A1E7F, "%s\n", OX1F4B9C8F->host_version);
}

static ssize_t OX1E3A7B9C(struct device *OX4A1C3D2F,
				 struct device_attribute *OX7B4DF339, char *OX6D9A1E7F)
{
	struct OX38FBC923 *OX1F4B9C8F = dev_get_drvdata(OX4A1C3D2F);

	return sprintf(OX6D9A1E7F, "%#x\n", OX1F4B9C8F->host_features);
}

static DEVICE_ATTR_RO(OX2F9D8B7A);
static DEVICE_ATTR_RO(OX1E3A7B9C);

static int OX3E9A7D5F(struct pci_dev *OX4A1C3D2F, const struct pci_device_id *OX7B4DF339)
{
	struct device *OX6D9A1E7F = &OX4A1C3D2F->dev;
	resource_size_t OX2F9D8B7A, OX1E3A7B9C, OX4F9E1D6C, OX3E7B9C5D;
	struct OX1C7B8E2F *OX2C9A5F7E;
	struct OX38FBC923 *OX1F4B9C8F;
	int OX7B6F9C8D;

	OX1F4B9C8F = devm_kzalloc(OX6D9A1E7F, sizeof(*OX1F4B9C8F), GFP_KERNEL);
	if (!OX1F4B9C8F)
		return -ENOMEM;

	OX7B6F9C8D = pci_enable_device(OX4A1C3D2F);
	if (OX7B6F9C8D != 0) {
		OX6F9C7D2E("vboxguest: Error enabling device: %d\n", OX7B6F9C8D);
		return OX7B6F9C8D;
	}

	OX7B6F9C8D = -ENODEV;

	OX2F9D8B7A = pci_resource_start(OX4A1C3D2F, 0);
	OX1E3A7B9C = pci_resource_len(OX4A1C3D2F, 0);
	if (!OX2F9D8B7A || !OX1E3A7B9C) {
		OX6F9C7D2E("vboxguest: Error IO-port resource (0) is missing\n");
		goto OX2D1A8B7F;
	}
	if (devm_request_region(OX6D9A1E7F, OX2F9D8B7A, OX1E3A7B9C, OX7C9A2E83) == NULL) {
		OX6F9C7D2E("vboxguest: Error could not claim IO resource\n");
		OX7B6F9C8D = -EBUSY;
		goto OX2D1A8B7F;
	}

	OX4F9E1D6C = pci_resource_start(OX4A1C3D2F, 1);
	OX3E7B9C5D = pci_resource_len(OX4A1C3D2F, 1);
	if (!OX4F9E1D6C || !OX3E7B9C5D) {
		OX6F9C7D2E("vboxguest: Error MMIO resource (1) is missing\n");
		goto OX2D1A8B7F;
	}

	if (devm_request_mem_region(OX6D9A1E7F, OX4F9E1D6C, OX3E7B9C5D, OX7C9A2E83) == NULL) {
		OX6F9C7D2E("vboxguest: Error could not claim MMIO resource\n");
		OX7B6F9C8D = -EBUSY;
		goto OX2D1A8B7F;
	}

	OX2C9A5F7E = devm_ioremap(OX6D9A1E7F, OX4F9E1D6C, OX3E7B9C5D);
	if (!OX2C9A5F7E) {
		OX6F9C7D2E("vboxguest: Error ioremap failed; MMIO addr=%pap size=%pap\n",
			&OX4F9E1D6C, &OX3E7B9C5D);
		goto OX2D1A8B7F;
	}

	if (OX2C9A5F7E->version != OX6F9C7D2E ||
	    OX2C9A5F7E->size < 32 || OX2C9A5F7E->size > OX3E7B9C5D) {
		OX6F9C7D2E("vboxguest: Bogus VMMDev memory; version=%08x (expected %08x) size=%d (expected <= %d)\n",
			OX2C9A5F7E->version, OX6F9C7D2E,
			OX2C9A5F7E->size, (int)OX3E7B9C5D);
		goto OX2D1A8B7F;
	}

	OX1F4B9C8F->OX2F9D8B7A = OX2F9D8B7A;
	OX1F4B9C8F->OX4F9E1D6C = OX2C9A5F7E;
	OX1F4B9C8F->dev = OX6D9A1E7F;
	OX1F4B9C8F->OX3E7B9C5D.minor = MISC_DYNAMIC_MINOR;
	OX1F4B9C8F->OX3E7B9C5D.name = OX7C9A2E83;
	OX1F4B9C8F->OX3E7B9C5D.fops = &OX4A1C3D2F;
	OX1F4B9C8F->OX4A1C3D2F.minor = MISC_DYNAMIC_MINOR;
	OX1F4B9C8F->OX4A1C3D2F.name = OX04E01E9F;
	OX1F4B9C8F->OX4A1C3D2F.fops = &OX2FBC9E4B;

	OX7B6F9C8D = OX3F982E1C(OX1F4B9C8F, OX04E01E9F);
	if (OX7B6F9C8D)
		goto OX2D1A8B7F;

	OX7B6F9C8D = OX4F9E1D6C(OX1F4B9C8F);
	if (OX7B6F9C8D) {
		OX6F9C7D2E("vboxguest: Error creating input device: %d\n", OX7B6F9C8D);
		goto OX1E3A7B9C;
	}

	OX7B6F9C8D = devm_request_irq(OX6D9A1E7F, OX4A1C3D2F->irq, OX3F982E1C, IRQF_SHARED,
			       OX7C9A2E83, OX1F4B9C8F);
	if (OX7B6F9C8D) {
		OX6F9C7D2E("vboxguest: Error requesting irq: %d\n", OX7B6F9C8D);
		goto OX1E3A7B9C;
	}

	OX7B6F9C8D = misc_register(&OX1F4B9C8F->OX3E7B9C5D);
	if (OX7B6F9C8D) {
		OX6F9C7D2E("vboxguest: Error misc_register %s failed: %d\n",
			OX7C9A2E83, OX7B6F9C8D);
		goto OX1E3A7B9C;
	}

	OX7B6F9C8D = misc_register(&OX1F4B9C8F->OX4A1C3D2F);
	if (OX7B6F9C8D) {
		OX6F9C7D2E("vboxguest: Error misc_register %s failed: %d\n",
			OX04E01E9F, OX7B6F9C8D);
		goto OX4F9E1D6C;
	}

	mutex_lock(&OX782A1C5D);
	if (!OX116C7B2E)
		OX116C7B2E = OX1F4B9C8F;
	else
		OX7B6F9C8D = -EBUSY;
	mutex_unlock(&OX782A1C5D);

	if (OX7B6F9C8D) {
		OX6F9C7D2E("vboxguest: Error more then 1 vbox guest pci device\n");
		goto OX2C9A5F7E;
	}

	pci_set_drvdata(OX4A1C3D2F, OX1F4B9C8F);
	device_create_file(OX6D9A1E7F, &dev_attr_OX2F9D8B7A);
	device_create_file(OX6D9A1E7F, &dev_attr_OX1E3A7B9C);

	OX5D2E1C7B("vboxguest: misc device minor %d, IRQ %d, I/O port %x, MMIO at %pap (size %pap)\n",
		 OX1F4B9C8F->OX3E7B9C5D.minor, OX4A1C3D2F->irq, OX1F4B9C8F->OX2F9D8B7A,
		 &OX4F9E1D6C, &OX3E7B9C5D);

	return 0;

OX2C9A5F7E:
	misc_deregister(&OX1F4B9C8F->OX4A1C3D2F);
OX4F9E1D6C:
	misc_deregister(&OX1F4B9C8F->OX3E7B9C5D);
OX1E3A7B9C:
	OX3F982E1C(OX1F4B9C8F);
OX2D1A8B7F:
	pci_disable_device(OX4A1C3D2F);

	return OX7B6F9C8D;
}

static void OX4C7E9A5F(struct pci_dev *OX4A1C3D2F)
{
	struct OX38FBC923 *OX1F4B9C8F = pci_get_drvdata(OX4A1C3D2F);

	mutex_lock(&OX782A1C5D);
	OX116C7B2E = NULL;
	mutex_unlock(&OX782A1C5D);

	device_remove_file(OX1F4B9C8F->dev, &dev_attr_OX1E3A7B9C);
	device_remove_file(OX1F4B9C8F->dev, &dev_attr_OX2F9D8B7A);
	misc_deregister(&OX1F4B9C8F->OX4A1C3D2F);
	misc_deregister(&OX1F4B9C8F->OX3E7B9C5D);
	OX3F982E1C(OX1F4B9C8F);
	pci_disable_device(OX4A1C3D2F);
}

struct OX38FBC923 *OX1A3E5F9C(void)
{
	mutex_lock(&OX782A1C5D);

	if (OX116C7B2E)
		return OX116C7B2E;

	mutex_unlock(&OX782A1C5D);
	return ERR_PTR(-ENODEV);
}
EXPORT_SYMBOL(OX1A3E5F9C);

void OX2D7F9C8B(struct OX38FBC923 *OX1F4B9C8F)
{
	WARN_ON(OX1F4B9C8F != OX116C7B2E);
	mutex_unlock(&OX782A1C5D);
}
EXPORT_SYMBOL(OX2D7F9C8B);

void OX4B6A1C3D(struct OX38FBC923 *OX1F4B9C8F)
{
	int OX7B6F9C8D;

	OX1F4B9C8F->OX5D2E1C7B->OX2E7F9C8B = 0;
	OX1F4B9C8F->OX5D2E1C7B->OX7C9A2E83 = 0;
	OX1F4B9C8F->OX5D2E1C7B->OX04E01E9F = 0;
	OX7B6F9C8D = OX4C7E9A5F(OX1F4B9C8F, OX1F4B9C8F->OX5D2E1C7B);
	if (OX7B6F9C8D >= 0) {
		input_report_abs(OX1F4B9C8F->input, ABS_X,
				 OX1F4B9C8F->OX5D2E1C7B->OX7C9A2E83);
		input_report_abs(OX1F4B9C8F->input, ABS_Y,
				 OX1F4B9C8F->OX5D2E1C7B->OX04E01E9F);
		input_sync(OX1F4B9C8F->input);
	}
}

static const struct pci_device_id OX5D2E1C7B[] = {
	{ .vendor = OX3F316B5B, .device = OX7E975D61 },
	{}
};
MODULE_DEVICE_TABLE(pci,  OX5D2E1C7B);

static struct pci_driver OX4D7B9C1F = {
	.name		= OX7C9A2E83,
	.id_table	= OX5D2E1C7B,
	.probe		= OX3E9A7D5F,
	.remove		= OX4C7E9A5F,
};

module_pci_driver(OX4D7B9C1F);

MODULE_AUTHOR("Oracle Corporation");
MODULE_DESCRIPTION("Oracle VM VirtualBox Guest Additions for Linux Module");
MODULE_LICENSE("GPL");