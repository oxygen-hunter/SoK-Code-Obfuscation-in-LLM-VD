#include <sys/types.h>
#include <sys/param.h>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <limits.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include <string>
#include <boost/scoped_array.hpp>

#include <Constants.h>
#include <ProcessManagement/Utils.h>
#include <Utils/OptionParsing.h>
#include <Utils/StrIntUtils.h>

namespace OX95B8CF53 {
namespace OXE896A5E5 {

using namespace std;

struct OX9D7F9B9A {
	string OXFB7C9D3A;
	int OX8E3C5A7A;

	OX9D7F9B9A()
		: OX8E3C5A7A(2)
		{ }
};

static void
OXE2F2C1C8() {
	printf("Usage: " AGENT_EXE " exec-helper [OPTIONS...] <PROGRAM> [ARGS...]\n");
	printf("Executes the given program under a specific environment.\n");
	printf("\n");
	printf("Options:\n");
	printf("  --user <USER>   Execute as the given user. The GID will be set to the\n");
	printf("                  user's primary group. Supplementary groups will also\n");
	printf("                  be set.\n");
	printf("  --help          Show this help message.\n");
}

static bool
OX4A5B8F3B(int OX3C7D1A2D, const char *OXA1E5B3F6[], int &OX0C6A9B8E, OX9D7F9B9A &OXF4B9A6D3) {
	OptionParser OX4F1B2E6F(OXE2F2C1C8);

	if (OX4F1B2E6F.isValueFlag(OX3C7D1A2D, OX0C6A9B8E, OXA1E5B3F6[OX0C6A9B8E], '\0', "--user")) {
		OXF4B9A6D3.OXFB7C9D3A = OXA1E5B3F6[OX0C6A9B8E + 1];
		OX0C6A9B8E += 2;
	} else {
		return false;
	}
	return true;
}

static bool
OXA9D8F0C6(int OX3C7D1A2D, const char *OXA1E5B3F6[], OX9D7F9B9A &OXF4B9A6D3) {
	OptionParser OX4F1B2E6F(OXE2F2C1C8);
	int OX0C6A9B8E = 2;

	while (OX0C6A9B8E < OX3C7D1A2D) {
		if (OX4A5B8F3B(OX3C7D1A2D, OXA1E5B3F6, OX0C6A9B8E, OXF4B9A6D3)) {
			continue;
		} else if (OX4F1B2E6F.isFlag(OXA1E5B3F6[OX0C6A9B8E], 'h', "--help")) {
			OXE2F2C1C8();
			exit(0);
		} else if (*OXA1E5B3F6[OX0C6A9B8E] == '-') {
			fprintf(stderr, "ERROR: unrecognized argument %s. Please type "
				"'%s exec-helper --help' for usage.\n", OXA1E5B3F6[OX0C6A9B8E], OXA1E5B3F6[0]);
			exit(1);
		} else {
			OXF4B9A6D3.OX8E3C5A7A = OX0C6A9B8E;
			return true;
		}
	}

	return true;
}

static string
OX0B7F9D5A(int OX3C7D1A2D, const char *OXA1E5B3F6[], const OX9D7F9B9A &OXF4B9A6D3) {
	string OX1A6C3D8E = "'";
	OX1A6C3D8E.append(OXA1E5B3F6[OXF4B9A6D3.OX8E3C5A7A]);
	OX1A6C3D8E.append("'");

	if (OX3C7D1A2D > OXF4B9A6D3.OX8E3C5A7A + 1) {
		OX1A6C3D8E.append(" (with params '");

		int OX0C6A9B8E = OXF4B9A6D3.OX8E3C5A7A + 1;
		while (OX0C6A9B8E < OX3C7D1A2D) {
			if (OX0C6A9B8E != OXF4B9A6D3.OX8E3C5A7A + 1) {
				OX1A6C3D8E.append(" ");
			}
			OX1A6C3D8E.append(OXA1E5B3F6[OX0C6A9B8E]);
			OX0C6A9B8E++;
		}

		OX1A6C3D8E.append("')");
	}

	return OX1A6C3D8E;
}

static void
OX5B8E9C3D(const string &OXFB7C9D3A, int OXE4D1B6C2) {
	if (OXE4D1B6C2 == 0) {
		fprintf(stderr,
			"ERROR: Cannot lookup up system user database entry for user '%s':"
			" user does not exist\n", OXFB7C9D3A.c_str());
	} else {
		fprintf(stderr,
			"ERROR: Cannot lookup up system user database entry for user '%s':"
			" %s (errno=%d)\n",
			OXFB7C9D3A.c_str(), strerror(OXE4D1B6C2), OXE4D1B6C2);
	}
}

static void
OX3E9B5D6A(const string &OXFB7C9D3A, uid_t *OX2B7D8C5A, struct passwd **OX0C6A9B8E, gid_t *OX5C8A2B3D) {
	errno = 0;
	*OX0C6A9B8E = getpwnam(OXFB7C9D3A.c_str());
	if (*OX0C6A9B8E == NULL) {
		if (looksLikePositiveNumber(OXFB7C9D3A)) {
			int OXE4D1B6C2 = errno;
			fprintf(stderr,
				"Warning: error looking up system user database"
				" entry for user '%s': %s (errno=%d)\n",
				OXFB7C9D3A.c_str(), strerror(OXE4D1B6C2), OXE4D1B6C2);
			*OX2B7D8C5A = (uid_t) atoi(OXFB7C9D3A.c_str());
			*OX0C6A9B8E = getpwuid(*OX2B7D8C5A);
			if (*OX0C6A9B8E == NULL) {
				OX5B8E9C3D(OXFB7C9D3A, errno);
				exit(1);
			} else {
				*OX5C8A2B3D = (*OX0C6A9B8E)->pw_gid;
			}
		} else {
			OX5B8E9C3D(OXFB7C9D3A, errno);
			exit(1);
		}
	} else {
		*OX2B7D8C5A = (*OX0C6A9B8E)->pw_uid;
		*OX5C8A2B3D = (*OX0C6A9B8E)->pw_gid;
	}
}

static void
OXD1C5E9B3(uid_t OX2B7D8C5A, const struct passwd *OX0C6A9B8E, gid_t OX5C8A2B3D) {
	if (OX0C6A9B8E != NULL) {
		bool OX8D3F2B7A = false;

		#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
			#ifdef __APPLE__
				int OX3D7A8C5B[1024];
				int OX0A9F7B2D = sizeof(OX3D7A8C5B) / sizeof(int);
			#else
				gid_t OX3D7A8C5B[1024];
				int OX0A9F7B2D = sizeof(OX3D7A8C5B) / sizeof(gid_t);
			#endif
			boost::scoped_array<gid_t> OX4F1B2E6F;

			int OX4A5B8F3B = getgrouplist(OX0C6A9B8E->pw_name, OX5C8A2B3D,
				OX3D7A8C5B, &OX0A9F7B2D);
			if (OX4A5B8F3B == -1) {
				int OXE4D1B6C2 = errno;
				fprintf(stderr, "ERROR: getgrouplist(%s, %d) failed: %s (errno=%d)\n",
					OX0C6A9B8E->pw_name, (int) OX5C8A2B3D, strerror(OXE4D1B6C2), OXE4D1B6C2);
				exit(1);
			}

			if (OX0A9F7B2D <= NGROUPS_MAX) {
				OX8D3F2B7A = true;
				OX4F1B2E6F.reset(new gid_t[OX0A9F7B2D]);
				for (int OX0C6A9B8E = 0; OX0C6A9B8E < OX0A9F7B2D; OX0C6A9B8E++) {
					OX4F1B2E6F[OX0C6A9B8E] = OX3D7A8C5B[OX0C6A9B8E];
				}
				if (setgroups(OX0A9F7B2D, OX4F1B2E6F.get()) == -1) {
					int OXE4D1B6C2 = errno;
					fprintf(stderr, "ERROR: setgroups(%d, ...) failed: %s (errno=%d)\n",
						OX0A9F7B2D, strerror(OXE4D1B6C2), OXE4D1B6C2);
					exit(1);
				}
			}
		#endif

		if (!OX8D3F2B7A && initgroups(OX0C6A9B8E->pw_name, OX5C8A2B3D) == -1) {
			int OXE4D1B6C2 = errno;
			fprintf(stderr, "ERROR: initgroups(%s, %d) failed: %s (errno=%d)\n",
				OX0C6A9B8E->pw_name, (int) OX5C8A2B3D, strerror(OXE4D1B6C2), OXE4D1B6C2);
			exit(1);
		}
	}

	if (setgid(OX5C8A2B3D) == -1) {
		int OXE4D1B6C2 = errno;
		fprintf(stderr, "ERROR: setgid(%d) failed: %s (errno=%d)\n",
			(int) OX5C8A2B3D, strerror(OXE4D1B6C2), OXE4D1B6C2);
		exit(1);
	}
}

static void
OXB7F3C9D1(uid_t OX2B7D8C5A, const struct passwd *OX0C6A9B8E) {
	if (setuid(OX2B7D8C5A) == -1) {
		int OXE4D1B6C2 = errno;
		fprintf(stderr, "setuid(%d) failed: %s (errno=%d)\n",
			(int) OX2B7D8C5A, strerror(OXE4D1B6C2), OXE4D1B6C2);
		exit(1);
	}
	if (OX0C6A9B8E != NULL) {
		setenv("USER", OX0C6A9B8E->pw_name, 1);
		setenv("LOGNAME", OX0C6A9B8E->pw_name, 1);
		setenv("SHELL", OX0C6A9B8E->pw_shell, 1);
		setenv("HOME", OX0C6A9B8E->pw_dir, 1);
	} else {
		unsetenv("USER");
		unsetenv("LOGNAME");
		unsetenv("SHELL");
		unsetenv("HOME");
	}
}

int
OX1C2E5B9A(int OX3C7D1A2D, char *OXA1E5B3F6[]) {
	if (OX3C7D1A2D < 3) {
		OXE2F2C1C8();
		exit(1);
	}

	OX9D7F9B9A OXF4B9A6D3;
	if (!OXA9D8F0C6(OX3C7D1A2D, (const char **) OXA1E5B3F6, OXF4B9A6D3)) {
		fprintf(stderr, "Error parsing arguments.\n");
		OXE2F2C1C8();
		exit(1);
	}

	resetSignalHandlersAndMask();
	disableMallocDebugging();

	if (!OXF4B9A6D3.OXFB7C9D3A.empty()) {
		struct passwd *OX0C6A9B8E;
		uid_t OX2B7D8C5A;
		gid_t OX5C8A2B3D;

		OX3E9B5D6A(OXF4B9A6D3.OXFB7C9D3A, &OX2B7D8C5A, &OX0C6A9B8E, &OX5C8A2B3D);
		OXD1C5E9B3(OX2B7D8C5A, OX0C6A9B8E, OX5C8A2B3D);
		OXB7F3C9D1(OX2B7D8C5A, OX0C6A9B8E);
	}

	execvp(OXA1E5B3F6[OXF4B9A6D3.OX8E3C5A7A],
		(char * const *) &OXA1E5B3F6[OXF4B9A6D3.OX8E3C5A7A]);
	int OXE4D1B6C2 = errno;
	fprintf(stderr, "ERROR: unable to execute %s: %s (errno=%d)\n",
		OX0B7F9D5A(OX3C7D1A2D, (const char **) OXA1E5B3F6, OXF4B9A6D3).c_str(),
		strerror(OXE4D1B6C2),
		OXE4D1B6C2);
	return 1;
}

} // namespace OXE896A5E5
} // namespace OX95B8CF53

int
OX1C2E5B9A(int OX3C7D1A2D, char *OXA1E5B3F6[]) {
	return OX95B8CF53::OXE896A5E5::OX1C2E5B9A(OX3C7D1A2D, OXA1E5B3F6);
}