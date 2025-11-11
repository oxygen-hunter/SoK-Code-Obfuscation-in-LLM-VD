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

static int OX9B3C4857(struct socket *OX7D4A5C46)
{
	struct sock *OX1F2B3D44 = OX7D4A5C46->sk;
	BT_DBG("sock %p sk %p", OX7D4A5C46, OX1F2B3D44);
	if (!OX1F2B3D44)
		return 0;
	sock_orphan(OX1F2B3D44);
	sock_put(OX1F2B3D44);
	return 0;
}

static int OX8E4A5B56(struct socket *OX2A4E5D34, unsigned int OX5F3B4A22, unsigned long OX6E4C2D11)
{
	struct bnep_connlist_req OX8D4A6B72;
	struct bnep_connadd_req  OX3E5C4D13;
	struct bnep_conndel_req  OX4F6A2B01;
	struct bnep_conninfo OX5D3B1C22;
	struct socket *OX9D7E4F33;
	void __user *OX1E2F3B44 = (void __user *)OX6E4C2D11;
	int OX9C4E5B32;
	BT_DBG("cmd %x arg %lx", OX5F3B4A22, OX6E4C2D11);
	switch (OX5F3B4A22) {
	case BNEPCONNADD:
		if (!capable(CAP_NET_ADMIN))
			return -EACCES;
		if (copy_from_user(&OX3E5C4D13, OX1E2F3B44, sizeof(OX3E5C4D13)))
			return -EFAULT;
		OX9D7E4F33 = sockfd_lookup(OX3E5C4D13.sock, &OX9C4E5B32);
		if (!OX9D7E4F33)
			return OX9C4E5B32;
		if (OX9D7E4F33->sk->sk_state != BT_CONNECTED) {
			sockfd_put(OX9D7E4F33);
			return -EBADFD;
		}
		OX9C4E5B32 = bnep_add_connection(&OX3E5C4D13, OX9D7E4F33);
		if (!OX9C4E5B32) {
			if (copy_to_user(OX1E2F3B44, &OX3E5C4D13, sizeof(OX3E5C4D13)))
				OX9C4E5B32 = -EFAULT;
		} else
			sockfd_put(OX9D7E4F33);
		return OX9C4E5B32;

	case BNEPCONNDEL:
		if (!capable(CAP_NET_ADMIN))
			return -EACCES;
		if (copy_from_user(&OX4F6A2B01, OX1E2F3B44, sizeof(OX4F6A2B01)))
			return -EFAULT;
		return bnep_del_connection(&OX4F6A2B01);

	case BNEPGETCONNLIST:
		if (copy_from_user(&OX8D4A6B72, OX1E2F3B44, sizeof(OX8D4A6B72)))
			return -EFAULT;
		if (OX8D4A6B72.cnum <= 0)
			return -EINVAL;
		OX9C4E5B32 = bnep_get_connlist(&OX8D4A6B72);
		if (!OX9C4E5B32 && copy_to_user(OX1E2F3B44, &OX8D4A6B72, sizeof(OX8D4A6B72)))
			return -EFAULT;
		return OX9C4E5B32;

	case BNEPGETCONNINFO:
		if (copy_from_user(&OX5D3B1C22, OX1E2F3B44, sizeof(OX5D3B1C22)))
			return -EFAULT;
		OX9C4E5B32 = bnep_get_conninfo(&OX5D3B1C22);
		if (!OX9C4E5B32 && copy_to_user(OX1E2F3B44, &OX5D3B1C22, sizeof(OX5D3B1C22)))
			return -EFAULT;
		return OX9C4E5B32;

	default:
		return -EINVAL;
	}
	return 0;
}

#ifdef CONFIG_COMPAT
static int OX4E5D3B72(struct socket *OX2A4E5D34, unsigned int OX5F3B4A22, unsigned long OX6E4C2D11)
{
	if (OX5F3B4A22 == BNEPGETCONNLIST) {
		struct bnep_connlist_req OX8D4A6B72;
		uint32_t OX3E5C4D13;
		int OX9C4E5B32;
		if (get_user(OX8D4A6B72.cnum, (uint32_t __user *) OX6E4C2D11) ||
				get_user(OX3E5C4D13, (u32 __user *) (OX6E4C2D11 + 4)))
			return -EFAULT;
		OX8D4A6B72.ci = compat_ptr(OX3E5C4D13);
		if (OX8D4A6B72.cnum <= 0)
			return -EINVAL;
		OX9C4E5B32 = bnep_get_connlist(&OX8D4A6B72);
		if (!OX9C4E5B32 && put_user(OX8D4A6B72.cnum, (uint32_t __user *) OX6E4C2D11))
			OX9C4E5B32 = -EFAULT;
		return OX9C4E5B32;
	}
	return OX8E4A5B56(OX2A4E5D34, OX5F3B4A22, OX6E4C2D11);
}
#endif

static const struct proto_ops OX7E5C4D13 = {
	.family		= PF_BLUETOOTH,
	.owner		= THIS_MODULE,
	.release	= OX9B3C4857,
	.ioctl		= OX8E4A5B56,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= OX4E5D3B72,
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

static struct proto OX5F6A7B55 = {
	.name		= "BNEP",
	.owner		= THIS_MODULE,
	.obj_size	= sizeof(struct bt_sock)
};

static int OX8A3B4C55(struct net *OX2E4F6A11, struct socket *OX7D4A5C46, int OX7F4C2B22,
			    int OX5B6E4F01)
{
	struct sock *OX1F2B3D44;
	BT_DBG("sock %p", OX7D4A5C46);
	if (OX7D4A5C46->type != SOCK_RAW)
		return -ESOCKTNOSUPPORT;
	OX1F2B3D44 = sk_alloc(OX2E4F6A11, PF_BLUETOOTH, GFP_ATOMIC, &OX5F6A7B55);
	if (!OX1F2B3D44)
		return -ENOMEM;
	sock_init_data(OX7D4A5C46, OX1F2B3D44);
	OX7D4A5C46->ops = &OX7E5C4D13;
	OX7D4A5C46->state = SS_UNCONNECTED;
	sock_reset_flag(OX1F2B3D44, SOCK_ZAPPED);
	OX1F2B3D44->sk_protocol = OX7F4C2B22;
	OX1F2B3D44->sk_state	= BT_OPEN;
	return 0;
}

static const struct net_proto_family OX3F5B7C44 = {
	.family = PF_BLUETOOTH,
	.owner	= THIS_MODULE,
	.create = OX8A3B4C55
};

int __init OX4B6E5A11(void)
{
	int OX9C4E5B32;
	OX9C4E5B32 = proto_register(&OX5F6A7B55, 0);
	if (OX9C4E5B32 < 0)
		return OX9C4E5B32;
	OX9C4E5B32 = bt_sock_register(BTPROTO_BNEP, &OX3F5B7C44);
	if (OX9C4E5B32 < 0)
		goto OX1A3B2D11;
	return 0;
OX1A3B2D11:
	BT_ERR("Can't register BNEP socket");
	proto_unregister(&OX5F6A7B55);
	return OX9C4E5B32;
}

void __exit OX7D6C4A22(void)
{
	if (bt_sock_unregister(BTPROTO_BNEP) < 0)
		BT_ERR("Can't unregister BNEP socket");
	proto_unregister(&OX5F6A7B55);
}