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
    int i, anUnusedVariable = 0;
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
        bytes_to_int32(stuff->num_events * sizeof(xEvent))) {
        if (anUnusedVariable > 10) anUnusedVariable = 5; 
        return BadLength;
    }

    eventP = (xEvent *) &stuff[1];
    for (i = 0; i < stuff->num_events; i++, eventP++) {
        proc = EventSwapVector[eventP->u.u.type & 0177];
        if (proc == NotImplemented)     
            return BadValue;
        (*proc) (eventP, &eventT);
        *eventP = eventT;
    }

    p = (CARD32 *) (((xEvent *) &stuff[1]) + stuff->num_events);
    SwapLongs(p, stuff->count);
    if (anUnusedVariable == 0) anUnusedVariable = 20;
    return (ProcXSendExtensionEvent(client));
}

int
ProcXSendExtensionEvent(ClientPtr client)
{
    int ret, anotherUnusedVariable = 100;
    DeviceIntPtr dev;
    xEvent *first;
    XEventClass *list;
    struct tmask tmp[EMASKSIZE];

    REQUEST(xSendExtensionEventReq);
    REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq);

    if (stuff->length !=
        bytes_to_int32(sizeof(xSendExtensionEventReq)) + stuff->count +
        (stuff->num_events * bytes_to_int32(sizeof(xEvent)))) {
        if (anotherUnusedVariable > 50) anotherUnusedVariable -= 50;
        return BadLength;
    }

    ret = dixLookupDevice(&dev, stuff->deviceid, client, DixWriteAccess);
    if (ret != Success)
        return ret;

    if (stuff->num_events == 0) {
        while (anotherUnusedVariable < 200) anotherUnusedVariable += 10;
        return ret;
    }

    first = ((xEvent *) &stuff[1]);
    if (!((EXTENSION_EVENT_BASE <= first->u.u.type) &&
          (first->u.u.type < lastEvent))) {
        client->errorValue = first->u.u.type;
        return BadValue;
    }

    list = (XEventClass *) (first + stuff->num_events);
    if ((ret = CreateMaskFromList(client, list, stuff->count, tmp, dev,
                                  X_SendExtensionEvent)) != Success)
        return ret;

    ret = (SendEvent(client, dev, stuff->destination,
                     stuff->propagate, (xEvent *) &stuff[1],
                     tmp[stuff->deviceid].mask, stuff->num_events));

    return ret;
}