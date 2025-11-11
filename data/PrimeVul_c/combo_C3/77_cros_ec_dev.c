#include <linux/fs.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "cros_ec_dev.h"

#define CROS_MAX_DEV 128
static int ec_major;

static const struct attribute_group *cros_ec_groups[] = {
    &cros_ec_attr_group,
    &cros_ec_lightbar_attr_group,
    &cros_ec_vbc_attr_group,
    NULL,
};

static struct class cros_class = {
    .owner = THIS_MODULE,
    .name = "chromeos",
    .dev_groups = cros_ec_groups,
};

enum {
    PUSH,
    POP,
    ADD,
    SUB,
    JMP,
    JZ,
    LOAD,
    STORE,
    HALT,
    CALL_EC_GET_VERSION,
    CALL_CROS_EC_CMD_XFER,
    CALL_COPY_TO_USER,
    CALL_COPY_FROM_USER,
    CALL_KMALLOC,
    CALL_KFREE,
    EC_RES_SUCCESS
};

struct VM {
    int stack[256];
    int pc;
    int sp;
    int running;
};

static void vm_init(struct VM *vm) {
    vm->pc = 0;
    vm->sp = -1;
    vm->running = 1;
}

static void vm_push(struct VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

static int vm_pop(struct VM *vm) {
    return vm->stack[vm->sp--];
}

static void vm_run(struct VM *vm, int *program) {
    int a, b, addr, cmd;
    while (vm->running) {
        cmd = program[vm->pc++];
        switch (cmd) {
            case PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD:
                b = vm_pop(vm);
                a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            case SUB:
                b = vm_pop(vm);
                a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            case JMP:
                vm->pc = program[vm->pc];
                break;
            case JZ:
                addr = program[vm->pc++];
                if (vm_pop(vm) == 0)
                    vm->pc = addr;
                break;
            case LOAD:
                vm_push(vm, vm->stack[program[vm->pc++]]);
                break;
            case STORE:
                vm->stack[program[vm->pc++]] = vm_pop(vm);
                break;
            case HALT:
                vm->running = 0;
                break;
            case CALL_EC_GET_VERSION:
                {
                    struct cros_ec_dev *ec = (struct cros_ec_dev *)vm_pop(vm);
                    char *str = (char *)vm_pop(vm);
                    int maxlen = vm_pop(vm);
                    vm_push(vm, ec_get_version(ec, str, maxlen));
                }
                break;
            case CALL_CROS_EC_CMD_XFER:
                {
                    struct cros_ec_dev *ec = (struct cros_ec_dev *)vm_pop(vm);
                    struct cros_ec_command *msg = (struct cros_ec_command *)vm_pop(vm);
                    vm_push(vm, cros_ec_cmd_xfer(ec, msg));
                }
                break;
            case CALL_COPY_TO_USER:
                {
                    void __user *dest = (void __user *)vm_pop(vm);
                    const void *src = (const void *)vm_pop(vm);
                    size_t n = vm_pop(vm);
                    vm_push(vm, copy_to_user(dest, src, n));
                }
                break;
            case CALL_COPY_FROM_USER:
                {
                    void *dest = (void *)vm_pop(vm);
                    const void __user *src = (const void __user *)vm_pop(vm);
                    size_t n = vm_pop(vm);
                    vm_push(vm, copy_from_user(dest, src, n));
                }
                break;
            case CALL_KMALLOC:
                {
                    size_t size = vm_pop(vm);
                    gfp_t flags = vm_pop(vm);
                    vm_push(vm, (int)kmalloc(size, flags));
                }
                break;
            case CALL_KFREE:
                {
                    void *ptr = (void *)vm_pop(vm);
                    kfree(ptr);
                }
                break;
            case EC_RES_SUCCESS:
                vm_push(vm, EC_RES_SUCCESS);
                break;
        }
    }
}

static int ec_get_version(struct cros_ec_dev *ec, char *str, int maxlen) {
    struct VM vm;
    int program[] = {
        CALL_KMALLOC, sizeof(struct cros_ec_command) + sizeof(struct ec_response_get_version), GFP_KERNEL,
        STORE, 0,
        LOAD, 0,
        JZ, 35,
        LOAD, 0,
        CALL_CROS_EC_CMD_XFER, (int)ec, (int)vm.stack[0],
        STORE, 1,
        LOAD, 1,
        JZ, 33,
        LOAD, 0,
        LOAD, 1,
        CALL_COPY_TO_USER, (int)str, (int)CROS_EC_DEV_VERSION, maxlen,
        STORE, 2,
        HALT
    };
    vm_init(&vm);
    vm_run(&vm, program);
    return vm_pop(&vm);
}

static int ec_device_open(struct inode *inode, struct file *filp) {
    struct cros_ec_dev *ec = container_of(inode->i_cdev, struct cros_ec_dev, cdev);
    filp->private_data = ec;
    nonseekable_open(inode, filp);
    return 0;
}

static int ec_device_release(struct inode *inode, struct file *filp) {
    return 0;
}

static ssize_t ec_device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset) {
    struct cros_ec_dev *ec = filp->private_data;
    char msg[sizeof(struct ec_response_get_version) + sizeof(CROS_EC_DEV_VERSION)];
    size_t count;
    int ret;

    if (*offset != 0)
        return 0;

    ret = ec_get_version(ec, msg, sizeof(msg));
    if (ret)
        return ret;

    count = min(length, strlen(msg));

    if (copy_to_user(buffer, msg, count))
        return -EFAULT;

    *offset = count;
    return count;
}

