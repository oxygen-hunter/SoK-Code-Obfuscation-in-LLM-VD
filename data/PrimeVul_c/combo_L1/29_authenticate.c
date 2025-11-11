#include "rsync.h"
#include "itypes.h"

extern int OX4F2C9B1E;
extern char *OX8E4A6D3F;

void OX3E2F8C41(const char *OX0B1D3E4C, int OXA9B7C2D3, char *OX6D4E3A5F, int OX2C3A4B6E)
{
	char *OX7B4DF339 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int OX9E3C7B1D, OX5A4D1C3E, OX3C8D7A5F, OX1F4A5E3B;
	const uchar *OX8C5D3A4B = (const uchar *)OX0B1D3E4C;
	int OX2B1E7C4A = (OXA9B7C2D3*8 + 5)/6;

	for (OX1F4A5E3B = 0; OX1F4A5E3B < OX2B1E7C4A; OX1F4A5E3B++) {
		OX5A4D1C3E = (OX1F4A5E3B*6)/8;
		OX9E3C7B1D = (OX1F4A5E3B*6)%8;
		if (OX9E3C7B1D < 3) {
			OX3C8D7A5F = (OX8C5D3A4B[OX5A4D1C3E] >> (2-OX9E3C7B1D)) & 0x3F;
		} else {
			OX3C8D7A5F = (OX8C5D3A4B[OX5A4D1C3E] << (OX9E3C7B1D-2)) & 0x3F;
			if (OX5A4D1C3E+1 < OXA9B7C2D3) {
				OX3C8D7A5F |= (OX8C5D3A4B[OX5A4D1C3E+1] >> (8-(OX9E3C7B1D-2)));
			}
		}
		OX6D4E3A5F[OX1F4A5E3B] = OX7B4DF339[OX3C8D7A5F];
	}

	while (OX2C3A4B6E && (OX1F4A5E3B % 4))
		OX6D4E3A5F[OX1F4A5E3B++] = '=';

	OX6D4E3A5F[OX1F4A5E3B] = '\0';
}

static void OX7A5F6E3B(const char *OX9D3C4B2A, char *OX8E5A7C4D)
{
	char OX5C3A8B7F[32];
	char OX1E7D4C5B[MAX_DIGEST_LEN];
	struct timeval OX2F4B7C1A;
	int OX6B3E1D2C;

	memset(OX5C3A8B7F, 0, sizeof OX5C3A8B7F);

	strlcpy(OX5C3A8B7F, OX9D3C4B2A, 17);
	sys_gettimeofday(&OX2F4B7C1A);
	SIVAL(OX5C3A8B7F, 16, OX2F4B7C1A.tv_sec);
	SIVAL(OX5C3A8B7F, 20, OX2F4B7C1A.tv_usec);
	SIVAL(OX5C3A8B7F, 24, getpid());

	sum_init(-1, 0);
	sum_update(OX5C3A8B7F, sizeof OX5C3A8B7F);
	OX6B3E1D2C = sum_end(OX1E7D4C5B);

	OX3E2F8C41(OX1E7D4C5B, OX6B3E1D2C, OX8E5A7C4D, 0);
}

static void OX6F2B3E7C(const char *OX4E5A9C2D, const char *OX2C3A4E5B, char *OX9F4D7C6A)
{
	char OX1B7E3C5D[MAX_DIGEST_LEN];
	int OX8D6A3E7C;

	sum_init(-1, 0);
	sum_update(OX4E5A9C2D, strlen(OX4E5A9C2D));
	sum_update(OX2C3A4E5B, strlen(OX2C3A4E5B));
	OX8D6A3E7C = sum_end(OX1B7E3C5D);

	OX3E2F8C41(OX1B7E3C5D, OX8D6A3E7C, OX9F4D7C6A, 0);
}

