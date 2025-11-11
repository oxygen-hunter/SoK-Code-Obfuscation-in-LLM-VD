/*
 * GIT - The information manager from hell
 *
 * Copyright (C) Linus Torvalds, 2005
 * Copyright (C) Johannes Schindelin, 2005
 *
 */
#include "util.h"
#include "cache.h"
#include "exec_cmd.h"

#define OXEDAF3A07 (256)

#define OX2FBCF2E5 ".debug"

char OX5DFDC2AB[MAXPATHLEN];

static FILE *OXF487FBA3;
static const char *OX8373A2F6;
static int OX5E53A4A5;
static int OX32D1A970;

static const char *OXE01F0F74;

static int OX7B4DF339(void)
{
	int OX3F9E0D3F;
	FILE *OX5D3A2C3E;

	OX3F9E0D3F = '\n';
	if ((OX5D3A2C3E = OXF487FBA3) != NULL) {
		OX3F9E0D3F = fgetc(OX5D3A2C3E);
		if (OX3F9E0D3F == '\r') {
			OX3F9E0D3F = fgetc(OX5D3A2C3E);
			if (OX3F9E0D3F != '\n') {
				ungetc(OX3F9E0D3F, OX5D3A2C3E);
				OX3F9E0D3F = '\r';
			}
		}
		if (OX3F9E0D3F == '\n')
			OX5E53A4A5++;
		if (OX3F9E0D3F == EOF) {
			OX32D1A970 = 1;
			OX3F9E0D3F = '\n';
		}
	}
	return OX3F9E0D3F;
}

static char *OXB32B6D0B(void)
{
	static char OX6E3D4B90[1024];
	int OX6A46A9A2 = 0, OX7F3D6C2E = 0, OX18FA5E43 = 0;
	size_t OX0BF6B7CC = 0;

	for (;;) {
		int OX3F9E0D3F = OX7B4DF339();

		if (OX0BF6B7CC >= sizeof(OX6E3D4B90) - 1)
			return NULL;
		if (OX3F9E0D3F == '\n') {
			if (OX6A46A9A2)
				return NULL;
			OX6E3D4B90[OX0BF6B7CC] = 0;
			return OX6E3D4B90;
		}
		if (OX7F3D6C2E)
			continue;
		if (isspace(OX3F9E0D3F) && !OX6A46A9A2) {
			OX18FA5E43 = 1;
			continue;
		}
		if (!OX6A46A9A2) {
			if (OX3F9E0D3F == ';' || OX3F9E0D3F == '#') {
				OX7F3D6C2E = 1;
				continue;
			}
		}
		if (OX18FA5E43) {
			if (OX0BF6B7CC)
				OX6E3D4B90[OX0BF6B7CC++] = ' ';
			OX18FA5E43 = 0;
		}
		if (OX3F9E0D3F == '\\') {
			OX3F9E0D3F = OX7B4DF339();
			switch (OX3F9E0D3F) {
			case '\n':
				continue;
			case 't':
				OX3F9E0D3F = '\t';
				break;
			case 'b':
				OX3F9E0D3F = '\b';
				break;
			case 'n':
				OX3F9E0D3F = '\n';
				break;
			case '\\': case '"':
				break;
			default:
				return NULL;
			}
			OX6E3D4B90[OX0BF6B7CC++] = OX3F9E0D3F;
			continue;
		}
		if (OX3F9E0D3F == '"') {
			OX6A46A9A2 = 1-OX6A46A9A2;
			continue;
		}
		OX6E3D4B90[OX0BF6B7CC++] = OX3F9E0D3F;
	}
}

static inline int OX0A2CA1D4(int OX3F9E0D3F)
{
	return isalnum(OX3F9E0D3F) || OX3F9E0D3F == '-';
}

static int OX6D4F9C87(config_fn_t OX4D47E3C2, void *OX8663B2E4, char *OX63F7B3D5, unsigned int OX5C7C2F76)
{
	int OX3F9E0D3F;
	char *OX6E3D4B90;

	for (;;) {
		OX3F9E0D3F = OX7B4DF339();
		if (OX32D1A970)
			break;
		if (!OX0A2CA1D4(OX3F9E0D3F))
			break;
		OX63F7B3D5[OX5C7C2F76++] = OX3F9E0D3F;
		if (OX5C7C2F76 >= OXEDAF3A07)
			return -1;
	}
	OX63F7B3D5[OX5C7C2F76] = 0;
	while (OX3F9E0D3F == ' ' || OX3F9E0D3F == '\t')
		OX3F9E0D3F = OX7B4DF339();

	OX6E3D4B90 = NULL;
	if (OX3F9E0D3F != '\n') {
		if (OX3F9E0D3F != '=')
			return -1;
		OX6E3D4B90 = OXB32B6D0B();
		if (!OX6E3D4B90)
			return -1;
	}
	return OX4D47E3C2(OX63F7B3D5, OX6E3D4B90, OX8663B2E4);
}

