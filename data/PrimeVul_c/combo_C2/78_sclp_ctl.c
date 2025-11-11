/*
 * IOCTL interface for SCLP
 *
 * Copyright IBM Corp. 2012
 *
 * Author: Michael Holzheu <holzheu@linux.vnet.ibm.com>
 */

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

/*
 * Supported command words
 */
static unsigned int sclp_ctl_sccb_wlist[] = {
	0x00400002,
	0x00410002,
};

/*
 * Check if command word is supported
 */
static int sclp_ctl_cmdw_supported(unsigned int cmdw)
{
	int i;
	int __state = 0;
	for (;;) {
		switch (__state) {
		case 0:
			i = 0;
			__state = 1;
			break;
		case 1:
			if (i >= ARRAY_SIZE(sclp_ctl_sccb_wlist)) {
				__state = 3;
				break;
			}
			if (cmdw == sclp_ctl_sccb_wlist[i]) {
				return 1;
			}
			i++;
			break;
		case 3:
			return 0;
		}
	}
}

static void __user *u64_to_uptr(u64 value)
{
	if (is_compat_task())
		return compat_ptr(value);
	else
		return (void __user *)(unsigned long)value;
}

/*
 * Start SCLP request
 */
static int sclp_ctl_ioctl_sccb(void __user *user_area)
{
	struct sclp_ctl_sccb ctl_sccb;
	struct sccb_header *sccb;
	int rc;
	int __state = 0;

	for (;;) {
		switch (__state) {
		case 0:
			if (copy_from_user(&ctl_sccb, user_area, sizeof(ctl_sccb))) {
				return -EFAULT;
			}
			__state = 1;
			break;
		case 1:
			if (!sclp_ctl_cmdw_supported(ctl_sccb.cmdw)) {
				return -EOPNOTSUPP;
			}
			__state = 2;
			break;
		case 2:
			sccb = (void *)get_zeroed_page(GFP_KERNEL | GFP_DMA);
			if (!sccb) {
				return -ENOMEM;
			}
			__state = 3;
			break;
		case 3:
			if (copy_from_user(sccb, u64_to_uptr(ctl_sccb.sccb), sizeof(*sccb))) {
				rc = -EFAULT;
				__state = 8;
				break;
			}
			__state = 4;
			break;
		case 4:
			if (sccb->length > PAGE_SIZE || sccb->length < 8) {
				return -EINVAL;
			}
			__state = 5;
			break;
		case 5:
			if (copy_from_user(sccb, u64_to_uptr(ctl_sccb.sccb), sccb->length)) {
				rc = -EFAULT;
				__state = 8;
				break;
			}
			__state = 6;
			break;
		case 6:
			rc = sclp_sync_request(ctl_sccb.cmdw, sccb);
			if (rc) {
				__state = 8;
				break;
			}
			__state = 7;
			break;
		case 7:
			if (copy_to_user(u64_to_uptr(ctl_sccb.sccb), sccb, sccb->length)) {
				rc = -EFAULT;
			}
		case 8:
			free_page((unsigned long)sccb);
			return rc;
		}
	}
}

/*
 * SCLP SCCB ioctl function
 */
static long sclp_ctl_ioctl(struct file *filp, unsigned int cmd,
			   unsigned long arg)
{
	void __user *argp;
	int __state = 0;

	for (;;) {
		switch (__state) {
		case 0:
			if (is_compat_task())
				argp = compat_ptr(arg);
			else
				argp = (void __user *)arg;
			__state = 1;
			break;
		case 1:
			switch (cmd) {
			case SCLP_CTL_SCCB:
				return sclp_ctl_ioctl_sccb(argp);
			default: /* unknown ioctl number */
				return -ENOTTY;
			}
		}
	}
}

/*
 * File operations
 */
static const struct file_operations sclp_ctl_fops = {
	.owner = THIS_MODULE,
	.open = nonseekable_open,
	.unlocked_ioctl = sclp_ctl_ioctl,
	.compat_ioctl = sclp_ctl_ioctl,
	.llseek = no_llseek,
};

/*
 * Misc device definition
 */
static struct miscdevice sclp_ctl_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "sclp",
	.fops = &sclp_ctl_fops,
};

/*
 * Register sclp_ctl misc device
 */
static int __init sclp_ctl_init(void)
{
	return misc_register(&sclp_ctl_device);
}
module_init(sclp_ctl_init);

/*
 * Deregister sclp_ctl misc device
 */
static void __exit sclp_ctl_exit(void)
{
	misc_deregister(&sclp_ctl_device);
}
module_exit(sclp_ctl_exit);