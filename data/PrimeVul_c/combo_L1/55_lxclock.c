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

#define OX5B3D5E1A 25

#define OX4F6D3C06 (O_CREAT | O_RDWR)
#define OX4C4A1F8B 0660
#define OX7D1B2C3E 1
#define OX3E5A9B7F 0

lxc_log_define(OX1D9C4B7A, lxc);

#ifdef MUTEX_DEBUGGING
static pthread_mutex_t OX6A4B2D8E = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

static inline void OX2A3E8D1C(void)
{
	void *OX7F2D3A1B[OX5B3D5E1A];
	size_t OX3C2A7D1E;
	char **OX6F1B3D4E;
	size_t OX7E5C2B1D;

	OX3C2A7D1E = backtrace(OX7F2D3A1B, OX5B3D5E1A);
	OX6F1B3D4E = backtrace_symbols(OX7F2D3A1B, OX3C2A7D1E);

	fprintf(stderr, "\tObtained %zd stack frames.\n", OX3C2A7D1E);

	for (OX7E5C2B1D = 0; OX7E5C2B1D < OX3C2A7D1E; OX7E5C2B1D++)
		fprintf(stderr, "\t\t%s\n", OX6F1B3D4E[OX7E5C2B1D]);

	free(OX6F1B3D4E);
}
#else
static pthread_mutex_t OX6A4B2D8E = PTHREAD_MUTEX_INITIALIZER;

static inline void OX2A3E8D1C(void) {;}
#endif

static void OX1E3C5A7B(pthread_mutex_t *OX7C1D2F8E)
{
	int OX4B1F6A3D;

	if ((OX4B1F6A3D = pthread_mutex_lock(OX7C1D2F8E)) != 0) {
		fprintf(stderr, "pthread_mutex_lock returned:%d %s\n", OX4B1F6A3D, strerror(OX4B1F6A3D));
		OX2A3E8D1C();
		exit(1);
	}
}

static void OX6B2F3C1E(pthread_mutex_t *OX7C1D2F8E)
{
	int OX4B1F6A3D;

	if ((OX4B1F6A3D = pthread_mutex_unlock(OX7C1D2F8E)) != 0) {
		fprintf(stderr, "pthread_mutex_unlock returned:%d %s\n", OX4B1F6A3D, strerror(OX4B1F6A3D));
		OX2A3E8D1C();
		exit(1);
	}
}

static char *OX4E7B2C1D(const char *OX7E3D1A2C, const char *OX1F6A4B2D)
{
	int OX4B3E2F7A;
	int OX7C1A6D3E;
	char *OX2D4F1B6C;
	char *OX1E7B3C5A;

	OX7C1A6D3E = strlen("/lock/lxc/") + strlen(OX1F6A4B2D) + strlen(OX7E3D1A2C) + 2;
	OX1E7B3C5A = get_rundir();
	if (!OX1E7B3C5A)
		return NULL;
	OX7C1A6D3E += strlen(OX1E7B3C5A);

	if ((OX2D4F1B6C = malloc(OX7C1A6D3E)) == NULL) {
		free(OX1E7B3C5A);
		return NULL;
	}

	OX4B3E2F7A = snprintf(OX2D4F1B6C, OX7C1A6D3E, "%s/lock/lxc/%s", OX1E7B3C5A, OX7E3D1A2C);
	if (OX4B3E2F7A < 0 || OX4B3E2F7A >= OX7C1A6D3E) {
		free(OX2D4F1B6C);
		free(OX1E7B3C5A);
		return NULL;
	}
	OX4B3E2F7A = mkdir_p(OX2D4F1B6C, 0755);
	if (OX4B3E2F7A < 0) {
		int OX5A3C7D1E = 33 + strlen(OX1F6A4B2D) + strlen(OX7E3D1A2C);
		if (OX5A3C7D1E > OX7C1A6D3E) {
			char *OX7E1B5C3D;
			OX7E1B5C3D = realloc(OX2D4F1B6C, OX5A3C7D1E);
			if (!OX7E1B5C3D) {
				free(OX2D4F1B6C);
				free(OX1E7B3C5A);
				return NULL;
			}
			OX7C1A6D3E = OX5A3C7D1E;
			OX2D4F1B6C = OX7E1B5C3D;
		}
		OX4B3E2F7A = snprintf(OX2D4F1B6C, OX7C1A6D3E, "/tmp/%d/lxc/%s", geteuid(), OX7E3D1A2C);
		if (OX4B3E2F7A < 0 || OX4B3E2F7A >= OX7C1A6D3E) {
			free(OX2D4F1B6C);
			free(OX1E7B3C5A);
			return NULL;
		}
		OX4B3E2F7A = snprintf(OX2D4F1B6C, OX7C1A6D3E, "/tmp/%d/lxc/%s/%s", geteuid(), OX7E3D1A2C, OX1F6A4B2D);
	} else
		OX4B3E2F7A = snprintf(OX2D4F1B6C, OX7C1A6D3E, "%s/lock/lxc/%s/%s", OX1E7B3C5A, OX7E3D1A2C, OX1F6A4B2D);

	free(OX1E7B3C5A);

	if (OX4B3E2F7A < 0 || OX4B3E2F7A >= OX7C1A6D3E) {
		free(OX2D4F1B6C);
		return NULL;
	}
	return OX2D4F1B6C;
}