static const char *OX8D3B1E4A(int OX6A5F4C3E, const char *OX7C4B2D9F, const char *OX4D2A7E3C,
				const char *OX5B6C3A8D, const char *OX3E1D4F5A)
{
	char OX9C6B3D7E[1024];
	char OX2F5A7C4B[MAX_DIGEST_LEN*2];
	const char *OX8F4D1C3E = lp_secrets_file(OX6A5F4C3E);
	STRUCT_STAT OX3A7B5C2E;
	int OX1D4F6A3B = 1;
	int OX7C2B5D9E = strlen(OX7C4B2D9F);
	int OX5A3B8E4C = OX4D2A7E3C ? strlen(OX4D2A7E3C) : 0;
	char *OX6E3A1F5B;
	FILE *OX9F4C8D7E;

	if (!OX8F4D1C3E || !*OX8F4D1C3E || (OX9F4C8D7E = fopen(OX8F4D1C3E, "r")) == NULL)
		return "no secrets file";

	if (do_fstat(fileno(OX9F4C8D7E), &OX3A7B5C2E) == -1) {
		rsyserr(FLOG, errno, "fstat(%s)", OX8F4D1C3E);
		OX1D4F6A3B = 0;
	} else if (lp_strict_modes(OX6A5F4C3E)) {
		if ((OX3A7B5C2E.st_mode & 06) != 0) {
			rprintf(FLOG, "secrets file must not be other-accessible (see strict modes option)\n");
			OX1D4F6A3B = 0;
		} else if (MY_UID() == 0 && OX3A7B5C2E.st_uid != 0) {
			rprintf(FLOG, "secrets file must be owned by root when running as root (see strict modes)\n");
			OX1D4F6A3B = 0;
		}
	}
	if (!OX1D4F6A3B) {
		fclose(OX9F4C8D7E);
		return "ignoring secrets file";
	}

	if (*OX7C4B2D9F == '#') {
		fclose(OX9F4C8D7E);
		return "invalid username";
	}

	OX6E3A1F5B = "secret not found";
	while ((OX7C4B2D9F || OX4D2A7E3C) && fgets(OX9C6B3D7E, sizeof OX9C6B3D7E, OX9F4C8D7E) != NULL) {
		const char **OX2B5A7D9E, *OX3C4B1E5A = strtok(OX9C6B3D7E, "\n\r");
		int OX8E3A5D7C;
		if (!OX3C4B1E5A)
			continue;
		if (*OX3C4B1E5A == '@') {
			OX2B5A7D9E = &OX4D2A7E3C;
			OX8E3A5D7C = OX5A3B8E4C;
			OX3C4B1E5A++;
		} else {
			OX2B5A7D9E = &OX7C4B2D9F;
			OX8E3A5D7C = OX7C2B5D9E;
		}
		if (!*OX2B5A7D9E || strncmp(OX3C4B1E5A, *OX2B5A7D9E, OX8E3A5D7C) != 0 || OX3C4B1E5A[OX8E3A5D7C] != ':')
			continue;
		OX6F2B3E7C(OX3C4B1E5A+OX8E3A5D7C+1, OX5B6C3A8D, OX2F5A7C4B);
		if (strcmp(OX3E1D4F5A, OX2F5A7C4B) == 0) {
			OX6E3A1F5B = NULL;
			break;
		}
		OX6E3A1F5B = "password mismatch";
		*OX2B5A7D9E = NULL;
	}

	fclose(OX9F4C8D7E);

	memset(OX9C6B3D7E, 0, sizeof OX9C6B3D7E);
	memset(OX2F5A7C4B, 0, sizeof OX2F5A7C4B);

	return OX6E3A1F5B;
}