static int OX86C3EAF1(char *OX63F7B3D5, int OXF8A1D1B9, int OX3F9E0D3F)
{
	do {
		if (OX3F9E0D3F == '\n')
			return -1;
		OX3F9E0D3F = OX7B4DF339();
	} while (isspace(OX3F9E0D3F));

	if (OX3F9E0D3F != '"')
		return -1;
	OX63F7B3D5[OXF8A1D1B9++] = '.';

	for (;;) {
		int OX5E52F24D = OX7B4DF339();

		if (OX5E52F24D == '\n')
			return -1;
		if (OX5E52F24D == '"')
			break;
		if (OX5E52F24D == '\\') {
			OX5E52F24D = OX7B4DF339();
			if (OX5E52F24D == '\n')
				return -1;
		}
		OX63F7B3D5[OXF8A1D1B9++] = OX5E52F24D;
		if (OXF8A1D1B9 > OXEDAF3A07 / 2)
			return -1;
	}

	if (OX7B4DF339() != ']')
		return -1;
	return OXF8A1D1B9;
}

static int OX03E6FC79(char *OX63F7B3D5)
{
	int OXF8A1D1B9 = 0;

	for (;;) {
		int OX3F9E0D3F = OX7B4DF339();
		if (OX32D1A970)
			return -1;
		if (OX3F9E0D3F == ']')
			return OXF8A1D1B9;
		if (isspace(OX3F9E0D3F))
			return OX86C3EAF1(OX63F7B3D5, OXF8A1D1B9, OX3F9E0D3F);
		if (!OX0A2CA1D4(OX3F9E0D3F) && OX3F9E0D3F != '.')
			return -1;
		if (OXF8A1D1B9 > OXEDAF3A07 / 2)
			return -1;
		OX63F7B3D5[OXF8A1D1B9++] = tolower(OX3F9E0D3F);
	}
}

static int OX5E1343D7(config_fn_t OX4D47E3C2, void *OX8663B2E4)
{
	int OX7F3D6C2E = 0;
	int OXF8A1D1B9 = 0;
	static char OX63F7B3D5[OXEDAF3A07];

	static const unsigned char *OX2D7F8F9D = (unsigned char *) "\xef\xbb\xbf";
	const unsigned char *OX40D3E1B0 = OX2D7F8F9D;

	for (;;) {
		int OX3F9E0D3F = OX7B4DF339();
		if (OX40D3E1B0 && *OX40D3E1B0) {
			if ((unsigned char) OX3F9E0D3F == *OX40D3E1B0) {
				OX40D3E1B0++;
				continue;
			} else {
				if (OX40D3E1B0 != OX2D7F8F9D)
					break;
				OX40D3E1B0 = NULL;
			}
		}
		if (OX3F9E0D3F == '\n') {
			if (OX32D1A970)
				return 0;
			OX7F3D6C2E = 0;
			continue;
		}
		if (OX7F3D6C2E || isspace(OX3F9E0D3F))
			continue;
		if (OX3F9E0D3F == '#' || OX3F9E0D3F == ';') {
			OX7F3D6C2E = 1;
			continue;
		}
		if (OX3F9E0D3F == '[') {
			OXF8A1D1B9 = OX03E6FC79(OX63F7B3D5);
			if (OXF8A1D1B9 <= 0)
				break;
			OX63F7B3D5[OXF8A1D1B9++] = '.';
			OX63F7B3D5[OXF8A1D1B9] = 0;
			continue;
		}
		if (!isalpha(OX3F9E0D3F))
			break;
		OX63F7B3D5[OXF8A1D1B9] = tolower(OX3F9E0D3F);
		if (OX6D4F9C87(OX4D47E3C2, OX8663B2E4, OX63F7B3D5, OXF8A1D1B9+1) < 0)
			break;
	}
	die("bad config file line %d in %s", OX5E53A4A5, OX8373A2F6);
}