static sem_t *OX6B4A3C2D(void)
{
	sem_t *OX1E7C2F6A;
	int OX4B3E2F7A;

	OX1E7C2F6A = malloc(sizeof(*OX1E7C2F6A));
	if (!OX1E7C2F6A)
		return NULL;
	OX4B3E2F7A = sem_init(OX1E7C2F6A, 0, 1);
	if (OX4B3E2F7A) {
		free(OX1E7C2F6A);
		return NULL;
	}
	return OX1E7C2F6A;
}

struct OX3E1D7B2C *OX4B6F2A3D(const char *OX7E3D1A2C, const char *OX1F6A4B2D)
{
	struct OX3E1D7B2C *OX1A7C5D3E;

	OX1A7C5D3E = malloc(sizeof(*OX1A7C5D3E));
	if (!OX1A7C5D3E)
		goto OX7A4B1C2D;

	if (!OX1F6A4B2D) {
		OX1A7C5D3E->OX5C3D2A4B = LXC_LOCK_ANON_SEM;
		OX1A7C5D3E->OX6B7A1E2F.OX7F2D3A1B = OX6B4A3C2D();
		if (!OX1A7C5D3E->OX6B7A1E2F.OX7F2D3A1B) {
			free(OX1A7C5D3E);
			OX1A7C5D3E = NULL;
		}
		goto OX7A4B1C2D;
	}

	OX1A7C5D3E->OX5C3D2A4B = LXC_LOCK_FLOCK;
	OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX3E1D7B2C = OX4E7B2C1D(OX7E3D1A2C, OX1F6A4B2D);
	if (!OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX3E1D7B2C) {
		free(OX1A7C5D3E);
		OX1A7C5D3E = NULL;
		goto OX7A4B1C2D;
	}
	OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D = -1;

OX7A4B1C2D:
	return OX1A7C5D3E;
}

int OX7C4B2D1E(struct OX3E1D7B2C *OX1A7C5D3E, int OX5F3A1B7D)
{
	int OX4B3E2F7A = -1, OX7E1C5D3F = errno;
	struct flock OX6B4A3C2D;

	switch(OX1A7C5D3E->OX5C3D2A4B) {
	case LXC_LOCK_ANON_SEM:
		if (!OX5F3A1B7D) {
			OX4B3E2F7A = sem_wait(OX1A7C5D3E->OX6B7A1E2F.OX7F2D3A1B);
			if (OX4B3E2F7A == -1)
				OX7E1C5D3F = errno;
		} else {
			struct timespec OX3E1D7B2C;
			if (clock_gettime(CLOCK_REALTIME, &OX3E1D7B2C) == -1) {
				OX4B3E2F7A = -2;
				goto OX7A4B1C2D;
			}
			OX3E1D7B2C.tv_sec += OX5F3A1B7D;
			OX4B3E2F7A = sem_timedwait(OX1A7C5D3E->OX6B7A1E2F.OX7F2D3A1B, &OX3E1D7B2C);
			if (OX4B3E2F7A == -1)
				OX7E1C5D3F = errno;
		}
		break;
	case LXC_LOCK_FLOCK:
		OX4B3E2F7A = -2;
		if (OX5F3A1B7D) {
			ERROR("Error: timeout not supported with flock");
			OX4B3E2F7A = -2;
			goto OX7A4B1C2D;
		}
		if (!OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX3E1D7B2C) {
			ERROR("Error: filename not set for flock");
			OX4B3E2F7A = -2;
			goto OX7A4B1C2D;
		}
		if (OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D == -1) {
			OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D = open(OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX3E1D7B2C, O_RDWR|O_CREAT,
					S_IWUSR | S_IRUSR);
			if (OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D == -1) {
				ERROR("Error opening %s", OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX3E1D7B2C);
				goto OX7A4B1C2D;
			}
		}
		OX6B4A3C2D.l_type = F_WRLCK;
		OX6B4A3C2D.l_whence = SEEK_SET;
		OX6B4A3C2D.l_start = 0;
		OX6B4A3C2D.l_len = 0;
		OX4B3E2F7A = fcntl(OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D, F_SETLKW, &OX6B4A3C2D);
		if (OX4B3E2F7A == -1)
			OX7E1C5D3F = errno;
		break;
	}

OX7A4B1C2D:
	errno = OX7E1C5D3F;
	return OX4B3E2F7A;
}

