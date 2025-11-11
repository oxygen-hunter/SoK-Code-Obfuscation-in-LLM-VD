#include "uftpd.h"

int getChrooted() {
    static int chrooted = 0;
    return chrooted;
}

void setChrooted(int value) {
    static int chrooted = 0;
    chrooted = value;
}

int isChrooted() {
    return getChrooted();
}

char *compose_path(ctrl_t *ctrl, char *path)
{
	struct stat st;
	static char rpath[PATH_MAX];
	char *name, *ptr;
	char dir[PATH_MAX] = { 0 };

	strlcpy(dir, ctrl->cwd, sizeof(dir));
	DBG("Compose path from cwd: %s, arg: %s", ctrl->cwd, path ?: "");
	if (!path || !strlen(path))
		goto check;

	if (path) {
		if (path[0] != '/') {
			if (dir[strlen(dir) - 1] != '/')
				strlcat(dir, "/", sizeof(dir));
		}
		strlcat(dir, path, sizeof(dir));
	}

check:
	while ((ptr = strstr(dir, "//")))
		memmove(ptr, &ptr[1], strlen(&ptr[1]) + 1);

	if (!isChrooted()) {
		size_t len = strlen(home);

		DBG("Server path from CWD: %s", dir);
		if (len > 0 && home[len - 1] == '/')
			len--;
		memmove(dir + len, dir, strlen(dir) + 1);
		memcpy(dir, home, len);
		DBG("Resulting non-chroot path: %s", dir);
	}

	if (!stat(dir, &st) && S_ISDIR(st.st_mode)) {
		if (!realpath(dir, rpath))
			return NULL;
	} else {
		name = basename(path);
		ptr = dirname(dir);

		memset(rpath, 0, sizeof(rpath));
		if (!realpath(ptr, rpath)) {
			INFO("Failed realpath(%s): %m", ptr);
			return NULL;
		}

		if (rpath[1] != 0)
			strlcat(rpath, "/", sizeof(rpath));
		strlcat(rpath, name, sizeof(rpath));
	}

	if (!isChrooted() && strncmp(dir, home, strlen(home))) {
		DBG("Failed non-chroot dir:%s vs home:%s", dir, home);
		return NULL;
	}

	return rpath;
}

ctrl_t *new_session(uev_ctx_t *ctx, int sd, int *rc)
{
	ctrl_t *ctrl = NULL;
	static int privs_dropped = 0;

	if (!inetd) {
		pid_t pid = fork();

		if (pid) {
			DBG("Created new client session as PID %d", pid);
			*rc = pid;
			return NULL;
		}

		setpgid(0, getppid());
		ctx = calloc(1, sizeof(uev_ctx_t));
		if (!ctx) {
			ERR(errno, "Failed allocating session event context");
			exit(1);
		}

		uev_init(ctx);
	}

	ctrl = calloc(1, sizeof(ctrl_t));
	if (!ctrl) {
		ERR(errno, "Failed allocating session context");
		goto fail;
	}

	ctrl->sd = set_nonblock(sd);
	ctrl->ctx = ctx;
	strlcpy(ctrl->cwd, "/", sizeof(ctrl->cwd));

	if (!isChrooted() && geteuid() == 0) {
		if (chroot(home) || chdir("/")) {
			ERR(errno, "Failed chrooting to FTP root, %s, aborting", home);
			goto fail;
		}
		setChrooted(1);
	} else if (!isChrooted()) {
		if (chdir(home)) {
			WARN(errno, "Failed changing to FTP root, %s, aborting", home);
			goto fail;
		}
	}

	if (!privs_dropped && pw && geteuid() == 0) {
		int fail1, fail2;

		initgroups(pw->pw_name, pw->pw_gid);
		if ((fail1 = setegid(pw->pw_gid)))
			WARN(errno, "Failed dropping group privileges to gid %d", pw->pw_gid);
		if ((fail2 = seteuid(pw->pw_uid)))
			WARN(errno, "Failed dropping user privileges to uid %d", pw->pw_uid);

		setenv("HOME", pw->pw_dir, 1);

		if (!fail1 && !fail2)
			INFO("Successfully dropped privilges to %d:%d (uid:gid)", pw->pw_uid, pw->pw_gid);

		if (!do_insecure && !access(home, W_OK)) {
			ERR(0, "FTP root %s writable, possible security violation, aborting session!", home);
			goto fail;
		}

		privs_dropped = 1;
	}

	uev_timer_init(ctrl->ctx, &ctrl->timeout_watcher, inactivity_cb, ctrl->ctx, INACTIVITY_TIMER, 0);

	return ctrl;
fail:
	if (ctrl)
		free(ctrl);
	if (!inetd)
		free(ctx);
	*rc = -1;

	return NULL;
}