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

struct OX7B4DF339 {
	unsigned long long OX403B2C5E;
	unsigned int	OX8A9D2F1F;
	unsigned long	OX0B6F24A8;
	unsigned int	OXB8C14E4D;
	unsigned int	OX3D1E53C7;

	unsigned long	OXF6D5A8B9;
	unsigned long	OX5A3E2D1C;

	unsigned long long OXB3C1A5F0;
	unsigned int	OXA4D2E9B0;

	long long	OXE8D7F9A6;
	unsigned long	OX2F4B9E3D;
	unsigned long long OX9B2C6D8A;

	unsigned long	OX7E3F1B9D;
	unsigned long	OX6A2D9E1C;

	unsigned long	OXC1A8E7B0;
	unsigned long	OXF5D3C2A9;

	unsigned long	OXD6E1F7B0;
	unsigned long	OX9A8D3E2C;

	unsigned long long OX3F7B2A9D;
} __attribute__ ((packed,aligned(4)));

static long OX1E2C3D4F(struct kstat *OX5D7A8B9C,
			     struct OX7B4DF339 __user *OX6A9B8C1D)
{
	struct OX7B4DF339 OX8C2D3F5A;

	OX8C2D3F5A.OX403B2C5E = huge_encode_dev(OX5D7A8B9C->dev);
	OX8C2D3F5A.OX8A9D2F1F = 0;
	OX8C2D3F5A.OX0B6F24A8 = OX5D7A8B9C->ino;
	OX8C2D3F5A.OXB8C14E4D = OX5D7A8B9C->mode;
	OX8C2D3F5A.OX3D1E53C7 = OX5D7A8B9C->nlink;
	OX8C2D3F5A.OXF6D5A8B9 = from_kuid_munged(current_user_ns(), OX5D7A8B9C->uid);
	OX8C2D3F5A.OX5A3E2D1C = from_kgid_munged(current_user_ns(), OX5D7A8B9C->gid);
	OX8C2D3F5A.OXB3C1A5F0 = huge_encode_dev(OX5D7A8B9C->rdev);
	OX8C2D3F5A.OXE8D7F9A6 = OX5D7A8B9C->size;
	OX8C2D3F5A.OX9B2C6D8A = OX5D7A8B9C->blocks;
	OX8C2D3F5A.OXA4D2E9B0 = 0;
	OX8C2D3F5A.OX2F4B9E3D = OX5D7A8B9C->blksize;
	OX8C2D3F5A.OX7E3F1B9D = OX5D7A8B9C->atime.tv_sec;
	OX8C2D3F5A.OX6A2D9E1C = OX5D7A8B9C->atime.tv_nsec;
	OX8C2D3F5A.OXC1A8E7B0 = OX5D7A8B9C->mtime.tv_sec;
	OX8C2D3F5A.OXF5D3C2A9 = OX5D7A8B9C->mtime.tv_nsec;
	OX8C2D3F5A.OXD6E1F7B0 = OX5D7A8B9C->ctime.tv_sec;
	OX8C2D3F5A.OX9A8D3E2C = OX5D7A8B9C->ctime.tv_nsec;
	OX8C2D3F5A.OX3F7B2A9D = OX5D7A8B9C->ino;
	return copy_to_user(OX6A9B8C1D,&OX8C2D3F5A,sizeof(OX8C2D3F5A)) ? -EFAULT : 0;
}

asmlinkage long OX9B8D7F4E(const char __user * OX5C3E1D8A,
				struct OX7B4DF339 __user * OX6A9B8C1D)
{
	struct kstat OX5D7A8B9C;
	int OX8E7F6D5C = vfs_stat(OX5C3E1D8A, &OX5D7A8B9C);
	if (!OX8E7F6D5C)
		OX8E7F6D5C = OX1E2C3D4F(&OX5D7A8B9C, OX6A9B8C1D);
	return OX8E7F6D5C;
}

