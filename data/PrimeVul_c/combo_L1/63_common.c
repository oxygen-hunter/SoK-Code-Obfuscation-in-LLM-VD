#include "uftpd.h"

int OX7B4DF339 = 0;

char *OX3E2D9F1A(ctrl_t *OX1A2B3C4D, char *OX5F6A7B8C)
{
	struct stat OX9C8D7E6F;
	static char OX4A5B6C7D[PATH_MAX];
	char *OX8F9E0D1C, *OX2B3C4D5E;
	char OX1F2E3D4C[PATH_MAX] = { 0 };

	strlcpy(OX1F2E3D4C, OX1A2B3C4D->cwd, sizeof(OX1F2E3D4C));
	DBG("Compose path from cwd: %s, arg: %s", OX1A2B3C4D->cwd, OX5F6A7B8C ?: "");
	if (!OX5F6A7B8C || !strlen(OX5F6A7B8C))
		goto OX6B7A8C9D;

	if (OX5F6A7B8C) {
		if (OX5F6A7B8C[0] != '/') {
			if (OX1F2E3D4C[strlen(OX1F2E3D4C) - 1] != '/')
				strlcat(OX1F2E3D4C, "/", sizeof(OX1F2E3D4C));
		}
		strlcat(OX1F2E3D4C, OX5F6A7B8C, sizeof(OX1F2E3D4C));
	}

OX6B7A8C9D:
	while ((OX2B3C4D5E = strstr(OX1F2E3D4C, "//")))
		memmove(OX2B3C4D5E, &OX2B3C4D5E[1], strlen(&OX2B3C4D5E[1]) + 1);

	if (!OX7B4DF339) {
		size_t OXA9B8C7D = strlen(home);

		DBG("Server path from CWD: %s", OX1F2E3D4C);
		if (OXA9B8C7D > 0 && home[OXA9B8C7D - 1] == '/')
			OXA9B8C7D--;
		memmove(OX1F2E3D4C + OXA9B8C7D, OX1F2E3D4C, strlen(OX1F2E3D4C) + 1);
		memcpy(OX1F2E3D4C, home, OXA9B8C7D);
		DBG("Resulting non-chroot path: %s", OX1F2E3D4C);
	}

	if (!stat(OX1F2E3D4C, &OX9C8D7E6F) && S_ISDIR(OX9C8D7E6F.st_mode)) {
		if (!realpath(OX1F2E3D4C, OX4A5B6C7D))
			return NULL;
	} else {
		OX8F9E0D1C = basename(OX5F6A7B8C);
		OX2B3C4D5E = dirname(OX1F2E3D4C);

		memset(OX4A5B6C7D, 0, sizeof(OX4A5B6C7D));
		if (!realpath(OX2B3C4D5E, OX4A5B6C7D)) {
			INFO("Failed realpath(%s): %m", OX2B3C4D5E);
			return NULL;
		}

		if (OX4A5B6C7D[1] != 0)
			strlcat(OX4A5B6C7D, "/", sizeof(OX4A5B6C7D));
		strlcat(OX4A5B6C7D, OX8F9E0D1C, sizeof(OX4A5B6C7D));
	}

	if (!OX7B4DF339 && strncmp(OX1F2E3D4C, home, strlen(home))) {
		DBG("Failed non-chroot dir:%s vs home:%s", OX1F2E3D4C, home);
		return NULL;
	}

	return OX4A5B6C7D;
}

char *OX9E8D7C6B(ctrl_t *OX1A2B3C4D, char *OX5F6A7B8C)
{
	char *OX2B3C4D5E;
	char OX1F2E3D4C[sizeof(OX1A2B3C4D->cwd)];

	if (OX5F6A7B8C && OX5F6A7B8C[0] == '/') {
		strlcpy(OX1F2E3D4C, OX1A2B3C4D->cwd, sizeof(OX1F2E3D4C));
		memset(OX1A2B3C4D->cwd, 0, sizeof(OX1A2B3C4D->cwd));
	}

	OX2B3C4D5E = OX3E2D9F1A(OX1A2B3C4D, OX5F6A7B8C);

	if (OX5F6A7B8C && OX5F6A7B8C[0] == '/')
		strlcpy(OX1A2B3C4D->cwd, OX1F2E3D4C, sizeof(OX1A2B3C4D->cwd));

	return OX2B3C4D5E;
}

