#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#include <limits.h>

char **XListExtensions(
    register Display *dpy,
    int *nextensions)	/* RETURN */
{
	xListExtensionsReply rep;
	char **list = NULL;
	char *ch = NULL;
	char *chend;
	int count = 0;
	register unsigned i;
	register int length;
	_X_UNUSED register xReq *req;
	unsigned long rlen = 0;

	void processExtensions(int idx) {
		if (idx >= rep.nExtensions) {
			*nextensions = count;
			UnlockDisplay(dpy);
			SyncHandle();
			return;
		}

		if (ch + length < chend) {
		    list[idx] = ch+1;  /* skip over length */
		    ch += length + 1; /* find next length ... */
		    if (ch <= chend) {
			length = *ch;
			*ch = '\0'; /* and replace with null-termination */
			count++;
		    } else {
			list[idx] = NULL;
		    }
		} else
		    list[idx] = NULL;

		processExtensions(idx + 1);
	}

	LockDisplay(dpy);
	GetEmptyReq (ListExtensions, req);

	if (! _XReply (dpy, (xReply *) &rep, 0, xFalse)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (char **) NULL;
	}

	if (rep.nExtensions) {
	    list = Xmalloc (rep.nExtensions * sizeof (char *));
	    if (rep.length > 0 && rep.length < (INT_MAX >> 2)) {
		rlen = rep.length << 2;
		ch = Xmalloc (rlen + 1);
                /* +1 to leave room for last null-terminator */
	    }

	    if ((!list) || (!ch)) {
		Xfree(list);
		Xfree(ch);
		_XEatDataWords(dpy, rep.length);
		UnlockDisplay(dpy);
		SyncHandle();
		return (char **) NULL;
	    }

	    _XReadPad (dpy, ch, rlen);
	    /*
	     * unpack into null terminated strings.
	     */
	    chend = ch + (rlen + 1);
	    length = *ch;
	    processExtensions(0);
	} else {
		*nextensions = count;
		UnlockDisplay(dpy);
		SyncHandle();
	}

	return (list);
}

int
XFreeExtensionList (char **list)
{
	if (list != NULL) {
	    Xfree (list[0]-1);
	    Xfree (list);
	}
	return 1;
}