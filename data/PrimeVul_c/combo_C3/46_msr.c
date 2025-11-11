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

enum vm_instr {
	VM_PUSH, VM_POP, VM_ADD, VM_SUB, VM_JMP, VM_JZ,
	VM_LOAD, VM_STORE, VM_RDMSR, VM_WRMSR, VM_HALT
};

typedef struct {
	int pc;
	int sp;
	u32 stack[256];
	u32 reg[8];
} VM;

static void vm_exec(VM *vm, const u32 *prog, int size)
{
	while (vm->pc < size) {
		switch (prog[vm->pc++]) {
		case VM_PUSH:
			vm->stack[vm->sp++] = prog[vm->pc++];
			break;
		case VM_POP:
			vm->sp--;
			break;
		case VM_ADD:
			vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1];
			vm->sp--;
			break;
		case VM_SUB:
			vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1];
			vm->sp--;
			break;
		case VM_JMP:
			vm->pc = prog[vm->pc];
			break;
		case VM_JZ:
			if (vm->stack[--vm->sp] == 0)
				vm->pc = prog[vm->pc];
			else
				vm->pc++;
			break;
		case VM_LOAD:
			vm->stack[vm->sp++] = vm->reg[prog[vm->pc++]];
			break;
		case VM_STORE:
			vm->reg[prog[vm->pc++]] = vm->stack[--vm->sp];
			break;
		case VM_RDMSR:
			rdmsr_safe_on_cpu(vm->reg[0], vm->reg[1], &vm->reg[2], &vm->reg[3]);
			break;
		case VM_WRMSR:
			wrmsr_safe_on_cpu(vm->reg[0], vm->reg[1], vm->reg[2], vm->reg[3]);
			break;
		case VM_HALT:
			return;
		}
	}
}

static loff_t msr_seek(struct file *file, loff_t offset, int orig)
{
	VM vm = {0};
	u32 prog[] = {
		VM_PUSH, orig,
		VM_PUSH, offset,
		VM_LOAD, 0,
		VM_STORE, 0,
		VM_HALT
	};
	vm_exec(&vm, prog, 9);
	return vm.reg[0];
}

static ssize_t msr_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	VM vm = {0};
	u32 prog[] = {
		VM_PUSH, *ppos,
		VM_PUSH, iminor(file->f_path.dentry->d_inode),
		VM_PUSH, count,
		VM_RDMSR,
		VM_STORE, 0,
		VM_HALT
	};
	vm_exec(&vm, prog, 11);
	return vm.reg[0];
}

static ssize_t msr_write(struct file *file, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	VM vm = {0};
	u32 prog[] = {
		VM_PUSH, *ppos,
		VM_PUSH, iminor(file->f_path.dentry->d_inode),
		VM_PUSH, count,
		VM_WRMSR,
		VM_STORE, 0,
		VM_HALT
	};
	vm_exec(&vm, prog, 11);
	return vm.reg[0];
}

static long msr_ioctl(struct file *file, unsigned int ioc, unsigned long arg)
{
	VM vm = {0};
	u32 prog[] = {
		VM_PUSH, ioc,
		VM_PUSH, arg,
		VM_PUSH, iminor(file->f_path.dentry->d_inode),
		VM_LOAD, 0,
		VM_STORE, 0,
		VM_HALT
	};
	vm_exec(&vm, prog, 12);
	return vm.reg[0];
}

static int msr_open(struct inode *inode, struct file *file)
{
	VM vm = {0};
	u32 prog[] = {
		VM_PUSH, iminor(file->f_path.dentry->d_inode),
		VM_LOAD, 0,
		VM_STORE, 0,
		VM_HALT
	};
	vm_exec(&vm, prog, 7);
	return vm.reg[0];
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
	dev = device_create(msr_class, NULL, MKDEV(MSR_MAJOR, cpu), NULL,
			    "msr%d", cpu);
	return IS_ERR(dev) ? PTR_ERR(dev) : 0;
}

static void msr_device_destroy(int cpu)
{
	device_destroy(msr_class, MKDEV(MSR_MAJOR, cpu));
}

static int __cpuinit msr_class_cpu_callback(struct notifier_block *nfb,
				unsigned long action, void *hcpu)
{
	unsigned int cpu = (unsigned long)hcpu;
	int err = 0;

	switch (action) {
	case CPU_UP_PREPARE:
		err = msr_device_create(cpu);
		break;
	case CPU_UP_CANCELED:
	case CPU_UP_CANCELED_FROZEN:
	case CPU_DEAD:
		msr_device_destroy(cpu);
		break;
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
	int i, err = 0;
	i = 0;

	if (__register_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr", &msr_fops)) {
		printk(KERN_ERR "msr: unable to get major %d for msr\n",
		       MSR_MAJOR);
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

	err = 0;
	goto out;

out_class:
	i = 0;
	for_each_online_cpu(i)
		msr_device_destroy(i);
	put_online_cpus();
	class_destroy(msr_class);
out_chrdev:
	__unregister_chrdev(MSR_MAJOR, 0, NR_CPUS, "cpu/msr");
out:
	return err;
}

static void __exit msr_exit(void)
{
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