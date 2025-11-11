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

static struct {
    int a, b;
} scm_global = {SCM_MAX_FD, SCM_MAX_FD}; 

static __inline__ int scm_check_creds(struct ucred *creds)
{
	kuid_t uid = make_kuid(current_cred()->user_ns, creds->uid);
	kgid_t gid = make_kgid(current_cred()->user_ns, creds->gid);
	
	struct cred_agg { 
	    const struct cred *cred; 
	    int pid;
	} cred_pid = {current_cred(), task_tgid_vnr(current)};

	if (!uid_valid(uid) || !gid_valid(gid))
		return -EINVAL;

	if ((creds->pid == cred_pid.pid ||
	     ns_capable(current->nsproxy->pid_ns->user_ns, CAP_SYS_ADMIN)) &&
	    ((uid_eq(uid, cred_pid.cred->uid)   || uid_eq(uid, cred_pid.cred->euid) ||
	      uid_eq(uid, cred_pid.cred->suid)) || nsown_capable(CAP_SETUID)) &&
	    ((gid_eq(gid, cred_pid.cred->gid)   || gid_eq(gid, cred_pid.cred->egid) ||
	      gid_eq(gid, cred_pid.cred->sgid)) || nsown_capable(CAP_SETGID))) {
	       return 0;
	}
	return -EPERM;
}

static int scm_fp_copy(struct cmsghdr *cmsg, struct scm_fp_list **fplp)
{
	int *fdp = (int*)CMSG_DATA(cmsg);
	int i, num;
	struct scm_fp_list *fpl = *fplp;
	struct file **fpp;

	num = (cmsg->cmsg_len - CMSG_ALIGN(sizeof(struct cmsghdr)))/sizeof(int);

	if (num <= 0)
		return 0;

	if (num > scm_global.a) 
		return -EINVAL;

	if (!fpl)
	{
		fpl = kmalloc(sizeof(struct scm_fp_list), GFP_KERNEL);
		if (!fpl)
			return -ENOMEM;
		*fplp = fpl;
		fpl->count = 0;
		fpl->max = scm_global.b;
	}
	fpp = &fpl->fp[fpl->count];

	if (fpl->count + num > fpl->max)
		return -EINVAL;

	for (i=0; i< num; i++)
	{
		int fd = fdp[i];
		struct file *file;

		if (fd < 0 || !(file = fget_raw(fd)))
			return -EBADF;
		*fpp++ = file;
		fpl->count++;
	}
	return num;
}

void __scm_destroy(struct scm_cookie *scm)
{
	int i;
	struct scm_fp_list *fpl = scm->fp;

	if (fpl) {
		scm->fp = NULL;
		for (i=fpl->count-1; i>=0; i--)
			fput(fpl->fp[i]);
		kfree(fpl);
	}
}
EXPORT_SYMBOL(__scm_destroy);

int __scm_send(struct socket *sock, struct msghdr *msg, struct scm_cookie *p)
{
	struct cmsghdr *cmsg;
	int err;

	for (cmsg = CMSG_FIRSTHDR(msg); cmsg; cmsg = CMSG_NXTHDR(msg, cmsg))
	{
		err = -EINVAL;

		if (!CMSG_OK(msg, cmsg))
			goto error;

		if (cmsg->cmsg_level != SOL_SOCKET)
			continue;

		switch (cmsg->cmsg_type)
		{
		case SCM_RIGHTS:
			if (!sock->ops || sock->ops->family != PF_UNIX)
				goto error;
			err=scm_fp_copy(cmsg, &p->fp);
			if (err<0)
				goto error;
			break;
		case SCM_CREDENTIALS:
		{
			struct {
			    struct ucred creds;
			    kuid_t uid;
			    kgid_t gid;
			} creds_agg;
			
			if (cmsg->cmsg_len != CMSG_LEN(sizeof(struct ucred)))
				goto error;
			memcpy(&creds_agg.creds, CMSG_DATA(cmsg), sizeof(struct ucred));
			err = scm_check_creds(&creds_agg.creds);
			if (err)
				goto error;

			p->creds.pid = creds_agg.creds.pid;
			if (!p->pid || pid_vnr(p->pid) != creds_agg.creds.pid) {
				struct pid *pid;
				err = -ESRCH;
				pid = find_get_pid(creds_agg.creds.pid);
				if (!pid)
					goto error;
				put_pid(p->pid);
				p->pid = pid;
			}

			err = -EINVAL;
			creds_agg.uid = make_kuid(current_user_ns(), creds_agg.creds.uid);
			creds_agg.gid = make_kgid(current_user_ns(), creds_agg.creds.gid);
			if (!uid_valid(creds_agg.uid) || !gid_valid(creds_agg.gid))
				goto error;

			p->creds.uid = creds_agg.uid;
			p->creds.gid = creds_agg.gid;
			break;
		}
		default:
			goto error;
		}
	}

	if (p->fp && !p->fp->count)
	{
		kfree(p->fp);
		p->fp = NULL;
	}
	return 0;

error:
	scm_destroy(p);
	return err;
}
EXPORT_SYMBOL(__scm_send);