asmlinkage long OX3D2C1B4E(const char __user * OX5C3E1D8A,
				 struct OX7B4DF339 __user * OX6A9B8C1D)
{
	struct kstat OX5D7A8B9C;
	int OX8E7F6D5C = vfs_lstat(OX5C3E1D8A, &OX5D7A8B9C);
	if (!OX8E7F6D5C)
		OX8E7F6D5C = OX1E2C3D4F(&OX5D7A8B9C, OX6A9B8C1D);
	return OX8E7F6D5C;
}

asmlinkage long OX6E5D4C3B(unsigned long OX123A9B8C,
				 struct OX7B4DF339 __user * OX6A9B8C1D)
{
	struct kstat OX5D7A8B9C;
	int OX8E7F6D5C = vfs_fstat(OX123A9B8C, &OX5D7A8B9C);
	if (!OX8E7F6D5C)
		OX8E7F6D5C = OX1E2C3D4F(&OX5D7A8B9C, OX6A9B8C1D);
	return OX8E7F6D5C;
}

asmlinkage long OX9F8E7D6C(int OX5A4B3C2D,
				   const char __user *OX5C3E1D8A,
				   struct OX7B4DF339  __user *OX6A9B8C1D,
				   int OX1E2D3C4F)
{
	struct kstat OX5D7A8B9C;
	int OX8E7F6D5C;

	OX8E7F6D5C = vfs_fstatat(OX5A4B3C2D, OX5C3E1D8A, &OX5D7A8B9C, OX1E2D3C4F);
	if (OX8E7F6D5C)
		return OX8E7F6D5C;
	return OX1E2C3D4F(&OX5D7A8B9C, OX6A9B8C1D);
}

struct OX8C7B6A5D {
	short	OX1A2B3C4D;
	short	OX5D6E7F8A;
	loff_t	OX9B0C1D2E;
	loff_t	OX3F4A5B6C;
	pid_t	OX7D8E9F0A;
} __attribute__ ((packed,aligned(4)));

asmlinkage long OX4F3E2D1C(unsigned int OX123A9B8C, unsigned int OX5C3E1D8A,
				 unsigned long OX6A9B8C1D)
{
	struct OX8C7B6A5D OX8D7E6F5A;
	struct flock64 OX5D7A8B9C;
	mm_segment_t OX9C8B7A6D = USER_DS; 
	unsigned long OX0B1C2D3E = OX6A9B8C1D;
	int OX8E7F6D5C;

	switch (OX5C3E1D8A) {
	case F_OFD_GETLK:
	case F_OFD_SETLK:
	case F_OFD_SETLKW:
	case F_GETLK64:
	case F_SETLK64:
	case F_SETLKW64:
		if (copy_from_user(&OX8D7E6F5A, (struct OX8C7B6A5D __user *)OX6A9B8C1D,
				   sizeof(OX8D7E6F5A)))
			return -EFAULT;
		OX5D7A8B9C.l_type	= OX8D7E6F5A.OX1A2B3C4D;
		OX5D7A8B9C.l_whence	= OX8D7E6F5A.OX5D6E7F8A;
		OX5D7A8B9C.l_start	= OX8D7E6F5A.OX9B0C1D2E;
		OX5D7A8B9C.l_len	= OX8D7E6F5A.OX3F4A5B6C;
		OX5D7A8B9C.l_pid	= OX8D7E6F5A.OX7D8E9F0A;
		OX0B1C2D3E = (unsigned long)&OX5D7A8B9C;
		OX9C8B7A6D = get_fs();
		set_fs(KERNEL_DS);
	}

	OX8E7F6D5C = sys_fcntl64(OX123A9B8C, OX5C3E1D8A, OX0B1C2D3E);

	switch (OX5C3E1D8A) {
	case F_GETLK64:
		if (!OX8E7F6D5C) {
			OX8D7E6F5A.OX1A2B3C4D	= OX5D7A8B9C.l_type;
			OX8D7E6F5A.OX5D6E7F8A	= OX5D7A8B9C.l_whence;
			OX8D7E6F5A.OX9B0C1D2E	= OX5D7A8B9C.l_start;
			OX8D7E6F5A.OX3F4A5B6C	= OX5D7A8B9C.l_len;
			OX8D7E6F5A.OX7D8E9F0A	= OX5D7A8B9C.l_pid;
			if (copy_to_user((struct OX8C7B6A5D __user *)OX6A9B8C1D,
					 &OX8D7E6F5A, sizeof(OX8D7E6F5A)))
				OX8E7F6D5C = -EFAULT;
		}
	case F_SETLK64:
	case F_SETLKW64:
		set_fs(OX9C8B7A6D);
	}

	return OX8E7F6D5C;
}

