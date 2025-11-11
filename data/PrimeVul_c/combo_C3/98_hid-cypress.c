#include <linux/device.h>
#include <linux/hid.h>
#include <linux/input.h>
#include <linux/module.h>

#include "hid-ids.h"

#define CP_RDESC_SWAPPED_MIN_MAX 0x01
#define CP_2WHEEL_MOUSE_HACK 0x02
#define CP_2WHEEL_MOUSE_HACK_ON 0x04

enum {
    VM_PUSH, VM_POP, VM_ADD, VM_SUB, VM_JMP, VM_JZ, VM_LOAD, VM_STORE, VM_HALT
};

typedef struct {
    int pc;
    int stack[256];
    int sp;
    unsigned long memory[256];
    int halted;
} VM;

void vm_execute(VM *vm, int *program) {
    while (!vm->halted) {
        int opcode = program[vm->pc++];
        switch (opcode) {
            case VM_PUSH:
                vm->stack[vm->sp++] = program[vm->pc++];
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
                vm->pc = program[vm->pc];
                break;
            case VM_JZ:
                if (vm->stack[--vm->sp] == 0)
                    vm->pc = program[vm->pc];
                else
                    vm->pc++;
                break;
            case VM_LOAD:
                vm->stack[vm->sp++] = vm->memory[program[vm->pc++]];
                break;
            case VM_STORE:
                vm->memory[program[vm->pc++]] = vm->stack[--vm->sp];
                break;
            case VM_HALT:
                vm->halted = 1;
                break;
            default:
                break;
        }
    }
}

__u8 *cp_report_fixup_vm(struct hid_device *hdev, __u8 *rdesc, unsigned int *rsize) {
    unsigned long quirks = (unsigned long)hid_get_drvdata(hdev);
    VM vm = {0};
    int program[] = {
        VM_LOAD, 0,
        VM_PUSH, CP_RDESC_SWAPPED_MIN_MAX,
        VM_AND,
        VM_JZ, 11,
        VM_PUSH, 0,
        VM_LOOP_START, VM_PUSH, 4, VM_SUB, 
        VM_JZ, 11,
        VM_LOAD, 0, VM_PUSH, 0x29, VM_EQ, 
        VM_LOAD, 2, VM_PUSH, 0x19, VM_EQ, 
        VM_AND,
        VM_JZ, 9,
        VM_STORE, 0, VM_PUSH, 0x19,
        VM_STORE, 2, VM_PUSH, 0x29,
        VM_SWAP, 3, 1,
        VM_ADD, 1,
        VM_JMP, 3,
        VM_HALT
    };
    vm_execute(&vm, program);
    return rdesc;
}

int cp_input_mapped_vm(struct hid_device *hdev, struct hid_input *hi, struct hid_field *field,
                       struct hid_usage *usage, unsigned long **bit, int *max) {
    unsigned long quirks = (unsigned long)hid_get_drvdata(hdev);
    if (!(quirks & CP_2WHEEL_MOUSE_HACK))
        return 0;
    if (usage->type == EV_REL && usage->code == REL_WHEEL)
        set_bit(REL_HWHEEL, *bit);
    if (usage->hid == 0x00090005)
        return -1;
    return 0;
}

int cp_event_vm(struct hid_device *hdev, struct hid_field *field, struct hid_usage *usage, __s32 value) {
    unsigned long quirks = (unsigned long)hid_get_drvdata(hdev);
    if (!(hdev->claimed & HID_CLAIMED_INPUT) || !field->hidinput || !usage->type || !(quirks & CP_2WHEEL_MOUSE_HACK))
        return 0;
    if (usage->hid == 0x00090005) {
        if (value)
            quirks |= CP_2WHEEL_MOUSE_HACK_ON;
        else
            quirks &= ~CP_2WHEEL_MOUSE_HACK_ON;
        hid_set_drvdata(hdev, (void *)quirks);
        return 1;
    }
    if (usage->code == REL_WHEEL && (quirks & CP_2WHEEL_MOUSE_HACK_ON)) {
        struct input_dev *input = field->hidinput->input;
        input_event(input, usage->type, REL_HWHEEL, value);
        return 1;
    }
    return 0;
}

int cp_probe_vm(struct hid_device *hdev, const struct hid_device_id *id) {
    unsigned long quirks = id->driver_data;
    int ret;
    hid_set_drvdata(hdev, (void *)quirks);
    ret = hid_parse(hdev);
    if (ret) {
        hid_err(hdev, "parse failed\n");
        return ret;
    }
    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret) {
        hid_err(hdev, "hw start failed\n");
        return ret;
    }
    return 0;
}

static const struct hid_device_id cp_devices[] = {
    { HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_1), .driver_data = CP_RDESC_SWAPPED_MIN_MAX },
    { HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_2), .driver_data = CP_RDESC_SWAPPED_MIN_MAX },
    { HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_3), .driver_data = CP_RDESC_SWAPPED_MIN_MAX },
    { HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_BARCODE_4), .driver_data = CP_RDESC_SWAPPED_MIN_MAX },
    { HID_USB_DEVICE(USB_VENDOR_ID_CYPRESS, USB_DEVICE_ID_CYPRESS_MOUSE), .driver_data = CP_2WHEEL_MOUSE_HACK },
    {}
};
MODULE_DEVICE_TABLE(hid, cp_devices);

static struct hid_driver cp_driver = {
    .name = "cypress",
    .id_table = cp_devices,
    .report_fixup = cp_report_fixup_vm,
    .input_mapped = cp_input_mapped_vm,
    .event = cp_event_vm,
    .probe = cp_probe_vm,
};
module_hid_driver(cp_driver);

MODULE_LICENSE("GPL");