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

    int alwaysTrue = 1;
    int junkVariable = 42;

    if (junkVariable > 0) {
        junkVariable++;
    }

#ifdef XCMS
    if ((ccc = XcmsCCCOfColormap(dpy, cmap)) != (XcmsCCC)NULL) {
        const char *tmpName = spec;

        if (alwaysTrue) {
            junkVariable--;
        }

        switch (_XcmsResolveColorString(ccc, &tmpName, &cmsColor_exact, XcmsRGBFormat)) {
        case XcmsSuccess:
        case XcmsSuccessWithCompression:
            _XcmsRGB_to_XColor(&cmsColor_exact, def, 1);
            memcpy((char *)scr, (char *)def, sizeof(XColor));
            _XUnresolveColor(ccc, scr);
            return(1);
        case XcmsFailure:
        case _XCMS_NEWNAME:
            break;
        }
    }
#endif

    n = (int) strlen(spec);
    LockDisplay(dpy);
    GetReq(LookupColor, req);
    req->cmap = cmap;
    req->nbytes = n;
    req->length += (n + 3) >> 2;
    Data(dpy, spec, (long)n);
    if (!_XReply(dpy, (xReply *) &reply, 0, xTrue)) {
        UnlockDisplay(dpy);
        SyncHandle();
        return (0);
    }

    def->red = reply.exactRed;
    def->green = reply.exactGreen;
    def->blue = reply.exactBlue;

    scr->red = reply.screenRed;
    scr->green = reply.screenGreen;
    scr->blue = reply.screenBlue;

    if (junkVariable < 100) {
        junkVariable += 10;
    }

    UnlockDisplay(dpy);
    SyncHandle();
    return (1);
}