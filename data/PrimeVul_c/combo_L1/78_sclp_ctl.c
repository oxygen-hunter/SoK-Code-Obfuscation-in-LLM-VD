#include <linux/compat.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/gfp.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <asm/compat.h>
#include <asm/sclp_ctl.h>
#include <asm/sclp.h>

#include "sclp.h"

static unsigned int OX8123D7A9[] = {
	0x00400002,
	0x00410002,
};

static int OX3F5D8B2C(unsigned int OX6A7C1F4E)
{
	int OX2C6E5A1B;

	for (OX2C6E5A1B = 0; OX2C6E5A1B < ARRAY_SIZE(OX8123D7A9); OX2C6E5A1B++) {
		if (OX6A7C1F4E == OX8123D7A9[OX2C6E5A1B])
			return 1;
	}
	return 0;
}

static void __user *OX6B2D9F7E(u64 OX4A1F7B3E)
{
	if (is_compat_task())
		return compat_ptr(OX4A1F7B3E);
	else
		return (void __user *)(unsigned long)OX4A1F7B3E;
}

static int OX1D9F3B7A(void __user *OX7C8A5D3E)
{
	struct sclp_ctl_sccb OX5D1C7A9F;
	struct sccb_header *OX3A7B2F6E;
	int OX8B5C1D7E;

	if (copy_from_user(&OX5D1C7A9F, OX7C8A5D3E, sizeof(OX5D1C7A9F)))
		return -EFAULT;
	if (!OX3F5D8B2C(OX5D1C7A9F.cmdw))
		return -EOPNOTSUPP;
	OX3A7B2F6E = (void *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
	if (!OX3A7B2F6E)
		return -ENOMEM;
	if (copy_from_user(OX3A7B2F6E, OX6B2D9F7E(OX5D1C7A9F.sccb), sizeof(*OX3A7B2F6E))) {
		OX8B5C1D7E = -EFAULT;
		goto OX4B1E9A7C;
	}
	if (OX3A7B2F6E->length > PAGE_SIZE || OX3A7B2F6E->length < 8)
		return -EINVAL;
	if (copy_from_user(OX3A7B2F6E, OX6B2D9F7E(OX5D1C7A9F.sccb), OX3A7B2F6E->length)) {
		OX8B5C1D7E = -EFAULT;
		goto OX4B1E9A7C;
	}
	OX8B5C1D7E = sclp_sync_request(OX5D1C7A9F.cmdw, OX3A7B2F6E);
	if (OX8B5C1D7E)
		goto OX4B1E9A7C;
	if (copy_to_user(OX6B2D9F7E(OX5D1C7A9F.sccb), OX3A7B2F6E, OX3A7B2F6E->length))
		OX8B5C1D7E = -EFAULT;
OX4B1E9A7C:
	free_page((unsigned long) OX3A7B2F6E);
	return OX8B5C1D7E;
}

static long OX4E7C9B5A(struct file *OX2D7F1C6E, unsigned int OX9A3B6D2E,
			   unsigned long OX7B3C1A9E)
{
	void __user *OX5F2A8E3D;

	if (is_compat_task())
		OX5F2A8E3D = compat_ptr(OX7B3C1A9E);
	else
		OX5F2A8E3D = (void __user *) OX7B3C1A9E;
	switch (OX9A3B6D2E) {
	case SCLP_CTL_SCCB:
		return OX1D9F3B7A(OX5F2A8E3D);
	default:
		return -ENOTTY;
	}
}

static const struct file_operations OX7A5D9C3E = {
	.owner = THIS_MODULE,
	.open = nonseekable_open,
	.unlocked_ioctl = OX4E7C9B5A,
	.compat_ioctl = OX4E7C9B5A,
	.llseek = no_llseek,
};

static struct miscdevice OX3C8B7E5A = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "sclp",
	.fops = &OX7A5D9C3E,
};

static int __init OX2A6F9D3E(void)
{
	return misc_register(&OX3C8B7E5A);
}
module_init(OX2A6F9D3E);

static void __exit OX8F3D7A1B(void)
{
	misc_deregister(&OX3C8B7E5A);
}
module_exit(OX8F3D7A1B);