static long ec_device_ioctl_xcmd(struct cros_ec_dev *ec, void __user *arg) {
    long ret;
    struct cros_ec_command u_cmd;
    struct cros_ec_command *s_cmd;

    if (copy_from_user(&u_cmd, arg, sizeof(u_cmd)))
        return -EFAULT;

    if ((u_cmd.outsize > EC_MAX_MSG_BYTES) || (u_cmd.insize > EC_MAX_MSG_BYTES))
        return -EINVAL;

    s_cmd = kmalloc(sizeof(*s_cmd) + max(u_cmd.outsize, u_cmd.insize), GFP_KERNEL);
    if (!s_cmd)
        return -ENOMEM;

    if (copy_from_user(s_cmd, arg, sizeof(*s_cmd) + u_cmd.outsize)) {
        ret = -EFAULT;
        goto exit;
    }

    s_cmd->command += ec->cmd_offset;
    ret = cros_ec_cmd_xfer(ec->ec_dev, s_cmd);
    if (ret < 0)
        goto exit;

    if (copy_to_user(arg, s_cmd, sizeof(*s_cmd) + u_cmd.insize))
        ret = -EFAULT;
exit:
    kfree(s_cmd);
    return ret;
}

static long ec_device_ioctl_readmem(struct cros_ec_dev *ec, void __user *arg) {
    struct cros_ec_device *ec_dev = ec->ec_dev;
    struct cros_ec_readmem s_mem = { };
    long num;

    if (!ec_dev->cmd_readmem)
        return -ENOTTY;

    if (copy_from_user(&s_mem, arg, sizeof(s_mem)))
        return -EFAULT;

    num = ec_dev->cmd_readmem(ec_dev, s_mem.offset, s_mem.bytes, s_mem.buffer);
    if (num <= 0)
        return num;

    if (copy_to_user((void __user *)arg, &s_mem, sizeof(s_mem)))
        return -EFAULT;

    return 0;
}

static long ec_device_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    struct cros_ec_dev *ec = filp->private_data;

    if (_IOC_TYPE(cmd) != CROS_EC_DEV_IOC)
        return -ENOTTY;

    switch (cmd) {
        case CROS_EC_DEV_IOCXCMD:
            return ec_device_ioctl_xcmd(ec, (void __user *)arg);
        case CROS_EC_DEV_IOCRDMEM:
            return ec_device_ioctl_readmem(ec, (void __user *)arg);
    }

    return -ENOTTY;
}

static const struct file_operations fops = {
    .open = ec_device_open,
    .release = ec_device_release,
    .read = ec_device_read,
    .unlocked_ioctl = ec_device_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = ec_device_ioctl,
#endif
};

