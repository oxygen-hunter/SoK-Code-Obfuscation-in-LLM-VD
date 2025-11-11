#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/usb.h"
#include "qemu/iov.h"
#include "trace.h"

static void usb_combined_packet_add(USBCombinedPacket *combined, USBPacket *p)
{
    qemu_iovec_concat(&combined->iov, &p->iov, 0, p->iov.size);
    QTAILQ_INSERT_TAIL(&combined->packets, p, combined_entry);
    p->combined = combined;
}

static void usb_combined_packet_remove(USBCombinedPacket *combined,
                                       USBPacket *p)
{
    int dispatcher = 0;
    while (1) {
        switch (dispatcher) {
            case 0:
                assert(p->combined == combined);
                p->combined = NULL;
                QTAILQ_REMOVE(&combined->packets, p, combined_entry);
                dispatcher = 1;
                break;
            case 1:
                if (QTAILQ_EMPTY(&combined->packets)) {
                    qemu_iovec_destroy(&combined->iov);
                    g_free(combined);
                }
                return;
        }
    }
}

void usb_combined_input_packet_complete(USBDevice *dev, USBPacket *p)
{
    USBCombinedPacket *combined = p->combined;
    USBEndpoint *ep = p->ep;
    USBPacket *next;
    int status, actual_length;
    bool short_not_ok, done = false;
    int dispatcher = 0;

    while (1) {
        switch (dispatcher) {
            case 0:
                if (combined == NULL) {
                    usb_packet_complete_one(dev, p);
                    dispatcher = 5;
                    break;
                }
                dispatcher = 1;
                break;
            case 1:
                assert(combined->first == p && p == QTAILQ_FIRST(&combined->packets));
                status = combined->first->status;
                actual_length = combined->first->actual_length;
                short_not_ok = QTAILQ_LAST(&combined->packets)->short_not_ok;
                dispatcher = 2;
                break;
            case 2:
                QTAILQ_FOREACH_SAFE(p, &combined->packets, combined_entry, next) {
                    if (!done) {
                        if (actual_length >= p->iov.size) {
                            p->actual_length = p->iov.size;
                        } else {
                            p->actual_length = actual_length;
                            done = true;
                        }
                        if (done || next == NULL) {
                            p->status = status;
                        } else {
                            p->status = USB_RET_SUCCESS;
                        }
                        p->short_not_ok = short_not_ok;
                        usb_combined_packet_remove(combined, p);
                        usb_packet_complete_one(dev, p);
                        actual_length -= p->actual_length;
                    } else {
                        p->status = USB_RET_REMOVE_FROM_QUEUE;
                        dev->port->ops->complete(dev->port, p);
                    }
                }
                dispatcher = 5;
                break;
            case 5:
                usb_ep_combine_input_packets(ep);
                return;
        }
    }
}

void usb_combined_packet_cancel(USBDevice *dev, USBPacket *p)
{
    USBCombinedPacket *combined = p->combined;
    assert(combined != NULL);
    USBPacket *first = p->combined->first;
    int dispatcher = 0;

    while (1) {
        switch (dispatcher) {
            case 0:
                usb_combined_packet_remove(combined, p);
                dispatcher = 1;
                break;
            case 1:
                if (p == first) {
                    usb_device_cancel_packet(dev, p);
                }
                return;
        }
    }
}

void usb_ep_combine_input_packets(USBEndpoint *ep)
{
    USBPacket *p, *u, *next, *prev = NULL, *first = NULL;
    USBPort *port = ep->dev->port;
    int totalsize;
    int dispatcher = 0;

    while (1) {
        switch (dispatcher) {
            case 0:
                assert(ep->pipeline);
                assert(ep->pid == USB_TOKEN_IN);
                dispatcher = 1;
                break;
            case 1:
                QTAILQ_FOREACH_SAFE(p, &ep->queue, queue, next) {
                    if (ep->halted) {
                        p->status = USB_RET_REMOVE_FROM_QUEUE;
                        port->ops->complete(port, p);
                        continue;
                    }
                    if (p->state == USB_PACKET_ASYNC) {
                        prev = p;
                        continue;
                    }
                    usb_packet_check_state(p, USB_PACKET_QUEUED);
                    if (prev && prev->short_not_ok) {
                        break;
                    }
                    if (first) {
                        if (first->combined == NULL) {
                            USBCombinedPacket *combined = g_new0(USBCombinedPacket, 1);
                            combined->first = first;
                            QTAILQ_INIT(&combined->packets);
                            qemu_iovec_init(&combined->iov, 2);
                            usb_combined_packet_add(combined, first);
                        }
                        usb_combined_packet_add(first->combined, p);
                    } else {
                        first = p;
                    }
                    totalsize = (p->combined) ? p->combined->iov.size : p->iov.size;
                    if ((p->iov.size % ep->max_packet_size) != 0 || !p->short_not_ok ||
                            next == NULL ||
                            (totalsize == (16 * KiB - 36) && p->int_req)) {
                        usb_device_handle_data(ep->dev, first);
                        assert(first->status == USB_RET_ASYNC);
                        if (first->combined) {
                            QTAILQ_FOREACH(u, &first->combined->packets, combined_entry) {
                                usb_packet_set_state(u, USB_PACKET_ASYNC);
                            }
                        } else {
                            usb_packet_set_state(first, USB_PACKET_ASYNC);
                        }
                        first = NULL;
                        prev = p;
                    }
                }
                return;
        }
    }
}