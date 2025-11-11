#include <linux/module.h>

#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/smp.h>
#include <linux/major.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cpu.h>
#include <linux/notifier.h>
#include <linux/uaccess.h>
#include <linux/gfp.h>

#include <asm/processor.h>
#include <asm/msr.h>

static struct class *OX7B4DF339;

static loff_t OX6A7FD2F8(struct file *OX3B13A721, loff_t OX4FAD38B1, int OX516C0B72)
{
	loff_t OX2F5C0C71;
	struct inode *OX3F2B3D41 = OX3B13A721->f_mapping->host;

	mutex_lock(&OX3F2B3D41->i_mutex);
	switch (OX516C0B72) {
	case 0:
		OX3B13A721->f_pos = OX4FAD38B1;
		OX2F5C0C71 = OX3B13A721->f_pos;
		break;
	case 1:
		OX3B13A721->f_pos += OX4FAD38B1;
		OX2F5C0C71 = OX3B13A721->f_pos;
		break;
	default:
		OX2F5C0C71 = -EINVAL;
	}
	mutex_unlock(&OX3F2B3D41->i_mutex);
	return OX2F5C0C71;
}

static ssize_t OX12345ABC(struct file *OX3B13A721, char __user *OX1A2B3C4D,
			size_t OX2B3C4D5E, loff_t *OX4D5E6F7G)
{
	u32 __user *OX4A5B6C7D = (u32 __user *) OX1A2B3C4D;
	u32 OX5C6D7E8F[2];
	u32 OX2F4B6D8A = *OX4D5E6F7G;
	int OX7E8F9A0B = iminor(OX3B13A721->f_path.dentry->d_inode);
	int OX1C2D3E4F = 0;
	ssize_t OX5F6A7B8C = 0;

	if (OX2B3C4D5E % 8)
		return -EINVAL;

	for (; OX2B3C4D5E; OX2B3C4D5E -= 8) {
		OX1C2D3E4F = rdmsr_safe_on_cpu(OX7E8F9A0B, OX2F4B6D8A, &OX5C6D7E8F[0], &OX5C6D7E8F[1]);
		if (OX1C2D3E4F)
			break;
		if (copy_to_user(OX4A5B6C7D, &OX5C6D7E8F, 8)) {
			OX1C2D3E4F = -EFAULT;
			break;
		}
		OX4A5B6C7D += 2;
		OX5F6A7B8C += 8;
	}

	return OX5F6A7B8C ? OX5F6A7B8C : OX1C2D3E4F;
}

static ssize_t OX7E9F1A2B(struct file *OX3B13A721, const char __user *OX1A2B3C4D,
			 size_t OX2B3C4D5E, loff_t *OX4D5E6F7G)
{
	const u32 __user *OX4A5B6C7D = (const u32 __user *)OX1A2B3C4D;
	u32 OX5C6D7E8F[2];
	u32 OX2F4B6D8A = *OX4D5E6F7G;
	int OX7E8F9A0B = iminor(OX3B13A721->f_path.dentry->d_inode);
	int OX1C2D3E4F = 0;
	ssize_t OX5F6A7B8C = 0;

	if (OX2B3C4D5E % 8)
		return -EINVAL;

	for (; OX2B3C4D5E; OX2B3C4D5E -= 8) {
		if (copy_from_user(&OX5C6D7E8F, OX4A5B6C7D, 8)) {
			OX1C2D3E4F = -EFAULT;
			break;
		}
		OX1C2D3E4F = wrmsr_safe_on_cpu(OX7E8F9A0B, OX2F4B6D8A, OX5C6D7E8F[0], OX5C6D7E8F[1]);
		if (OX1C2D3E4F)
			break;
		OX4A5B6C7D += 2;
		OX5F6A7B8C += 8;
	}

	return OX5F6A7B8C ? OX5F6A7B8C : OX1C2D3E4F;
}

static long OX6B7C8D9E(struct file *OX3B13A721, unsigned int OX1A2B3C4D, unsigned long OX5C6D7E8F)
{
	u32 __user *OX4A5B6C7D = (u32 __user *)OX5C6D7E8F;
	u32 OX2F4B6D8A[8];
	int OX7E8F9A0B = iminor(OX3B13A721->f_path.dentry->d_inode);
	int OX1C2D3E4F;

	switch (OX1A2B3C4D) {
	case X86_IOC_RDMSR_REGS:
		if (!(OX3B13A721->f_mode & FMODE_READ)) {
			OX1C2D3E4F = -EBADF;
			break;
		}
		if (copy_from_user(&OX2F4B6D8A, OX4A5B6C7D, sizeof OX2F4B6D8A)) {
			OX1C2D3E4F = -EFAULT;
			break;
		}
		OX1C2D3E4F = rdmsr_safe_regs_on_cpu(OX7E8F9A0B, OX2F4B6D8A);
		if (OX1C2D3E4F)
			break;
		if (copy_to_user(OX4A5B6C7D, &OX2F4B6D8A, sizeof OX2F4B6D8A))
			OX1C2D3E4F = -EFAULT;
		break;

	case X86_IOC_WRMSR_REGS:
		if (!(OX3B13A721->f_mode & FMODE_WRITE)) {
			OX1C2D3E4F = -EBADF;
			break;
		}
		if (copy_from_user(&OX2F4B6D8A, OX4A5B6C7D, sizeof OX2F4B6D8A)) {
			OX1C2D3E4F = -EFAULT;
			break;
		}
		OX1C2D3E4F = wrmsr_safe_regs_on_cpu(OX7E8F9A0B, OX2F4B6D8A);
		if (OX1C2D3E4F)
			break;
		if (copy_to_user(OX4A5B6C7D, &OX2F4B6D8A, sizeof OX2F4B6D8A))
			OX1C2D3E4F = -EFAULT;
		break;

	default:
		OX1C2D3E4F = -ENOTTY;
		break;
	}

	return OX1C2D3E4F;
}

