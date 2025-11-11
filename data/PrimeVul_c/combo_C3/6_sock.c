#include <linux/module.h>
#include <linux/types.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/fcntl.h>
#include <linux/skbuff.h>
#include <linux/socket.h>
#include <linux/ioctl.h>
#include <linux/file.h>
#include <linux/init.h>
#include <linux/compat.h>
#include <linux/gfp.h>
#include <net/sock.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include "bnep.h"

#define MAX_STACK 256

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} InstructionSet;

typedef struct {
    int stack[MAX_STACK];
    int sp;
    int pc;
    int running;
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void vm_push(VM *vm, int value) {
	if(vm->sp < MAX_STACK - 1) {
		vm->stack[++vm->sp] = value;
	}
}

int vm_pop(VM *vm) {
    if(vm->sp >= 0) {
        return vm->stack[vm->sp--];
    }
	return 0;
}

void vm_execute(VM *vm, const int *program) {
    while (vm->running) {
        switch (program[vm->pc++]) {
            case PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP:
                vm->pc = program[vm->pc];
                break;
            case JZ: {
                int addr = program[vm->pc++];
                if (vm_pop(vm) == 0)
                    vm->pc = addr;
                break;
            }
            case LOAD:
                vm_push(vm, vm->stack[program[vm->pc++]]);
                break;
            case STORE:
                vm->stack[program[vm->pc++]] = vm_pop(vm);
                break;
            case CALL:
                vm_push(vm, vm->pc + 1);
                vm->pc = program[vm->pc];
                break;
            case RET:
                vm->pc = vm_pop(vm);
                break;
            case HALT:
                vm->running = 0;
                break;
        }
    }
}

static int bnep_sock_release(struct socket *sock) {
    VM vm;
    vm_init(&vm);
    const int program[] = {PUSH, (int)(size_t)sock, LOAD, HALT};
    vm_execute(&vm, program);
    struct sock *sk = (struct sock *)(size_t)vm_pop(&vm);
    if (!sk) return 0;
    sock_orphan(sk);
    sock_put(sk);
    return 0;
}

static int bnep_sock_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg) {
    VM vm;
    vm_init(&vm);
    const int program[] = {
        PUSH, (int)(size_t)sock, PUSH, (int)cmd, PUSH, (int)arg, LOAD, HALT
    };
    vm_execute(&vm, program);
    struct bnep_connlist_req cl;
    struct bnep_connadd_req ca;
    struct bnep_conndel_req cd;
    struct bnep_conninfo ci;
    struct socket *nsock;
    void __user *argp = (void __user *)vm_pop(&vm);
    int err;
    unsigned int cmd_val = vm_pop(&vm);

    switch (cmd_val) {
        case BNEPCONNADD:
            if (!capable(CAP_NET_ADMIN)) return -EACCES;
            if (copy_from_user(&ca, argp, sizeof(ca))) return -EFAULT;
            nsock = sockfd_lookup(ca.sock, &err);
            if (!nsock) return err;
            if (nsock->sk->sk_state != BT_CONNECTED) {
                sockfd_put(nsock);
                return -EBADFD;
            }
            err = bnep_add_connection(&ca, nsock);
            if (!err) {
                if (copy_to_user(argp, &ca, sizeof(ca))) err = -EFAULT;
            } else sockfd_put(nsock);
            return err;
        case BNEPCONNDEL:
            if (!capable(CAP_NET_ADMIN)) return -EACCES;
            if (copy_from_user(&cd, argp, sizeof(cd))) return -EFAULT;
            return bnep_del_connection(&cd);
        case BNEPGETCONNLIST:
            if (copy_from_user(&cl, argp, sizeof(cl))) return -EFAULT;
            if (cl.cnum <= 0) return -EINVAL;
            err = bnep_get_connlist(&cl);
            if (!err && copy_to_user(argp, &cl, sizeof(cl))) return -EFAULT;
            return err;
        case BNEPGETCONNINFO:
            if (copy_from_user(&ci, argp, sizeof(ci))) return -EFAULT;
            err = bnep_get_conninfo(&ci);
            if (!err && copy_to_user(argp, &ci, sizeof(ci))) return -EFAULT;
            return err;
        default:
            return -EINVAL;
    }
    return 0;
}

