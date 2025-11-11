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
static char *getNextToken (char **);

static char *
getNextToken(
	char	**str
	)
{
	register char *a;
	char *b;
	a = *str;
	while (*a == ' ' || *a == '\t')
		a++;
	b = a;
	while (*a != '\0' && *a != '\n' && *a != ' '
	       && *a != '\t' && *a != '#')
		a++;
	if (b == a)
		return NULL;
	if (*a == ' ' || *a == '\t')
		*a++ = '\0';
	else
		*a = '\0';
	*str = a;
	return b;
}

int
authreadkeys(
	const char *filename
	)
{
	FILE	*myfile;
	char	*myline;
	char	*mytoken;
	keyid_t	mykeyno;
	int	mytype;
	char	mybuffer[512];
	u_char	mkeystr[32];
	size_t	mlen;
	size_t	mj;

	myfile = fopen(filename, "r");
	if (myfile == NULL) {
		msyslog(LOG_ERR, "authreadkeys: file %s: %m",
		    filename);
		return (0);
	}
	INIT_SSL();
	auth_delkeys();
	while ((myline = fgets(mybuffer, sizeof mybuffer, myfile)) != NULL) {
		mytoken = getNextToken(&myline);
		if (mytoken == NULL)
			continue;
		mykeyno = atoi(mytoken);
		if (mykeyno == 0) {
			msyslog(LOG_ERR,
			    "authreadkeys: cannot change key %s", mytoken);
			continue;
		}
		if (mykeyno > NTP_MAXKEY) {
			msyslog(LOG_ERR,
			    "authreadkeys: key %s > %d reserved for Autokey",
			    mytoken, NTP_MAXKEY);
			continue;
		}
		mytoken = getNextToken(&myline);
		if (mytoken == NULL) {
			msyslog(LOG_ERR,
			    "authreadkeys: no key type for key %d", mykeyno);
			continue;
		}
#ifdef OPENSSL
		mytype = keytype_from_text(mytoken, NULL);
		if (mytype == 0) {
			msyslog(LOG_ERR,
			    "authreadkeys: invalid type for key %d", mykeyno);
			continue;
		}
		if (EVP_get_digestbynid(mytype) == NULL) {
			msyslog(LOG_ERR,
			    "authreadkeys: no algorithm for key %d", mykeyno);
			continue;
		}
#else	/* !OPENSSL follows */
		if (!(*mytoken == 'M' || *mytoken == 'm')) {
			msyslog(LOG_ERR,
			    "authreadkeys: invalid type for key %d", mykeyno);
			continue;
		}
		mytype = KEY_TYPE_MD5;
#endif	/* !OPENSSL */
		mytoken = getNextToken(&myline);
		if (mytoken == NULL) {
			msyslog(LOG_ERR,
			    "authreadkeys: no key for key %d", mykeyno);
			continue;
		}
		mlen = strlen(mytoken);
		if (mlen <= 20) {
			MD5auth_setkey(mykeyno, mytype, (u_char *)mytoken, mlen);
		} else {
			char	hex[] = "0123456789abcdef";
			u_char	temp;
			char	*ptr;
			size_t	jlim;
			jlim = min(mlen, 2 * sizeof(mkeystr));
			for (mj = 0; mj < jlim; mj++) {
				ptr = strchr(hex, tolower((unsigned char)mytoken[mj]));
				if (ptr == NULL)
					break;
				temp = (u_char)(ptr - hex);
				if (mj & 1)
					mkeystr[mj / 2] |= temp;
				else
					mkeystr[mj / 2] = temp << 4;
			}
			if (mj < jlim) {
				msyslog(LOG_ERR,
					"authreadkeys: invalid hex digit for key %d", mykeyno);
				continue;
			}
			MD5auth_setkey(mykeyno, mytype, mkeystr, jlim / 2);
		}
	}
	fclose(myfile);
	return (1);
}