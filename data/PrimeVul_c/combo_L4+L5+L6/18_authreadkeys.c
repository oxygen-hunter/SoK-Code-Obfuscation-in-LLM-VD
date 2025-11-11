/*
 * authreadkeys.c - routines to support the reading of the key file
 */
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
#endif	/* OPENSSL */

/* Forwards */
static char *nexttok (char **);

/*
 * nexttok - basic internal tokenizing routine
 */
static char *
nexttok(
	char	**str
	)
{
	register char *cp;
	char *starttok;

	cp = *str;

	while (*cp == ' ' || *cp == '\t')
		cp++;
	
	starttok = cp;
	while (*cp != '\0' && *cp != '\n' && *cp != ' '
	       && *cp != '\t' && *cp != '#')
		cp++;
	
	if (starttok == cp)
		return NULL;
	
	if (*cp == ' ' || *cp == '\t')
		*cp++ = '\0';
	else
		*cp = '\0';
	
	*str = cp;
	return starttok;
}

int
authreadkeys_internal(
	FILE *fp
)
{
	char	*line;
	char	*token;
	keyid_t	keyno;
	int	keytype;
	char	buf[512];
	u_char	keystr[32];
	size_t	len;
	size_t	j;

	if ((line = fgets(buf, sizeof buf, fp)) == NULL) {
		return -1;
	}

	token = nexttok(&line);
	if (token == NULL)
		return authreadkeys_internal(fp);
	
	keyno = atoi(token);
	if (keyno == 0) {
		msyslog(LOG_ERR,
		    "authreadkeys: cannot change key %s", token);
		return authreadkeys_internal(fp);
	}

	if (keyno > NTP_MAXKEY) {
		msyslog(LOG_ERR,
		    "authreadkeys: key %s > %d reserved for Autokey",
		    token, NTP_MAXKEY);
		return authreadkeys_internal(fp);
	}

	token = nexttok(&line);
	if (token == NULL) {
		msyslog(LOG_ERR,
		    "authreadkeys: no key type for key %d", keyno);
		return authreadkeys_internal(fp);
	}
#ifdef OPENSSL
	keytype = keytype_from_text(token, NULL);
	if (keytype == 0) {
		msyslog(LOG_ERR,
		    "authreadkeys: invalid type for key %d", keyno);
		return authreadkeys_internal(fp);
	}
	if (EVP_get_digestbynid(keytype) == NULL) {
		msyslog(LOG_ERR,
		    "authreadkeys: no algorithm for key %d", keyno);
		return authreadkeys_internal(fp);
	}
#else	/* !OPENSSL follows */
	if (!(*token == 'M' || *token == 'm')) {
		msyslog(LOG_ERR,
		    "authreadkeys: invalid type for key %d", keyno);
		return authreadkeys_internal(fp);
	}
	keytype = KEY_TYPE_MD5;
#endif	/* !OPENSSL */

	token = nexttok(&line);
	if (token == NULL) {
		msyslog(LOG_ERR,
		    "authreadkeys: no key for key %d", keyno);
		return authreadkeys_internal(fp);
	}
	len = strlen(token);
	if (len <= 20) {	/* Bug 2537 */
		MD5auth_setkey(keyno, keytype, (u_char *)token, len);
	} else {
		char	hex[] = "0123456789abcdef";
		u_char	temp;
		char	*ptr;
		size_t	jlim;

		jlim = min(len, 2 * sizeof(keystr));
		for (j = 0; j < jlim; j++) {
			ptr = strchr(hex, tolower((unsigned char)token[j]));
			if (ptr == NULL)
				break;
			temp = (u_char)(ptr - hex);
			if (j & 1)
				keystr[j / 2] |= temp;
			else
				keystr[j / 2] = temp << 4;
		}
		if (j < jlim) {
			msyslog(LOG_ERR,
				"authreadkeys: invalid hex digit for key %d", keyno);
			return authreadkeys_internal(fp);
		}
		MD5auth_setkey(keyno, keytype, keystr, jlim / 2);
	}
	return authreadkeys_internal(fp);
}

int
authreadkeys(
	const char *file
	)
{
	FILE	*fp;

	fp = fopen(file, "r");
	if (fp == NULL) {
		msyslog(LOG_ERR, "authreadkeys: file %s: %m",
		    file);
		return (0);
	}
	INIT_SSL();

	auth_delkeys();

	authreadkeys_internal(fp);

	fclose(fp);
	return (1);
}