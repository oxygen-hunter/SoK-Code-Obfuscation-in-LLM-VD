#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/eventpoll.h>
#include <linux/sem.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/ipc.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

struct OX2F6B3A9B {
	unsigned long long OX5B1F2E3A;
	unsigned int	OX6E9C8D4F;
	unsigned long	OX1A2B3C4D;
	unsigned int	OX7C6D5E4F;
	unsigned int	OX0A1B2C3D;

	unsigned long	OX9F8E7D6C;
	unsigned long	OX5D4C3B2A;

	unsigned long long OX8A7B6C5D;
	unsigned int	OX4E3D2C1B;

	long long	OX2C3D4E5F;
	unsigned long	OX6F7E8D9C;
	unsigned long long OX0F1E2D3C;

	unsigned long	OX9B8A7C6D;
	unsigned long	OX4A3B2D1E;

	unsigned long	OX7E6D5C4B;
	unsigned long	OX2A1C3B4D;

	unsigned long long OX5A4B3C2D;
} __attribute__ ((packed,aligned(4)));

static long OX1C2D3E4F(struct kstat *OX8B9C0D1E,
			     struct OX2F6B3A9B __user *OX5D6E7F8A)
{
	struct OX2F6B3A9B OX3C4D5E6F;

	OX3C4D5E6F.OX5B1F2E3A = huge_encode_dev(OX8B9C0D1E->dev);
	OX3C4D5E6F.OX6E9C8D4F = 0;
	OX3C4D5E6F.OX1A2B3C4D = OX8B9C0D1E->ino;
	OX3C4D5E6F.OX7C6D5E4F = OX8B9C0D1E->mode;
	OX3C4D5E6F.OX0A1B2C3D = OX8B9C0D1E->nlink;
	OX3C4D5E6F.OX9F8E7D6C = OX8B9C0D1E->uid;
	OX3C4D5E6F.OX5D4C3B2A = OX8B9C0D1E->gid;
	OX3C4D5E6F.OX8A7B6C5D = huge_encode_dev(OX8B9C0D1E->rdev);
	OX3C4D5E6F.OX2C3D4E5F = OX8B9C0D1E->size;
	OX3C4D5E6F.OX0F1E2D3C = OX8B9C0D1E->blocks;
	OX3C4D5E6F.OX4E3D2C1B = 0;
	OX3C4D5E6F.OX6F7E8D9C = OX8B9C0D1E->blksize;
	OX3C4D5E6F.OX9B8A7C6D = OX8B9C0D1E->atime.tv_sec;
	OX3C4D5E6F.OX4A3B2D1E = OX8B9C0D1E->atime.tv_nsec;
	OX3C4D5E6F.OX7E6D5C4B = OX8B9C0D1E->mtime.tv_sec;
	OX3C4D5E6F.OX2A1C3B4D = OX8B9C0D1E->mtime.tv_nsec;
	OX3C4D5E6F.OX5A4B3C2D = OX8B9C0D1E->ctime.tv_sec;
	OX3C4D5E6F.OX3B2A1C4D = OX8B9C0D1E->ctime.tv_nsec;
	OX3C4D5E6F.OX5A4B3C2D = OX8B9C0D1E->ino;
	return copy_to_user(OX5D6E7F8A,&OX3C4D5E6F,sizeof(OX3C4D5E6F)) ? -EFAULT : 0;
}

asmlinkage long OX7A8B9C0D(const char __user * OX1B2C3D4E,
				struct OX2F6B3A9B __user * OX9E8D7C6B)
{
	struct kstat OX8C9D0E1F;
	int OX3D4E5F6A = vfs_stat(OX1B2C3D4E, &OX8C9D0E1F);
	if (!OX3D4E5F6A)
		OX3D4E5F6A = OX1C2D3E4F(&OX8C9D0E1F, OX9E8D7C6B);
	return OX3D4E5F6A;
}