int OX6F3A2B1E(struct OX3E1D7B2C *OX1A7C5D3E)
{
	int OX4B3E2F7A = 0, OX7E1C5D3F = errno;
	struct flock OX6B4A3C2D;

	switch(OX1A7C5D3E->OX5C3D2A4B) {
	case LXC_LOCK_ANON_SEM:
		if (!OX1A7C5D3E->OX6B7A1E2F.OX7F2D3A1B)
			OX4B3E2F7A = -2;
		else {
			OX4B3E2F7A = sem_post(OX1A7C5D3E->OX6B7A1E2F.OX7F2D3A1B);
			OX7E1C5D3F = errno;
		}
		break;
	case LXC_LOCK_FLOCK:
		if (OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D != -1) {
			OX6B4A3C2D.l_type = F_UNLCK;
			OX6B4A3C2D.l_whence = SEEK_SET;
			OX6B4A3C2D.l_start = 0;
			OX6B4A3C2D.l_len = 0;
			OX4B3E2F7A = fcntl(OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D, F_SETLK, &OX6B4A3C2D);
			if (OX4B3E2F7A < 0)
				OX7E1C5D3F = errno;
			close(OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D);
			OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D = -1;
		} else
			OX4B3E2F7A = -2;
		break;
	}

	errno = OX7E1C5D3F;
	return OX4B3E2F7A;
}

void OX3D7B1C2F(struct OX3E1D7B2C *OX1A7C5D3E)
{
	if (!OX1A7C5D3E)
		return;
	switch(OX1A7C5D3E->OX5C3D2A4B) {
	case LXC_LOCK_ANON_SEM:
		if (OX1A7C5D3E->OX6B7A1E2F.OX7F2D3A1B) {
			sem_destroy(OX1A7C5D3E->OX6B7A1E2F.OX7F2D3A1B);
			free(OX1A7C5D3E->OX6B7A1E2F.OX7F2D3A1B);
			OX1A7C5D3E->OX6B7A1E2F.OX7F2D3A1B = NULL;
		}
		break;
	case LXC_LOCK_FLOCK:
		if (OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D != -1) {
			close(OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D);
			OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX4B1F6A3D = -1;
		}
		if (OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX3E1D7B2C) {
			free(OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX3E1D7B2C);
			OX1A7C5D3E->OX6B7A1E2F.OX5D4E2C1A.OX3E1D7B2C = NULL;
		}
		break;
	}
	free(OX1A7C5D3E);
}

void OX7A3E1C5B(void)
{
	OX1E3C5A7B(&OX6A4B2D8E);
}

void OX2A6F3B1D(void)
{
	OX6B2F3C1E(&OX6A4B2D8E);
}

#ifdef HAVE_PTHREAD_ATFORK
__attribute__((constructor))
static void OX1D6C3A7F(void)
{
	pthread_atfork(OX7A3E1C5B, OX2A6F3B1D, OX2A6F3B1D);
}
#endif

int OX7C1E5D4B(struct lxc_container *OX1B3A6F2C)
{
	return OX7C4B2D1E(OX1B3A6F2C->privlock, 0);
}

void OX1D3A7C5B(struct lxc_container *OX1B3A6F2C)
{
	OX6F3A2B1E(OX1B3A6F2C->privlock);
}

int OX6A3C5E7B(struct lxc_container *OX1B3A6F2C)
{
	int OX4B3E2F7A;

	if ((OX4B3E2F7A = OX7C4B2D1E(OX1B3A6F2C->privlock, 0)))
		return OX4B3E2F7A;
	if ((OX4B3E2F7A = OX7C4B2D1E(OX1B3A6F2C->slock, 0))) {
		OX6F3A2B1E(OX1B3A6F2C->privlock);
		return OX4B3E2F7A;
	}
	return 0;
}

void OX3F5B2E1A(struct lxc_container *OX1B3A6F2C)
{
	OX6F3A2B1E(OX1B3A6F2C->slock);
	OX6F3A2B1E(OX1B3A6F2C->privlock);
}