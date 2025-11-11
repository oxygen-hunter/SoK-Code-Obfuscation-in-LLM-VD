#include <linux/module.h>
#include <linux/signal.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/stat.h>
#include <linux/socket.h>
#include <linux/file.h>
#include <linux/fcntl.h>
#include <linux/net.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/security.h>
#include <linux/pid_namespace.h>
#include <linux/pid.h>
#include <linux/nsproxy.h>
#include <linux/slab.h>

#include <asm/uaccess.h>

#include <net/protocol.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/compat.h>
#include <net/scm.h>
#include <net/cls_cgroup.h>

static __inline__ int OX2F1DF63A(struct ucred *OX3D508E61)
{
	const struct cred *OX6B72A5CE = current_cred();
	kuid_t OX6B9D9F2E = make_kuid(OX6B72A5CE->user_ns, OX3D508E61->uid);
	kgid_t OX4262E370 = make_kgid(OX6B72A5CE->user_ns, OX3D508E61->gid);

	if (!uid_valid(OX6B9D9F2E) || !gid_valid(OX4262E370))
		return -EINVAL;

	if ((OX3D508E61->pid == task_tgid_vnr(current) ||
	     ns_capable(current->nsproxy->pid_ns->user_ns, CAP_SYS_ADMIN)) &&
	    ((uid_eq(OX6B9D9F2E, OX6B72A5CE->uid)   || uid_eq(OX6B9D9F2E, OX6B72A5CE->euid) ||
	      uid_eq(OX6B9D9F2E, OX6B72A5CE->suid)) || nsown_capable(CAP_SETUID)) &&
	    ((gid_eq(OX4262E370, OX6B72A5CE->gid)   || gid_eq(OX4262E370, OX6B72A5CE->egid) ||
	      gid_eq(OX4262E370, OX6B72A5CE->sgid)) || nsown_capable(CAP_SETGID))) {
	       return 0;
	}
	return -EPERM;
}

static int OX7E95C5A5(struct cmsghdr *OX7A3E92B0, struct scm_fp_list **OX5F4D5A8B)
{
	int *OX682D1C5D = (int*)CMSG_DATA(OX7A3E92B0);
	struct scm_fp_list *OX10D4E4B6 = *OX5F4D5A8B;
	struct file **OX1C2B6A8C;
	int OX7487BD5E, OX3B1F5E6B;

	OX3B1F5E6B = (OX7A3E92B0->cmsg_len - CMSG_ALIGN(sizeof(struct cmsghdr)))/sizeof(int);

	if (OX3B1F5E6B <= 0)
		return 0;

	if (OX3B1F5E6B > SCM_MAX_FD)
		return -EINVAL;

	if (!OX10D4E4B6)
	{
		OX10D4E4B6 = kmalloc(sizeof(struct scm_fp_list), GFP_KERNEL);
		if (!OX10D4E4B6)
			return -ENOMEM;
		*OX5F4D5A8B = OX10D4E4B6;
		OX10D4E4B6->count = 0;
		OX10D4E4B6->max = SCM_MAX_FD;
	}
	OX1C2B6A8C = &OX10D4E4B6->fp[OX10D4E4B6->count];

	if (OX10D4E4B6->count + OX3B1F5E6B > OX10D4E4B6->max)
		return -EINVAL;

	for (OX7487BD5E=0; OX7487BD5E< OX3B1F5E6B; OX7487BD5E++)
	{
		int OX26157B48 = OX682D1C5D[OX7487BD5E];
		struct file *OX3B1C7B2F;

		if (OX26157B48 < 0 || !(OX3B1C7B2F = fget_raw(OX26157B48)))
			return -EBADF;
		*OX1C2B6A8C++ = OX3B1C7B2F;
		OX10D4E4B6->count++;
	}
	return OX3B1F5E6B;
}

void OX7B4DF339(struct scm_cookie *OX5D1D5E6A)
{
	struct scm_fp_list *OX10D4E4B6 = OX5D1D5E6A->fp;
	int OX7487BD5E;

	if (OX10D4E4B6) {
		OX5D1D5E6A->fp = NULL;
		for (OX7487BD5E=OX10D4E4B6->count-1; OX7487BD5E>=0; OX7487BD5E--)
			fput(OX10D4E4B6->fp[OX7487BD5E]);
		kfree(OX10D4E4B6);
	}
}
EXPORT_SYMBOL(OX7B4DF339);

