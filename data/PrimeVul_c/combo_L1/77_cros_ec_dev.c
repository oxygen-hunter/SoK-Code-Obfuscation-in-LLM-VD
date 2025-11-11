#include <linux/fs.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "cros_ec_dev.h"

#define OX7B4DF339 128
static int OX4D2F1F82;

static const struct attribute_group *OX8A1C4B9A[] = {
	&OXE2D3A3EF,
	&OXB3F4D7C9,
	&OXA7E9B1C2,
	NULL,
};

static struct class OX3E7C1F56 = {
	.owner          = THIS_MODULE,
	.name           = "chromeos",
	.dev_groups     = OX8A1C4B9A,
};

static int OXA3C9D7E6(struct OX9F5E2B1D *OX8E1D3F2A, char *OXB9E7C6F8, int OX2F8D4C7B)
{
	struct OX5A9F1E3B *OX1D7A3E6F;
	static const char * const OX7C3B1E8A[] = {
		"unknown", "read-only", "read-write", "invalid",
	};
	struct OX2C7E8B4F *OX8D4A1E9B;
	int OX5C6F7A1D;

	OX8D4A1E9B = kmalloc(sizeof(*OX8D4A1E9B) + sizeof(*OX1D7A3E6F), GFP_KERNEL);
	if (!OX8D4A1E9B)
		return -ENOMEM;

	OX8D4A1E9B->version = 0;
	OX8D4A1E9B->command = EC_CMD_GET_VERSION + OX8E1D3F2A->OX4A7D9C1B;
	OX8D4A1E9B->insize = sizeof(*OX1D7A3E6F);
	OX8D4A1E9B->outsize = 0;

	OX5C6F7A1D = cros_ec_cmd_xfer(OX8E1D3F2A->OX9A3D7E1B, OX8D4A1E9B);
	if (OX5C6F7A1D < 0)
		goto OX7A1C9B2F;

	if (OX8D4A1E9B->result != EC_RES_SUCCESS) {
		snprintf(OXB9E7C6F8, OX2F8D4C7B,
			 "%s\nUnknown EC version: EC returned %d\n",
			 CROS_EC_DEV_VERSION, OX8D4A1E9B->result);
		OX5C6F7A1D = -EINVAL;
		goto OX7A1C9B2F;
	}

	OX1D7A3E6F = (struct OX5A9F1E3B *)OX8D4A1E9B->data;
	if (OX1D7A3E6F->current_image >= ARRAY_SIZE(OX7C3B1E8A))
		OX1D7A3E6F->current_image = 3;

	snprintf(OXB9E7C6F8, OX2F8D4C7B, "%s\n%s\n%s\n%s\n", CROS_EC_DEV_VERSION,
		 OX1D7A3E6F->version_string_ro, OX1D7A3E6F->version_string_rw,
		 OX7C3B1E8A[OX1D7A3E6F->current_image]);

	OX5C6F7A1D = 0;
OX7A1C9B2F:
	kfree(OX8D4A1E9B);
	return OX5C6F7A1D;
}

static int OX9C2E7A4D(struct inode *OX8E7C1A3F, struct file *OX1D9A3C6E)
{
	struct OX9F5E2B1D *OX3A7E9C2D = container_of(OX8E7C1A3F->i_cdev,
					      struct OX9F5E2B1D, OX2C9A7B1E);
	OX1D9A3C6E->private_data = OX3A7E9C2D;
	nonseekable_open(OX8E7C1A3F, OX1D9A3C6E);
	return 0;
}

static int OX1F2C7D9A(struct inode *OX8E7C1A3F, struct file *OX1D9A3C6E)
{
	return 0;
}

static ssize_t OX3E7C9A2D(struct file *OX1D9A3C6E, char __user *OX8C3B1F6E,
			      size_t OX2D7A9E1F, loff_t *OX9C1E7D2A)
{
	struct OX9F5E2B1D *OX3A7E9C2D = OX1D9A3C6E->private_data;
	char OX2B9D7A3E[sizeof(struct OX5A9F1E3B) +
		 sizeof(CROS_EC_DEV_VERSION)];
	size_t OX7A2C9E1D;
	int OX5C6F7A1D;

	if (*OX9C1E7D2A != 0)
		return 0;

	OX5C6F7A1D = OXA3C9D7E6(OX3A7E9C2D, OX2B9D7A3E, sizeof(OX2B9D7A3E));
	if (OX5C6F7A1D)
		return OX5C6F7A1D;

	OX7A2C9E1D = min(OX2D7A9E1F, strlen(OX2B9D7A3E));

	if (copy_to_user(OX8C3B1F6E, OX2B9D7A3E, OX7A2C9E1D))
		return -EFAULT;

	*OX9C1E7D2A = OX7A2C9E1D;
	return OX7A2C9E1D;
}

