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

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

#define STACK_SIZE 128

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

int vm_run(VM *vm, int *program) {
    while (1) {
        switch (program[vm->pc++]) {
            case PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD:
                vm_push(vm, vm_pop(vm) + vm_pop(vm));
                break;
            case SUB:
                vm_push(vm, vm_pop(vm) - vm_pop(vm));
                break;
            case JMP:
                vm->pc = program[vm->pc];
                break;
            case JZ: {
                int addr = program[vm->pc++];
                if (vm_pop(vm) == 0)
                    vm->pc = addr;
                break;
            }
            case LOAD: {
                int addr = program[vm->pc++];
                vm_push(vm, program[addr]);
                break;
            }
            case STORE: {
                int addr = program[vm->pc++];
                program[addr] = vm_pop(vm);
                break;
            }
            case CALL: {
                int addr = program[vm->pc++];
                vm_push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case RET:
                vm->pc = vm_pop(vm);
                break;
            case HALT:
                return vm_pop(vm);
        }
    }
}

static int sclp_ctl_cmdw_supported(unsigned int cmdw) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        PUSH, 0x00400002,
        PUSH, 0x00410002,
        LOAD, 0,
        PUSH, cmdw,
        SUB,
        JZ, 16,
        LOAD, 1,
        PUSH, cmdw,
        SUB,
        JZ, 16,
        PUSH, 0,
        HALT,
        PUSH, 1,
        HALT
    };
    return vm_run(&vm, program);
}

static void __user *u64_to_uptr(u64 value) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        CALL, 5,
        PUSH, (int)(unsigned long)value,
        HALT,
        PUSH, is_compat_task(),
        JZ, 10,
        LOAD, 1,
        RET,
        LOAD, 2,
        RET
    };
    return (void __user *)(unsigned long)vm_run(&vm, program);
}

static int sclp_ctl_ioctl_sccb(void __user *user_area) {
    struct sclp_ctl_sccb ctl_sccb;
    struct sccb_header *sccb;
    int rc;
    if (copy_from_user(&ctl_sccb, user_area, sizeof(ctl_sccb)))
        return -EFAULT;
    if (!sclp_ctl_cmdw_supported(ctl_sccb.cmdw))
        return -EOPNOTSUPP;
    sccb = (void *) get_zeroed_page(GFP_KERNEL | GFP_DMA);
    if (!sccb)
        return -ENOMEM;
    if (copy_from_user(sccb, u64_to_uptr(ctl_sccb.sccb), sizeof(*sccb))) {
        rc = -EFAULT;
        goto out_free;
    }
    if (sccb->length > PAGE_SIZE || sccb->length < 8)
        return -EINVAL;
    if (copy_from_user(sccb, u64_to_uptr(ctl_sccb.sccb), sccb->length)) {
        rc = -EFAULT;
        goto out_free;
    }
    rc = sclp_sync_request(ctl_sccb.cmdw, sccb);
    if (rc)
        goto out_free;
    if (copy_to_user(u64_to_uptr(ctl_sccb.sccb), sccb, sccb->length))
        rc = -EFAULT;
out_free:
    free_page((unsigned long) sccb);
    return rc;
}

static long sclp_ctl_ioctl(struct file *filp, unsigned int cmd,
                           unsigned long arg) {
    void __user *argp;
    if (is_compat_task())
        argp = compat_ptr(arg);
    else
        argp = (void __user *) arg;
    switch (cmd) {
    case SCLP_CTL_SCCB:
        return sclp_ctl_ioctl_sccb(argp);
    default:
        return -ENOTTY;
    }
}

static const struct file_operations sclp_ctl_fops = {
    .owner = THIS_MODULE,
    .open = nonseekable_open,
    .unlocked_ioctl = sclp_ctl_ioctl,
    .compat_ioctl = sclp_ctl_ioctl,
    .llseek = no_llseek,
};

static struct miscdevice sclp_ctl_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "sclp",
    .fops = &sclp_ctl_fops,
};

static int __init sclp_ctl_init(void) {
    return misc_register(&sclp_ctl_device);
}
module_init(sclp_ctl_init);

static void __exit sclp_ctl_exit(void) {
    misc_deregister(&sclp_ctl_device);
}
module_exit(sclp_ctl_exit);