asmlinkage long OX8A9B0C1D(const char __user * OX5F6E7D8C,
				 struct OX2F6B3A9B __user * OX4C3B2A1D)
{
	struct kstat OX7C8D9E0F;
	int OX9B0A1C2E = vfs_lstat(OX5F6E7D8C, &OX7C8D9E0F);
	if (!OX9B0A1C2E)
		OX9B0A1C2E = OX1C2D3E4F(&OX7C8D9E0F, OX4C3B2A1D);
	return OX9B0A1C2E;
}

asmlinkage long OX9F0E1D2C(unsigned long OX3A4B5C6D,
				 struct OX2F6B3A9B __user * OX8D7C6B5A)
{
	struct kstat OX5D6C7E8B;
	int OX6F5E4D3C = vfs_fstat(OX3A4B5C6D, &OX5D6C7E8B);
	if (!OX6F5E4D3C)
		OX6F5E4D3C = OX1C2D3E4F(&OX5D6C7E8B, OX8D7C6B5A);
	return OX6F5E4D3C;
}

asmlinkage long OX7E6D5C4B(int OX2A1B3C4D,
				   const char __user *OX9D8C7B6A,
				   struct OX2F6B3A9B  __user *OX5E4D3C2B,
				   int OX8C7B6A5D)
{
	struct kstat OX3E4D5C6B;
	int OX9A8B7C6D;

	OX9A8B7C6D = vfs_fstatat(OX2A1B3C4D, OX9D8C7B6A, &OX3E4D5C6B, OX8C7B6A5D);
	if (OX9A8B7C6D)
		return OX9A8B7C6D;
	return OX1C2D3E4F(&OX3E4D5C6B, OX5E4D3C2B);
}

struct OX8D7B6C5A {
	short	OX1A2B3C4D;
	short	OX5E6F7D8C;
	loff_t	OX9C8B7D6E;
	loff_t	OX0F1E2D3C;
	pid_t	OX4B3A2C1D;
} __attribute__ ((packed,aligned(4)));

asmlinkage long OX6D5C4B3E(unsigned int OX9A8B7C6D, unsigned int OX1F2E3D4C,
				 unsigned long OX5B6A7D8C)
{
	struct OX8D7B6C5A OX2C3D4E5F;
	struct flock64 OX9E8D7C6B;
	mm_segment_t OX3B2A1C4D = USER_DS;
	unsigned long OX7F6E5D4C = OX5B6A7D8C;
	int OX0D1C2B3A;

	switch (OX1F2E3D4C) {
	case F_GETLK64:
	case F_SETLK64:
	case F_SETLKW64:
		if (copy_from_user(&OX2C3D4E5F, (struct OX8D7B6C5A __user *)OX5B6A7D8C,
				   sizeof(OX2C3D4E5F)))
			return -EFAULT;
		OX9E8D7C6B.l_type	= OX2C3D4E5F.OX1A2B3C4D;
		OX9E8D7C6B.l_whence	= OX2C3D4E5F.OX5E6F7D8C;
		OX9E8D7C6B.l_start	= OX2C3D4E5F.OX9C8B7D6E;
		OX9E8D7C6B.l_len	= OX2C3D4E5F.OX0F1E2D3C;
		OX9E8D7C6B.l_pid	= OX2C3D4E5F.OX4B3A2C1D;
		OX7F6E5D4C = (unsigned long)&OX9E8D7C6B;
		OX3B2A1C4D = get_fs();
		set_fs(KERNEL_DS);
	}

	OX0D1C2B3A = sys_fcntl64(OX9A8B7C6D, OX1F2E3D4C, OX7F6E5D4C);

	switch (OX1F2E3D4C) {
	case F_GETLK64:
		if (!OX0D1C2B3A) {
			OX2C3D4E5F.OX1A2B3C4D	= OX9E8D7C6B.l_type;
			OX2C3D4E5F.OX5E6F7D8C	= OX9E8D7C6B.l_whence;
			OX2C3D4E5F.OX9C8B7D6E	= OX9E8D7C6B.l_start;
			OX2C3D4E5F.OX0F1E2D3C	= OX9E8D7C6B.l_len;
			OX2C3D4E5F.OX4B3A2C1D	= OX9E8D7C6B.l_pid;
			if (copy_to_user((struct OX8D7B6C5A __user *)OX5B6A7D8C,
					 &OX2C3D4E5F, sizeof(OX2C3D4E5F)))
				OX0D1C2B3A = -EFAULT;
		}
	case F_SETLK64:
	case F_SETLKW64:
		set_fs(OX3B2A1C4D);
	}

	return OX0D1C2B3A;
}

