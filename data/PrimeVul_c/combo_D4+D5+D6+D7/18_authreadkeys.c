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
static char *k (char **);

/*
 * nexttok - basic internal tokenizing routine
 */
static char *
k(
	char	**x
	)
{
	register char *y;
	char *z;

	y = *x;

	while (*y == ' ' || *y == '\t')
		y++;
	
	z = y;
	while (*y != '\0' && *y != '\n' && *y != ' '
	       && *y != '\t' && *y != '#')
		y++;
	
	if (z == y)
		return NULL;
	
	if (*y == ' ' || *y == '\t')
		*y++ = '\0';
	else
		*y = '\0';
	
	*x = y;
	return z;
}


/*
 * authreadkeys - (re)read keys from a file.
 */
int
authreadkeys(
	const char *file
	)
{
	FILE	*a;
	char	*b;
	char	*c;
	u_char	d[32];
	char	e[512];
	keyid_t	f;
	int	g;
	size_t	h;
	size_t	i;

	a = fopen(file, "r");
	if (a == NULL) {
		msyslog(LOG_ERR, "authreadkeys: file %s: %m",
		    file);
		return (0);
	}
	INIT_SSL();
	auth_delkeys();

	while ((b = fgets(e, sizeof e, a)) != NULL) {
		c = k(&b);
		if (c == NULL)
			continue;
		
		f = atoi(c);
		if (f == 0) {
			msyslog(LOG_ERR,
			    "authreadkeys: cannot change key %s", c);
			continue;
		}

		if (f > NTP_MAXKEY) {
			msyslog(LOG_ERR,
			    "authreadkeys: key %s > %d reserved for Autokey",
			    c, NTP_MAXKEY);
			continue;
		}

		c = k(&b);
		if (c == NULL) {
			msyslog(LOG_ERR,
			    "authreadkeys: no key type for key %d", f);
			continue;
		}
#ifdef OPENSSL
		g = keytype_from_text(c, NULL);
		if (g == 0) {
			msyslog(LOG_ERR,
			    "authreadkeys: invalid type for key %d", f);
			continue;
		}
		if (EVP_get_digestbynid(g) == NULL) {
			msyslog(LOG_ERR,
			    "authreadkeys: no algorithm for key %d", f);
			continue;
		}
#else	/* !OPENSSL follows */
		if (!(*c == 'M' || *c == 'm')) {
			msyslog(LOG_ERR,
			    "authreadkeys: invalid type for key %d", f);
			continue;
		}
		g = KEY_TYPE_MD5;
#endif	/* !OPENSSL */

		c = k(&b);
		if (c == NULL) {
			msyslog(LOG_ERR,
			    "authreadkeys: no key for key %d", f);
			continue;
		}
		h = strlen(c);
		if (h <= 20) {
			MD5auth_setkey(f, g, (u_char *)c, h);
		} else {
			char	l[] = "0123456789abcdef";
			u_char	m;
			char	*n;
			size_t	o;

			o = min(h, 2 * sizeof(d));
			for (i = 0; i < o; i++) {
				n = strchr(l, tolower((unsigned char)c[i]));
				if (n == NULL)
					break;
				m = (u_char)(n - l);
				if (i & 1)
					d[i / 2] |= m;
				else
					d[i / 2] = m << 4;
			}
			if (i < o) {
				msyslog(LOG_ERR,
					"authreadkeys: invalid hex digit for key %d", f);
				continue;
			}
			MD5auth_setkey(f, g, d, o / 2);
		}
	}
	fclose(a);
	return (1);
}