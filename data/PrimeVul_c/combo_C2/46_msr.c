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
	int state = 0;

	while (1) {
		switch (state) {
		case 0:
			mutex_lock(&inode->i_mutex);
			state = 1;
			break;
		case 1:
			if (orig == 0) {
				file->f_pos = offset;
				ret = file->f_pos;
				state = 3;
			} else if (orig == 1) {
				file->f_pos += offset;
				ret = file->f_pos;
				state = 3;
			} else {
				ret = -EINVAL;
				state = 3;
			}
			break;
		case 3:
			mutex_unlock(&inode->i_mutex);
			return ret;
		}
	}
}

static ssize_t msr_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	u32 __user *tmp = (u32 __user *) buf;
	u32 data[2];
	u32 reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = 0;
	ssize_t bytes = 0;
	int state = 0;

	while (1) {
		switch (state) {
		case 0:
			if (count % 8)
				return -EINVAL;
			state = 1;
			break;
		case 1:
			if (count == 0) return bytes ? bytes : err;
			err = rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1]);
			if (err) return bytes ? bytes : err;
			if (copy_to_user(tmp, &data, 8)) {
				err = -EFAULT;
				return bytes ? bytes : err;
			}
			tmp += 2;
			bytes += 8;
			count -= 8;
			break;
		}
	}
}

static ssize_t msr_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	const u32 __user *tmp = (const u32 __user *)buf;
	u32 data[2];
	u32 reg = *ppos;
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err = 0;
	ssize_t bytes = 0;
	int state = 0;

	while (1) {
		switch (state) {
		case 0:
			if (count % 8)
				return -EINVAL;
			state = 1;
			break;
		case 1:
			if (count == 0) return bytes ? bytes : err;
			if (copy_from_user(&data, tmp, 8)) {
				err = -EFAULT;
				return bytes ? bytes : err;
			}
			err = wrmsr_safe_on_cpu(cpu, reg, data[0], data[1]);
			if (err) return bytes ? bytes : err;
			tmp += 2;
			bytes += 8;
			count -= 8;
			break;
		}
	}
}

static long msr_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
	u32 __user *uregs = (u32 __user *)arg;
	u32 regs[8];
	int cpu = iminor(file->f_path.dentry->d_inode);
	int err;
	int state = 0;

	while (1) {
		switch (state) {
		case 0:
			if (ioc == X86_IOC_RDMSR_REGS) {
				if (!(file->f_mode & FMODE_READ)) {
					err = -EBADF;
					state = 3;
				} else {
					if (copy_from_user(&regs, uregs, sizeof regs)) {
						err = -EFAULT;
						state = 3;
					} else {
						err = rdmsr_safe_regs_on_cpu(cpu, regs);
						if (err) {
							state = 3;
						} else {
							if (copy_to_user(uregs, &regs, sizeof regs))
								err = -EFAULT;
							state = 3;
						}
					}
				}
			} else if (ioc == X86_IOC_WRMSR_REGS) {
				if (!(file->f_mode & FMODE_WRITE)) {
					err = -EBADF;
					state = 3;
				} else {
					if (copy_from_user(&regs, uregs, sizeof regs)) {
						err = -EFAULT;
						state = 3;
					} else {
						err = wrmsr_safe_regs_on_cpu(cpu, regs);
						if (err) {
							state = 3;
						} else {
							if (copy_to_user(uregs, &regs, sizeof regs))
								err = -EFAULT;
							state = 3;
						}
					}
				}
			} else {
				err = -ENOTTY;
				state = 3;
			}
			break;
		case 3:
			return err;
		}
	}
}

static int msr_open(struct inode *inode, struct file *file)
{
	unsigned int cpu;
	struct cpuinfo_x86 *c;
	int state = 0;
	int ret = 0;

	while (1) {
		switch (state) {
		case 0:
			cpu = iminor(file->f_path.dentry->d_inode);
			if (cpu >= nr_cpu_ids || !cpu_online(cpu)) {
				ret = -ENXIO;
				state = 2;
			} else {
				c = &cpu_data(cpu);
				if (!cpu_has(c, X86_FEATURE_MSR)) {
					ret = -EIO;
					state = 2;
				} else {
					ret = 0;
					state = 2;
				}
			}
			break;
		case 2:
			return ret;
		}
	}
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
	struct device *dev;
	int ret;

	dev = device_create(msr_class, NULL, MKDEV(MSR_MAJOR, cpu), NULL, "msr%d", cpu);
	ret = IS_ERR(dev) ? PTR_ERR(dev) : 0;
	return ret;
}

static void msr_device_destroy(int cpu)
{
	device_destroy(msr_class, MKDEV(MSR_MAJOR, cpu));
}

static int __cpuinit msr_class_cpu_callback(struct notifier_block *nfb, unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	int err = 0;
	int state = 0;

	while (1) {
		switch (state) {
		case 0:
			if (action == CPU_UP_PREPARE) {
				err = msr_device_create(cpu);
				state = 2;
			} else if (action == CPU_UP_CANCELED || action == CPU_UP_CANCELED_FROZEN || action == CPU_DEAD) {
				msr_device_destroy(cpu);
				state = 2;
			} else {
				state = 2;
			}
			break;
		case 2:
			return notifier_from_errno(err);
		}
	}
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
	int i, err = 0;
	i = 0;
	int state = 0;

	while (1) {
		switch (state) {
		case 0:
			if (__register_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr", &msr_fops)) {
				printk(KERN_ERR "msr: unable to get major %d for msr\n", MSR_MAJOR);
				err = -EBUSY;
				state = 5;
			} else {
				msr_class = class_create(THIS_MODULE, "msr");
				if (IS_ERR(msr_class)) {
					err = PTR_ERR(msr_class);
					state = 3;
				} else {
					msr_class->devnode = msr_devnode;
					get_online_cpus();
					state = 1;
				}
			}
			break;
		case 1:
			for_each_online_cpu(i) {
				err = msr_device_create(i);
				if (err != 0) {
					state = 2;
					break;
				}
			}
			if (state != 2) {
				register_hotcpu_notifier(&msr_class_cpu_notifier);
				put_online_cpus();
				err = 0;
				state = 5;
			}
			break;
		case 2:
			i = 0;
			for_each_online_cpu(i) msr_device_destroy(i);
			put_online_cpus();
			class_destroy(msr_class);
			state = 3;
			break;
		case 3:
			__unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
			state = 5;
			break;
		case 5:
			return err;
		}
	}
}

static void __exit msr_exit(void)
{
	int cpu = 0;
	int state = 0;

	while (1) {
		switch (state) {
		case 0:
			get_online_cpus();
			state = 1;
			break;
		case 1:
			for_each_online_cpu(cpu) msr_device_destroy(cpu);
			class_destroy(msr_class);
			__unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
			unregister_hotcpu_notifier(&msr_class_cpu_notifier);
			put_online_cpus();
			state = 2;
			break;
		case 2:
			return;
		}
	}
}

module_init(msr_init);
module_exit(msr_exit)

MODULE_AUTHOR("H. Peter Anvin <hpa@zytor.com>");
MODULE_DESCRIPTION("x86 generic MSR driver");
MODULE_LICENSE("GPL");