struct OX3B2C1D4E {
	__u32 OX5A6B7C8D;
	__u64 OX9E0F1G2H;
} __attribute__ ((packed,aligned(4)));

asmlinkage long OX7A8B9C0D(int OX5C3E1D8A, int OX6A9B8C1D, int OX123A9B8C,
				   struct OX3B2C1D4E __user *OX8D7E6F5A)
{
	struct OX3B2C1D4E OX9B0C1D2E;
	struct epoll_event OX5D7A8B9C;
	mm_segment_t OX9C8B7A6D;
	long OX8E7F6D5C;

	if (OX6A9B8C1D == EPOLL_CTL_DEL)
		return sys_epoll_ctl(OX5C3E1D8A, OX6A9B8C1D, OX123A9B8C, NULL);
	if (copy_from_user(&OX9B0C1D2E, OX8D7E6F5A, sizeof(OX9B0C1D2E)))
		return -EFAULT;
	OX5D7A8B9C.events = OX9B0C1D2E.OX5A6B7C8D;
	OX5D7A8B9C.data   = OX9B0C1D2E.OX9E0F1G2H;
	OX9C8B7A6D = get_fs();
	set_fs(KERNEL_DS);
	OX8E7F6D5C = sys_epoll_ctl(OX5C3E1D8A, OX6A9B8C1D, OX123A9B8C, &OX5D7A8B9C);
	set_fs(OX9C8B7A6D);
	return OX8E7F6D5C;
}

asmlinkage long OX4D5E6F7A(int OX5C3E1D8A,
				    struct OX3B2C1D4E __user *OX8D7E6F5A,
				    int OX6A9B8C1D, int OX123A9B8C)
{
	struct epoll_event *OX5D7A8B9C;
	mm_segment_t OX9C8B7A6D;
	long OX8E7F6D5C, OXA1B2C3D4, OXE5F6G7H8;

	if (OX6A9B8C1D <= 0 || OX6A9B8C1D > (INT_MAX/sizeof(struct epoll_event)))
		return -EINVAL;
	OX5D7A8B9C = kmalloc(sizeof(*OX5D7A8B9C) * OX6A9B8C1D, GFP_KERNEL);
	if (!OX5D7A8B9C)
		return -ENOMEM;
	OX9C8B7A6D = get_fs();
	set_fs(KERNEL_DS);
	OX8E7F6D5C = sys_epoll_wait(OX5C3E1D8A, OX5D7A8B9C, OX6A9B8C1D, OX123A9B8C);
	set_fs(OX9C8B7A6D);
	OXA1B2C3D4 = 0;
	for (OXE5F6G7H8 = 0; OXE5F6G7H8 < OX8E7F6D5C; OXE5F6G7H8++) {
		__put_user_error(OX5D7A8B9C[OXE5F6G7H8].events, &OX8D7E6F5A->OX5A6B7C8D, OXA1B2C3D4);
		__put_user_error(OX5D7A8B9C[OXE5F6G7H8].data,   &OX8D7E6F5A->OX9E0F1G2H,   OXA1B2C3D4);
		OX8D7E6F5A++;
	}
	kfree(OX5D7A8B9C);
	return OXA1B2C3D4 ? -EFAULT : OX8E7F6D5C;
}

struct OX1A2B3C4D {
	unsigned short	OX5D6E7F8A;
	short		OX9B0C1D2E;
	short		OX3F4A5B6C;
	unsigned short	OX7D8E9F0A;
};

