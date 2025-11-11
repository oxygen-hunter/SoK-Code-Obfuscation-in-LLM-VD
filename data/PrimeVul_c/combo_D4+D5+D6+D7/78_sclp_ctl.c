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
static unsigned int sccb_supported_words[] = {0x00400002, 0x00410002};

/*
 * Check if command word is supported
 */
static int cmdw_supported(unsigned int cmdw)
{
	int j;
	for (j = 0; j < ARRAY_SIZE(sccb_supported_words); j++) {
		if (cmdw == sccb_supported_words[j])
			return 1;
	}
	return 0;
}

static void __user *ptr_converter(u64 value)
{
	if (is_compat_task())
		return compat_ptr(value);
	else
		return (void __user *)(unsigned long)value;
}

/*
 * Start SCLP request
 */
static int ioctl_sccb_handler(void __user *user_area)
{
	struct sclp_ctl_sccb sccb_ctl;
	struct sccb_header *header;
	int result_code;

	if (copy_from_user(&sccb_ctl, user_area, sizeof(sccb_ctl)))
		return -EFAULT;
	if (!cmdw_supported(sccb_ctl.cmdw))
		return -EOPNOTSUPP;
	header = (void *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
	if (!header)
		return -ENOMEM;
	if (copy_from_user(header, ptr_converter(sccb_ctl.sccb), sizeof(*header))) {
		result_code = -EFAULT;
		goto free_out;
	}
	if (header->length > PAGE_SIZE || header->length < 8)
		return -EINVAL;
	if (copy_from_user(header, ptr_converter(sccb_ctl.sccb), header->length)) {
		result_code = -EFAULT;
		goto free_out;
	}
	result_code = sclp_sync_request(sccb_ctl.cmdw, header);
	if (result_code)
		goto free_out;
	if (copy_to_user(ptr_converter(sccb_ctl.sccb), header, header->length))
		result_code = -EFAULT;
free_out:
	free_page((unsigned long) header);
	return result_code;
}

/*
 * SCLP SCCB ioctl function
 */
static long ioctl_handler(struct file *fl, unsigned int cmd, unsigned long arg)
{
	void __user *ptr_arg;
	if (is_compat_task())
		ptr_arg = compat_ptr(arg);
	else
		ptr_arg = (void __user *) arg;
	switch (cmd) {
	case SCLP_CTL_SCCB:
		return ioctl_sccb_handler(ptr_arg);
	default: /* unknown ioctl number */
		return -ENOTTY;
	}
}

/*
 * File operations
 */
static const struct file_operations f_ops = {
	.unlocked_ioctl = ioctl_handler,
	.owner = THIS_MODULE,
	.open = nonseekable_open,
	.compat_ioctl = ioctl_handler,
	.llseek = no_llseek,
};

/*
 * Misc device definition
 */
struct miscdevice misc_dev = {.minor = MISC_DYNAMIC_MINOR, .name = "sclp", .fops = &f_ops};

/*
 * Register sclp_ctl misc device
 */
static int __init init_device(void) { return misc_register(&misc_dev); }
module_init(init_device);

/*
 * Deregister sclp_ctl misc device
 */
static void __exit exit_device(void) { misc_deregister(&misc_dev); }
module_exit(exit_device);