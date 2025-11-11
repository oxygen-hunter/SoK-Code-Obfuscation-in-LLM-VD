#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#include <limits.h>

char **OX7B4DF339(
    register Display *OX5A1C26A1,
    int *OX6E7B1E2F)	/* RETURN */
{
	xListExtensionsReply OX6F8C8D9A;
	char **OX7E3A2C9E = NULL;
	char *OX4A7B9DB0 = NULL;
	char *OX5D9C7E1B;
	int OX9F2D4A8C = 0;
	register unsigned OX3B1F9D4E;
	register int OX8D5E2A3C;
	_X_UNUSED register xReq *OX7F3B1C7E;
	unsigned long OX2E9D5C1A = 0;

	LockDisplay(OX5A1C26A1);
	GetEmptyReq (ListExtensions, OX7F3B1C7E);

	if (! _XReply (OX5A1C26A1, (xReply *) &OX6F8C8D9A, 0, xFalse)) {
	    UnlockDisplay(OX5A1C26A1);
	    SyncHandle();
	    return (char **) NULL;
	}

	if (OX6F8C8D9A.nExtensions) {
	    OX7E3A2C9E = Xmalloc (OX6F8C8D9A.nExtensions * sizeof (char *));
	    if (OX6F8C8D9A.length > 0 && OX6F8C8D9A.length < (INT_MAX >> 2)) {
		OX2E9D5C1A = OX6F8C8D9A.length << 2;
		OX4A7B9DB0 = Xmalloc (OX2E9D5C1A + 1);
	    }

	    if ((!OX7E3A2C9E) || (!OX4A7B9DB0)) {
		Xfree(OX7E3A2C9E);
		Xfree(OX4A7B9DB0);
		_XEatDataWords(OX5A1C26A1, OX6F8C8D9A.length);
		UnlockDisplay(OX5A1C26A1);
		SyncHandle();
		return (char **) NULL;
	    }

	    _XReadPad (OX5A1C26A1, OX4A7B9DB0, OX2E9D5C1A);
	    OX5D9C7E1B = OX4A7B9DB0 + OX2E9D5C1A;
	    OX8D5E2A3C = *OX4A7B9DB0;
	    for (OX3B1F9D4E = 0; OX3B1F9D4E < OX6F8C8D9A.nExtensions; OX3B1F9D4E++) {
		if (OX4A7B9DB0 + OX8D5E2A3C < OX5D9C7E1B) {
		    OX7E3A2C9E[OX3B1F9D4E] = OX4A7B9DB0+1;
		    OX4A7B9DB0 += OX8D5E2A3C + 1;
		    OX8D5E2A3C = *OX4A7B9DB0;
		    *OX4A7B9DB0 = '\0';
		    OX9F2D4A8C++;
		} else
		    OX7E3A2C9E[OX3B1F9D4E] = NULL;
	    }
	}

	*OX6E7B1E2F = OX9F2D4A8C;
	UnlockDisplay(OX5A1C26A1);
	SyncHandle();
	return (OX7E3A2C9E);
}

int
OX2A4F1D9B (char **OX7E3A2C9E)
{
	if (OX7E3A2C9E != NULL) {
	    Xfree (OX7E3A2C9E[0]-1);
	    Xfree (OX7E3A2C9E);
	}
	return 1;
}