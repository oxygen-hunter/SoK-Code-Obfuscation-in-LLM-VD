#define _GNU_SOURCE
#include "lxclock.h"
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>

#include <lxc/lxccontainer.h>

#include "utils.h"
#include "log.h"

#ifdef MUTEX_DEBUGGING
#include <execinfo.h>
#endif

#define MAX_STACKDEPTH 25

#define OFLAG (O_CREAT | O_RDWR)
#define SEMMODE 0660
#define SEMVALUE 1
#define SEMVALUE_LOCKED 0

lxc_log_define(lxc_lock, lxc);

#ifdef MUTEX_DEBUGGING
static pthread_mutex_t thread_mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

static inline void dump_stacktrace(void)
{
	void *array[MAX_STACKDEPTH];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, MAX_STACKDEPTH);
	strings = backtrace_symbols(array, size);

	fprintf(stderr, "\tObtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
		fprintf(stderr, "\t\t%s\n", strings[i]);

	free(strings);
}
#else
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline void dump_stacktrace(void) {;}
#endif

static void lock_mutex(pthread_mutex_t *l)
{
	int ret;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			if ((ret = pthread_mutex_lock(l)) != 0) {
				dispatcher = 1;
			} else {
				dispatcher = 2;
			}
			break;
		case 1:
			fprintf(stderr, "pthread_mutex_lock returned:%d %s\n", ret, strerror(ret));
			dump_stacktrace();
			exit(1);
			break;
		case 2:
			return;
		}
	}
}

static void unlock_mutex(pthread_mutex_t *l)
{
	int ret;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			if ((ret = pthread_mutex_unlock(l)) != 0) {
				dispatcher = 1;
			} else {
				dispatcher = 2;
			}
			break;
		case 1:
			fprintf(stderr, "pthread_mutex_unlock returned:%d %s\n", ret, strerror(ret));
			dump_stacktrace();
			exit(1);
			break;
		case 2:
			return;
		}
	}
}

static char *lxclock_name(const char *p, const char *n)
{
	int ret;
	int len;
	char *dest;
	char *rundir;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			len = strlen("/lock/lxc/") + strlen(n) + strlen(p) + 2;
			rundir = get_rundir();
			if (!rundir) {
				dispatcher = 1;
			} else {
				len += strlen(rundir);
				if ((dest = malloc(len)) == NULL) {
					dispatcher = 2;
				} else {
					ret = snprintf(dest, len, "%s/lock/lxc/%s", rundir, p);
					if (ret < 0 || ret >= len) {
						dispatcher = 3;
					} else {
						ret = mkdir_p(dest, 0755);
						if (ret < 0) {
							dispatcher = 4;
						} else {
							ret = snprintf(dest, len, "%s/lock/lxc/%s/%s", rundir, p, n);
							dispatcher = 5;
						}
					}
				}
			}
			break;
		case 1:
			return NULL;
		case 2:
			free(rundir);
			return NULL;
		case 3:
			free(dest);
			free(rundir);
			return NULL;
		case 4:
			int l2 = 33 + strlen(n) + strlen(p);
			if (l2 > len) {
				char *d;
				d = realloc(dest, l2);
				if (!d) {
					dispatcher = 6;
				} else {
					len = l2;
					dest = d;
					ret = snprintf(dest, len, "/tmp/%d/lxc/%s", geteuid(), p);
					if (ret < 0 || ret >= len) {
						dispatcher = 7;
					} else {
						ret = snprintf(dest, len, "/tmp/%d/lxc/%s/%s", geteuid(), p, n);
						dispatcher = 5;
					}
				}
			} else {
				ret = snprintf(dest, len, "/tmp/%d/lxc/%s", geteuid(), p);
				if (ret < 0 || ret >= len) {
					dispatcher = 8;
				} else {
					ret = snprintf(dest, len, "/tmp/%d/lxc/%s/%s", geteuid(), p, n);
					dispatcher = 5;
				}
			}
			break;
		case 5:
			free(rundir);
			if (ret < 0 || ret >= len) {
				dispatcher = 9;
			} else {
				return dest;
			}
			break;
		case 6:
			free(dest);
			free(rundir);
			return NULL;
		case 7:
			free(dest);
			free(rundir);
			return NULL;
		case 8:
			free(dest);
			free(rundir);
			return NULL;
		case 9:
			free(dest);
			return NULL;
		}
	}
}

static sem_t *lxc_new_unnamed_sem(void)
{
	sem_t *s;
	int ret;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			s = malloc(sizeof(*s));
			if (!s) {
				dispatcher = 1;
			} else {
				ret = sem_init(s, 0, 1);
				if (ret) {
					dispatcher = 2;
				} else {
					return s;
				}
			}
			break;
		case 1:
			return NULL;
		case 2:
			free(s);
			return NULL;
		}
	}
}

struct lxc_lock *lxc_newlock(const char *lxcpath, const char *name)
{
	struct lxc_lock *l;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			l = malloc(sizeof(*l));
			if (!l) {
				dispatcher = 1;
			} else {
				if (!name) {
					l->type = LXC_LOCK_ANON_SEM;
					l->u.sem = lxc_new_unnamed_sem();
					if (!l->u.sem) {
						free(l);
						l = NULL;
					}
					dispatcher = 2;
				} else {
					l->type = LXC_LOCK_FLOCK;
					l->u.f.fname = lxclock_name(lxcpath, name);
					if (!l->u.f.fname) {
						free(l);
						l = NULL;
						dispatcher = 2;
					} else {
						l->u.f.fd = -1;
						dispatcher = 2;
					}
				}
			}
			break;
		case 1:
			goto out;
		case 2:
			goto out;
		}
	}