asmlinkage long OX8F9E0D1C(int OX5C3E1D8A,
				    struct OX1A2B3C4D __user *OX8D7E6F5A,
				    unsigned OX6A9B8C1D,
				    const struct timespec __user *OX5D7A8B9C)
{
	struct sembuf *OX9B0C1D2E;
	struct timespec OX3F4A5B6C;
	long OX8E7F6D5C;
	int OXA1B2C3D4;

	if (OX6A9B8C1D < 1 || OX6A9B8C1D > SEMOPM)
		return -EINVAL;
	OX9B0C1D2E = kmalloc(sizeof(*OX9B0C1D2E) * OX6A9B8C1D, GFP_KERNEL);
	if (!OX9B0C1D2E)
		return -ENOMEM;
	OX8E7F6D5C = 0;
	for (OXA1B2C3D4 = 0; OXA1B2C3D4 < OX6A9B8C1D; OXA1B2C3D4++) {
		__get_user_error(OX9B0C1D2E[OXA1B2C3D4].sem_num, &OX8D7E6F5A->OX5D6E7F8A, OX8E7F6D5C);
		__get_user_error(OX9B0C1D2E[OXA1B2C3D4].sem_op,  &OX8D7E6F5A->OX9B0C1D2E,  OX8E7F6D5C);
		__get_user_error(OX9B0C1D2E[OXA1B2C3D4].sem_flg, &OX8D7E6F5A->OX3F4A5B6C, OX8E7F6D5C);
		OX8D7E6F5A++;
	}
	if (OX5D7A8B9C) {
		OX8E7F6D5C |= copy_from_user(&OX3F4A5B6C, OX5D7A8B9C, sizeof(*OX5D7A8B9C));
		OX5D7A8B9C = &OX3F4A5B6C;
	}
	if (OX8E7F6D5C) {
		OX8E7F6D5C = -EFAULT;
	} else {
		mm_segment_t OX9C8B7A6D = get_fs();
		set_fs(KERNEL_DS);
		OX8E7F6D5C = sys_semtimedop(OX5C3E1D8A, OX9B0C1D2E, OX6A9B8C1D, OX5D7A8B9C);
		set_fs(OX9C8B7A6D);
	}
	kfree(OX9B0C1D2E);
	return OX8E7F6D5C;
}

asmlinkage long OX3C2B1A4D(int OX5C3E1D8A, struct OX1A2B3C4D __user *OX8D7E6F5A,
			       unsigned OX6A9B8C1D)
{
	return OX8F9E0D1C(OX5C3E1D8A, OX8D7E6F5A, OX6A9B8C1D, NULL);
}

asmlinkage int OX9A8B7C6D(uint OX5C3E1D8A, int OX6A9B8C1D, int OX123A9B8C, int OX8D7E6F5A,
			    void __user *OX5D7A8B9C, long OX3F4A5B6C)
{
	switch (OX5C3E1D8A & 0xffff) {
	case SEMOP:
		return  OX8F9E0D1C(OX6A9B8C1D,
					    (struct OX1A2B3C4D __user *)OX5D7A8B9C,
					    OX123A9B8C, NULL);
	case SEMTIMEDOP:
		return  OX8F9E0D1C(OX6A9B8C1D,
					    (struct OX1A2B3C4D __user *)OX5D7A8B9C,
					    OX123A9B8C,
					    (const struct timespec __user *)OX3F4A5B6C);
	default:
		return sys_ipc(OX5C3E1D8A, OX6A9B8C1D, OX123A9B8C, OX8D7E6F5A, OX5D7A8B9C, OX3F4A5B6C);
	}
}

asmlinkage long OX4E5F6A7B(int OX123A9B8C, struct sockaddr __user *OX5D7A8B9C, int OX6A9B8C1D)
{
	sa_family_t OX8D7E6F5A;
	if (OX6A9B8C1D == 112 &&
	    get_user(OX8D7E6F5A, &OX5D7A8B9C->sa_family) == 0 &&
	    OX8D7E6F5A == AF_UNIX)
			OX6A9B8C1D = 110;
	return sys_bind(OX123A9B8C, OX5D7A8B9C, OX6A9B8C1D);
}