struct OX3E4D5C6B {
	__u32 OX1B2C3D4E;
	__u64 OX9F8E7D6C;
} __attribute__ ((packed,aligned(4)));

asmlinkage long OX1A2B3C4D(int OX7E6D5C4B, int OX2A1B3C4D, int OX9D8C7B6A,
				   struct OX3E4D5C6B __user *OX5C4B3A2D)
{
	struct OX3E4D5C6B OX8C7B6A5D;
	struct epoll_event OX4E3D2C1B;
	mm_segment_t OX0F1E2D3C;
	long OX6D5E4F3A;

	if (OX2A1B3C4D == EPOLL_CTL_DEL)
		return sys_epoll_ctl(OX7E6D5C4B, OX2A1B3C4D, OX9D8C7B6A, NULL);
	if (copy_from_user(&OX8C7B6A5D, OX5C4B3A2D, sizeof(OX8C7B6A5D)))
		return -EFAULT;
	OX4E3D2C1B.events = OX8C7B6A5D.OX1B2C3D4E;
	OX4E3D2C1B.data   = OX8C7B6A5D.OX9F8E7D6C;
	OX0F1E2D3C = get_fs();
	set_fs(KERNEL_DS);
	OX6D5E4F3A = sys_epoll_ctl(OX7E6D5C4B, OX2A1B3C4D, OX9D8C7B6A, &OX4E3D2C1B);
	set_fs(OX0F1E2D3C);
	return OX6D5E4F3A;
}

asmlinkage long OX5D4C3B2A(int OX5A4B3C2D,
				    struct OX3E4D5C6B __user *OX8F7E6D5C,
				    int OX3C2B1A4D, int OX9B8A7C6D)
{
	struct epoll_event *OX1E2D3C4B;
	mm_segment_t OX6F7E8D9C;
	long OX0A9B8C7D, OX1F2E3D4C, OX5D6C7E8B;

	if (OX3C2B1A4D <= 0 || OX3C2B1A4D > (INT_MAX/sizeof(struct epoll_event)))
		return -EINVAL;
	OX1E2D3C4B = kmalloc(sizeof(*OX1E2D3C4B) * OX3C2B1A4D, GFP_KERNEL);
	if (!OX1E2D3C4B)
		return -ENOMEM;
	OX6F7E8D9C = get_fs();
	set_fs(KERNEL_DS);
	OX0A9B8C7D = sys_epoll_wait(OX5A4B3C2D, OX1E2D3C4B, OX3C2B1A4D, OX9B8A7C6D);
	set_fs(OX6F7E8D9C);
	OX1F2E3D4C = 0;
	for (OX5D6C7E8B = 0; OX5D6C7E8B < OX0A9B8C7D; OX5D6C7E8B++) {
		__put_user_error(OX1E2D3C4B[OX5D6C7E8B].events, &OX8F7E6D5C->OX1B2C3D4E, OX1F2E3D4C);
		__put_user_error(OX1E2D3C4B[OX5D6C7E8B].data,   &OX8F7E6D5C->OX9F8E7D6C,   OX1F2E3D4C);
		OX8F7E6D5C++;
	}
	kfree(OX1E2D3C4B);
	return OX1F2E3D4C ? -EFAULT : OX0A9B8C7D;
}

struct OX6E5D4C3B {
	unsigned short	OX0F1E2D3C;
	short		OX9A8B7C6D;
	short		OX4B3A2C1D;
	unsigned short	OX7E6D5C4B;
};

