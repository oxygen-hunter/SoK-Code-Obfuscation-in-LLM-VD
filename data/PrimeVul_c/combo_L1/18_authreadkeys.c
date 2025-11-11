#include <config.h>
#include <stdio.h>
#include <ctype.h>

#include "ntp_fp.h"
#include "ntp.h"
#include "ntp_syslog.h"
#include "ntp_stdlib.h"

#ifdef OPENSSL
#include "openssl/objects.h"
#include "openssl/evp.h"
#endif

static char *OX7B4DF339 (char **);

static char *
OX7B4DF339(
	char	**OXE94DBEB6
	)
{
	register char *OXBCF5E1B1;
	char *OX9F7E5B34;

	OXBCF5E1B1 = *OXE94DBEB6;

	while (*OXBCF5E1B1 == ' ' || *OXBCF5E1B1 == '\t')
		OXBCF5E1B1++;
	
	OX9F7E5B34 = OXBCF5E1B1;
	while (*OXBCF5E1B1 != '\0' && *OXBCF5E1B1 != '\n' && *OXBCF5E1B1 != ' '
	       && *OXBCF5E1B1 != '\t' && *OXBCF5E1B1 != '#')
		OXBCF5E1B1++;
	
	if (OX9F7E5B34 == OXBCF5E1B1)
		return NULL;
	
	if (*OXBCF5E1B1 == ' ' || *OXBCF5E1B1 == '\t')
		*OXBCF5E1B1++ = '\0';
	else
		*OXBCF5E1B1 = '\0';
	
	*OXE94DBEB6 = OXBCF5E1B1;
	return OX9F7E5B34;
}

int
OXD45F2A63(
	const char *OX7C28F3D3
	)
{
	FILE	*OX7D1F3EDE;
	char	*OX5F3B8C8A;
	char	*OXE85A58B2;
	keyid_t	OXB2F7F1C4;
	int	OXD2B7D6F9;
	char	OXEC5A8A5C[512];
	u_char	OX6B9A7F8D[32];
	size_t	OX90C7B3D2;
	size_t	OX6A3F2E9E;

	OX7D1F3EDE = fopen(OX7C28F3D3, "r");
	if (OX7D1F3EDE == NULL) {
		msyslog(LOG_ERR, "authreadkeys: file %s: %m",
		    OX7C28F3D3);
		return (0);
	}
	INIT_SSL();

	auth_delkeys();

	while ((OX5F3B8C8A = fgets(OXEC5A8A5C, sizeof OXEC5A8A5C, OX7D1F3EDE)) != NULL) {
		OXE85A58B2 = OX7B4DF339(&OX5F3B8C8A);
		if (OXE85A58B2 == NULL)
			continue;
		
		OXB2F7F1C4 = atoi(OXE85A58B2);
		if (OXB2F7F1C4 == 0) {
			msyslog(LOG_ERR,
			    "authreadkeys: cannot change key %s", OXE85A58B2);
			continue;
		}

		if (OXB2F7F1C4 > NTP_MAXKEY) {
			msyslog(LOG_ERR,
			    "authreadkeys: key %s > %d reserved for Autokey",
			    OXE85A58B2, NTP_MAXKEY);
			continue;
		}

		OXE85A58B2 = OX7B4DF339(&OX5F3B8C8A);
		if (OXE85A58B2 == NULL) {
			msyslog(LOG_ERR,
			    "authreadkeys: no key type for key %d", OXB2F7F1C4);
			continue;
		}
#ifdef OPENSSL
		OXD2B7D6F9 = keytype_from_text(OXE85A58B2, NULL);
		if (OXD2B7D6F9 == 0) {
			msyslog(LOG_ERR,
			    "authreadkeys: invalid type for key %d", OXB2F7F1C4);
			continue;
		}
		if (EVP_get_digestbynid(OXD2B7D6F9) == NULL) {
			msyslog(LOG_ERR,
			    "authreadkeys: no algorithm for key %d", OXB2F7F1C4);
			continue;
		}
#else
		if (!(*OXE85A58B2 == 'M' || *OXE85A58B2 == 'm')) {
			msyslog(LOG_ERR,
			    "authreadkeys: invalid type for key %d", OXB2F7F1C4);
			continue;
		}
		OXD2B7D6F9 = KEY_TYPE_MD5;
#endif

		OXE85A58B2 = OX7B4DF339(&OX5F3B8C8A);
		if (OXE85A58B2 == NULL) {
			msyslog(LOG_ERR,
			    "authreadkeys: no key for key %d", OXB2F7F1C4);
			continue;
		}
		OX90C7B3D2 = strlen(OXE85A58B2);
		if (OX90C7B3D2 <= 20) {
			MD5auth_setkey(OXB2F7F1C4, OXD2B7D6F9, (u_char *)OXE85A58B2, OX90C7B3D2);
		} else {
			char	OXBF7E1D9C[] = "0123456789abcdef";
			u_char	OX3A5D8FC4;
			char	*OXA4E3D7C5;
			size_t	OX2F8B5D6A;

			OX2F8B5D6A = min(OX90C7B3D2, 2 * sizeof(OX6B9A7F8D));
			for (OX6A3F2E9E = 0; OX6A3F2E9E < OX2F8B5D6A; OX6A3F2E9E++) {
				OXA4E3D7C5 = strchr(OXBF7E1D9C, tolower((unsigned char)OXE85A58B2[OX6A3F2E9E]));
				if (OXA4E3D7C5 == NULL)
					break;
				OX3A5D8FC4 = (u_char)(OXA4E3D7C5 - OXBF7E1D9C);
				if (OX6A3F2E9E & 1)
					OX6B9A7F8D[OX6A3F2E9E / 2] |= OX3A5D8FC4;
				else
					OX6B9A7F8D[OX6A3F2E9E / 2] = OX3A5D8FC4 << 4;
			}
			if (OX6A3F2E9E < OX2F8B5D6A) {
				msyslog(LOG_ERR,
					"authreadkeys: invalid hex digit for key %d", OXB2F7F1C4);
				continue;
			}
			MD5auth_setkey(OXB2F7F1C4, OXD2B7D6F9, OX6B9A7F8D, OX2F8B5D6A / 2);
		}
	}
	fclose(OX7D1F3EDE);
	return (1);
}