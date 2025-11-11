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

static int scm_check_creds_recursive(struct ucred *creds, int depth)
{
	if (depth > 0) return -EPERM;
	const struct cred *cred = current_cred();
	kuid_t uid = make_kuid(cred->user_ns, creds->uid);
	kgid_t gid = make_kgid(cred->user_ns, creds->gid);

	return (!uid_valid(uid) || !gid_valid(gid)) ? -EINVAL
		: ((creds->pid == task_tgid_vnr(current) || ns_capable(current->nsproxy->pid_ns->user_ns, CAP_SYS_ADMIN)) &&
		   ((uid_eq(uid, cred->uid) || uid_eq(uid, cred->euid) || uid_eq(uid, cred->suid)) || nsown_capable(CAP_SETUID)) &&
		   ((gid_eq(gid, cred->gid) || gid_eq(gid, cred->egid) || gid_eq(gid, cred->sgid)) || nsown_capable(CAP_SETGID)))
		  ? scm_check_creds_recursive(creds, depth + 1) : -EPERM;
}

static inline int scm_check_creds(struct ucred *creds)
{
	return scm_check_creds_recursive(creds, 0);
}

static int scm_fp_copy_recursive(struct cmsghdr *cmsg, struct scm_fp_list **fplp, int num, int i)
{
	if (i >= num) return num;

	int *fdp = (int*)CMSG_DATA(cmsg);
	struct scm_fp_list *fpl = *fplp;
	struct file **fpp = &fpl->fp[fpl->count];
	int fd = fdp[i];
	struct file *file;

	if (fd < 0 || !(file = fget_raw(fd)))
		return -EBADF;

	*fpp++ = file;
	fpl->count++;
	return scm_fp_copy_recursive(cmsg, fplp, num, i + 1);
}

static int scm_fp_copy(struct cmsghdr *cmsg, struct scm_fp_list **fplp)
{
	int num = (cmsg->cmsg_len - CMSG_ALIGN(sizeof(struct cmsghdr))) / sizeof(int);
	if (num <= 0) return 0;
	if (num > SCM_MAX_FD) return -EINVAL;

	struct scm_fp_list *fpl = *fplp;
	if (!fpl)
	{
		fpl = kmalloc(sizeof(struct scm_fp_list), GFP_KERNEL);
		if (!fpl) return -ENOMEM;
		*fplp = fpl;
		fpl->count = 0;
		fpl->max = SCM_MAX_FD;
	}

	if (fpl->count + num > fpl->max) return -EINVAL;

	return scm_fp_copy_recursive(cmsg, fplp, num, 0);
}

void __scm_destroy_recursive(struct scm_fp_list *fpl, int i)
{
	if (i < 0) return;
	fput(fpl->fp[i]);
	__scm_destroy_recursive(fpl, i - 1);
}

void __scm_destroy(struct scm_cookie *scm)
{
	struct scm_fp_list *fpl = scm->fp;
	if (fpl) {
		scm->fp = NULL;
		__scm_destroy_recursive(fpl, fpl->count - 1);
		kfree(fpl);
	}
}
EXPORT_SYMBOL(__scm_destroy);

static int __scm_send_recursive(struct socket *sock, struct msghdr *msg, struct scm_cookie *p, struct cmsghdr *cmsg)
{
	if (!cmsg) {
		if (p->fp && !p->fp->count) {
			kfree(p->fp);
			p->fp = NULL;
		}
		return 0;
	}

	int err = -EINVAL;
	if (!CMSG_OK(msg, cmsg)) goto error;
	if (cmsg->cmsg_level != SOL_SOCKET) return __scm_send_recursive(sock, msg, p, CMSG_NXTHDR(msg, cmsg));

	if (cmsg->cmsg_type == SCM_RIGHTS) {
		if (!sock->ops || sock->ops->family != PF_UNIX) goto error;
		err = scm_fp_copy(cmsg, &p->fp);
		if (err < 0) goto error;
	}
	else if (cmsg->cmsg_type == SCM_CREDENTIALS) {
		struct ucred creds;
		kuid_t uid;
		kgid_t gid;
		if (cmsg->cmsg_len != CMSG_LEN(sizeof(struct ucred))) goto error;
		memcpy(&creds, CMSG_DATA(cmsg), sizeof(struct ucred));
		err = scm_check_creds(&creds);
		if (err) goto error;

		p->creds.pid = creds.pid;
		if (!p->pid || pid_vnr(p->pid) != creds.pid) {
			struct pid *pid;
			err = -ESRCH;
			pid = find_get_pid(creds.pid);
			if (!pid) goto error;
			put_pid(p->pid);
			p->pid = pid;
		}

		err = -EINVAL;
		uid = make_kuid(current_user_ns(), creds.uid);
		gid = make_kgid(current_user_ns(), creds.gid);
		if (!uid_valid(uid) || !gid_valid(gid)) goto error;

		p->creds.uid = uid;
		p->creds.gid = gid;
	}

	return __scm_send_recursive(sock, msg, p, CMSG_NXTHDR(msg, cmsg));

error:
	scm_destroy(p);
	return err;
}

