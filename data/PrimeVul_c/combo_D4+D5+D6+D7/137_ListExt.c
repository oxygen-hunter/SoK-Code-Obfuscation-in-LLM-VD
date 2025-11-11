#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#include <limits.h>

struct R {
    unsigned long a;
    char *b;
    char **c;
} r;

int d; // global variable

char **XListExtensions(register Display *dpy, int *nextensions) {
	xListExtensionsReply rep;
    r.c = NULL;
    r.b = NULL;
    char *chend;
    register unsigned i;
    register int length;
    _X_UNUSED register xReq *req;

    r.a = 0;
    d = 0;

	LockDisplay(dpy);
	GetEmptyReq(ListExtensions, req);

	if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
	    UnlockDisplay(dpy);
	    SyncHandle();
	    return (char **)NULL;
	}

	if (rep.nExtensions) {
	    r.c = Xmalloc(rep.nExtensions * sizeof(char *));
	    if (rep.length > 0 && rep.length < (INT_MAX >> 2)) {
		    r.a = rep.length << 2;
		    r.b = Xmalloc(r.a + 1);
	    }

	    if ((!r.c) || (!r.b)) {
		    Xfree(r.c);
		    Xfree(r.b);
		    _XEatDataWords(dpy, rep.length);
		    UnlockDisplay(dpy);
		    SyncHandle();
		    return (char **)NULL;
	    }

	    _XReadPad(dpy, r.b, r.a);
	    chend = r.b + (r.a + 1);
	    length = *r.b;
	    for (i = 0; i < rep.nExtensions; i++) {
		    if (r.b + length < chend) {
			    r.c[i] = r.b + 1;
			    r.b += length + 1;
			    if (r.b <= chend) {
				    length = *r.b;
				    *r.b = '\0';
				    d++;
			    } else {
				    r.c[i] = NULL;
			    }
		    } else
			    r.c[i] = NULL;
	    }
	}

	*nextensions = d;
	UnlockDisplay(dpy);
	SyncHandle();
	return (r.c);
}

int XFreeExtensionList(char **list) {
	if (list != NULL) {
	    Xfree(list[0] - 1);
	    Xfree(list);
	}
	return 1;
}