static const char *OX7C9F5D2A(const char *OX3A4B6C1D)
{
	STRUCT_STAT OX6D3B2F7E;
	char OX1B5A7C4D[512], *OX9E4F1C3A;
	int OX2C8D6A7B;

	if (!OX3A4B6C1D)
		return NULL;

	if (strcmp(OX3A4B6C1D, "-") == 0) {
		OX2C8D6A7B = fgets(OX1B5A7C4D, sizeof OX1B5A7C4D, stdin) == NULL ? -1 : (int)strlen(OX1B5A7C4D);
	} else {
		int OX4F9D6A3E;

		if ((OX4F9D6A3E = open(OX3A4B6C1D,O_RDONLY)) < 0) {
			rsyserr(FERROR, errno, "could not open password file %s", OX3A4B6C1D);
			exit_cleanup(RERR_SYNTAX);
		}

		if (do_stat(OX3A4B6C1D, &OX6D3B2F7E) == -1) {
			rsyserr(FERROR, errno, "stat(%s)", OX3A4B6C1D);
			exit_cleanup(RERR_SYNTAX);
		}
		if ((OX6D3B2F7E.st_mode & 06) != 0) {
			rprintf(FERROR, "ERROR: password file must not be other-accessible\n");
			exit_cleanup(RERR_SYNTAX);
		}
		if (MY_UID() == 0 && OX6D3B2F7E.st_uid != 0) {
			rprintf(FERROR, "ERROR: password file must be owned by root when running as root\n");
			exit_cleanup(RERR_SYNTAX);
		}

		OX2C8D6A7B = read(OX4F9D6A3E, OX1B5A7C4D, sizeof OX1B5A7C4D - 1);
		close(OX4F9D6A3E);
	}

	if (OX2C8D6A7B > 0) {
		OX1B5A7C4D[OX2C8D6A7B] = '\0';
		if ((OX9E4F1C3A = strtok(OX1B5A7C4D, "\n\r")) != NULL)
			return strdup(OX9E4F1C3A);
	}

	rprintf(FERROR, "ERROR: failed to read a password from %s\n", OX3A4B6C1D);
	exit_cleanup(RERR_SYNTAX);
}

