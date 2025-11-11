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

static int bnep_sock_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	int state = 0;

	BT_DBG("sock %p sk %p", sock, sk);

	while (state >= 0) {
		switch (state) {
			case 0:
				if (!sk) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				return 0;
			case 2:
				sock_orphan(sk);
				sock_put(sk);
				state = 3;
				break;
			case 3:
				return 0;
		}
	}
	return 0;
}

static int bnep_sock_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	struct bnep_connlist_req cl;
	struct bnep_connadd_req  ca;
	struct bnep_conndel_req  cd;
	struct bnep_conninfo ci;
	struct socket *nsock;
	void __user *argp = (void __user *)arg;
	int err;
	int state = 0;

	BT_DBG("cmd %x arg %lx", cmd, arg);

	while (state >= 0) {
		switch (state) {
			case 0:
				if (cmd == BNEPCONNADD) {
					state = 1;
					break;
				}
				state = 5;
				break;
			case 1:
				if (!capable(CAP_NET_ADMIN)) {
					state = 2;
					break;
				}
				state = 3;
				break;
			case 2:
				return -EACCES;
			case 3:
				if (copy_from_user(&ca, argp, sizeof(ca))) {
					state = 4;
					break;
				}
				state = 6;
				break;
			case 4:
				return -EFAULT;
			case 5:
				if (cmd == BNEPCONNDEL) {
					state = 7;
					break;
				}
				state = 12;
				break;
			case 6:
				nsock = sockfd_lookup(ca.sock, &err);
				if (!nsock) {
					state = 17;
					break;
				}
				state = 18;
				break;
			case 7:
				if (!capable(CAP_NET_ADMIN)) {
					state = 8;
					break;
				}
				state = 9;
				break;
			case 8:
				return -EACCES;
			case 9:
				if (copy_from_user(&cd, argp, sizeof(cd))) {
					state = 10;
					break;
				}
				state = 11;
				break;
			case 10:
				return -EFAULT;
			case 11:
				return bnep_del_connection(&cd);
			case 12:
				if (cmd == BNEPGETCONNLIST) {
					state = 13;
					break;
				}
				state = 21;
				break;
			case 13:
				if (copy_from_user(&cl, argp, sizeof(cl))) {
					state = 14;
					break;
				}
				state = 15;
				break;
			case 14:
				return -EFAULT;
			case 15:
				if (cl.cnum <= 0) {
					state = 16;
					break;
				}
				state = 19;
				break;
			case 16:
				return -EINVAL;
			case 17:
				return err;
			case 18:
				if (nsock->sk->sk_state != BT_CONNECTED) {
					sockfd_put(nsock);
					state = 16;
					break;
				}
				state = 20;
				break;
			case 19:
				err = bnep_get_connlist(&cl);
				if (!err && copy_to_user(argp, &cl, sizeof(cl))) {
					state = 14;
					break;
				}
				state = 22;
				break;
			case 20:
				err = bnep_add_connection(&ca, nsock);
				if (!err) {
					if (copy_to_user(argp, &ca, sizeof(ca))) {
						err = -EFAULT;
					}
				} else {
					sockfd_put(nsock);
				}
				state = 22;
				break;
			case 21:
				if (cmd == BNEPGETCONNINFO) {
					state = 23;
					break;
				}
				state = 26;
				break;
			case 22:
				return err;
			case 23:
				if (copy_from_user(&ci, argp, sizeof(ci))) {
					state = 14;
					break;
				}
				state = 24;
				break;
			case 24:
				err = bnep_get_conninfo(&ci);
				if (!err && copy_to_user(argp, &ci, sizeof(ci))) {
					state = 14;
					break;
				}
				state = 22;
				break;
			case 25:
				return -EFAULT;
			case 26:
				return -EINVAL;
		}
	}
	return 0;
}

#ifdef CONFIG_COMPAT
static int bnep_sock_compat_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	int state = 0;
	while (state >= 0) {
		switch (state) {
			case 0:
				if (cmd == BNEPGETCONNLIST) {
					state = 1;
					break;
				}
				state = 9;
				break;
			case 1:
				{
					struct bnep_connlist_req cl;
					uint32_t uci;
					int err;

					if (get_user(cl.cnum, (uint32_t __user *) arg) ||
							get_user(uci, (u32 __user *) (arg + 4))) {
						state = 3;
						break;
					}
					cl.ci = compat_ptr(uci);

					if (cl.cnum <= 0) {
						state = 5;
						break;
					}

					err = bnep_get_connlist(&cl);

					if (!err && put_user(cl.cnum, (uint32_t __user *) arg)) {
						state = 7;
						break;
					}

					state = 8;
					break;
				}
			case 3:
				return -EFAULT;
			case 5:
				return -EINVAL;
			case 7:
				err = -EFAULT;
				state = 8;
				break;
			case 8:
				return err;
			case 9:
				return bnep_sock_ioctl(sock, cmd, arg);
		}
	}
	return 0;
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

static int bnep_sock_create(struct net *net, struct socket *sock, int protocol,
			    int kern)
{
	struct sock *sk;
	int state = 0;

	BT_DBG("sock %p", sock);

	while (state >= 0) {
		switch (state) {
			case 0:
				if (sock->type != SOCK_RAW) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				return -ESOCKTNOSUPPORT;
			case 2:
				sk = sk_alloc(net, PF_BLUETOOTH, GFP_ATOMIC, &bnep_proto);
				if (!sk) {
					state = 3;
					break;
				}
				state = 4;
				break;
			case 3:
				return -ENOMEM;
			case 4:
				sock_init_data(sock, sk);
				sock->ops = &bnep_sock_ops;
				sock->state = SS_UNCONNECTED;
				sock_reset_flag(sk, SOCK_ZAPPED);
				sk->sk_protocol = protocol;
				sk->sk_state	= BT_OPEN;
				state = 5;
				break;
			case 5:
				return 0;
		}
	}
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
	int state = 0;

	while (state >= 0) {
		switch (state) {
			case 0:
				err = proto_register(&bnep_proto, 0);
				if (err < 0) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				return err;
			case 2:
				err = bt_sock_register(BTPROTO_BNEP, &bnep_sock_family_ops);
				if (err < 0) {
					state = 3;
					break;
				}
				state = 4;
				break;
			case 3:
				goto error;
			case 4:
				return 0;
		}
	}

error:
	BT_ERR("Can't register BNEP socket");
	proto_unregister(&bnep_proto);
	return err;
}

void __exit bnep_sock_cleanup(void)
{
	int state = 0;

	while (state >= 0) {
		switch (state) {
			case 0:
				if (bt_sock_unregister(BTPROTO_BNEP) < 0) {
					state = 1;
					break;
				}
				state = 2;
				break;
			case 1:
				BT_ERR("Can't unregister BNEP socket");
			case 2:
				proto_unregister(&bnep_proto);
				state = 3;
				break;
			case 3:
				return;
		}
	}
}