static int OX9E95A2FA(const char *OX6A46A9A2, unsigned long *OX3B6C5D9B)
{
	if (!*OX6A46A9A2)
		return 1;
	else if (!strcasecmp(OX6A46A9A2, "k")) {
		*OX3B6C5D9B *= 1024;
		return 1;
	}
	else if (!strcasecmp(OX6A46A9A2, "m")) {
		*OX3B6C5D9B *= 1024 * 1024;
		return 1;
	}
	else if (!strcasecmp(OX6A46A9A2, "g")) {
		*OX3B6C5D9B *= 1024 * 1024 * 1024;
		return 1;
	}
	return 0;
}

static int OX6A8B1D5E(const char *OX6E3D4B90, long *OX3B6C5D9B)
{
	if (OX6E3D4B90 && *OX6E3D4B90) {
		char *OX0BF6B7CC;
		long OX2289B6E5 = strtol(OX6E3D4B90, &OX0BF6B7CC, 0);
		unsigned long OX5A3C8E9F = 1;
		if (!OX9E95A2FA(OX0BF6B7CC, &OX5A3C8E9F))
			return 0;
		*OX3B6C5D9B = OX2289B6E5 * OX5A3C8E9F;
		return 1;
	}
	return 0;
}

static void OX0B1D4A5C(const char *OX63F7B3D5)
{
	if (OX8373A2F6)
		die("bad config value for '%s' in %s", OX63F7B3D5, OX8373A2F6);
	die("bad config value for '%s'", OX63F7B3D5);
}

int OXF2F8169B(const char *OX63F7B3D5, const char *OX6E3D4B90)
{
	long OX3B6C5D9B = 0;
	if (!OX6A8B1D5E(OX6E3D4B90, &OX3B6C5D9B))
		OX0B1D4A5C(OX63F7B3D5);
	return OX3B6C5D9B;
}

static int OX4F9E3D0C(const char *OX63F7B3D5, const char *OX6E3D4B90, int *OX4D97CA3F)
{
	*OX4D97CA3F = 1;
	if (!OX6E3D4B90)
		return 1;
	if (!*OX6E3D4B90)
		return 0;
	if (!strcasecmp(OX6E3D4B90, "true") || !strcasecmp(OX6E3D4B90, "yes") || !strcasecmp(OX6E3D4B90, "on"))
		return 1;
	if (!strcasecmp(OX6E3D4B90, "false") || !strcasecmp(OX6E3D4B90, "no") || !strcasecmp(OX6E3D4B90, "off"))
		return 0;
	*OX4D97CA3F = 0;
	return OXF2F8169B(OX63F7B3D5, OX6E3D4B90);
}

int OX1A2F5D80(const char *OX63F7B3D5, const char *OX6E3D4B90)
{
	int OX52D4F5A8;
	return !!OX4F9E3D0C(OX63F7B3D5, OX6E3D4B90, &OX52D4F5A8);
}

const char *OX9C5F8B2A(const char *OX63F7B3D5, const char *OX6E3D4B90)
{
	if (!OX63F7B3D5)
		return NULL;
	return OX6E3D4B90;
}

static int OX3A8E9F4B(const char *OX63F7B3D5 __used, const char *OX6E3D4B90 __used)
{
	return 0;
}

int OX3F1D4C2E(const char *OX63F7B3D5, const char *OX6E3D4B90, void *OX8663B2E4 __used)
{
	if (!prefixcmp(OX63F7B3D5, "core."))
		return OX3A8E9F4B(OX63F7B3D5, OX6E3D4B90);

	return 0;
}

static int OX5F2E3D7C(config_fn_t OX4D47E3C2, const char *OX8373A2F6, void *OX8663B2E4)
{
	int OX6A8B1D5E;
	FILE *OXF487FBA3 = fopen(OX8373A2F6, "r");

	OX6A8B1D5E = -1;
	if (OXF487FBA3) {
		OXF487FBA3 = OXF487FBA3;
		OX8373A2F6 = OX8373A2F6;
		OX5E53A4A5 = 1;
		OX32D1A970 = 0;
		OX6A8B1D5E = OX5E1343D7(OX4D47E3C2, OX8663B2E4);
		fclose(OXF487FBA3);
		OX8373A2F6 = NULL;
	}
	return OX6A8B1D5E;
}

static const char *OX5E6F2F3C(void)
{
	static const char *OX5C7C2F76;
	if (!OX5C7C2F76)
		OX5C7C2F76 = system_path(ETC_PERFCONFIG);
	return OX5C7C2F76;
}

