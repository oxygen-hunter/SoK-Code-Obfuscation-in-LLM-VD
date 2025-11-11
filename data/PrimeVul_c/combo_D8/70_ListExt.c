#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#include <limits.h>

static int getRepLength(xListExtensionsReply *rep) {
    return rep->length;
}

static int getRepNExtensions(xListExtensionsReply *rep) {
    return rep->nExtensions;
}

static unsigned long getRlen(xListExtensionsReply *rep) {
    return getRepLength(rep) << 2;
}

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

	LockDisplay(dpy);
	GetEmptyReq (ListExtensions, req);

	if (! _XReply (dpy, (xReply *) &rep, 0, xFalse)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (char **) NULL;
	}

	if (getRepNExtensions(&rep)) {
	    list = Xmalloc (getRepNExtensions(&rep) * sizeof (char *));
	    if (getRepLength(&rep) > 0 && getRepLength(&rep) < (INT_MAX >> 2)) {
		rlen = getRlen(&rep);
		ch = Xmalloc (rlen + 1);
	    }

	    if ((!list) || (!ch)) {
		Xfree(list);
		Xfree(ch);
		_XEatDataWords(dpy, getRepLength(&rep));
		UnlockDisplay(dpy);
		SyncHandle();
		return (char **) NULL;
	    }

	    _XReadPad (dpy, ch, rlen);
	    chend = ch + rlen;
	    length = *ch;
	    for (i = 0; i < getRepNExtensions(&rep); i++) {
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
	return 1;
}