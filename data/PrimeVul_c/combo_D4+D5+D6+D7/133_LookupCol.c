#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include "Xlibint.h"
#include "Xcmsint.h"

typedef struct {
    XColor def;
    XColor scr;
    xLookupColorReply reply;
} ColorData;

typedef struct {
    Display *dpy;
    Colormap cmap;
    _Xconst char *spec;
    int n;
} LookupContext;

Status
XLookupColor (
	register Display *d,
        Colormap cm,
	_Xconst char *sp,
	XColor *dColor,
	XColor *sColor)
{
	LookupContext ctx;
	ColorData cData;
	register xLookupColorReq *rq;
	XcmsCCC cc;
	XcmsColor cColor_exact;

	ctx.dpy = d;
	ctx.cmap = cm;
	ctx.spec = sp;

#ifdef XCMS
	if ((cc = XcmsCCCOfColormap(ctx.dpy, ctx.cmap)) != (XcmsCCC)NULL) {
	    const char *tName = ctx.spec;

	    switch (_XcmsResolveColorString(cc, &tName, &cColor_exact,
					    XcmsRGBFormat)) {
	    case XcmsSuccess:
	    case XcmsSuccessWithCompression:
		_XcmsRGB_to_XColor(&cColor_exact, dColor, 1);
		memcpy((char *)sColor, (char *)dColor, sizeof(XColor));
		_XUnresolveColor(cc, sColor);
		return(1);
	    case XcmsFailure:
	    case _XCMS_NEWNAME:
		break;
	    }
	}
#endif

	ctx.n = (int) strlen (ctx.spec);
	LockDisplay(ctx.dpy);
	GetReq (LookupColor, rq);
	rq->cmap = ctx.cmap;
	rq->nbytes = ctx.n;
	rq->length += (ctx.n + 3) >> 2;
	Data (ctx.dpy, ctx.spec, (long)ctx.n);
	if (!_XReply (ctx.dpy, (xReply *) &cData.reply, 0, xTrue)) {
	    UnlockDisplay(ctx.dpy);
	    SyncHandle();
	    return (0);
	    }
	cData.def.red   = cData.reply.exactRed;
	cData.def.green = cData.reply.exactGreen;
	cData.def.blue  = cData.reply.exactBlue;

	cData.scr.red   = cData.reply.screenRed;
	cData.scr.green = cData.reply.screenGreen;
	cData.scr.blue  = cData.reply.screenBlue;

	*dColor = cData.def;
	*sColor = cData.scr;

	UnlockDisplay(ctx.dpy);
	SyncHandle();
	return (1);
}