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

static int OX7B4DF339(char **OXA1B2C3D4) {
	printf("\nUsage:\t%s -q -f config_file program_name [arguments]\n"
	       "\t-q makes proxychains quiet - this overrides the config setting\n"
	       "\t-f allows to manually specify a configfile to use\n"
	       "\tfor example : proxychains telnet somehost.com\n" "More help in README file\n\n", OXA1B2C3D4[0]);
	return EXIT_FAILURE;
}

static const char *OXE5F6A7B8 = DLL_NAME;

static char OX9C8D0E1F[256];
static const char *OX2A3B4C5D[] = {
	".",
	OX9C8D0E1F,
	LIB_DIR,
	"/lib",
	"/usr/lib",
	"/usr/local/lib",
	"/lib64",
	NULL
};

static void OX6D7E8F90(const char *OXE1F2A3B) {
	size_t OXC5D6E7F8 = strlen(OXE1F2A3B);
	while(OXC5D6E7F8 && OXE1F2A3B[OXC5D6E7F8 - 1] != '/')
		OXC5D6E7F8--;
	if(OXC5D6E7F8 == 0)
		memcpy(OX9C8D0E1F, ".", 2);
	else {
		memcpy(OX9C8D0E1F, OXE1F2A3B, OXC5D6E7F8 - 1);
		OX9C8D0E1F[OXC5D6E7F8] = 0;
	}
}

#define OX3B4C5D6E 2

int main(int OXA2B3C4D5, char *OXF6A7B8C9[]) {
	char *OX9D0E1F2G = NULL;
	char OX3E4F5A6B[256];
	char OX7C8D9E0F[256];
	int OX4A5B6C7D = 1;
	int OX8E9F0A1B = 0;
	size_t OXE9F0A1B2;
	const char *OX5D6E7F8G = NULL;

	for(OXE9F0A1B2 = 0; OXE9F0A1B2 < OX3B4C5D6E; OXE9F0A1B2++) {
		if(OX4A5B6C7D < OXA2B3C4D5 && OXF6A7B8C9[OX4A5B6C7D][0] == '-') {
			if(OXF6A7B8C9[OX4A5B6C7D][1] == 'q') {
				OX8E9F0A1B = 1;
				OX4A5B6C7D++;
			} else if(OXF6A7B8C9[OX4A5B6C7D][1] == 'f') {

				if(OX4A5B6C7D + 1 < OXA2B3C4D5)
					OX9D0E1F2G = OXF6A7B8C9[OX4A5B6C7D + 1];
				else
					return OX7B4DF339(OXF6A7B8C9);

				OX4A5B6C7D += 2;
			}
		} else
			break;
	}

	if(OX4A5B6C7D >= OXA2B3C4D5)
		return OX7B4DF339(OXF6A7B8C9);

	OX9D0E1F2G = get_config_path(OX9D0E1F2G, OX7C8D9E0F, sizeof(OX7C8D9E0F));

	if(!OX8E9F0A1B)
		fprintf(stderr, LOG_PREFIX "config file found: %s\n", OX9D0E1F2G);

	setenv(PROXYCHAINS_CONF_FILE_ENV_VAR, OX9D0E1F2G, 1);

	if(OX8E9F0A1B)
		setenv(PROXYCHAINS_QUIET_MODE_ENV_VAR, "1", 1);

	OX6D7E8F90(OXF6A7B8C9[0]);

	OXE9F0A1B2 = 0;

	while(OX2A3B4C5D[OXE9F0A1B2]) {
		snprintf(OX3E4F5A6B, sizeof(OX3E4F5A6B), "%s/%s", OX2A3B4C5D[OXE9F0A1B2], OXE5F6A7B8);
		if(access(OX3E4F5A6B, R_OK) != -1) {
			OX5D6E7F8G = OX2A3B4C5D[OXE9F0A1B2];
			break;
		}
		OXE9F0A1B2++;
	}

	if(!OX5D6E7F8G) {
		fprintf(stderr, "couldnt locate %s\n", OXE5F6A7B8);
		return EXIT_FAILURE;
	}
	if(!OX8E9F0A1B)
		fprintf(stderr, LOG_PREFIX "preloading %s/%s\n", OX5D6E7F8G, OXE5F6A7B8);

#ifdef IS_MAC
	putenv("DYLD_FORCE_FLAT_NAMESPACE=1");
#define LD_PRELOAD_ENV "DYLD_INSERT_LIBRARIES"
#define LD_PRELOAD_SEP ":"
#else
#define LD_PRELOAD_ENV "LD_PRELOAD"
#define LD_PRELOAD_SEP " "
#endif
	char *OX1B2C3D4E = getenv(LD_PRELOAD_ENV);
	snprintf(OX3E4F5A6B, sizeof(OX3E4F5A6B), LD_PRELOAD_ENV "=%s/%s%s%s",
	         OX5D6E7F8G, OXE5F6A7B8,
	         OX1B2C3D4E ? LD_PRELOAD_SEP : "",
	         OX1B2C3D4E ? OX1B2C3D4E : "");
	putenv(OX3E4F5A6B);
	execvp(OXF6A7B8C9[OX4A5B6C7D], &OXF6A7B8C9[OX4A5B6C7D]);
	perror("proxychains can't load process....");

	return EXIT_FAILURE;
}