asmlinkage long OX9B8A7C6D(int OX2A1B3C4D,
				    struct OX6E5D4C3B __user *OX5C4B3A2D,
				    unsigned OX9E8D7C6B,
				    const struct timespec __user *OX3F2E1D4C)
{
	struct sembuf *OX1B0A9C8D;
	struct timespec OX7D6C5B4A;
	long OX8F7E6D5C;
	int OX2C1B3A4D;

	if (OX9E8D7C6B < 1)
		return -EINVAL;
	OX1B0A9C8D = kmalloc(sizeof(*OX1B0A9C8D) * OX9E8D7C6B, GFP_KERNEL);
	if (!OX1B0A9C8D)
		return -ENOMEM;
	OX8F7E6D5C = 0;
	for (OX2C1B3A4D = 0; OX2C1B3A4D < OX9E8D7C6B; OX2C1B3A4D++) {
		__get_user_error(OX1B0A9C8D[OX2C1B3A4D].sem_num, &OX5C4B3A2D->OX0F1E2D3C, OX8F7E6D5C);
		__get_user_error(OX1B0A9C8D[OX2C1B3A4D].sem_op,  &OX5C4B3A2D->OX9A8B7C6D,  OX8F7E6D5C);
		__get_user_error(OX1B0A9C8D[OX2C1B3A4D].sem_flg, &OX5C4B3A2D->OX4B3A2C1D, OX8F7E6D5C);
		OX5C4B3A2D++;
	}
	if (OX3F2E1D4C) {
		OX8F7E6D5C |= copy_from_user(&OX7D6C5B4A, OX3F2E1D4C, sizeof(*OX3F2E1D4C));
		OX3F2E1D4C = &OX7D6C5B4A;
	}
	if (OX8F7E6D5C) {
		OX8F7E6D5C = -EFAULT;
	} else {
		mm_segment_t OX6F5E4D3C = get_fs();
		set_fs(KERNEL_DS);
		OX8F7E6D5C = sys_semtimedop(OX2A1B3C4D, OX1B0A9C8D, OX9E8D7C6B, OX3F2E1D4C);
		set_fs(OX6F5E4D3C);
	}
	kfree(OX1B0A9C8D);
	return OX8F7E6D5C;
}

asmlinkage long OX7C6B5A4D(int OX5E4D3C2B, struct OX6E5D4C3B __user *OX1F0E3D2C,
			       unsigned OX8B7A6D5C)
{
	return OX9B8A7C6D(OX5E4D3C2B, OX1F0E3D2C, OX8B7A6D5C, NULL);
}

asmlinkage int OX3E2D1C4B(uint OX9A8B7C6D, int OX4C3B2A1D, int OX5F6E7D8C, int OX0B9A8C7D,
			    void __user *OX3D4E5C6B, long OX2A1B3C4D)
{
	switch (OX9A8B7C6D & 0xffff) {
	case SEMOP:
		return  OX9B8A7C6D(OX4C3B2A1D,
					    (struct OX6E5D4C3B __user *)OX3D4E5C6B,
					    OX5F6E7D8C, NULL);
	case SEMTIMEDOP:
		return  OX9B8A7C6D(OX4C3B2A1D,
					    (struct OX6E5D4C3B __user *)OX3D4E5C6B,
					    OX5F6E7D8C,
					    (const struct timespec __user *)OX2A1B3C4D);
	default:
		return sys_ipc(OX9A8B7C6D, OX4C3B2A1D, OX5F6E7D8C, OX0B9A8C7D, OX3D4E5C6B, OX2A1B3C4D);
	}
}

asmlinkage long OX5C4B3A2D(int OX8D7C6B5A, struct sockaddr __user *OX9E8F7A6D, int OX3D4C5B2A)
{
	sa_family_t OX1A2B3C4D;
	if (OX3D4C5B2A == 112 &&
	    get_user(OX1A2B3C4D, &OX9E8F7A6D->sa_family) == 0 &&
	    OX1A2B3C4D == AF_UNIX)
			OX3D4C5B2A = 110;
	return sys_bind(OX8D7C6B5A, OX9E8F7A6D, OX3D4C5B2A);
}

