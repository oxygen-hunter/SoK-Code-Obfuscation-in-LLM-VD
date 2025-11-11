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

#define OX0F7A6B2 (O_CREAT | O_RDWR)
#define OX2B3C4A5 0660
#define OX3D4E5F6 1
#define OX4E5F6G7 0

lxc_log_define(OX5F6G7H8, OX6G7H8I9);

#ifdef MUTEX_DEBUGGING
static pthread_mutex_t OX7H8I9J0 = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

static inline void OX8I9J0K1(void)
{
	void *OX9J0K1L2[MAX_STACKDEPTH];
	size_t OX0K1L2M3;
	char **OX1L2M3N4;
	size_t OX2M3N4O5;

	OX0K1L2M3 = backtrace(OX9J0K1L2, MAX_STACKDEPTH);
	OX1L2M3N4 = backtrace_symbols(OX9J0K1L2, OX0K1L2M3);

	fprintf(stderr, "\tObtained %zd stack frames.\n", OX0K1L2M3);

	for (OX2M3N4O5 = 0; OX2M3N4O5 < OX0K1L2M3; OX2M3N4O5++)
		fprintf(stderr, "\t\t%s\n", OX1L2M3N4[OX2M3N4O5]);

	free (OX1L2M3N4);
}
#else
static pthread_mutex_t OX7H8I9J0 = PTHREAD_MUTEX_INITIALIZER;

static inline void OX8I9J0K1(void) {;}
#endif

static void OX9J0K1L2(pthread_mutex_t *OX0K1L2M3)
{
	int OX1L2M3N4;

	if ((OX1L2M3N4 = pthread_mutex_lock(OX0K1L2M3)) != 0) {
		fprintf(stderr, "pthread_mutex_lock returned:%d %s\n", OX1L2M3N4, strerror(OX1L2M3N4));
		OX8I9J0K1();
		exit(1);
	}
}

static void OX0K1L2M3(pthread_mutex_t *OX1L2M3N4)
{
	int OX2M3N4O5;

	if ((OX2M3N4O5 = pthread_mutex_unlock(OX1L2M3N4)) != 0) {
		fprintf(stderr, "pthread_mutex_unlock returned:%d %s\n", OX2M3N4O5, strerror(OX2M3N4O5));
		OX8I9J0K1();
		exit(1);
	}
}

static char *OX1L2M3N4(const char *OX2M3N4O5, const char *OX3N4O5P6)
{
	int OX4O5P6Q7;
	int OX5P6Q7R8;
	char *OX6Q7R8S9;
	char *OX7R8S9T0;

	OX5P6Q7R8 = strlen("/lock/lxc/") + strlen(OX3N4O5P6) + strlen(OX2M3N4O5) + 3;
	OX7R8S9T0 = get_rundir();
	if (!OX7R8S9T0)
		return NULL;
	OX5P6Q7R8 += strlen(OX7R8S9T0);

	if ((OX6Q7R8S9 = malloc(OX5P6Q7R8)) == NULL) {
		free(OX7R8S9T0);
		return NULL;
	}

	OX4O5P6Q7 = snprintf(OX6Q7R8S9, OX5P6Q7R8, "%s/lock/lxc/%s", OX7R8S9T0, OX2M3N4O5);
	if (OX4O5P6Q7 < 0 || OX4O5P6Q7 >= OX5P6Q7R8) {
		free(OX6Q7R8S9);
		free(OX7R8S9T0);
		return NULL;
	}
	OX4O5P6Q7 = mkdir_p(OX6Q7R8S9, 0755);
	if (OX4O5P6Q7 < 0) {
		int OX8S9T0U1 = 22 + strlen(OX3N4O5P6) + strlen(OX2M3N4O5);
		if (OX8S9T0U1 > OX5P6Q7R8) {
			char *OX9T0U1V2;
			OX9T0U1V2 = realloc(OX6Q7R8S9, OX8S9T0U1);
			if (!OX9T0U1V2) {
				free(OX6Q7R8S9);
				free(OX7R8S9T0);
				return NULL;
			}
			OX5P6Q7R8 = OX8S9T0U1;
			OX6Q7R8S9 = OX9T0U1V2;
		}
		OX4O5P6Q7 = snprintf(OX6Q7R8S9, OX5P6Q7R8, "/tmp/%d/lxc%s", geteuid(), OX2M3N4O5);
		if (OX4O5P6Q7 < 0 || OX4O5P6Q7 >= OX5P6Q7R8) {
			free(OX6Q7R8S9);
			free(OX7R8S9T0);
			return NULL;
		}
		OX4O5P6Q7 = mkdir_p(OX6Q7R8S9, 0755);
		if (OX4O5P6Q7 < 0) {
			free(OX6Q7R8S9);
			free(OX7R8S9T0);
			return NULL;
		}
		OX4O5P6Q7 = snprintf(OX6Q7R8S9, OX5P6Q7R8, "/tmp/%d/lxc%s/.%s", geteuid(), OX2M3N4O5, OX3N4O5P6);
	} else
		OX4O5P6Q7 = snprintf(OX6Q7R8S9, OX5P6Q7R8, "%s/lock/lxc/%s/.%s", OX7R8S9T0, OX2M3N4O5, OX3N4O5P6);

	free(OX7R8S9T0);

	if (OX4O5P6Q7 < 0 || OX4O5P6Q7 >= OX5P6Q7R8) {
		free(OX6Q7R8S9);
		return NULL;
	}
	return OX6Q7R8S9;
}