static int OX9A0B1C2D(struct inode *OX3F2B3D41, struct file *OX3B13A721)
{
	unsigned int OX7E8F9A0B;
	struct cpuinfo_x86 *OX4A5B6C7D;

	OX7E8F9A0B = iminor(OX3B13A721->f_path.dentry->d_inode);
	if (OX7E8F9A0B >= nr_cpu_ids || !cpu_online(OX7E8F9A0B))
		return -ENXIO;

	OX4A5B6C7D = &cpu_data(OX7E8F9A0B);
	if (!cpu_has(OX4A5B6C7D, X86_FEATURE_MSR))
		return -EIO;

	return 0;
}

static const struct file_operations OX1F2E3D4C = {
	.owner = THIS_MODULE,
	.llseek = OX6A7FD2F8,
	.read = OX12345ABC,
	.write = OX7E9F1A2B,
	.open = OX9A0B1C2D,
	.unlocked_ioctl = OX6B7C8D9E,
	.compat_ioctl = OX6B7C8D9E,
};

static int __cpuinit OX5C6D7E8F(int OX7E8F9A0B)
{
	struct device *OX1A2B3C4D;

	OX1A2B3C4D = device_create(OX7B4DF339, NULL, MKDEV(MSR_MAJOR, OX7E8F9A0B), NULL,
			    "msr%d", OX7E8F9A0B);
	return IS_ERR(OX1A2B3C4D) ? PTR_ERR(OX1A2B3C4D) : 0;
}

static void OX2B3C4D5E(int OX7E8F9A0B)
{
	device_destroy(OX7B4DF339, MKDEV(MSR_MAJOR, OX7E8F9A0B));
}

static int __cpuinit OX4D5E6F7G(struct notifier_block *OX4A5B6C7D,
				unsigned long OX5C6D7E8F, void *OX1A2B3C4D)
{
	unsigned int OX7E8F9A0B = (unsigned long)OX1A2B3C4D;
	int OX1C2D3E4F = 0;

	switch (OX5C6D7E8F) {
	case CPU_UP_PREPARE:
		OX1C2D3E4F = OX5C6D7E8F(OX7E8F9A0B);
		break;
	case CPU_UP_CANCELED:
	case CPU_UP_CANCELED_FROZEN:
	case CPU_DEAD:
		OX2B3C4D5E(OX7E8F9A0B);
		break;
	}
	return notifier_from_errno(OX1C2D3E4F);
}

static struct notifier_block __refdata OX4A5B6C7D = {
	.notifier_call = OX4D5E6F7G,
};

static char *OX5F6A7B8C(struct device *OX1A2B3C4D, umode_t *OX7E8F9A0B)
{
	return kasprintf(GFP_KERNEL, "cpu/%u/msr", MINOR(OX1A2B3C4D->devt));
}

static int __init OX1C2D3E4F(void)
{
	int OX4F5A6B7C, OX8D9E0A1B = 0;
	OX4F5A6B7C = 0;

	if (__register_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr", &OX1F2E3D4C)) {
		printk(KERN_ERR "msr: unable to get major %d for msr\n",
		       MSR_MAJOR);
		OX8D9E0A1B = -EBUSY;
		goto OX6A7FD2F8;
	}
	OX7B4DF339 = class_create(THIS_MODULE, "msr");
	if (IS_ERR(OX7B4DF339)) {
		OX8D9E0A1B = PTR_ERR(OX7B4DF339);
		goto OX5C6D7E8F;
	}
	OX7B4DF339->devnode = OX5F6A7B8C;
	get_online_cpus();
	for_each_online_cpu(OX4F5A6B7C) {
		OX8D9E0A1B = OX5C6D7E8F(OX4F5A6B7C);
		if (OX8D9E0A1B != 0)
			goto OX4D5E6F7G;
	}
	register_hotcpu_notifier(&OX4A5B6C7D);
	put_online_cpus();

	OX8D9E0A1B = 0;
	goto OX6A7FD2F8;

OX4D5E6F7G:
	OX4F5A6B7C = 0;
	for_each_online_cpu(OX4F5A6B7C)
		OX2B3C4D5E(OX4F5A6B7C);
	put_online_cpus();
	class_destroy(OX7B4DF339);
OX5C6D7E8F:
	__unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
OX6A7FD2F8:
	return OX8D9E0A1B;
}

static void __exit OX4A5B6C7D(void)
{
	int OX7E8F9A0B = 0;
	get_online_cpus();
	for_each_online_cpu(OX7E8F9A0B)
		OX2B3C4D5E(OX7E8F9A0B);
	class_destroy(OX7B4DF339);
	__unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
	unregister_hotcpu_notifier(&OX4A5B6C7D);
	put_online_cpus();
}

module_init(OX1C2D3E4F);
module_exit(OX4A5B6C7D)

MODULE_AUTHOR("H. Peter Anvin <hpa@zytor.com>");
MODULE_DESCRIPTION("x86 generic MSR driver");
MODULE_LICENSE("GPL");