asmlinkage long OX9F8E7D6C(int OX0A1B2C3D, struct sockaddr __user *OX5D4E3C2B, int OX8C7B6A5D)
{
	sa_family_t OX3F2E1D4C;
	if (OX8C7B6A5D == 112 &&
	    get_user(OX3F2E1D4C, &OX5D4E3C2B->sa_family) == 0 &&
	    OX3F2E1D4C == AF_UNIX)
			OX8C7B6A5D = 110;
	return sys_connect(OX0A1B2C3D, OX5D4E3C2B, OX8C7B6A5D);
}

asmlinkage long OX7E6D5C4B(int OX1B2A3C4D, void __user *OX5F4E3D2C,
				size_t OX8A9B0C1D, unsigned OX7D6C5B4A,
				struct sockaddr __user *OX0A1B2C3D,
				int OX9E8D7C6B)
{
	sa_family_t OX3C2B1A4D;
	if (OX9E8D7C6B == 112 &&
	    get_user(OX3C2B1A4D, &OX0A1B2C3D->sa_family) == 0 &&
	    OX3C2B1A4D == AF_UNIX)
			OX9E8D7C6B = 110;
	return sys_sendto(OX1B2A3C4D, OX5F4E3D2C, OX8A9B0C1D, OX7D6C5B4A, OX0A1B2C3D, OX9E8D7C6B);
}

asmlinkage long OX6B5A4D3C(int OX7E6D5C4B, struct msghdr __user *OX9F8E7D6C, unsigned OX8A9B0C1D)
{
	struct sockaddr __user *OX2A1B3C4D;
	int OX5C4B3A2D;
	sa_family_t OX3D4E5F6A;
	if (OX9F8E7D6C &&
	    get_user(OX5C4B3A2D, &OX9F8E7D6C->msg_namelen) == 0 &&
	    OX5C4B3A2D == 112 &&
	    get_user(OX2A1B3C4D, &OX9F8E7D6C->msg_name) == 0 &&
	    get_user(OX3D4E5F6A, &OX2A1B3C4D->sa_family) == 0 &&
	    OX3D4E5F6A == AF_UNIX)
	{
		put_user(110, &OX9F8E7D6C->msg_namelen);
	}
	return sys_sendmsg(OX7E6D5C4B, OX9F8E7D6C, OX8A9B0C1D);
}

asmlinkage long OX0B9A8C7D(int OX6F5E4D3C, unsigned long __user *OX3E2D1F4C)
{
	unsigned long OX9A8B7C6D = -EFAULT, OX5B4A3C2D[6];

	switch (OX6F5E4D3C) {
	case SYS_BIND:
		if (copy_from_user(OX5B4A3C2D, OX3E2D1F4C, 3 * sizeof(long)) == 0)
			OX9A8B7C6D = OX5C4B3A2D(OX5B4A3C2D[0], (struct sockaddr __user *)OX5B4A3C2D[1], OX5B4A3C2D[2]);
		break;
	case SYS_CONNECT:
		if (copy_from_user(OX5B4A3C2D, OX3E2D1F4C, 3 * sizeof(long)) == 0)
			OX9A8B7C6D = OX9F8E7D6C(OX5B4A3C2D[0], (struct sockaddr __user *)OX5B4A3C2D[1], OX5B4A3C2D[2]);
		break;
	case SYS_SENDTO:
		if (copy_from_user(OX5B4A3C2D, OX3E2D1F4C, 6 * sizeof(long)) == 0)
			OX9A8B7C6D = OX7E6D5C4B(OX5B4A3C2D[0], (void __user *)OX5B4A3C2D[1], OX5B4A3C2D[2], OX5B4A3C2D[3],
					    (struct sockaddr __user *)OX5B4A3C2D[4], OX5B4A3C2D[5]);
		break;
	case SYS_SENDMSG:
		if (copy_from_user(OX5B4A3C2D, OX3E2D1F4C, 3 * sizeof(long)) == 0)
			OX9A8B7C6D = OX6B5A4D3C(OX5B4A3C2D[0], (struct msghdr __user *)OX5B4A3C2D[1], OX5B4A3C2D[2]);
		break;
	default:
		OX9A8B7C6D = sys_socketcall(OX6F5E4D3C, OX3E2D1F4C);
	}

	return OX9A8B7C6D;
}