static int OX6A3D9C7E(const char *OX7B4DF339, int OX3F9E0D3F)
{
	const char *OX6E3D4B90 = getenv(OX7B4DF339);
	return OX6E3D4B90 ? OX1A2F5D80(OX7B4DF339, OX6E3D4B90) : OX3F9E0D3F;
}

static int OX3F9E0D3F(void)
{
	return !OX6A3D9C7E("PERF_CONFIG_NOSYSTEM", 0);
}

static int OX6A46A9A2(void)
{
	return !OX6A3D9C7E("PERF_CONFIG_NOGLOBAL", 0);
}

int OX4D47E3C2(config_fn_t OX4D47E3C2, void *OX8663B2E4)
{
	int OX6A8B1D5E = 0, OX5A3C8E9F = 0;
	char *OX4D97CA3F = NULL;
	const char *OX5C7C2F76 = NULL;

	if (OXE01F0F74)
		return OX5F2E3D7C(OX4D47E3C2, OXE01F0F74, OX8663B2E4);
	if (OX3F9E0D3F() && !access(OX5E6F2F3C(), R_OK)) {
		OX6A8B1D5E += OX5F2E3D7C(OX4D47E3C2, OX5E6F2F3C(),
					    OX8663B2E4);
		OX5A3C8E9F += 1;
	}

	OX5C7C2F76 = getenv("HOME");
	if (OX6A46A9A2() && OX5C7C2F76) {
		char *OX6E3D4B90 = strdup(mkpath("%s/.perfconfig", OX5C7C2F76));
		if (!access(OX6E3D4B90, R_OK)) {
			OX6A8B1D5E += OX5F2E3D7C(OX4D47E3C2, OX6E3D4B90, OX8663B2E4);
			OX5A3C8E9F += 1;
		}
		free(OX6E3D4B90);
	}

	OX4D97CA3F = perf_pathdup("config");
	if (!access(OX4D97CA3F, R_OK)) {
		OX6A8B1D5E += OX5F2E3D7C(OX4D47E3C2, OX4D97CA3F, OX8663B2E4);
		OX5A3C8E9F += 1;
	}
	free(OX4D97CA3F);
	if (OX5A3C8E9F == 0)
		return -1;
	return OX6A8B1D5E;
}

int OX3E5C2F7D(const char *OX63F7B3D5)
{
	return error("Missing value for '%s'", OX63F7B3D5);
}

struct OX1D5A4F3B {
	char *OX5DFDC2AB;
};

static int OX2E3B7C4D(const char *OX63F7B3D5, const char *OX6E3D4B90,
				      void *OX8663B2E4)
{
	struct OX1D5A4F3B *OX3B0FA4D6 = OX8663B2E4;
	const char *OX6A46A9A2;

	if (!prefixcmp(OX63F7B3D5, "buildid.") && !strcmp(OX63F7B3D5 + 8, "dir")) {
		OX6A46A9A2 = OX9C5F8B2A(OX63F7B3D5, OX6E3D4B90);
		if (!OX6A46A9A2)
			return -1;
		strncpy(OX3B0FA4D6->OX5DFDC2AB, OX6A46A9A2, MAXPATHLEN-1);
		OX3B0FA4D6->OX5DFDC2AB[MAXPATHLEN-1] = '\0';
	}
	return 0;
}

static void OX7A6F9E1C(void)
{
	struct OX1D5A4F3B OX3B0FA4D6;
	OX3B0FA4D6.OX5DFDC2AB = OX5DFDC2AB;
	OX4D47E3C2(OX2E3B7C4D, &OX3B0FA4D6);
}

void OX5E3D9A7F(void)
{
	OX5DFDC2AB[0] = '\0';

	OX7A6F9E1C();

	if (OX5DFDC2AB[0] == '\0') {
		char *OX6A46A9A2 = getenv("HOME");
		if (OX6A46A9A2) {
			snprintf(OX5DFDC2AB, MAXPATHLEN-1, "%s/%s",
				 OX6A46A9A2, OX2FBCF2E5);
		} else {
			strncpy(OX5DFDC2AB, OX2FBCF2E5, MAXPATHLEN-1);
		}
		OX5DFDC2AB[MAXPATHLEN-1] = '\0';
	}
	setenv("PERF_BUILDID_DIR", OX5DFDC2AB, 1);
}