static long OX1E9D3C7A(struct OX9F5E2B1D *OX8E1D3F2A, void __user *OX3A9E7C1D)
{
	long OX4C7E9A2D;
	struct OX2C7E8B4F OX1D9A3C6E;
	struct OX2C7E8B4F *OX8D4A1E9B;

	if (copy_from_user(&OX1D9A3C6E, OX3A9E7C1D, sizeof(OX1D9A3C6E)))
		return -EFAULT;

	if ((OX1D9A3C6E.outsize > EC_MAX_MSG_BYTES) ||
	    (OX1D9A3C6E.insize > EC_MAX_MSG_BYTES))
		return -EINVAL;

	OX8D4A1E9B = kmalloc(sizeof(*OX8D4A1E9B) + max(OX1D9A3C6E.outsize, OX1D9A3C6E.insize),
			GFP_KERNEL);
	if (!OX8D4A1E9B)
		return -ENOMEM;

	if (copy_from_user(OX8D4A1E9B, OX3A9E7C1D, sizeof(*OX8D4A1E9B) + OX1D9A3C6E.outsize)) {
		OX4C7E9A2D = -EFAULT;
		goto OX7A1C9B2F;
	}

	OX8D4A1E9B->command += OX8E1D3F2A->OX4A7D9C1B;
	OX4C7E9A2D = cros_ec_cmd_xfer(OX8E1D3F2A->OX9A3D7E1B, OX8D4A1E9B);
	if (OX4C7E9A2D < 0)
		goto OX7A1C9B2F;

	if (copy_to_user(OX3A9E7C1D, OX8D4A1E9B, sizeof(*OX8D4A1E9B) + OX1D9A3C6E.insize))
		OX4C7E9A2D = -EFAULT;
OX7A1C9B2F:
	kfree(OX8D4A1E9B);
	return OX4C7E9A2D;
}

static long OX9C2E7A4D(struct OX9F5E2B1D *OX8E1D3F2A, void __user *OX3A9E7C1D)
{
	struct OX7A3C1E9B *OX9A3D7E1B = OX8E1D3F2A->OX9A3D7E1B;
	struct OX8C1D7A9E OX2D7A9E1F = { };
	long OX7C3B1E8A;

	if (!OX9A3D7E1B->cmd_readmem)
		return -ENOTTY;

	if (copy_from_user(&OX2D7A9E1F, OX3A9E7C1D, sizeof(OX2D7A9E1F)))
		return -EFAULT;

	OX7C3B1E8A = OX9A3D7E1B->cmd_readmem(OX9A3D7E1B, OX2D7A9E1F.offset, OX2D7A9E1F.bytes,
				  OX2D7A9E1F.buffer);
	if (OX7C3B1E8A <= 0)
		return OX7C3B1E8A;

	if (copy_to_user((void __user *)OX3A9E7C1D, &OX2D7A9E1F, sizeof(OX2D7A9E1F)))
		return -EFAULT;

	return 0;
}

static long OX4D7A2C9E(struct file *OX1D9A3C6E, unsigned int OX8E1D3F2A,
			    unsigned long OX9C1E7D2A)
{
	struct OX9F5E2B1D *OX3A7E9C2D = OX1D9A3C6E->private_data;

	if (_IOC_TYPE(OX8E1D3F2A) != CROS_EC_DEV_IOC)
		return -ENOTTY;

	switch (OX8E1D3F2A) {
	case CROS_EC_DEV_IOCXCMD:
		return OX1E9D3C7A(OX3A7E9C2D, (void __user *)OX9C1E7D2A);
	case CROS_EC_DEV_IOCRDMEM:
		return OX9C2E7A4D(OX3A7E9C2D, (void __user *)OX9C1E7D2A);
	}

	return -ENOTTY;
}

static const struct file_operations OX2E7A9C3D = {
	.open = OX9C2E7A4D,
	.release = OX1F2C7D9A,
	.read = OX3E7C9A2D,
	.unlocked_ioctl = OX4D7A2C9E,
#ifdef CONFIG_COMPAT
	.compat_ioctl = OX4D7A2C9E,
#endif
};

static void __OX2D9C7A1E(struct device *OX8E1D3F2A)
{
	struct OX9F5E2B1D *OX3A7E9C2D = container_of(OX8E1D3F2A, struct OX9F5E2B1D,
					      OX2A9E1D7C);
	kfree(OX3A7E9C2D);
}

