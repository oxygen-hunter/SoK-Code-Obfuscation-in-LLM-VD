#include "qemu/osdep.h"
#include "qemu/units.h"
#include "hw/usb.h"
#include "qemu/iov.h"
#include "trace.h"

// Define VM instruction set
typedef enum {
    VM_PUSH, VM_POP, VM_ADD, VM_SUB, VM_JMP, VM_JZ, VM_LOAD, VM_STORE, VM_CALL, VM_RETURN
} VMInstruction;

// Define the stack-based VM with a program counter and stack
typedef struct {
    int pc;
    int stack[256];
    int sp;
    int memory[256];
} VM;

// VM interpreter function
void vm_execute(VM *vm, VMInstruction *program) {
    while (1) {
        switch (program[vm->pc]) {
            case VM_PUSH:
                vm->stack[vm->sp++] = program[++vm->pc];
                break;
            case VM_POP:
                vm->sp--;
                break;
            case VM_ADD:
                vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1];
                vm->sp--;
                break;
            case VM_SUB:
                vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1];
                vm->sp--;
                break;
            case VM_JMP:
                vm->pc = program[++vm->pc] - 1;
                break;
            case VM_JZ:
                if (vm->stack[--vm->sp] == 0)
                    vm->pc = program[++vm->pc] - 1;
                else
                    vm->pc++;
                break;
            case VM_LOAD:
                vm->stack[vm->sp++] = vm->memory[program[++vm->pc]];
                break;
            case VM_STORE:
                vm->memory[program[++vm->pc]] = vm->stack[--vm->sp];
                break;
            case VM_CALL:
                vm->memory[program[++vm->pc]] = vm->pc + 1;
                vm->pc = program[++vm->pc] - 1;
                break;
            case VM_RETURN:
                vm->pc = vm->memory[program[++vm->pc]] - 1;
                break;
            default:
                return;
        }
        vm->pc++;
    }
}

// Original functions compiled to VM bytecode
VMInstruction vm_bytecode[] = {
    VM_PUSH, 0, VM_LOAD, 0, VM_LOAD, 1, VM_CALL, 100, 
    VM_RETURN, 0, VM_PUSH, 1, VM_STORE, 2, VM_RETURN, 0, 
    VM_PUSH, 0, VM_PUSH, 1, VM_ADD, VM_STORE, 2, VM_RETURN, 0
};

// Custom stack-based VM for usb_combined_packet_add function
void usb_combined_packet_add(USBCombinedPacket *combined, USBPacket *p) {
    VM vm = {0};
    vm.memory[0] = (int)combined;
    vm.memory[1] = (int)p;
    vm_execute(&vm, vm_bytecode);
    qemu_iovec_concat(&combined->iov, &p->iov, 0, p->iov.size);
    QTAILQ_INSERT_TAIL(&combined->packets, p, combined_entry);
    p->combined = combined;
}

void usb_combined_packet_remove(USBCombinedPacket *combined, USBPacket *p) {
    assert(p->combined == combined);
    p->combined = NULL;
    QTAILQ_REMOVE(&combined->packets, p, combined_entry);
    if (QTAILQ_EMPTY(&combined->packets)) {
        qemu_iovec_destroy(&combined->iov);
        g_free(combined);
    }
}

void usb_combined_input_packet_complete(USBDevice *dev, USBPacket *p) {
    USBCombinedPacket *combined = p->combined;
    USBEndpoint *ep = p->ep;
    USBPacket *next;
    int status, actual_length;
    bool short_not_ok, done = false;

    if (combined == NULL) {
        usb_packet_complete_one(dev, p);
        goto leave;
    }

    assert(combined->first == p && p == QTAILQ_FIRST(&combined->packets));

    status = combined->first->status;
    actual_length = combined->first->actual_length;
    short_not_ok = QTAILQ_LAST(&combined->packets)->short_not_ok;

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
leave:
    usb_ep_combine_input_packets(ep);
}

void usb_combined_packet_cancel(USBDevice *dev, USBPacket *p) {
    USBCombinedPacket *combined = p->combined;
    assert(combined != NULL);
    USBPacket *first = p->combined->first;

    usb_combined_packet_remove(combined, p);
    if (p == first) {
        usb_device_cancel_packet(dev, p);
    }
}

void usb_ep_combine_input_packets(USBEndpoint *ep) {
    USBPacket *p, *u, *next, *prev = NULL, *first = NULL;
    USBPort *port = ep->dev->port;
    int totalsize;

    assert(ep->pipeline);
    assert(ep->pid == USB_TOKEN_IN);

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
}