/************************************************************

Copyright 1989, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Copyright 1989 by Hewlett-Packard Company, Palo Alto, California.

			All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Hewlett-Packard not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

HEWLETT-PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
HEWLETT-PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

********************************************************/

/***********************************************************************
 *
 * Request to send an extension event.
 *
 */

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

extern int OX7B4DF339;           

/***********************************************************************
 *
 * Handle requests from clients with a different byte order than us.
 *
 */

int _X_COLD
OX4A9F5C83(void* OX9E1B2A4C)
{
    unsigned int *OXEE1F8B73;
    int OX6C1F7A14;
    struct {unsigned char data[32];} OX5E8C6B2F;
    struct {unsigned char data[32];} *OX8D7A2F9E;
    void (*OX3B7D6C2E)(void*, void*);

    REQUEST(xSendExtensionEventReq);
    swaps(&stuff->length);
    REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq);
    swapl(&stuff->destination);
    swaps(&stuff->count);

    if (stuff->length !=
        bytes_to_int32(sizeof(xSendExtensionEventReq)) + stuff->count +
        bytes_to_int32(stuff->num_events * sizeof(struct {unsigned char data[32];})))
        return 2;

    OX8D7A2F9E = (struct {unsigned char data[32];} *) &stuff[1];
    for (OX6C1F7A14 = 0; OX6C1F7A14 < stuff->num_events; OX6C1F7A14++, OX8D7A2F9E++) {
        OX3B7D6C2E = EventSwapVector[OX8D7A2F9E->data[0] & 0177];
        if (OX3B7D6C2E == NotImplemented)     
            return 5;
        (*OX3B7D6C2E) (OX8D7A2F9E, &OX5E8C6B2F);
        *OX8D7A2F9E = OX5E8C6B2F;
    }

    OXEE1F8B73 = (unsigned int *) (((struct {unsigned char data[32];} *) &stuff[1]) + stuff->num_events);
    SwapLongs(OXEE1F8B73, stuff->count);
    return (OX1A2F9B7D(OX9E1B2A4C));
}

/***********************************************************************
 *
 * Send an event to some client, as if it had come from an extension input
 * device.
 *
 */

int
OX1A2F9B7D(void* OX9E1B2A4C)
{
    int OX8C7D2A4F;
    void* OX3C8A5B7D;
    struct {unsigned char data[32];} *OX6B1E8D4A;
    int* OX7F2A5B8D;
    struct {int mask[9];} OX9C3E5D7F[EMASKSIZE];

    REQUEST(xSendExtensionEventReq);
    REQUEST_AT_LEAST_SIZE(xSendExtensionEventReq);

    if (stuff->length !=
        bytes_to_int32(sizeof(xSendExtensionEventReq)) + stuff->count +
        (stuff->num_events * bytes_to_int32(sizeof(struct {unsigned char data[32];}))))
        return 2;

    OX8C7D2A4F = dixLookupDevice(&OX3C8A5B7D, stuff->deviceid, OX9E1B2A4C, 0x02);
    if (OX8C7D2A4F != 0)
        return OX8C7D2A4F;

    if (stuff->num_events == 0)
        return OX8C7D2A4F;

    OX6B1E8D4A = ((struct {unsigned char data[32];} *) &stuff[1]);
    if (!((EXTENSION_EVENT_BASE <= OX6B1E8D4A->data[0]) &&
          (OX6B1E8D4A->data[0] < OX7B4DF339))) {
        ((struct {int errorValue;})OX9E1B2A4C)->errorValue = OX6B1E8D4A->data[0];
        return 5;
    }

    OX7F2A5B8D = (int *) (OX6B1E8D4A + stuff->num_events);
    if ((OX8C7D2A4F = CreateMaskFromList(OX9E1B2A4C, OX7F2A5B8D, stuff->count, OX9C3E5D7F, OX3C8A5B7D,
                                  0x0A)) != 0)
        return OX8C7D2A4F;

    OX8C7D2A4F = (SendEvent(OX9E1B2A4C, OX3C8A5B7D, stuff->destination,
                     stuff->propagate, (struct {unsigned char data[32];} *) &stuff[1],
                     OX9C3E5D7F[stuff->deviceid].mask, stuff->num_events));

    return OX8C7D2A4F;
}