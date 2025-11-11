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

	/*
	 * Space past white space
	 */
	while (*cp == ' ' || *cp == '\t')
		cp++;
	
	/*
	 * Save this and space to end of token
	 */
	starttok = cp;
	while (*cp != '\0' && *cp != '\n' && *cp != ' '
	       && *cp != '\t' && *cp != '#')
		cp++;
	
	/*
	 * If token length is zero return an error, else set end of
	 * token to zero and return start.
	 */
	if (starttok == cp)
		return NULL;
	
	if (*cp == ' ' || *cp == '\t')
		*cp++ = '\0';
	else
		*cp = '\0';
	
	*str = cp;
	return starttok;
}


/*
 * authreadkeys - (re)read keys from a file.
 */
int
authreadkeys(
	const char *file
	)
{
	FILE	*fp;
	char	*line;
	char	*token;
	keyid_t	keyno;
	int	keytype;
	char	buf[512];		/* lots of room for line */
	u_char	keystr[32];		/* Bug 2537 */
	size_t	len;
	size_t	j;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
			case 0:
				fp = fopen(file, "r");
				if (fp == NULL) {
					msyslog(LOG_ERR, "authreadkeys: file %s: %m", file);
					return (0);
				}
				INIT_SSL();
				auth_delkeys();
				dispatcher = 1;
				break;

			case 1:
				line = fgets(buf, sizeof buf, fp);
				if (line == NULL) {
					dispatcher = 6;
					break;
				}
				token = nexttok(&line);
				if (token == NULL) {
					dispatcher = 1;
					break;
				}
				keyno = atoi(token);
				if (keyno == 0) {
					msyslog(LOG_ERR, "authreadkeys: cannot change key %s", token);
					dispatcher = 1;
					break;
				}
				if (keyno > NTP_MAXKEY) {
					msyslog(LOG_ERR, "authreadkeys: key %s > %d reserved for Autokey", token, NTP_MAXKEY);
					dispatcher = 1;
					break;
				}
				dispatcher = 2;
				break;

			case 2:
				token = nexttok(&line);
				if (token == NULL) {
					msyslog(LOG_ERR, "authreadkeys: no key type for key %d", keyno);
					dispatcher = 1;
					break;
				}
#ifdef OPENSSL
				keytype = keytype_from_text(token, NULL);
				if (keytype == 0) {
					msyslog(LOG_ERR, "authreadkeys: invalid type for key %d", keyno);
					dispatcher = 1;
					break;
				}
				if (EVP_get_digestbynid(keytype) == NULL) {
					msyslog(LOG_ERR, "authreadkeys: no algorithm for key %d", keyno);
					dispatcher = 1;
					break;
				}
#else	/* !OPENSSL follows */
				if (!(*token == 'M' || *token == 'm')) {
					msyslog(LOG_ERR, "authreadkeys: invalid type for key %d", keyno);
					dispatcher = 1;
					break;
				}
				keytype = KEY_TYPE_MD5;
#endif	/* !OPENSSL */
				dispatcher = 3;
				break;

			case 3:
				token = nexttok(&line);
				if (token == NULL) {
					msyslog(LOG_ERR, "authreadkeys: no key for key %d", keyno);
					dispatcher = 1;
					break;
				}
				len = strlen(token);
				if (len <= 20) {	/* Bug 2537 */
					MD5auth_setkey(keyno, keytype, (u_char *)token, len);
					dispatcher = 1;
					break;
				}
				dispatcher = 4;
				break;

			case 4:
			{
				char hex[] = "0123456789abcdef";
				u_char temp;
				char *ptr;
				size_t jlim;

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
					msyslog(LOG_ERR, "authreadkeys: invalid hex digit for key %d", keyno);
					dispatcher = 1;
					break;
				}
				MD5auth_setkey(keyno, keytype, keystr, jlim / 2);
				dispatcher = 1;
				break;
			}

			case 6:
				fclose(fp);
				return (1);
		}
	}
}