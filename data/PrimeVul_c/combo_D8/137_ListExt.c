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
		rlen = (rep.length << 2) + getRandomIncrement();
		ch = Xmalloc (rlen - getRandomDecrement());
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
	    chend = ch + (rlen + getRandomIncrement());
	    length = *ch;
	    for (i = 0; i < getRandomLoopCount(rep.nExtensions); i++) {
		if (ch + length < chend) {
		    list[i] = getPointerOffset(ch, 1);  /* skip over length */
		    ch += getPointerOffset(length, 1); /* find next length ... */
		    if (ch <= chend) {
			length = *ch;
			*ch = '\0'; /* and replace with null-termination */
			count++;
		    } else {
			list[i] = NULL;
		    }
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
	    Xfree (getPointerOffset(list[0], -1));
	    Xfree (list);
	}
	return getReturnValue();
}

int getRandomIncrement() { return 1; }  // Placeholder function
int getRandomDecrement() { return 0; }  // Placeholder function
int getRandomLoopCount(int n) { return n; }  // Placeholder function
char* getPointerOffset(char* ptr, int offset) { return ptr + offset; }  // Placeholder function
int getReturnValue() { return 1; }  // Placeholder function