#include <Python.h>
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

struct oldabi_stat64 {
	unsigned long long st_dev;
	unsigned int	__pad1;
	unsigned long	__st_ino;
	unsigned int	st_mode;
	unsigned int	st_nlink;

	unsigned long	st_uid;
	unsigned long	st_gid;

	unsigned long long st_rdev;
	unsigned int	__pad2;

	long long	st_size;
	unsigned long	st_blksize;
	unsigned long long st_blocks;

	unsigned long	st_atime;
	unsigned long	st_atime_nsec;

	unsigned long	st_mtime;
	unsigned long	st_mtime_nsec;

	unsigned long	st_ctime;
	unsigned long	st_ctime_nsec;

	unsigned long long st_ino;
} __attribute__ ((packed,aligned(4)));

static PyObject* cp_oldabi_stat64(PyObject* self, PyObject* args) {
	PyObject* stat_obj;
	PyObject* statbuf_obj;

	if (!PyArg_ParseTuple(args, "OO", &stat_obj, &statbuf_obj)) {
		return NULL;
	}

	struct kstat* stat = (struct kstat*)PyLong_AsVoidPtr(stat_obj);
	struct oldabi_stat64 __user* statbuf = (struct oldabi_stat64 __user*)PyLong_AsVoidPtr(statbuf_obj);

	struct oldabi_stat64 tmp;

	tmp.st_dev = huge_encode_dev(stat->dev);
	tmp.__pad1 = 0;
	tmp.__st_ino = stat->ino;
	tmp.st_mode = stat->mode;
	tmp.st_nlink = stat->nlink;
	tmp.st_uid = stat->uid;
	tmp.st_gid = stat->gid;
	tmp.st_rdev = huge_encode_dev(stat->rdev);
	tmp.st_size = stat->size;
	tmp.st_blocks = stat->blocks;
	tmp.__pad2 = 0;
	tmp.st_blksize = stat->blksize;
	tmp.st_atime = stat->atime.tv_sec;
	tmp.st_atime_nsec = stat->atime.tv_nsec;
	tmp.st_mtime = stat->mtime.tv_sec;
	tmp.st_mtime_nsec = stat->mtime.tv_nsec;
	tmp.st_ctime = stat->ctime.tv_sec;
	tmp.st_ctime_nsec = stat->ctime.tv_nsec;
	tmp.st_ino = stat->ino;

	int result = copy_to_user(statbuf, &tmp, sizeof(tmp)) ? -EFAULT : 0;

	return PyLong_FromLong(result);
}

asmlinkage long sys_oabi_stat64(const char __user* filename, struct oldabi_stat64 __user* statbuf) {
	struct kstat stat;
	int error = vfs_stat(filename, &stat);
	if (!error) {
		Py_Initialize();
		PyObject* module = PyImport_ImportModule("__main__");
		PyObject* result = PyObject_CallMethod(module, "cp_oldabi_stat64", "kk", &stat, statbuf);
		error = PyLong_AsLong(result);
		Py_Finalize();
	}
	return error;
}

asmlinkage long sys_oabi_lstat64(const char __user* filename, struct oldabi_stat64 __user* statbuf) {
	struct kstat stat;
	int error = vfs_lstat(filename, &stat);
	if (!error) {
		Py_Initialize();
		PyObject* module = PyImport_ImportModule("__main__");
		PyObject* result = PyObject_CallMethod(module, "cp_oldabi_stat64", "kk", &stat, statbuf);
		error = PyLong_AsLong(result);
		Py_Finalize();
	}
	return error;
}

asmlinkage long sys_oabi_fstat64(unsigned long fd, struct oldabi_stat64 __user* statbuf) {
	struct kstat stat;
	int error = vfs_fstat(fd, &stat);
	if (!error) {
		Py_Initialize();
		PyObject* module = PyImport_ImportModule("__main__");
		PyObject* result = PyObject_CallMethod(module, "cp_oldabi_stat64", "kk", &stat, statbuf);
		error = PyLong_AsLong(result);
		Py_Finalize();
	}
	return error;
}

asmlinkage long sys_oabi_fstatat64(int dfd, const char __user* filename, struct oldabi_stat64 __user* statbuf, int flag) {
	struct kstat stat;
	int error;

	error = vfs_fstatat(dfd, filename, &stat, flag);
	if (error)
		return error;

	Py_Initialize();
	PyObject* module = PyImport_ImportModule("__main__");
	PyObject* result = PyObject_CallMethod(module, "cp_oldabi_stat64", "kk", &stat, statbuf);
	error = PyLong_AsLong(result);
	Py_Finalize();

	return error;
}