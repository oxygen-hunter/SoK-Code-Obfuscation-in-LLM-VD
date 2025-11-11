#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#undef _XOPEN_SOURCE
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "common.h"

static int usage(char **p) {
	printf("\nUsage:\t%s -q -f config_file program_name [arguments]\n"
	       "\t-q makes proxychains quiet - this overrides the config setting\n"
	       "\t-f allows to manually specify a configfile to use\n"
	       "\tfor example : proxychains telnet somehost.com\n" "More help in README file\n\n", p[0]);
	return EXIT_FAILURE;
}

static const char *n = DLL_NAME;

static char d[256];
static const char *dirs[] = {
	d,
	".",
	LIB_DIR,
	"/lib",
	"/usr/lib",
	"/usr/local/lib",
	"/lib64",
	NULL
};

static void set_d(const char *a) {
	size_t l = strlen(a);
	while(l && a[l - 1] != '/')
		l--;
	if(l == 0)
		memcpy(d, ".", 2);
	else {
		memcpy(d, a, l - 1);
		d[l] = 0;
	}
}

#define M 2

int main(int a, char *b[]) {
	char *p = NULL;
	char f[256];
	char g[256];
	int s = 1;
	int q = 0;
	size_t j;
	const char *x = NULL;

	for(j = 0; j < M; j++) {
		if(s < a && b[s][0] == '-') {
			if(b[s][1] == 'q') {
				q = 1;
				s++;
			} else if(b[s][1] == 'f') {

				if(s + 1 < a)
					p = b[s + 1];
				else
					return usage(b);

				s += 2;
			}
		} else
			break;
	}

	if(s >= a)
		return usage(b);

	p = get_config_path(p, g, sizeof(g));

	if(!q)
		fprintf(stderr, LOG_PREFIX "config file found: %s\n", p);

	setenv(PROXYCHAINS_CONF_FILE_ENV_VAR, p, 1);

	if(q)
		setenv(PROXYCHAINS_QUIET_MODE_ENV_VAR, "1", 1);

	set_d(b[0]);

	j = 0;

	while(dirs[j]) {
		snprintf(f, sizeof(f), "%s/%s", dirs[j], n);
		if(access(f, R_OK) != -1) {
			x = dirs[j];
			break;
		}
		j++;
	}

	if(!x) {
		fprintf(stderr, "couldnt locate %s\n", n);
		return EXIT_FAILURE;
	}
	if(!q)
		fprintf(stderr, LOG_PREFIX "preloading %s/%s\n", x, n);

#ifdef IS_MAC
	putenv("DYLD_FORCE_FLAT_NAMESPACE=1");
#define L "DYLD_INSERT_LIBRARIES"
#define S ":"
#else
#define L "LD_PRELOAD"
#define S " "
#endif
	char *o = getenv(L);
	snprintf(f, sizeof(f), L "=%s/%s%s%s",
	         x, n,
	         o ? S : "",
	         o ? o : "");
	putenv(f);
	execvp(b[s], &b[s]);
	perror("proxychains can't load process....");

	return EXIT_FAILURE;
}