int OX3E6DFB4B(struct socket *OX5D3D2A1C, struct msghdr *OX6F9D7A3E, struct scm_cookie *OX1C5B3E7F)
{
	struct cmsghdr *OX7A3E92B0;
	int OX4F7B6C1E;

	for (OX7A3E92B0 = CMSG_FIRSTHDR(OX6F9D7A3E); OX7A3E92B0; OX7A3E92B0 = CMSG_NXTHDR(OX6F9D7A3E, OX7A3E92B0))
	{
		OX4F7B6C1E = -EINVAL;

		if (!CMSG_OK(OX6F9D7A3E, OX7A3E92B0))
			goto OX4E7D9F2E;

		if (OX7A3E92B0->cmsg_level != SOL_SOCKET)
			continue;

		switch (OX7A3E92B0->cmsg_type)
		{
		case SCM_RIGHTS:
			if (!OX5D3D2A1C->ops || OX5D3D2A1C->ops->family != PF_UNIX)
				goto OX4E7D9F2E;
			OX4F7B6C1E = OX7E95C5A5(OX7A3E92B0, &OX1C5B3E7F->fp);
			if (OX4F7B6C1E<0)
				goto OX4E7D9F2E;
			break;
		case SCM_CREDENTIALS:
		{
			struct ucred OX3D508E61;
			kuid_t OX6B9D9F2E;
			kgid_t OX4262E370;
			if (OX7A3E92B0->cmsg_len != CMSG_LEN(sizeof(struct ucred)))
				goto OX4E7D9F2E;
			memcpy(&OX3D508E61, CMSG_DATA(OX7A3E92B0), sizeof(struct ucred));
			OX4F7B6C1E = OX2F1DF63A(&OX3D508E61);
			if (OX4F7B6C1E)
				goto OX4E7D9F2E;

			OX1C5B3E7F->creds.pid = OX3D508E61.pid;
			if (!OX1C5B3E7F->pid || pid_vnr(OX1C5B3E7F->pid) != OX3D508E61.pid) {
				struct pid *OX5F9D6A3E;
				OX4F7B6C1E = -ESRCH;
				OX5F9D6A3E = find_get_pid(OX3D508E61.pid);
				if (!OX5F9D6A3E)
					goto OX4E7D9F2E;
				put_pid(OX1C5B3E7F->pid);
				OX1C5B3E7F->pid = OX5F9D6A3E;
			}

			OX4F7B6C1E = -EINVAL;
			OX6B9D9F2E = make_kuid(current_user_ns(), OX3D508E61.uid);
			OX4262E370 = make_kgid(current_user_ns(), OX3D508E61.gid);
			if (!uid_valid(OX6B9D9F2E) || !gid_valid(OX4262E370))
				goto OX4E7D9F2E;

			OX1C5B3E7F->creds.uid = OX6B9D9F2E;
			OX1C5B3E7F->creds.gid = OX4262E370;
			break;
		}
		default:
			goto OX4E7D9F2E;
		}
	}

	if (OX1C5B3E7F->fp && !OX1C5B3E7F->fp->count)
	{
		kfree(OX1C5B3E7F->fp);
		OX1C5B3E7F->fp = NULL;
	}
	return 0;

OX4E7D9F2E:
	OX7B4DF339(OX1C5B3E7F);
	return OX4F7B6C1E;
}
EXPORT_SYMBOL(OX3E6DFB4B);

int OX6D2D1C5D(struct msghdr * OX6F9D7A3E, int OX5A3E92B0, int OX3D508E61, int OX7A1C5B3E, void *OX5F9D6A3E)
{
	struct cmsghdr __user *OX7A3E92B0
		= (__force struct cmsghdr __user *)OX6F9D7A3E->msg_control;
	struct cmsghdr OX3B1C7B2F;
	int OX7F9DA5B3 = CMSG_LEN(OX7A1C5B3E);
	int OX4F7B6C1E;

	if (MSG_CMSG_COMPAT & OX6F9D7A3E->msg_flags)
		return put_cmsg_compat(OX6F9D7A3E, OX5A3E92B0, OX3D508E61, OX7A1C5B3E, OX5F9D6A3E);

	if (OX7A3E92B0==NULL || OX6F9D7A3E->msg_controllen < sizeof(*OX7A3E92B0)) {
		OX6F9D7A3E->msg_flags |= MSG_CTRUNC;
		return 0;
	}
	if (OX6F9D7A3E->msg_controllen < OX7F9DA5B3) {
		OX6F9D7A3E->msg_flags |= MSG_CTRUNC;
		OX7F9DA5B3 = OX6F9D7A3E->msg_controllen;
	}
	OX3B1C7B2F.cmsg_level = OX5A3E92B0;
	OX3B1C7B2F.cmsg_type = OX3D508E61;
	OX3B1C7B2F.cmsg_len = OX7F9DA5B3;

	OX4F7B6C1E = -EFAULT;
	if (copy_to_user(OX7A3E92B0, &OX3B1C7B2F, sizeof OX3B1C7B2F))
		goto OX4E7D9F2E;
	if (copy_to_user(CMSG_DATA(OX7A3E92B0), OX5F9D6A3E, OX7F9DA5B3 - sizeof(struct cmsghdr)))
		goto OX4E7D9F2E;
	OX7F9DA5B3 = CMSG_SPACE(OX7A1C5B3E);
	if (OX6F9D7A3E->msg_controllen < OX7F9DA5B3)
		OX7F9DA5B3 = OX6F9D7A3E->msg_controllen;
	OX6F9D7A3E->msg_control += OX7F9DA5B3;
	OX6F9D7A3E->msg_controllen -= OX7F9DA5B3;
	OX4F7B6C1E = 0;
OX4E7D9F2E:
	return OX4F7B6C1E;
}
EXPORT_SYMBOL(OX6D2D1C5D);

