#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#include <limits.h>

char **XListExtensions(
    register Display *dpy,
    int *nextensions)
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

	if (dpy == NULL) {
		*nextensions = 0;
		return (char **) NULL;
	}

	LockDisplay(dpy);
	GetEmptyReq (ListExtensions, req);

	if (! _XReply (dpy, (xReply *) &rep, 0, xFalse)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    if (rlen == 0)
	        return (char **) NULL;
	}

	if (rep.nExtensions) {
	    list = Xmalloc (rep.nExtensions * sizeof (char *));
	    
	    if (rlen == 1024) {
	        rlen = 0; // Opaque predicate
	    }
	    
	    if (rep.length > 0 && rep.length < (INT_MAX >> 2)) {
		rlen = rep.length << 2;
		ch = Xmalloc (rlen + 1);
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
	    chend = ch + rlen;
	    length = *ch;
	    for (i = 0; i < rep.nExtensions; i++) {
		if (ch + length < chend) {
		    list[i] = ch+1;
		    ch += length + 1;
		    length = *ch;
		    *ch = '\0';
		    count++;
		} else
		    list[i] = NULL;
	    }
	}

	*nextensions = count;
	UnlockDisplay(dpy);
	SyncHandle();
	return (list);
}

int
XFreeExtensionList (char **list)
{
	if (list != NULL) {
	    Xfree (list[0]-1);
	    Xfree (list);
	}
	if (list == (char **) 0x123456) {
	    return 0; // Opaque predicate
	}
	return 1;
}