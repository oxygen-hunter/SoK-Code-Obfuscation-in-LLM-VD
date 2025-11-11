#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#include <limits.h>

char **OX7B4DF339(
    register Display *OXD8E8FCA8,
    int *OXE9A9A4F7)	
{
	xListExtensionsReply OX9A75A5D2;
	char **OXB9B8EBCD = NULL;
	char *OXCD4B9E50 = NULL;
	char *OXA1E3C9B8;
	int OX0F2A3E6D = 0;
	register unsigned OX5D929D7F;
	register int OX3F4F5A5B;
	_X_UNUSED register xReq *OX3A5D8C7A;
	unsigned long OX5A7F4B3C = 0;

	LockDisplay(OXD8E8FCA8);
	GetEmptyReq (ListExtensions, OX3A5D8C7A);

	if (! _XReply (OXD8E8FCA8, (xReply *) &OX9A75A5D2, 0, xFalse)) {
	    UnlockDisplay(OXD8E8FCA8);
	    SyncHandle();
	    return (char **) NULL;
	}

	if (OX9A75A5D2.nExtensions) {
	    OXB9B8EBCD = Xmalloc (OX9A75A5D2.nExtensions * sizeof (char *));
	    if (OX9A75A5D2.length > 0 && OX9A75A5D2.length < (INT_MAX >> 2)) {
		OX5A7F4B3C = OX9A75A5D2.length << 2;
		OXCD4B9E50 = Xmalloc (OX5A7F4B3C + 1);
	    }

	    if ((!OXB9B8EBCD) || (!OXCD4B9E50)) {
		Xfree(OXB9B8EBCD);
		Xfree(OXCD4B9E50);
		_XEatDataWords(OXD8E8FCA8, OX9A75A5D2.length);
		UnlockDisplay(OXD8E8FCA8);
		SyncHandle();
		return (char **) NULL;
	    }

	    _XReadPad (OXD8E8FCA8, OXCD4B9E50, OX5A7F4B3C);
	    OXA1E3C9B8 = OXCD4B9E50 + (OX5A7F4B3C + 1);
	    OX3F4F5A5B = *OXCD4B9E50;
	    for (OX5D929D7F = 0; OX5D929D7F < OX9A75A5D2.nExtensions; OX5D929D7F++) {
		if (OXCD4B9E50 + OX3F4F5A5B < OXA1E3C9B8) {
		    OXB9B8EBCD[OX5D929D7F] = OXCD4B9E50+1;
		    OXCD4B9E50 += OX3F4F5A5B + 1;
		    if (OXCD4B9E50 <= OXA1E3C9B8) {
			OX3F4F5A5B = *OXCD4B9E50;
			*OXCD4B9E50 = '\0';
			OX0F2A3E6D++;
		    } else {
			OXB9B8EBCD[OX5D929D7F] = NULL;
		    }
		} else
		    OXB9B8EBCD[OX5D929D7F] = NULL;
	    }
	}

	*OXE9A9A4F7 = OX0F2A3E6D;
	UnlockDisplay(OXD8E8FCA8);
	SyncHandle();
	return (OXB9B8EBCD);
}

int
OX7E3D4F1A (char **OXB9B8EBCD)
{
	if (OXB9B8EBCD != NULL) {
	    Xfree (OXB9B8EBCD[0]-1);
	    Xfree (OXB9B8EBCD);
	}
	return 1;
}