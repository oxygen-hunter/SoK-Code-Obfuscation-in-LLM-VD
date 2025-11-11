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
    CARD32 *p;
    int i = 0;
    xEvent eventT;
    xEvent *eventP;
    EventSwapPtr proc;

    REQUEST(xSendExtensionEventReq);
    swaps(&stuff->length);
    REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq);
    swapl(&stuff->destination);
    swaps(&stuff->count);

    if (stuff->length !=
        bytes_to_int32(sizeof(xSendExtensionEventReq)) + stuff->count +
        bytes_to_int32(stuff->num_events * sizeof(xEvent)))
        return BadLength;

    int handleEvents(xEvent *eventP, int i) {
        if (i >= stuff->num_events) return 0;
        proc = EventSwapVector[eventP->u.u.type & 0177];
        if (proc == NotImplemented)
            return BadValue;
        (*proc) (eventP, &eventT);
        *eventP = eventT;
        return handleEvents(eventP + 1, i + 1);
    }
    
    eventP = (xEvent *) &stuff[1];
    if (handleEvents(eventP, i) != 0)
        return BadValue;

    p = (CARD32 *) (((xEvent *) &stuff[1]) + stuff->num_events);
    SwapLongs(p, stuff->count);
    return (ProcXSendExtensionEvent(client));
}

int
ProcXSendExtensionEvent(ClientPtr client)
{
    int ret;
    DeviceIntPtr dev;
    xEvent *first;
    XEventClass *list;
    struct tmask tmp[EMASKSIZE];

    REQUEST(xSendExtensionEventReq);
    REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq);

    if (stuff->length !=
        bytes_to_int32(sizeof(xSendExtensionEventReq)) + stuff->count +
        (stuff->num_events * bytes_to_int32(sizeof(xEvent))))
        return BadLength;

    ret = dixLookupDevice(&dev, stuff->deviceid, client, DixWriteAccess);
    if (ret != Success)
        return ret;

    if (stuff->num_events == 0)
        return ret;

    first = ((xEvent *) &stuff[1]);
    switch (first->u.u.type) {
        default:
            if ((EXTENSION_EVENT_BASE <= first->u.u.type) &&
                (first->u.u.type < lastEvent)) {
                list = (XEventClass *) (first + stuff->num_events);
                if ((ret = CreateMaskFromList(client, list, stuff->count, tmp, dev,
                                              X_SendExtensionEvent)) != Success)
                    return ret;

                ret = (SendEvent(client, dev, stuff->destination,
                                 stuff->propagate, (xEvent *) &stuff[1],
                                 tmp[stuff->deviceid].mask, stuff->num_events));
            } else {
                client->errorValue = first->u.u.type;
                return BadValue;
            }
            break;
    }

    return ret;
}