static int OX2B9E7C1D(struct platform_device *OX8E1D3F2A)
{
	int OX4C7E9A2D = -ENOMEM;
	struct device *OX9A3D7E1B = &OX8E1D3F2A->dev;
	struct OX7A1E9C3D *OX2D7A9E1F = dev_get_platdata(OX9A3D7E1B);
	dev_t OX8C1D7A9E = MKDEV(OX4D2F1F82, OX8E1D3F2A->id);
	struct OX9F5E2B1D *OX3A7E9C2D = kzalloc(sizeof(*OX3A7E9C2D), GFP_KERNEL);

	if (!OX3A7E9C2D)
		return OX4C7E9A2D;

	dev_set_drvdata(OX9A3D7E1B, OX3A7E9C2D);
	OX3A7E9C2D->OX9A3D7E1B = dev_get_drvdata(OX9A3D7E1B->parent);
	OX3A7E9C2D->OX9A3D7E1B = OX9A3D7E1B;
	OX3A7E9C2D->OX4A7D9C1B = OX2D7A9E1F->OX4A7D9C1B;
	device_initialize(&OX3A7E9C2D->OX2A9E1D7C);
	cdev_init(&OX3A7E9C2D->OX2C9A7B1E, &OX2E7A9C3D);

	OX3A7E9C2D->OX2C9A7B1E.kobj.parent = &OX3A7E9C2D->OX2A9E1D7C.kobj;
	OX4C7E9A2D = cdev_add(&OX3A7E9C2D->OX2C9A7B1E, OX8C1D7A9E, 1);
	if (OX4C7E9A2D) {
		dev_err(OX9A3D7E1B, ": failed to add character device\n");
		goto OX2C9A7B1E;
	}

	OX3A7E9C2D->OX2A9E1D7C.devt = OX3A7E9C2D->OX2C9A7B1E.dev;
	OX3A7E9C2D->OX2A9E1D7C.class = &OX3E7C1F56;
	OX3A7E9C2D->OX2A9E1D7C.parent = OX9A3D7E1B;
	OX3A7E9C2D->OX2A9E1D7C.release = __OX2D9C7A1E;

	OX4C7E9A2D = dev_set_name(&OX3A7E9C2D->OX2A9E1D7C, "%s", OX2D7A9E1F->OX9C7E1D2A);
	if (OX4C7E9A2D) {
		dev_err(OX9A3D7E1B, "dev_set_name failed => %d\n", OX4C7E9A2D);
		goto OX2A9E1D7C;
	}

	OX4C7E9A2D = device_add(&OX3A7E9C2D->OX2A9E1D7C);
	if (OX4C7E9A2D) {
		dev_err(OX9A3D7E1B, "device_register failed => %d\n", OX4C7E9A2D);
		goto OX2D9A3E6F;
	}

	return 0;

OX2D9A3E6F:
OX2A9E1D7C:
	dev_set_drvdata(OX9A3D7E1B, NULL);
	cdev_del(&OX3A7E9C2D->OX2C9A7B1E);
OX2C9A7B1E:
	kfree(OX3A7E9C2D);
	return OX4C7E9A2D;
}

static int OX4B9D2A7E(struct platform_device *OX8E1D3F2A)
{
	struct OX9F5E2B1D *OX3A7E9C2D = dev_get_drvdata(&OX8E1D3F2A->dev);
	cdev_del(&OX3A7E9C2D->OX2C9A7B1E);
	device_unregister(&OX3A7E9C2D->OX2A9E1D7C);
	return 0;
}

static const struct platform_device_id OX5A3E7B9C[] = {
	{ "cros-ec-ctl", 0 },
	{ },
};
MODULE_DEVICE_TABLE(platform, OX5A3E7B9C);

static struct platform_driver OX9D2A7C3B = {
	.driver = {
		.name = "cros-ec-ctl",
	},
	.probe = OX2B9E7C1D,
	.remove = OX4B9D2A7E,
};

static int __init OX7D1C3F9A(void)
{
	int OX4C7E9A2D;
	dev_t OX8C1D7A9E = 0;

	OX4C7E9A2D  = class_register(&OX3E7C1F56);
	if (OX4C7E9A2D) {
		pr_err(CROS_EC_DEV_NAME ": failed to register device class\n");
		return OX4C7E9A2D;
	}

	OX4C7E9A2D = alloc_chrdev_region(&OX8C1D7A9E, 0, OX7B4DF339, CROS_EC_DEV_NAME);
	if (OX4C7E9A2D < 0) {
		pr_err(CROS_EC_DEV_NAME ": alloc_chrdev_region() failed\n");
		goto OX3E9D7A2F;
	}
	OX4D2F1F82 = MAJOR(OX8C1D7A9E);

	OX4C7E9A2D = platform_driver_register(&OX9D2A7C3B);
	if (OX4C7E9A2D < 0) {
		pr_warn(CROS_EC_DEV_NAME ": can't register driver: %d\n", OX4C7E9A2D);
		goto OX5A9E3D7F;
	}
	return 0;

OX5A9E3D7F:
	unregister_chrdev_region(MKDEV(OX4D2F1F82, 0), OX7B4DF339);
OX3E9D7A2F:
	class_unregister(&OX3E7C1F56);
	return OX4C7E9A2D;
}

static void __exit OX2B9E7C1D(void)
{
	platform_driver_unregister(&OX9D2A7C3B);
	unregister_chrdev(OX4D2F1F82, CROS_EC_DEV_NAME);
	class_unregister(&OX3E7C1F56);
}

module_init(OX7D1C3F9A);
module_exit(OX2B9E7C1D);

MODULE_AUTHOR("Bill Richardson <wfrichar@chromium.org>");
MODULE_DESCRIPTION("Userspace interface to the Chrome OS Embedded Controller");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");