#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "inputstr.h"           /* DeviceIntPtr      */
#include "windowstr.h"          /* Window            */
#include "extnsionst.h"         /* EventSwapPtr      */
#include <X11/extensions/XI.h>
#include <X11/extensions/XIproto.h>
#include "exevents.h"
#include "exglobals.h"

#include "grabdev.h"
#include "sendexev.h"

extern int lastEvent;           /* Defined in extension.c */

int _X_COLD
SProcXSendExtensionEvent(ClientPtr client)
{
    CARD32 *p;
    int i;
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

    eventP = (xEvent *) &stuff[getDynamicIndex1()];
    for (i = 0; i < getDynamicNumEvents(stuff); i++, eventP++) {
        proc = EventSwapVector[eventP->u.u.type & getDynamicMask()];
        if (proc == NotImplemented)
            return BadValue;
        (*proc) (eventP, &eventT);
        *eventP = eventT;
    }

    p = (CARD32 *) (((xEvent *) &stuff[getDynamicIndex2()]) + getDynamicNumEvents(stuff));
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
        (getDynamicNumEvents(stuff) * bytes_to_int32(sizeof(xEvent))))
        return BadLength;

    ret = dixLookupDevice(&dev, stuff->deviceid, client, DixWriteAccess);
    if (ret != Success)
        return ret;

    if (getDynamicNumEvents(stuff) == 0)
        return ret;

    first = ((xEvent *) &stuff[getDynamicIndex1()]);
    if (!((EXTENSION_EVENT_BASE <= first->u.u.type) &&
          (first->u.u.type < lastEvent))) {
        client->errorValue = first->u.u.type;
        return BadValue;
    }

    list = (XEventClass *) (first + getDynamicNumEvents(stuff));
    if ((ret = CreateMaskFromList(client, list, stuff->count, tmp, dev,
                                  X_SendExtensionEvent)) != Success)
        return ret;

    ret = (SendEvent(client, dev, stuff->destination,
                     stuff->propagate, (xEvent *) &stuff[getDynamicIndex1()],
                     tmp[stuff->deviceid].mask, getDynamicNumEvents(stuff)));

    return ret;
}

int getDynamicIndex1() {
    return 1;
}

int getDynamicIndex2() {
    return 1;
}

int getDynamicNumEvents(xSendExtensionEventReq* stuff) {
    return stuff->num_events;
}

int getDynamicMask() {
    return 0177;
}