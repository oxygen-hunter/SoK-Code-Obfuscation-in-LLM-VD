#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/usb.h"
#include "qemu/iov.h"
#include "trace.h"

static void usb_combined_packet_add(USBCombinedPacket *x, USBPacket *y)
{
    qemu_iovec_concat(&x->iov, &y->iov, 0, y->iov.size);
    QTAILQ_INSERT_TAIL(&x->packets, y, combined_entry);
    y->combined = x;
}

static void usb_combined_packet_remove(USBCombinedPacket *x,
                                       USBPacket *y)
{
    assert(y->combined == x);
    y->combined = NULL;
    QTAILQ_REMOVE(&x->packets, y, combined_entry);
    if (QTAILQ_EMPTY(&x->packets)) {
        qemu_iovec_destroy(&x->iov);
        g_free(x);
    }
}

void usb_combined_input_packet_complete(USBDevice *z, USBPacket *y)
{
    USBCombinedPacket *t = y->combined;
    USBEndpoint *u = y->ep;
    USBPacket *w;
    int q, r;
    bool q1, q2 = false;

    if (t == NULL) {
        usb_packet_complete_one(z, y);
        goto leave;
    }

    assert(t->first == y && y == QTAILQ_FIRST(&t->packets));

    q = t->first->status;
    r = t->first->actual_length;
    q1 = QTAILQ_LAST(&t->packets)->short_not_ok;

    QTAILQ_FOREACH_SAFE(y, &t->packets, combined_entry, w) {
        if (!q2) {
            if (r >= y->iov.size) {
                y->actual_length = y->iov.size;
            } else {
                y->actual_length = r;
                q2 = true;
            }
            if (q2 || w == NULL) {
                y->status = q;
            } else {
                y->status = USB_RET_SUCCESS;
            }
            y->short_not_ok = q1;
            usb_combined_packet_remove(t, y);
            usb_packet_complete_one(z, y);
            r -= y->actual_length;
        } else {
            y->status = USB_RET_REMOVE_FROM_QUEUE;
            z->port->ops->complete(z->port, y);
        }
    }
leave:
    usb_ep_combine_input_packets(u);
}

void usb_combined_packet_cancel(USBDevice *z, USBPacket *y)
{
    USBCombinedPacket *t = y->combined;
    assert(t != NULL);
    USBPacket *v = y->combined->first;

    usb_combined_packet_remove(t, y);
    if (y == v) {
        usb_device_cancel_packet(z, y);
    }
}

void usb_ep_combine_input_packets(USBEndpoint *u)
{
    USBPacket *y, *v, *w, *x = NULL, *z = NULL;
    USBPort *p = u->dev->port;
    int q;

    assert(u->pipeline);
    assert(u->pid == USB_TOKEN_IN);

    QTAILQ_FOREACH_SAFE(y, &u->queue, queue, w) {
        if (u->halted) {
            y->status = USB_RET_REMOVE_FROM_QUEUE;
            p->ops->complete(p, y);
            continue;
        }

        if (y->state == USB_PACKET_ASYNC) {
            x = y;
            continue;
        }
        usb_packet_check_state(y, USB_PACKET_QUEUED);

        if (x && x->short_not_ok) {
            break;
        }

        if (z) {
            if (z->combined == NULL) {
                USBCombinedPacket *t = g_new0(USBCombinedPacket, 1);

                t->first = z;
                QTAILQ_INIT(&t->packets);
                qemu_iovec_init(&t->iov, 2);
                usb_combined_packet_add(t, z);
            }
            usb_combined_packet_add(z->combined, y);
        } else {
            z = y;
        }

        q = (y->combined) ? y->combined->iov.size : y->iov.size;
        if ((y->iov.size % u->max_packet_size) != 0 || !y->short_not_ok ||
                w == NULL ||
                (q == (16 * KiB - 36) && y->int_req)) {
            usb_device_handle_data(u->dev, z);
            assert(z->status == USB_RET_ASYNC);
            if (z->combined) {
                QTAILQ_FOREACH(v, &z->combined->packets, combined_entry) {
                    usb_packet_set_state(v, USB_PACKET_ASYNC);
                }
            } else {
                usb_packet_set_state(z, USB_PACKET_ASYNC);
            }
            z = NULL;
            x = y;
        }
    }
}