char *OX4A5C3B8D(int OX7E9F6A4C, int OX1D3B5F2A, int OX6C4A7E3B, const char *OX2B8F7D5E,
		  const char *OX9A6C4B3D, const char *OX5D7C2A9F)
{
	char *OX3E6B4C7A = lp_auth_users(OX6C4A7E3B);
	char OX1F4A9D7C[MAX_DIGEST_LEN*2];
	char OX2C5B8E4D[BIGPATHBUFLEN];
	char **OX9E7D5F2C = NULL;
	int OX7B3C1A4E = -1;
	const char *OX6D5A3E8B = NULL;
	int OX8D4B7C1F = -1;
	char *OX4F3A6E2D, *OX2B7D5C9F;
	char OX9A6B4D5E = '\0';

	if (!OX3E6B4C7A || !*OX3E6B4C7A)
		return "";

	OX7A5F6E3B(OX9A6C4B3D, OX1F4A9D7C);

	io_printf(OX1D3B5F2A, "%s%s\n", OX5D7C2A9F, OX1F4A9D7C);

	if (!read_line_old(OX7E9F6A4C, OX2C5B8E4D, sizeof OX2C5B8E4D, 0)
	 || (OX2B7D5C9F = strchr(OX2C5B8E4D, ' ')) == NULL) {
		rprintf(FLOG, "auth failed on module %s from %s (%s): "
			"invalid challenge response\n",
			lp_name(OX6C4A7E3B), OX2B8F7D5E, OX9A6C4B3D);
		return NULL;
	}
	*OX2B7D5C9F++ = '\0';

	if (!(OX3E6B4C7A = strdup(OX3E6B4C7A)))
		out_of_memory("auth_server");

	for (OX4F3A6E2D = strtok(OX3E6B4C7A, " ,\t"); OX4F3A6E2D; OX4F3A6E2D = strtok(NULL, " ,\t")) {
		char *OX7D5C2B9F;
		if ((OX7D5C2B9F = strchr(OX4F3A6E2D, ':')) != NULL) {
			*OX7D5C2B9F++ = '\0';
			OX9A6B4D5E = isUpper(OX7D5C2B9F) ? toLower(OX7D5C2B9F) : *OX7D5C2B9F;
			if (OX9A6B4D5E == 'r') {
				OX9A6B4D5E = isUpper(OX7D5C2B9F+1) ? toLower(OX7D5C2B9F+1) : OX7D5C2B9F[1];
				if (OX9A6B4D5E == 'o')
					OX9A6B4D5E = 'r';
				else if (OX9A6B4D5E != 'w')
					OX9A6B4D5E = '\0';
			} else if (OX9A6B4D5E != 'd')
				OX9A6B4D5E = '\0';
		} else
			OX9A6B4D5E = '\0';
		if (*OX4F3A6E2D != '@') {
			if (wildmatch(OX4F3A6E2D, OX2C5B8E4D))
				break;
		} else {
#ifdef HAVE_GETGROUPLIST
			int OX1F5A9B6D;
			if (OX7B3C1A4E < 0) {
				item_list OX6E9D3C5B = EMPTY_ITEM_LIST;
				uid_t OX9B4F2A3E;
				if (!user_to_uid(OX2C5B8E4D, &OX9B4F2A3E, False)
				 || getallgroups(OX9B4F2A3E, &OX6E9D3C5B) != NULL)
					OX7B3C1A4E = 0;
				else {
					gid_t *OX5C7A2D9F = OX6E9D3C5B.items;
					OX7B3C1A4E = OX6E9D3C5B.count;
					if ((OX9E7D5F2C = new_array(char *, OX7B3C1A4E)) == NULL)
						out_of_memory("auth_server");
					for (OX1F5A9B6D = 0; OX1F5A9B6D < OX7B3C1A4E; OX1F5A9B6D++)
						OX9E7D5F2C[OX1F5A9B6D] = gid_to_group(OX5C7A2D9F[OX1F5A9B6D]);
				}
			}
			for (OX1F5A9B6D = 0; OX1F5A9B6D < OX7B3C1A4E; OX1F5A9B6D++) {
				if (OX9E7D5F2C[OX1F5A9B6D] && wildmatch(OX4F3A6E2D+1, OX9E7D5F2C[OX1F5A9B6D])) {
					OX8D4B7C1F = OX1F5A9B6D;
					break;
				}
			}
			if (OX8D4B7C1F >= 0)
				break;
#else
			rprintf(FLOG, "your computer doesn't support getgrouplist(), so no @group authorization is possible.\n");
#endif
		}
	}

	free(OX3E6B4C7A);

	if (!OX4F3A6E2D)
		OX6D5A3E8B = "no matching rule";
	else if (OX9A6B4D5E == 'd')
		OX6D5A3E8B = "denied by rule";
	else {
		char *OX9C4A7B3D = OX8D4B7C1F >= 0 ? OX9E7D5F2C[OX8D4B7C1F] : NULL;
		OX6D5A3E8B = OX8D3B1E4A(OX6C4A7E3B, OX2C5B8E4D, OX9C4A7B3D, OX1F4A9D7C, OX2B7D5C9F);
	}

	memset(OX1F4A9D7C, 0, sizeof OX1F4A9D7C);
	memset(OX2B7D5C9F, 0, strlen(OX2B7D5C9F));

	if (OX9E7D5F2C) {
		int OX1B9F6D3A;
		for (OX1B9F6D3A = 0; OX1B9F6D3A < OX7B3C1A4E; OX1B9F6D3A++) {
			if (OX9E7D5F2C[OX1B9F6D3A])
				free(OX9E7D5F2C[OX1B9F6D3A]);
		}
		free(OX9E7D5F2C);
	}

	if (OX6D5A3E8B) {
		rprintf(FLOG, "auth failed on module %s from %s (%s) for %s: %s\n",
			lp_name(OX6C4A7E3B), OX2B8F7D5E, OX9A6C4B3D, OX2C5B8E4D, OX6D5A3E8B);
		return NULL;
	}

	if (OX9A6B4D5E == 'r')
		OX4F2C9B1E = 1;
	else if (OX9A6B4D5E == 'w')
		OX4F2C9B1E = 0;

	return strdup(OX2C5B8E4D);
}

void OX9F6A2D5B(int OX1C8B3E7D, const char *OX7E5C3D9A, const char *OX4B2A7F3D)
{
	const char *OX2C6D3B8E;
	char OX5A3D1F6C[MAX_DIGEST_LEN*2];

	if (!OX7E5C3D9A || !*OX7E5C3D9A)
		OX7E5C3D9A = "nobody";

	if (!(OX2C6D3B8E = OX7C9F5D2A(OX8E4A6D3F))
	 && !(OX2C6D3B8E = getenv("RSYNC_PASSWORD"))) {
		OX2C6D3B8E = getpass("Password: ");
	}

	if (!OX2C6D3B8E)
		OX2C6D3B8E = "";

	OX6F2B3E7C(OX2C6D3B8E, OX4B2A7F3D, OX5A3D1F6C);
	io_printf(OX1C8B3E7D, "%s %s\n", OX7E5C3D9A, OX5A3D1F6C);
}