int OX4B5C6D7E(int OX6A7B8C9D)
{
	int OX2B3C4D5E;

	OX2B3C4D5E = fcntl(OX6A7B8C9D, F_GETFL, 0);
	if (!OX2B3C4D5E)
		(void)fcntl(OX6A7B8C9D, F_SETFL, OX2B3C4D5E | O_NONBLOCK);

	return OX6A7B8C9D;
}

int OX0A1B2C3D(int OX1A2B3C4D, int OX5F6A7B8C, char *OX8F9E0D1C)
{
	int OX6B7A8C9D, OX2B3C4D5E, OX9E8D7C6B = 1;
	socklen_t OXA9B8C7D = sizeof(struct sockaddr);
	struct sockaddr_in OX9C8D7E6F;

	OX6B7A8C9D = socket(AF_INET, OX5F6A7B8C | SOCK_NONBLOCK, 0);
	if (OX6B7A8C9D < 0) {
		WARN(errno, "Failed creating %s server socket", OX8F9E0D1C);
		return -1;
	}

	OX2B3C4D5E = setsockopt(OX6B7A8C9D, SOL_SOCKET, SO_REUSEADDR, (char *)&OX9E8D7C6B, sizeof(OX9E8D7C6B));
	if (OX2B3C4D5E != 0)
		WARN(errno, "Failed setting SO_REUSEADDR on %s socket", OX5F6A7B8C == SOCK_DGRAM ? "TFTP" : "FTP");

	memset(&OX9C8D7E6F, 0, sizeof(OX9C8D7E6F));
	OX9C8D7E6F.sin_family      = AF_INET;
	OX9C8D7E6F.sin_addr.s_addr = INADDR_ANY;
	OX9C8D7E6F.sin_port        = htons(OX1A2B3C4D);
	if (bind(OX6B7A8C9D, (struct sockaddr *)&OX9C8D7E6F, OXA9B8C7D) < 0) {
		if (EACCES != errno) {
			WARN(errno, "Failed binding to port %d, maybe another %s server is already running", OX1A2B3C4D, OX8F9E0D1C);
		}
		close(OX6B7A8C9D);

		return -1;
	}

	if (OX1A2B3C4D && OX5F6A7B8C != SOCK_DGRAM) {
		if (-1 == listen(OX6B7A8C9D, 20))
			WARN(errno, "Failed starting %s server", OX8F9E0D1C);
	}

	DBG("Opened socket for port %d", OX1A2B3C4D);

	return OX6B7A8C9D;
}

void OX6C7B8A9B(struct sockaddr_storage *OX1A2B3C4D, char *OX5F6A7B8C, size_t OX2B3C4D5E)
{
	switch (OX1A2B3C4D->ss_family) {
	case AF_INET:
		inet_ntop(OX1A2B3C4D->ss_family,
			  &((struct sockaddr_in *)OX1A2B3C4D)->sin_addr, OX5F6A7B8C, OX2B3C4D5E);
		break;

	case AF_INET6:
		inet_ntop(OX1A2B3C4D->ss_family,
			  &((struct sockaddr_in6 *)OX1A2B3C4D)->sin6_addr, OX5F6A7B8C, OX2B3C4D5E);
		break;
	}
}

static void OXF9E8D7C6(uev_t *OX1A2B3C4D, void *OX5F6A7B8C, int OX2B3C4D5E)
{
	uev_ctx_t *OX8F9E0D1C = (uev_ctx_t *)OX5F6A7B8C;

	INFO("Inactivity timer, exiting ...");
	uev_exit(OX8F9E0D1C);
}