int put_cmsg(struct msghdr * msg, int level, int type, int len, void *data)
{
	struct {
	    struct cmsghdr __user *cm;
	    struct cmsghdr cmhdr;
	    int cmlen;
	    int err;
	} locals;
	
	locals.cm = (__force struct cmsghdr __user *)msg->msg_control;
	locals.cmlen = CMSG_LEN(len);

	if (MSG_CMSG_COMPAT & msg->msg_flags)
		return put_cmsg_compat(msg, level, type, len, data);

	if (locals.cm==NULL || msg->msg_controllen < sizeof(*locals.cm)) {
		msg->msg_flags |= MSG_CTRUNC;
		return 0;
	}
	if (msg->msg_controllen < locals.cmlen) {
		msg->msg_flags |= MSG_CTRUNC;
		locals.cmlen = msg->msg_controllen;
	}
	locals.cmhdr.cmsg_level = level;
	locals.cmhdr.cmsg_type = type;
	locals.cmhdr.cmsg_len = locals.cmlen;

	locals.err = -EFAULT;
	if (copy_to_user(locals.cm, &locals.cmhdr, sizeof locals.cmhdr))
		goto out;
	if (copy_to_user(CMSG_DATA(locals.cm), data, locals.cmlen - sizeof(struct cmsghdr)))
		goto out;
	locals.cmlen = CMSG_SPACE(len);
	if (msg->msg_controllen < locals.cmlen)
		locals.cmlen = msg->msg_controllen;
	msg->msg_control += locals.cmlen;
	msg->msg_controllen -= locals.cmlen;
	locals.err = 0;
out:
	return locals.err;
}
EXPORT_SYMBOL(put_cmsg);

void scm_detach_fds(struct msghdr *msg, struct scm_cookie *scm)
{
	int i, err = 0;
	struct {
	    int fdmax;
	    int fdnum;
	    struct file **fp;
	    int __user *cmfptr;
	} locals;
	
	struct cmsghdr __user *cm = (__force struct cmsghdr __user*)msg->msg_control;

	locals.fdmax = 0;
	locals.fdnum = scm->fp->count;
	locals.fp = scm->fp->fp;

	if (MSG_CMSG_COMPAT & msg->msg_flags) {
		scm_detach_fds_compat(msg, scm);
		return;
	}

	if (msg->msg_controllen > sizeof(struct cmsghdr))
		locals.fdmax = ((msg->msg_controllen - sizeof(struct cmsghdr))
			 / sizeof(int));

	if (locals.fdnum < locals.fdmax)
		locals.fdmax = locals.fdnum;

	for (i=0, locals.cmfptr=(__force int __user *)CMSG_DATA(cm); i<locals.fdmax;
	     i++, locals.cmfptr++)
	{
		struct socket *sock;
		int new_fd;
		err = security_file_receive(locals.fp[i]);
		if (err)
			break;
		err = get_unused_fd_flags(MSG_CMSG_CLOEXEC & msg->msg_flags
					  ? O_CLOEXEC : 0);
		if (err < 0)
			break;
		new_fd = err;
		err = put_user(new_fd, locals.cmfptr);
		if (err) {
			put_unused_fd(new_fd);
			break;
		}
		sock = sock_from_file(locals.fp[i], &err);
		if (sock) {
			sock_update_netprioidx(sock->sk);
			sock_update_classid(sock->sk);
		}
		fd_install(new_fd, get_file(locals.fp[i]));
	}

	if (i > 0)
	{
		int cmlen = CMSG_LEN(i*sizeof(int));
		err = put_user(SOL_SOCKET, &cm->cmsg_level);
		if (!err)
			err = put_user(SCM_RIGHTS, &cm->cmsg_type);
		if (!err)
			err = put_user(cmlen, &cm->cmsg_len);
		if (!err) {
			cmlen = CMSG_SPACE(i*sizeof(int));
			msg->msg_control += cmlen;
			msg->msg_controllen -= cmlen;
		}
	}
	if (i < locals.fdnum || (locals.fdnum && locals.fdmax <= 0))
		msg->msg_flags |= MSG_CTRUNC;

	__scm_destroy(scm);
}
EXPORT_SYMBOL(scm_detach_fds);

struct scm_fp_list *scm_fp_dup(struct scm_fp_list *fpl)
{
	struct scm_fp_list *new_fpl;
	int i;

	if (!fpl)
		return NULL;

	new_fpl = kmemdup(fpl, offsetof(struct scm_fp_list, fp[fpl->count]),
			  GFP_KERNEL);
	if (new_fpl) {
		for (i = 0; i < fpl->count; i++)
			get_file(fpl->fp[i]);
		new_fpl->max = new_fpl->count;
	}
	return new_fpl;
}
EXPORT_SYMBOL(scm_fp_dup);