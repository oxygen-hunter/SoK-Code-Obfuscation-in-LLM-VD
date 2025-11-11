#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "inputstr.h"           
#include "windowstr.h"          
#include "extnsionst.h"         
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "exevents.h"
#include "exglobals.h"

#include "grabdev.h"
#include "sendexev.h"

extern int lastEvent;

int _X_COLD
SProcXSendExtensionEvent(ClientPtr client)
{
    struct {
        CARD32 *p;
        int i;
        xEvent eventT;
        xEvent *eventP;
        EventSwapPtr proc;
    } localVars;

    REQUEST(xSendExtensionEventReq);
    swaps(&stuff->length);
    REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq);
    swapl(&stuff->destination);
    swaps(&stuff->count);

    if (stuff->length !=
        bytes_to_int32(sizeof(xSendExtensionEventReq)) + stuff->count +
        bytes_to_int32(stuff->num_events * sizeof(xEvent)))
        return BadLength;

    localVars.eventP = (xEvent *) &stuff[1];
    for (localVars.i = 0; localVars.i < stuff->num_events; localVars.i++, localVars.eventP++) {
        localVars.proc = EventSwapVector[localVars.eventP->u.u.type & 0177];
        if (localVars.proc == NotImplemented)
            return BadValue;
        (*localVars.proc) (localVars.eventP, &localVars.eventT);
        *localVars.eventP = localVars.eventT;
    }

    localVars.p = (CARD32 *) (((xEvent *) &stuff[1]) + stuff->num_events);
    SwapLongs(localVars.p, stuff->count);
    return (ProcXSendExtensionEvent(client));
}

int
ProcXSendExtensionEvent(ClientPtr client)
{
    struct {
        int ret;
        DeviceIntPtr dev;
        xEvent *first;
        XEventClass *list;
        struct tmask tmp[EMASKSIZE];
    } localVars;

    REQUEST(xSendExtensionEventReq);
    REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq);

    if (stuff->length !=
        bytes_to_int32(sizeof(xSendExtensionEventReq)) + stuff->count +
        (stuff->num_events * bytes_to_int32(sizeof(xEvent))))
        return BadLength;

    localVars.ret = dixLookupDevice(&localVars.dev, stuff->deviceid, client, DixWriteAccess);
    if (localVars.ret != Success)
        return localVars.ret;

    if (stuff->num_events == 0)
        return localVars.ret;

    localVars.first = ((xEvent *) &stuff[1]);
    if (!((EXTENSION_EVENT_BASE <= localVars.first->u.u.type) &&
          (localVars.first->u.u.type < lastEvent))) {
        client->errorValue = localVars.first->u.u.type;
        return BadValue;
    }

    localVars.list = (XEventClass *) (localVars.first + stuff->num_events);
    if ((localVars.ret = CreateMaskFromList(client, localVars.list, stuff->count, localVars.tmp, localVars.dev,
                                  X_SendExtensionEvent)) != Success)
        return localVars.ret;

    localVars.ret = (SendEvent(client, localVars.dev, stuff->destination,
                     stuff->propagate, (xEvent *) &stuff[1],
                     localVars.tmp[stuff->deviceid].mask, stuff->num_events));

    return localVars.ret;
}