#ifdef CONFIG_COMPAT
static int bnep_sock_compat_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg) {
    VM vm;
    vm_init(&vm);
    const int program[] = {
        PUSH, (int)(size_t)sock, PUSH, (int)cmd, PUSH, (int)arg, LOAD, HALT
    };
    vm_execute(&vm, program);
    unsigned long arg_val = vm_pop(&vm);
    unsigned int cmd_val = vm_pop(&vm);
    if (cmd_val == BNEPGETCONNLIST) {
        struct bnep_connlist_req cl;
        uint32_t uci;
        int err;
        if (get_user(cl.cnum, (uint32_t __user *)arg_val) || get_user(uci, (u32 __user *)(arg_val + 4))) return -EFAULT;
        cl.ci = compat_ptr(uci);
        if (cl.cnum <= 0) return -EINVAL;
        err = bnep_get_connlist(&cl);
        if (!err && put_user(cl.cnum, (uint32_t __user *)arg_val)) err = -EFAULT;
        return err;
    }
    return bnep_sock_ioctl(sock, cmd_val, arg_val);
}
#endif

static const struct proto_ops bnep_sock_ops = {
    .family = PF_BLUETOOTH,
    .owner = THIS_MODULE,
    .release = bnep_sock_release,
    .ioctl = bnep_sock_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = bnep_sock_compat_ioctl,
#endif
    .bind = sock_no_bind,
    .getname = sock_no_getname,
    .sendmsg = sock_no_sendmsg,
    .recvmsg = sock_no_recvmsg,
    .poll = sock_no_poll,
    .listen = sock_no_listen,
    .shutdown = sock_no_shutdown,
    .setsockopt = sock_no_setsockopt,
    .getsockopt = sock_no_getsockopt,
    .connect = sock_no_connect,
    .socketpair = sock_no_socketpair,
    .accept = sock_no_accept,
    .mmap = sock_no_mmap
};

static struct proto bnep_proto = {
    .name = "BNEP",
    .owner = THIS_MODULE,
    .obj_size = sizeof(struct bt_sock)
};

static int bnep_sock_create(struct net *net, struct socket *sock, int protocol, int kern) {
    VM vm;
    vm_init(&vm);
    const int program[] = {PUSH, (int)(size_t)sock, LOAD, HALT};
    vm_execute(&vm, program);
    struct sock *sk;
    struct socket *sock_val = (struct socket *)(size_t)vm_pop(&vm);
    if (sock_val->type != SOCK_RAW) return -ESOCKTNOSUPPORT;
    sk = sk_alloc(net, PF_BLUETOOTH, GFP_ATOMIC, &bnep_proto);
    if (!sk) return -ENOMEM;
    sock_init_data(sock_val, sk);
    sock_val->ops = &bnep_sock_ops;
    sock_val->state = SS_UNCONNECTED;
    sock_reset_flag(sk, SOCK_ZAPPED);
    sk->sk_protocol = protocol;
    sk->sk_state = BT_OPEN;
    return 0;
}

static const struct net_proto_family bnep_sock_family_ops = {
    .family = PF_BLUETOOTH,
    .owner = THIS_MODULE,
    .create = bnep_sock_create
};

int __init bnep_sock_init(void) {
    int err;
    err = proto_register(&bnep_proto, 0);
    if (err < 0) return err;
    err = bt_sock_register(BTPROTO_BNEP, &bnep_sock_family_ops);
    if (err < 0) goto error;
    return 0;
error:
    BT_ERR("Can't register BNEP socket");
    proto_unregister(&bnep_proto);
    return err;
}

void __exit bnep_sock_cleanup(void) {
    if (bt_sock_unregister(BTPROTO_BNEP) < 0) BT_ERR("Can't unregister BNEP socket");
    proto_unregister(&bnep_proto);
}