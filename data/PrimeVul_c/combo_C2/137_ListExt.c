#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "Xlibint.h"
#include <limits.h>

char **XListExtensions(
    register Display *dpy,
    int *nextensions) /* RETURN */
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
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                LockDisplay(dpy);
                GetEmptyReq(ListExtensions, req);
                state = 1;
                break;
            case 1:
                if (!_XReply(dpy, (xReply *)&rep, 0, xFalse)) {
                    UnlockDisplay(dpy);
                    SyncHandle();
                    return (char **)NULL;
                }
                state = 2;
                break;
            case 2:
                if (rep.nExtensions) {
                    list = Xmalloc(rep.nExtensions * sizeof(char *));
                    if (rep.length > 0 && rep.length < (INT_MAX >> 2)) {
                        rlen = rep.length << 2;
                        ch = Xmalloc(rlen + 1);
                    }
                    state = 3;
                } else {
                    state = 6;
                }
                break;
            case 3:
                if ((!list) || (!ch)) {
                    Xfree(list);
                    Xfree(ch);
                    _XEatDataWords(dpy, rep.length);
                    UnlockDisplay(dpy);
                    SyncHandle();
                    return (char **)NULL;
                }
                _XReadPad(dpy, ch, rlen);
                chend = ch + (rlen + 1);
                length = *ch;
                state = 4;
                break;
            case 4:
                for (i = 0; i < rep.nExtensions; i++) {
                    if (ch + length < chend) {
                        list[i] = ch + 1;
                        ch += length + 1;
                        if (ch <= chend) {
                            length = *ch;
                            *ch = '\0';
                            count++;
                        } else {
                            list[i] = NULL;
                        }
                    } else
                        list[i] = NULL;
                }
                state = 5;
                break;
            case 5:
                *nextensions = count;
                UnlockDisplay(dpy);
                SyncHandle();
                return (list);
            case 6:
                *nextensions = count;
                UnlockDisplay(dpy);
                SyncHandle();
                return (list);
        }
    }
}

int XFreeExtensionList(char **list) {
    if (list != NULL) {
        Xfree(list[0] - 1);
        Xfree(list);
    }
    return 1;
}