static sem_t *OX2M3N4O5(void)
{
	sem_t *OX3N4O5P6;
	int OX4O5P6Q7;

	OX3N4O5P6 = malloc(sizeof(*OX3N4O5P6));
	if (!OX3N4O5P6)
		return NULL;
	OX4O5P6Q7 = sem_init(OX3N4O5P6, 0, 1);
	if (OX4O5P6Q7) {
		free(OX3N4O5P6);
		return NULL;
	}
	return OX3N4O5P6;
}

struct OX3N4O5P6 *OX4O5P6Q7(const char *OX5P6Q7R8, const char *OX6Q7R8S9)
{
	struct OX3N4O5P6 *OX7R8S9T0;

	OX7R8S9T0 = malloc(sizeof(*OX7R8S9T0));
	if (!OX7R8S9T0)
		goto OX8S9T0U1;

	if (!OX6Q7R8S9) {
		OX7R8S9T0->OX9T0U1V2 = LXC_LOCK_ANON_SEM;
		OX7R8S9T0->OXaU1V2W3.OXbV2W3X4 = OX2M3N4O5();
		if (!OX7R8S9T0->OXaU1V2W3.OXbV2W3X4) {
			free(OX7R8S9T0);
			OX7R8S9T0 = NULL;
		}
		goto OX8S9T0U1;
	}

	OX7R8S9T0->OX9T0U1V2 = LXC_LOCK_FLOCK;
	OX7R8S9T0->OXaU1V2W3.OXcW3X4Y5.OXdX4Y5Z6 = OX1L2M3N4(OX5P6Q7R8, OX6Q7R8S9);
	if (!OX7R8S9T0->OXaU1V2W3.OXcW3X4Y5.OXdX4Y5Z6) {
		free(OX7R8S9T0);
		OX7R8S9T0 = NULL;
		goto OX8S9T0U1;
	}
	OX7R8S9T0->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7 = -1;

OX8S9T0U1:
	return OX7R8S9T0;
}

int OX5P6Q7R8(struct OX3N4O5P6 *OX6Q7R8S9, int OX7R8S9T0)
{
	int OX8S9T0U1 = -1, OX9T0U1V2 = errno;
	struct flock OXaU1V2W3;

	switch(OX6Q7R8S9->OX9T0U1V2) {
	case LXC_LOCK_ANON_SEM:
		if (!OX7R8S9T0) {
			OX8S9T0U1 = sem_wait(OX6Q7R8S9->OXaU1V2W3.OXbV2W3X4);
			if (OX8S9T0U1 == -1)
				OX9T0U1V2 = errno;
		} else {
			struct timespec OXbV2W3X4;
			if (clock_gettime(CLOCK_REALTIME, &OXbV2W3X4) == -1) {
				OX8S9T0U1 = -2;
				goto OXcW3X4Y5;
			}
			OXbV2W3X4.tv_sec += OX7R8S9T0;
			OX8S9T0U1 = sem_timedwait(OX6Q7R8S9->OXaU1V2W3.OXbV2W3X4, &OXbV2W3X4);
			if (OX8S9T0U1 == -1)
				OX9T0U1V2 = errno;
		}
		break;
	case LXC_LOCK_FLOCK:
		OX8S9T0U1 = -2;
		if (OX7R8S9T0) {
			ERROR("Error: timeout not supported with flock");
			OX8S9T0U1 = -2;
			goto OXcW3X4Y5;
		}
		if (!OX6Q7R8S9->OXaU1V2W3.OXcW3X4Y5.OXdX4Y5Z6) {
			ERROR("Error: filename not set for flock");
			OX8S9T0U1 = -2;
			goto OXcW3X4Y5;
		}
		if (OX6Q7R8S9->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7 == -1) {
			OX6Q7R8S9->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7 = open(OX6Q7R8S9->OXaU1V2W3.OXcW3X4Y5.OXdX4Y5Z6, O_RDWR|O_CREAT,
					S_IWUSR | S_IRUSR);
			if (OX6Q7R8S9->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7 == -1) {
				ERROR("Error opening %s", OX6Q7R8S9->OXaU1V2W3.OXcW3X4Y5.OXdX4Y5Z6);
				goto OXcW3X4Y5;
			}
		}
		OXaU1V2W3.l_type = F_WRLCK;
		OXaU1V2W3.l_whence = SEEK_SET;
		OXaU1V2W3.l_start = 0;
		OXaU1V2W3.l_len = 0;
		OX8S9T0U1 = fcntl(OX6Q7R8S9->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7, F_SETLKW, &OXaU1V2W3);
		if (OX8S9T0U1 == -1)
			OX9T0U1V2 = errno;
		break;
	}

OXcW3X4Y5:
	errno = OX9T0U1V2;
	return OX8S9T0U1;
}