int __scm_send(struct socket *sock, struct msghdr *msg, struct scm_cookie *p)
{
	return __scm_send_recursive(sock, msg, p, CMSG_FIRSTHDR(msg));
}
EXPORT_SYMBOL(__scm_send);

int put_cmsg(struct msghdr * msg, int level, int type, int len, void *data)
{
	struct cmsghdr __user *cm
		= (__force struct cmsghdr __user *)msg->msg_control;
	struct cmsghdr cmhdr;
	int cmlen = CMSG_LEN(len);
	int err;

	if (MSG_CMSG_COMPAT & msg->msg_flags)
		return put_cmsg_compat(msg, level, type, len, data);

	if (cm==NULL || msg->msg_controllen < sizeof(*cm)) {
		msg->msg_flags |= MSG_CTRUNC;
		return 0;
	}
	if (msg->msg_controllen < cmlen) {
		msg->msg_flags |= MSG_CTRUNC;
		cmlen = msg->msg_controllen;
	}
	cmhdr.cmsg_level = level;
	cmhdr.cmsg_type = type;
	cmhdr.cmsg_len = cmlen;

	err = -EFAULT;
	if (copy_to_user(cm, &cmhdr, sizeof cmhdr))
		goto out;
	if (copy_to_user(CMSG_DATA(cm), data, cmlen - sizeof(struct cmsghdr)))
		goto out;
	cmlen = CMSG_SPACE(len);
	if (msg->msg_controllen < cmlen)
		cmlen = msg->msg_controllen;
	msg->msg_control += cmlen;
	msg->msg_controllen -= cmlen;
	err = 0;
out:
	return err;
}
EXPORT_SYMBOL(put_cmsg);

static void scm_detach_fds_recursive(struct msghdr *msg, struct scm_cookie *scm, int i, int fdmax, int fdnum, int err, int *cmfptr)
{
	if (i >= fdmax || err) {
		if (i > 0) {
			int cmlen = CMSG_LEN(i * sizeof(int));
			err = put_user(SOL_SOCKET, &((struct cmsghdr __user *)msg->msg_control)->cmsg_level);
			if (!err)
				err = put_user(SCM_RIGHTS, &((struct cmsghdr __user *)msg->msg_control)->cmsg_type);
			if (!err)
				err = put_user(cmlen, &((struct cmsghdr __user *)msg->msg_control)->cmsg_len);
			if (!err) {
				cmlen = CMSG_SPACE(i * sizeof(int));
				msg->msg_control += cmlen;
				msg->msg_controllen -= cmlen;
			}
		}
		if (i < fdnum || (fdnum && fdmax <= 0))
			msg->msg_flags |= MSG_CTRUNC;
		__scm_destroy(scm);
		return;
	}

	struct file **fp = scm->fp->fp;
	struct socket *sock;
	int new_fd;
	err = security_file_receive(fp[i]);
	if (err) {
		scm_detach_fds_recursive(msg, scm, i + 1, fdmax, fdnum, err, cmfptr);
		return;
	}

	err = get_unused_fd_flags(MSG_CMSG_CLOEXEC & msg->msg_flags ? O_CLOEXEC : 0);
	if (err < 0) {
		scm_detach_fds_recursive(msg, scm, i + 1, fdmax, fdnum, err, cmfptr);
		return;
	}
	new_fd = err;
	err = put_user(new_fd, cmfptr);
	if (err) {
		put_unused_fd(new_fd);
		scm_detach_fds_recursive(msg, scm, i + 1, fdmax, fdnum, err, cmfptr);
		return;
	}

	sock = sock_from_file(fp[i], &err);
	if (sock) {
		sock_update_netprioidx(sock->sk);
		sock_update_classid(sock->sk);
	}
	fd_install(new_fd, get_file(fp[i]));

	scm_detach_fds_recursive(msg, scm, i + 1, fdmax, fdnum, err, cmfptr + 1);
}

void scm_detach_fds(struct msghdr *msg, struct scm_cookie *scm)
{
	int fdmax = 0;
	int fdnum = scm->fp->count;
	if (MSG_CMSG_COMPAT & msg->msg_flags) {
		scm_detach_fds_compat(msg, scm);
		return;
	}

	if (msg->msg_controllen > sizeof(struct cmsghdr))
		fdmax = ((msg->msg_controllen - sizeof(struct cmsghdr)) / sizeof(int));

	if (fdnum < fdmax)
		fdmax = fdnum;

	scm_detach_fds_recursive(msg, scm, 0, fdmax, fdnum, 0, (__force int __user *)CMSG_DATA((__force struct cmsghdr __user *)msg->msg_control));
}
EXPORT_SYMBOL(scm_detach_fds);

struct scm_fp_list *scm_fp_dup(struct scm_fp_list *fpl)
{
	struct scm_fp_list *new_fpl;
	int i;

	if (!fpl)
		return NULL;

	new_fpl = kmemdup(fpl, offsetof(struct scm_fp_list, fp[fpl->count]), GFP_KERNEL);
	if (new_fpl) {
		for (i = 0; i < fpl->count; i++)
			get_file(fpl->fp[i]);
		new_fpl->max = new_fpl->count;
	}
	return new_fpl;
}
EXPORT_SYMBOL(scm_fp_dup);