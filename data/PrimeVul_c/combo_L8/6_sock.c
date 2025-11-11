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
#include <Python.h>
#include <dlfcn.h>

static PyObject *py_module = NULL;

static int bnep_sock_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	BT_DBG("sock %p sk %p", sock, sk);
	if (!sk)
		return 0;
	sock_orphan(sk);
	sock_put(sk);
	return 0;
}

static int bnep_sock_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	PyObject *py_func, *py_result;
	int err = 0;

	if (py_module == NULL) {
		dlerror(); // Clear any existing error
		void *handle = dlopen("libpython3.8.so", RTLD_LAZY);
		if (!handle) return -1;
		Py_Initialize();
		PyRun_SimpleString("import sys; sys.path.append('.')");
		PyRun_SimpleString("import bnep_operations");
		py_module = PyImport_ImportModule("bnep_operations");
		if (!py_module) return -1;
	}

	py_func = PyObject_GetAttrString(py_module, "bnep_sock_ioctl");
	if (py_func && PyCallable_Check(py_func)) {
		py_result = PyObject_CallFunction(py_func, "I", cmd);
		if (py_result != NULL) {
			err = PyLong_AsLong(py_result);
			Py_DECREF(py_result);
		}
	}
	Py_XDECREF(py_func);
	return err;
}

#ifdef CONFIG_COMPAT
static int bnep_sock_compat_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	if (cmd == BNEPGETCONNLIST) {
		struct bnep_connlist_req cl;
		uint32_t uci;
		int err;

		if (get_user(cl.cnum, (uint32_t __user *) arg) ||
				get_user(uci, (u32 __user *) (arg + 4)))
			return -EFAULT;

		cl.ci = compat_ptr(uci);

		if (cl.cnum <= 0)
			return -EINVAL;

		err = bnep_get_connlist(&cl);

		if (!err && put_user(cl.cnum, (uint32_t __user *) arg))
			err = -EFAULT;

		return err;
	}

	return bnep_sock_ioctl(sock, cmd, arg);
}
#endif

static const struct proto_ops bnep_sock_ops = {
	.family		= PF_BLUETOOTH,
	.owner		= THIS_MODULE,
	.release	= bnep_sock_release,
	.ioctl		= bnep_sock_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= bnep_sock_compat_ioctl,
#endif
	.bind		= sock_no_bind,
	.getname	= sock_no_getname,
	.sendmsg	= sock_no_sendmsg,
	.recvmsg	= sock_no_recvmsg,
	.poll		= sock_no_poll,
	.listen		= sock_no_listen,
	.shutdown	= sock_no_shutdown,
	.setsockopt	= sock_no_setsockopt,
	.getsockopt	= sock_no_getsockopt,
	.connect	= sock_no_connect,
	.socketpair	= sock_no_socketpair,
	.accept		= sock_no_accept,
	.mmap		= sock_no_mmap
};

static struct proto bnep_proto = {
	.name		= "BNEP",
	.owner		= THIS_MODULE,
	.obj_size	= sizeof(struct bt_sock)
};

static int bnep_sock_create(struct net *net, struct socket *sock, int protocol, int kern)
{
	struct sock *sk;
	BT_DBG("sock %p", sock);
	if (sock->type != SOCK_RAW)
		return -ESOCKTNOSUPPORT;
	sk = sk_alloc(net, PF_BLUETOOTH, GFP_ATOMIC, &bnep_proto);
	if (!sk)
		return -ENOMEM;
	sock_init_data(sock, sk);
	sock->ops = &bnep_sock_ops;
	sock->state = SS_UNCONNECTED;
	sock_reset_flag(sk, SOCK_ZAPPED);
	sk->sk_protocol = protocol;
	sk->sk_state	= BT_OPEN;
	return 0;
}

static const struct net_proto_family bnep_sock_family_ops = {
	.family = PF_BLUETOOTH,
	.owner	= THIS_MODULE,
	.create = bnep_sock_create
};

int __init bnep_sock_init(void)
{
	int err;
	err = proto_register(&bnep_proto, 0);
	if (err < 0)
		return err;
	err = bt_sock_register(BTPROTO_BNEP, &bnep_sock_family_ops);
	if (err < 0)
		goto error;
	return 0;
error:
	BT_ERR("Can't register BNEP socket");
	proto_unregister(&bnep_proto);
	return err;
}

void __exit bnep_sock_cleanup(void)
{
	if (bt_sock_unregister(BTPROTO_BNEP) < 0)
		BT_ERR("Can't unregister BNEP socket");
	proto_unregister(&bnep_proto);
	Py_Finalize();
}