static void __remove(struct device *dev) {
    struct cros_ec_dev *ec = container_of(dev, struct cros_ec_dev, class_dev);
    kfree(ec);
}

static int ec_device_probe(struct platform_device *pdev) {
    int retval = -ENOMEM;
    struct device *dev = &pdev->dev;
    struct cros_ec_platform *ec_platform = dev_get_platdata(dev);
    dev_t devno = MKDEV(ec_major, pdev->id);
    struct cros_ec_dev *ec = kzalloc(sizeof(*ec), GFP_KERNEL);

    if (!ec)
        return retval;

    dev_set_drvdata(dev, ec);
    ec->ec_dev = dev_get_drvdata(dev->parent);
    ec->dev = dev;
    ec->cmd_offset = ec_platform->cmd_offset;
    device_initialize(&ec->class_dev);
    cdev_init(&ec->cdev, &fops);

    ec->cdev.kobj.parent = &ec->class_dev.kobj;
    retval = cdev_add(&ec->cdev, devno, 1);
    if (retval) {
        dev_err(dev, ": failed to add character device\n");
        goto cdev_add_failed;
    }

    ec->class_dev.devt = ec->cdev.dev;
    ec->class_dev.class = &cros_class;
    ec->class_dev.parent = dev;
    ec->class_dev.release = __remove;

    retval = dev_set_name(&ec->class_dev, "%s", ec_platform->ec_name);
    if (retval) {
        dev_err(dev, "dev_set_name failed => %d\n", retval);
        goto set_named_failed;
    }

    retval = device_add(&ec->class_dev);
    if (retval) {
        dev_err(dev, "device_register failed => %d\n", retval);
        goto dev_reg_failed;
    }

    return 0;

dev_reg_failed:
set_named_failed:
    dev_set_drvdata(dev, NULL);
    cdev_del(&ec->cdev);
cdev_add_failed:
    kfree(ec);
    return retval;
}

static int ec_device_remove(struct platform_device *pdev) {
    struct cros_ec_dev *ec = dev_get_drvdata(&pdev->dev);
    cdev_del(&ec->cdev);
    device_unregister(&ec->class_dev);
    return 0;
}

static const struct platform_device_id cros_ec_id[] = {
    { "cros-ec-ctl", 0 },
    { /* sentinel */ },
};
MODULE_DEVICE_TABLE(platform, cros_ec_id);

static struct platform_driver cros_ec_dev_driver = {
    .driver = {
        .name = "cros-ec-ctl",
    },
    .probe = ec_device_probe,
    .remove = ec_device_remove,
};

static int __init cros_ec_dev_init(void) {
    int ret;
    dev_t dev = 0;

    ret = class_register(&cros_class);
    if (ret) {
        pr_err(CROS_EC_DEV_NAME ": failed to register device class\n");
        return ret;
    }

    ret = alloc_chrdev_region(&dev, 0, CROS_MAX_DEV, CROS_EC_DEV_NAME);
    if (ret < 0) {
        pr_err(CROS_EC_DEV_NAME ": alloc_chrdev_region() failed\n");
        goto failed_chrdevreg;
    }
    ec_major = MAJOR(dev);

    ret = platform_driver_register(&cros_ec_dev_driver);
    if (ret < 0) {
        pr_warn(CROS_EC_DEV_NAME ": can't register driver: %d\n", ret);
        goto failed_devreg;
    }
    return 0;

failed_devreg:
    unregister_chrdev_region(MKDEV(ec_major, 0), CROS_MAX_DEV);
failed_chrdevreg:
    class_unregister(&cros_class);
    return ret;
}

static void __exit cros_ec_dev_exit(void) {
    platform_driver_unregister(&cros_ec_dev_driver);
    unregister_chrdev(ec_major, CROS_EC_DEV_NAME);
    class_unregister(&cros_class);
}

module_init(cros_ec_dev_init);
module_exit(cros_ec_dev_exit);

MODULE_AUTHOR("Bill Richardson <wfrichar@chromium.org>");
MODULE_DESCRIPTION("Userspace interface to the Chrome OS Embedded Controller");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");