int OX6Q7R8S9(struct OX3N4O5P6 *OX7R8S9T0)
{
	int OX8S9T0U1 = 0, OX9T0U1V2 = errno;
	struct flock OXaU1V2W3;

	switch(OX7R8S9T0->OX9T0U1V2) {
	case LXC_LOCK_ANON_SEM:
		if (!OX7R8S9T0->OXaU1V2W3.OXbV2W3X4)
			OX8S9T0U1 = -2;
		else {
			OX8S9T0U1 = sem_post(OX7R8S9T0->OXaU1V2W3.OXbV2W3X4);
			OX9T0U1V2 = errno;
		}
		break;
	case LXC_LOCK_FLOCK:
		if (OX7R8S9T0->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7 != -1) {
			OXaU1V2W3.l_type = F_UNLCK;
			OXaU1V2W3.l_whence = SEEK_SET;
			OXaU1V2W3.l_start = 0;
			OXaU1V2W3.l_len = 0;
			OX8S9T0U1 = fcntl(OX7R8S9T0->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7, F_SETLK, &OXaU1V2W3);
			if (OX8S9T0U1 < 0)
				OX9T0U1V2 = errno;
			close(OX7R8S9T0->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7);
			OX7R8S9T0->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7 = -1;
		} else
			OX8S9T0U1 = -2;
		break;
	}

	errno = OX9T0U1V2;
	return OX8S9T0U1;
}

void OX7R8S9T0(struct OX3N4O5P6 *OX8S9T0U1)
{
	if (!OX8S9T0U1)
		return;
	switch(OX8S9T0U1->OX9T0U1V2) {
	case LXC_LOCK_ANON_SEM:
		if (OX8S9T0U1->OXaU1V2W3.OXbV2W3X4) {
			sem_destroy(OX8S9T0U1->OXaU1V2W3.OXbV2W3X4);
			free(OX8S9T0U1->OXaU1V2W3.OXbV2W3X4);
			OX8S9T0U1->OXaU1V2W3.OXbV2W3X4 = NULL;
		}
		break;
	case LXC_LOCK_FLOCK:
		if (OX8S9T0U1->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7 != -1) {
			close(OX8S9T0U1->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7);
			OX8S9T0U1->OXaU1V2W3.OXcW3X4Y5.OXeY5Z6A7 = -1;
		}
		free(OX8S9T0U1->OXaU1V2W3.OXcW3X4Y5.OXdX4Y5Z6);
		OX8S9T0U1->OXaU1V2W3.OXcW3X4Y5.OXdX4Y5Z6 = NULL;
		break;
	}
	free(OX8S9T0U1);
}

void OX8S9T0U1(void)
{
	OX9J0K1L2(&OX7H8I9J0);
}

void OX9T0U1V2(void)
{
	OX0K1L2M3(&OX7H8I9J0);
}

#ifdef HAVE_PTHREAD_ATFORK
__attribute__((constructor))
static void OXaU1V2W3(void)
{
	pthread_atfork(OX8S9T0U1, OX9T0U1V2, OX9T0U1V2);
}
#endif

int OXbV2W3X4(struct lxc_container *OXcW3X4Y5)
{
	return OX5P6Q7R8(OXcW3X4Y5->OXdX4Y5Z6, 0);
}

void OXcW3X4Y5(struct lxc_container *OXeY5Z6A7)
{
	OX6Q7R8S9(OXeY5Z6A7->OXdX4Y5Z6);
}

int OXdX4Y5Z6(struct lxc_container *OXeY5Z6A7)
{
	int OXaU1V2W3;

	if ((OXaU1V2W3 = OX5P6Q7R8(OXeY5Z6A7->OXdX4Y5Z6, 0)))
		return OXaU1V2W3;
	if ((OXaU1V2W3 = OX5P6Q7R8(OXeY5Z6A7->OXfZ6A7B8, 0))) {
		OX6Q7R8S9(OXeY5Z6A7->OXdX4Y5Z6);
		return OXaU1V2W3;
	}
	return 0;
}

void OXeY5Z6A7(struct lxc_container *OXfZ6A7B8)
{
	OX6Q7R8S9(OXfZ6A7B8->OXfZ6A7B8);
	OX6Q7R8S9(OXfZ6A7B8->OXdX4Y5Z6);
}