out:
	return l;
}

int lxclock(struct lxc_lock *l, int timeout)
{
	int ret = -1, saved_errno = errno;
	struct flock lk;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			switch (l->type) {
			case LXC_LOCK_ANON_SEM:
				if (!timeout) {
					ret = sem_wait(l->u.sem);
					if (ret == -1)
						saved_errno = errno;
				} else {
					struct timespec ts;
					if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
						ret = -2;
						goto out;
					}
					ts.tv_sec += timeout;
					ret = sem_timedwait(l->u.sem, &ts);
					if (ret == -1)
						saved_errno = errno;
				}
				dispatcher = 1;
				break;
			case LXC_LOCK_FLOCK:
				ret = -2;
				if (timeout) {
					ERROR("Error: timeout not supported with flock");
					ret = -2;
					dispatcher = 2;
				} else {
					if (!l->u.f.fname) {
						ERROR("Error: filename not set for flock");
						ret = -2;
						dispatcher = 2;
					} else {
						if (l->u.f.fd == -1) {
							l->u.f.fd = open(l->u.f.fname, O_RDWR | O_CREAT, S_IWUSR | S_IRUSR);
							if (l->u.f.fd == -1) {
								ERROR("Error opening %s", l->u.f.fname);
								dispatcher = 2;
							} else {
								lk.l_type = F_WRLCK;
								lk.l_whence = SEEK_SET;
								lk.l_start = 0;
								lk.l_len = 0;
								ret = fcntl(l->u.f.fd, F_SETLKW, &lk);
								if (ret == -1)
									saved_errno = errno;
								dispatcher = 2;
							}
						} else {
							lk.l_type = F_WRLCK;
							lk.l_whence = SEEK_SET;
							lk.l_start = 0;
							lk.l_len = 0;
							ret = fcntl(l->u.f.fd, F_SETLKW, &lk);
							if (ret == -1)
								saved_errno = errno;
							dispatcher = 2;
						}
					}
				}
				break;
			}
			break;
		case 1:
			goto out;
		case 2:
			goto out;
		}
	}

out:
	errno = saved_errno;
	return ret;
}

int lxcunlock(struct lxc_lock *l)
{
	int ret = 0, saved_errno = errno;
	struct flock lk;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			switch (l->type) {
			case LXC_LOCK_ANON_SEM:
				if (!l->u.sem)
					ret = -2;
				else {
					ret = sem_post(l->u.sem);
					saved_errno = errno;
				}
				dispatcher = 1;
				break;
			case LXC_LOCK_FLOCK:
				if (l->u.f.fd != -1) {
					lk.l_type = F_UNLCK;
					lk.l_whence = SEEK_SET;
					lk.l_start = 0;
					lk.l_len = 0;
					ret = fcntl(l->u.f.fd, F_SETLK, &lk);
					if (ret < 0)
						saved_errno = errno;
					close(l->u.f.fd);
					l->u.f.fd = -1;
				} else
					ret = -2;
				dispatcher = 1;
				break;
			}
			break;
		case 1:
			goto end;
		}
	}

end:
	errno = saved_errno;
	return ret;
}

void lxc_putlock(struct lxc_lock *l)
{
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			if (!l) {
				dispatcher = 1;
			} else {
				switch (l->type) {
				case LXC_LOCK_ANON_SEM:
					if (l->u.sem) {
						sem_destroy(l->u.sem);
						free(l->u.sem);
						l->u.sem = NULL;
					}
					dispatcher = 2;
					break;
				case LXC_LOCK_FLOCK:
					if (l->u.f.fd != -1) {
						close(l->u.f.fd);
						l->u.f.fd = -1;
					}
					if (l->u.f.fname) {
						free(l->u.f.fname);
						l->u.f.fname = NULL;
					}
					dispatcher = 2;
					break;
				}
			}
			break;
		case 1:
			return;
		case 2:
			free(l);
			return;
		}
	}
}

void process_lock(void)
{
	lock_mutex(&thread_mutex);
}

void process_unlock(void)
{
	unlock_mutex(&thread_mutex);
}

#ifdef HAVE_PTHREAD_ATFORK
__attribute__((constructor))
static void process_lock_setup_atfork(void)
{
	pthread_atfork(process_lock, process_unlock, process_unlock);
}
#endif

int container_mem_lock(struct lxc_container *c)
{
	return lxclock(c->privlock, 0);
}

void container_mem_unlock(struct lxc_container *c)
{
	lxcunlock(c->privlock);
}

int container_disk_lock(struct lxc_container *c)
{
	int ret;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			ret = lxclock(c->privlock, 0);
			if (ret) {
				dispatcher = 1;
			} else {
				ret = lxclock(c->slock, 0);
				if (ret) {
					lxcunlock(c->privlock);
					dispatcher = 1;
				} else {
					return 0;
				}
			}
			break;
		case 1:
			return ret;
		}
	}
}

void container_disk_unlock(struct lxc_container *c)
{
	lxcunlock(c->slock);
	lxcunlock(c->privlock);
}