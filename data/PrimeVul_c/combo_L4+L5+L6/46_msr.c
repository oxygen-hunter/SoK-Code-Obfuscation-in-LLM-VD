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

static struct class *msr_class;

static loff_t msr_seek(struct file *file, loff_t offset, int orig)
{
	loff_t ret;
	struct inode *inode = file->f_mapping->host;

	mutex_lock(&inode->i_mutex);

	if (orig == 0) {
		file->f_pos = offset;
		ret = file->f_pos;
	} else if (orig == 1) {
		file->f_pos += offset;
		ret = file->f_pos;
	} else {
		ret = -EINVAL;
	}

	mutex_unlock(&inode->i_mutex);
	return ret;
}

static ssize_t msr_read_rec(struct file *file, char __user *buf, size_t count, loff_t *ppos, u32 __user *tmp, ssize_t bytes)
{
	if (count == 0)
		return bytes;

	u32 data[2];
	u32 reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
	if (err)
		return bytes ? bytes : err;
	if (copy_to_user(tmp, &data, 8))
		return -EFAULT;

	return msr_read_rec(file, buf, count - 8, ppos, tmp + 2, bytes + 8);
}

static ssize_t msr_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	if (count % 8)
		return -EINVAL;

	return msr_read_rec(file, buf, count, ppos, (u32 __user *)buf, 0);
}

static ssize_t msr_write_rec(struct file *file, const char __user *buf, size_t count, loff_t *ppos, const u32 __user *tmp, ssize_t bytes)
{
	if (count == 0)
		return bytes;

	u32 data[2];
	u32 reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	if (copy_from_user(&data, tmp, 8))
		return -EFAULT;
	int err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
	if (err)
		return bytes ? bytes : err;

	return msr_write_rec(file, buf, count - 8, ppos, tmp + 2, bytes + 8);
}

static ssize_t msr_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	if (count % 8)
		return -EINVAL;

	return msr_write_rec(file, buf, count, ppos, (const u32 __user *)buf, 0);
}

static long msr_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
	u32 __user *uregs = (u32 __user *)arg;
	u32 regs[8];
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err;

	if (ioc == X86_IOC_RDMSR_REGS) {
		if (!(file->f_mode & FMODE_READ))
			return -EBADF;
		if (copy_from_user(&regs, uregs, sizeof regs))
			return -EFAULT;
		err = rdmsr_safe_regs_on_cpu(cpu, regs);
		if (err)
			return err;
		if (copy_to_user(uregs, &regs, sizeof regs))
			return -EFAULT;
		return 0;
	} else if (ioc == X86_IOC_WRMSR_REGS) {
		if (!(file->f_mode & FMODE_WRITE))
			return -EBADF;
		if (copy_from_user(&regs, uregs, sizeof regs))
			return -EFAULT;
		err = wrmsr_safe_regs_on_cpu(cpu, regs);
		if (err)
			return err;
		if (copy_to_user(uregs, &regs, sizeof regs))
			return -EFAULT;
		return 0;
	} else {
		return -ENOTTY;
	}
}

static int msr_open(struct inode *inode, struct file *file)
{
	unsigned int cpu = iminor(file->f_path.dentry->d_inode);
	struct cpuinfo_x86 *c;

	if (cpu >= nr_cpu_ids || !cpu_online(cpu))
		return -ENXIO;

	c = &cpu_data(cpu);
	if (!cpu_has(c, X86_FEATURE_MSR))
		return -EIO;

	return 0;
}

static const struct file_operations msr_fops = {
	.owner = THIS_MODULE,
	.llseek = msr_seek,
	.read = msr_read,
	.write = msr_write,
	.open = msr_open,
	.unlocked_ioctl = msr_ioctl,
	.compat_ioctl = msr_ioctl,
};

static int __cpuinit msr_device_create(int cpu)
{
	struct device *dev = device_create(msr_class, NULL, MKDEV(MSR_MAJOR, cpu), NULL, "msr%d", cpu);
	return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

static void msr_device_destroy(int cpu)
{
	device_destroy(msr_class, MKDEV(MSR_MAJOR, cpu));
}

static int __cpuinit msr_class_cpu_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	int err = 0;

	if (action == CPU_UP_PREPARE) {
		err = msr_device_create(cpu);
	} else if (action == CPU_UP_CANCELED || action == CPU_UP_CANCELED_FROZEN || action == CPU_DEAD) {
		msr_device_destroy(cpu);
	}
	return notifier_from_errno(err);
}

static struct notifier_block __refdata msr_class_cpu_notifier = {
	.notifier_call = msr_class_cpu_callback,
};

static char *msr_devnode(struct device *dev, umode_t *mode)
{
	return kasprintf(GFP_KERNEL, "cpu/%u/msr", MINOR(dev->devt));
}

static int __init msr_init(void)
{
	int err = 0;

	if (__register_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr", &msr_fops)) {
		printk(KERN_ERR "msr: unable to get major %d for msr\n", MSR_MAJOR);
		return -EBUSY;
	}
	msr_class = class_create(THIS_MODULE, "msr");
	if (IS_ERR(msr_class)) {
		err = PTR_ERR(msr_class);
		goto out_chrdev;
	}
	msr_class->devnode = msr_devnode;
	get_online_cpus();
	for_each_online_cpu(int i) {
		err = msr_device_create(i);
		if (err)
			goto out_class;
	}
	register_hotcpu_notifier(&msr_class_cpu_notifier);
	put_online_cpus();

	return 0;

out_class:
	for_each_online_cpu(int i)
		msr_device_destroy(i);
	put_online_cpus();
	class_destroy(msr_class);
out_chrdev:
	__unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
	return err;
}

static void __exit msr_exit(void)
{
	get_online_cpus();
	for_each_online_cpu(int cpu)
		msr_device_destroy(cpu);
	class_destroy(msr_class);
	__unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
	unregister_hotcpu_notifier(&msr_class_cpu_notifier);
	put_online_cpus();
}

module_init(msr_init);
module_exit(msr_exit);

MODULE_AUTHOR("H. Peter Anvin <hpa@zytor.com>");
MODULE_DESCRIPTION("x86 generic MSR driver");
MODULE_LICENSE("GPL");