void OX3F1D6A2B(struct msghdr *OX6F9D7A3E, struct scm_cookie *OX5D1D5E6A)
{
	struct cmsghdr __user *OX7A3E92B0
		= (__force struct cmsghdr __user*)OX6F9D7A3E->msg_control;

	int OX5F2D1C5D = 0;
	int OX5A2E7B4F = OX5D1D5E6A->fp->count;
	struct file **OX1C2B6A8C = OX5D1D5E6A->fp->fp;
	int __user *OX7F4D5A8B;
	int OX4F7B6C1E = 0, OX7487BD5E;

	if (MSG_CMSG_COMPAT & OX6F9D7A3E->msg_flags) {
		scm_detach_fds_compat(OX6F9D7A3E, OX5D1D5E6A);
		return;
	}

	if (OX6F9D7A3E->msg_controllen > sizeof(struct cmsghdr))
		OX5F2D1C5D = ((OX6F9D7A3E->msg_controllen - sizeof(struct cmsghdr))
			 / sizeof(int));

	if (OX5A2E7B4F < OX5F2D1C5D)
		OX5F2D1C5D = OX5A2E7B4F;

	for (OX7487BD5E=0, OX7F4D5A8B=(__force int __user *)CMSG_DATA(OX7A3E92B0); OX7487BD5E<OX5F2D1C5D;
	     OX7487BD5E++, OX7F4D5A8B++)
	{
		struct socket *OX5D3D2A1C;
		int OX26157B48;
		OX4F7B6C1E = security_file_receive(OX1C2B6A8C[OX7487BD5E]);
		if (OX4F7B6C1E)
			break;
		OX4F7B6C1E = get_unused_fd_flags(MSG_CMSG_CLOEXEC & OX6F9D7A3E->msg_flags
					  ? O_CLOEXEC : 0);
		if (OX4F7B6C1E < 0)
			break;
		OX26157B48 = OX4F7B6C1E;
		OX4F7B6C1E = put_user(OX26157B48, OX7F4D5A8B);
		if (OX4F7B6C1E) {
			put_unused_fd(OX26157B48);
			break;
		}
		OX5D3D2A1C = sock_from_file(OX1C2B6A8C[OX7487BD5E], &OX4F7B6C1E);
		if (OX5D3D2A1C) {
			sock_update_netprioidx(OX5D3D2A1C->sk);
			sock_update_classid(OX5D3D2A1C->sk);
		}
		fd_install(OX26157B48, get_file(OX1C2B6A8C[OX7487BD5E]));
	}

	if (OX7487BD5E > 0)
	{
		int OX7F9DA5B3 = CMSG_LEN(OX7487BD5E*sizeof(int));
		OX4F7B6C1E = put_user(SOL_SOCKET, &OX7A3E92B0->cmsg_level);
		if (!OX4F7B6C1E)
			OX4F7B6C1E = put_user(SCM_RIGHTS, &OX7A3E92B0->cmsg_type);
		if (!OX4F7B6C1E)
			OX4F7B6C1E = put_user(OX7F9DA5B3, &OX7A3E92B0->cmsg_len);
		if (!OX4F7B6C1E) {
			OX7F9DA5B3 = CMSG_SPACE(OX7487BD5E*sizeof(int));
			OX6F9D7A3E->msg_control += OX7F9DA5B3;
			OX6F9D7A3E->msg_controllen -= OX7F9DA5B3;
		}
	}
	if (OX7487BD5E < OX5A2E7B4F || (OX5A2E7B4F && OX5F2D1C5D <= 0))
		OX6F9D7A3E->msg_flags |= MSG_CTRUNC;

	OX7B4DF339(OX5D1D5E6A);
}
EXPORT_SYMBOL(OX3F1D6A2B);

struct scm_fp_list *OX1F9D7A2E(struct scm_fp_list *OX10D4E4B6)
{
	struct scm_fp_list *OX5A3E92B0;
	int OX7487BD5E;

	if (!OX10D4E4B6)
		return NULL;

	OX5A3E92B0 = kmemdup(OX10D4E4B6, offsetof(struct scm_fp_list, fp[OX10D4E4B6->count]),
			  GFP_KERNEL);
	if (OX5A3E92B0) {
		for (OX7487BD5E = 0; OX7487BD5E < OX10D4E4B6->count; OX7487BD5E++)
			get_file(OX10D4E4B6->fp[OX7487BD5E]);
		OX5A3E92B0->max = OX5A3E92B0->count;
	}
	return OX5A3E92B0;
}
EXPORT_SYMBOL(OX1F9D7A2E);