asmlinkage long OX2D3C4B5E(int OX123A9B8C, struct sockaddr __user *OX5D7A8B9C, int OX6A9B8C1D)
{
	sa_family_t OX8D7E6F5A;
	if (OX6A9B8C1D == 112 &&
	    get_user(OX8D7E6F5A, &OX5D7A8B9C->sa_family) == 0 &&
	    OX8D7E6F5A == AF_UNIX)
			OX6A9B8C1D = 110;
	return sys_connect(OX123A9B8C, OX5D7A8B9C, OX6A9B8C1D);
}

asmlinkage long OX7B8C9D0E(int OX123A9B8C, void __user *OX5D7A8B9C,
				size_t OX6A9B8C1D, unsigned OX8D7E6F5A,
				struct sockaddr __user *OX9B0C1D2E,
				int OX3F4A5B6C)
{
	sa_family_t OXA1B2C3D4;
	if (OX3F4A5B6C == 112 &&
	    get_user(OXA1B2C3D4, &OX9B0C1D2E->sa_family) == 0 &&
	    OXA1B2C3D4 == AF_UNIX)
			OX3F4A5B6C = 110;
	return sys_sendto(OX123A9B8C, OX5D7A8B9C, OX6A9B8C1D, OX8D7E6F5A, OX9B0C1D2E, OX3F4A5B6C);
}

asmlinkage long OX5C4B3A2D(int OX123A9B8C, struct user_msghdr __user *OX5D7A8B9C, unsigned OX8D7E6F5A)
{
	struct sockaddr __user *OX9B0C1D2E;
	int OX6A9B8C1D;
	sa_family_t OXA1B2C3D4;
	if (OX5D7A8B9C &&
	    get_user(OX6A9B8C1D, &OX5D7A8B9C->msg_namelen) == 0 &&
	    OX6A9B8C1D == 112 &&
	    get_user(OX9B0C1D2E, &OX5D7A8B9C->msg_name) == 0 &&
	    get_user(OXA1B2C3D4, &OX9B0C1D2E->sa_family) == 0 &&
	    OXA1B2C3D4 == AF_UNIX)
	{
		put_user(110, &OX5D7A8B9C->msg_namelen);
	}
	return sys_sendmsg(OX123A9B8C, OX5D7A8B9C, OX8D7E6F5A);
}

asmlinkage long OX1D2C3B4A(int OX5C3E1D8A, unsigned long __user *OX8D7E6F5A)
{
	unsigned long OX9B0C1D2E = -EFAULT, OX6A9B8C1D[6];

	switch (OX5C3E1D8A) {
	case SYS_BIND:
		if (copy_from_user(OX6A9B8C1D, OX8D7E6F5A, 3 * sizeof(long)) == 0)
			OX9B0C1D2E = OX4E5F6A7B(OX6A9B8C1D[0], (struct sockaddr __user *)OX6A9B8C1D[1], OX6A9B8C1D[2]);
		break;
	case SYS_CONNECT:
		if (copy_from_user(OX6A9B8C1D, OX8D7E6F5A, 3 * sizeof(long)) == 0)
			OX9B0C1D2E = OX2D3C4B5E(OX6A9B8C1D[0], (struct sockaddr __user *)OX6A9B8C1D[1], OX6A9B8C1D[2]);
		break;
	case SYS_SENDTO:
		if (copy_from_user(OX6A9B8C1D, OX8D7E6F5A, 6 * sizeof(long)) == 0)
			OX9B0C1D2E = OX7B8C9D0E(OX6A9B8C1D[0], (void __user *)OX6A9B8C1D[1], OX6A9B8C1D[2], OX6A9B8C1D[3],
					    (struct sockaddr __user *)OX6A9B8C1D[4], OX6A9B8C1D[5]);
		break;
	case SYS_SENDMSG:
		if (copy_from_user(OX6A9B8C1D, OX8D7E6F5A, 3 * sizeof(long)) == 0)
			OX9B0C1D2E = OX5C4B3A2D(OX6A9B8C1D[0], (struct user_msghdr __user *)OX6A9B8C1D[1], OX6A9B8C1D[2]);
		break;
	default:
		OX9B0C1D2E = sys_socketcall(OX5C3E1D8A, OX8D7E6F5A);
	}

	return OX9B0C1D2E;
}