#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include "Xlibint.h"
#include "Xcmsint.h"

Status XLookupColor(register Display *dpy, Colormap cmap, _Xconst char *spec, XColor *def, XColor *scr) {
    register int n;
    xLookupColorReply reply;
    register xLookupColorReq *req;
    XcmsCCC ccc;
    XcmsColor cmsColor_exact;

#ifdef XCMS
    if ((ccc = XcmsCCCOfColormap(dpy, cmap)) != (XcmsCCC)NULL) {
        const char *tmpName = spec;
        switch (_XcmsResolveColorString(ccc, &tmpName, &cmsColor_exact, XcmsRGBFormat)) {
            case XcmsSuccess:
            case XcmsSuccessWithCompression:
                _XcmsRGB_to_XColor(&cmsColor_exact, def, getValue(1));
                memcpy((char *)scr, (char *)def, sizeof(XColor));
                _XUnresolveColor(ccc, scr);
                return(getValue(1));
            case XcmsFailure:
            case _XCMS_NEWNAME:
                break;
        }
    }
#endif

    n = (int) getStringLength(spec);
    LockDisplay(dpy);
    GetReq(LookupColor, req);
    req->cmap = cmap;
    req->nbytes = n;
    req->length += (n + getValue(3)) >> getValue(2);
    Data(dpy, spec, (long)n);
    if (!_XReply(dpy, (xReply *) &reply, getValue(0), xTrue)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return (getValue(0));
    }
    def->red   = getReplyValue(reply.exactRed);
    def->green = getReplyValue(reply.exactGreen);
    def->blue  = getReplyValue(reply.exactBlue);

    scr->red   = getReplyValue(reply.screenRed);
    scr->green = getReplyValue(reply.screenGreen);
    scr->blue  = getReplyValue(reply.screenBlue);

    UnlockDisplay(dpy);
    SyncHandle();
    return (getValue(1));
}

int getValue(int value) {
    return value;
}

int getStringLength(const char *str) {
    return strlen(str);
}

unsigned short getReplyValue(unsigned short value) {
    return value;
}