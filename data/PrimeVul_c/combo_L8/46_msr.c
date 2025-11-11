#include <Python.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
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

typedef long (*rdmsr_safe_on_cpu_t)(int, u32, u32*, u32*);
typedef long (*wrmsr_safe_on_cpu_t)(int, u32, u32, u32);

static loff_t msr_seek(struct file *file, loff_t offset, int orig) {
    loff_t ret;
    struct inode *inode = file->f_mapping->host;
    mutex_lock(&inode->i_mutex);
    switch (orig) {
    case 0:
        file->f_pos = offset;
        ret = file->f_pos;
        break;
    case 1:
        file->f_pos += offset;
        ret = file->f_pos;
        break;
    default:
        ret = -EINVAL;
    }
    mutex_unlock(&inode->i_mutex);
    return ret;
}

static ssize_t msr_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    u32 __user *tmp = (u32 __user *)buf;
    u32 data[2];
    u32 reg = *ppos;
    int cpu = iminor(file->f_path.dentry->d_inode);
    ssize_t bytes = 0;
    rdmsr_safe_on_cpu_t rdmsr_safe_on_cpu;

    if (count % 8)
        return -EINVAL;

    void *handle = dlopen("/path/to/your/libmsr.so", RTLD_LAZY);
    if (!handle) return -EFAULT;

    rdmsr_safe_on_cpu = (rdmsr_safe_on_cpu_t)dlsym(handle, "rdmsr_safe_on_cpu");
    if (!rdmsr_safe_on_cpu) {
        dlclose(handle);
        return -EFAULT;
    }

    for (; count; count -= 8) {
        if (rdmsr_safe_on_cpu(cpu, reg, &data[0], &data[1])) break;
        if (copy_to_user(tmp, &data, 8)) {
            dlclose(handle);
            return -EFAULT;
        }
        tmp += 2;
        bytes += 8;
    }
    dlclose(handle);
    return bytes;
}

static ssize_t msr_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    const u32 __user *tmp = (const u32 __user *)buf;
    u32 data[2];
    u32 reg = *ppos;
    int cpu = iminor(file->f_path.dentry->d_inode);
    ssize_t bytes = 0;
    wrmsr_safe_on_cpu_t wrmsr_safe_on_cpu;

    if (count % 8)
        return -EINVAL;

    void *handle = dlopen("/path/to/your/libmsr.so", RTLD_LAZY);
    if (!handle) return -EFAULT;

    wrmsr_safe_on_cpu = (wrmsr_safe_on_cpu_t)dlsym(handle, "wrmsr_safe_on_cpu");
    if (!wrmsr_safe_on_cpu) {
        dlclose(handle);
        return -EFAULT;
    }

    for (; count; count -= 8) {
        if (copy_from_user(&data, tmp, 8)) {
            dlclose(handle);
            return -EFAULT;
        }
        if (wrmsr_safe_on_cpu(cpu, reg, data[0], data[1])) break;
        tmp += 2;
        bytes += 8;
    }
    dlclose(handle);
    return bytes;
}

static long msr_ioctl(struct file *file, unsigned int ioc, unsigned long arg) {
    // Similar to msr_read and msr_write, handle dynamic library loading for MSR operations
    // This is a placeholder to indicate the pattern
    return 0;
}

static int msr_open(struct inode *inode, struct file *file) {
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

static int __init msr_init(void) {
    int i, err = 0;
    if (__register_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr", &msr_fops)) {
        printk(KERN_ERR "msr: unable to get major %d for msr\n", MSR_MAJOR);
        err = -EBUSY;
        goto out;
    }
    msr_class = class_create(THIS_MODULE, "msr");
    if (IS_ERR(msr_class)) {
        err = PTR_ERR(msr_class);
        goto out_chrdev;
    }
    msr_class->devnode = msr_devnode;
    get_online_cpus();
    for_each_online_cpu(i) {
        err = msr_device_create(i);
        if (err != 0)
            goto out_class;
    }
    register_hotcpu_notifier(&msr_class_cpu_notifier);
    put_online_cpus();

out_class:
    for_each_online_cpu(i)
        msr_device_destroy(i);
    put_online_cpus();
    class_destroy(msr_class);
out_chrdev:
    __unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
out:
    return err;
}

static void __exit msr_exit(void) {
    int cpu = 0;
    get_online_cpus();
    for_each_online_cpu(cpu)
        msr_device_destroy(cpu);
    class_destroy(msr_class);
    __unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
    unregister_hotcpu_notifier(&msr_class_cpu_notifier);
    put_online_cpus();
}

module_init(msr_init);
module_exit(msr_exit)

MODULE_AUTHOR("H. Peter Anvin <hpa@zytor.com>");
MODULE_DESCRIPTION("x86 generic MSR driver");
MODULE_LICENSE("GPL");