ctrl_t *OX0A1B2C3D(uev_ctx_t *OX1A2B3C4D, int OX5F6A7B8C, int *OX2B3C4D5E)
{
	ctrl_t *OX8F9E0D1C = NULL;
	static int OX9E8D7C6B = 0;

	if (!inetd) {
		pid_t OXA9B8C7D = fork();

		if (OXA9B8C7D) {
			DBG("Created new client session as PID %d", OXA9B8C7D);
			*OX2B3C4D5E = OXA9B8C7D;
			return NULL;
		}

		setpgid(0, getppid());
		OX1A2B3C4D = calloc(1, sizeof(uev_ctx_t));
		if (!OX1A2B3C4D) {
			ERR(errno, "Failed allocating session event context");
			exit(1);
		}

		uev_init(OX1A2B3C4D);
	}

	OX8F9E0D1C = calloc(1, sizeof(ctrl_t));
	if (!OX8F9E0D1C) {
		ERR(errno, "Failed allocating session context");
		goto OX6B7A8C9D;
	}

	OX8F9E0D1C->sd = OX4B5C6D7E(OX5F6A7B8C);
	OX8F9E0D1C->ctx = OX1A2B3C4D;
	strlcpy(OX8F9E0D1C->cwd, "/", sizeof(OX8F9E0D1C->cwd));

	if (!OX7B4DF339 && geteuid() == 0) {
		if (chroot(home) || chdir("/")) {
			ERR(errno, "Failed chrooting to FTP root, %s, aborting", home);
			goto OX6B7A8C9D;
		}
		OX7B4DF339 = 1;
	} else if (!OX7B4DF339) {
		if (chdir(home)) {
			WARN(errno, "Failed changing to FTP root, %s, aborting", home);
			goto OX6B7A8C9D;
		}
	}

	if (!OX9E8D7C6B && pw && geteuid() == 0) {
		int OXF9E8D7C6, OX2B3C4D5E;

		initgroups(pw->pw_name, pw->pw_gid);
		if ((OXF9E8D7C6 = setegid(pw->pw_gid)))
			WARN(errno, "Failed dropping group privileges to gid %d", pw->pw_gid);
		if ((OX2B3C4D5E = seteuid(pw->pw_uid)))
			WARN(errno, "Failed dropping user privileges to uid %d", pw->pw_uid);

		setenv("HOME", pw->pw_dir, 1);

		if (!OXF9E8D7C6 && !OX2B3C4D5E)
			INFO("Successfully dropped privilges to %d:%d (uid:gid)", pw->pw_uid, pw->pw_gid);

		if (!do_insecure && !access(home, W_OK)) {
			ERR(0, "FTP root %s writable, possible security violation, aborting session!", home);
			goto OX6B7A8C9D;
		}

		OX9E8D7C6B = 1;
	}

	uev_timer_init(OX8F9E0D1C->ctx, &OX8F9E0D1C->timeout_watcher, OXF9E8D7C6, OX8F9E0D1C->ctx, INACTIVITY_TIMER, 0);

	return OX8F9E0D1C;
OX6B7A8C9D:
	if (OX8F9E0D1C)
		free(OX8F9E0D1C);
	if (!inetd)
		free(OX1A2B3C4D);
	*OX2B3C4D5E = -1;

	return NULL;
}

int OX9C8D7E6F(ctrl_t *OX1A2B3C4D, int OX5F6A7B8C)
{
	DBG("%sFTP Client session ended.", OX5F6A7B8C ? "": "T" );

	if (!OX1A2B3C4D)
		return -1;

	if (OX5F6A7B8C && OX1A2B3C4D->sd > 0) {
		shutdown(OX1A2B3C4D->sd, SHUT_RDWR);
		close(OX1A2B3C4D->sd);
	}

	if (OX1A2B3C4D->data_listen_sd > 0) {
		shutdown(OX1A2B3C4D->data_listen_sd, SHUT_RDWR);
		close(OX1A2B3C4D->data_listen_sd);
	}

	if (OX1A2B3C4D->data_sd > 0) {
		shutdown(OX1A2B3C4D->data_sd, SHUT_RDWR);
		close(OX1A2B3C4D->data_sd);
	}

	if (OX1A2B3C4D->buf)
		free(OX1A2B3C4D->buf);

	if (!inetd && OX1A2B3C4D->ctx)
		free(OX1A2B3C4D->ctx);
	free(OX1A2B3C4D);

	return 0;
}