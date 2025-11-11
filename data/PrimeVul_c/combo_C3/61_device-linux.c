#include "config.h"
#include "includes.h"
#include "radvd.h"
#include "defaults.h"
#include "pathnames.h"

#ifndef IPV6_ADDR_LINKLOCAL
#define IPV6_ADDR_LINKLOCAL   0x0020U
#endif

typedef enum {
    VM_NOP,
    VM_PUSH,
    VM_POP,
    VM_ADD,
    VM_SUB,
    VM_JMP,
    VM_JZ,
    VM_LOAD,
    VM_STORE,
    VM_CALL_SETUP_DEVICEINFO,
    VM_CALL_SETUP_LINKLOCAL_ADDR,
    VM_CALL_SETUP_ALLROUTERS_MEMBERSHIP,
    VM_CALL_CHECK_ALLROUTERS_MEMBERSHIP,
    VM_CALL_SET_INTERFACE_VAR,
    VM_CALL_SET_INTERFACE_LINKMTU,
    VM_CALL_SET_INTERFACE_CURHLIM,
    VM_CALL_SET_INTERFACE_REACHTIME,
    VM_CALL_SET_INTERFACE_RETRANSTIMER,
    VM_HALT
} Instruction;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int halted;
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->halted = 0;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_execute(VM *vm, const Instruction *program) {
    while (!vm->halted) {
        switch (program[vm->pc++]) {
            case VM_NOP:
                break;
            case VM_PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case VM_POP:
                vm_pop(vm);
                break;
            case VM_ADD:
                vm_push(vm, vm_pop(vm) + vm_pop(vm));
                break;
            case VM_SUB:
                vm_push(vm, vm_pop(vm) - vm_pop(vm));
                break;
            case VM_JMP:
                vm->pc = program[vm->pc];
                break;
            case VM_JZ: {
                int target = program[vm->pc++];
                if (vm_pop(vm) == 0)
                    vm->pc = target;
                break;
            }
            case VM_LOAD:
            case VM_STORE:
                // Implement as needed
                break;
            case VM_CALL_SETUP_DEVICEINFO:
                setup_deviceinfo((struct Interface *)vm_pop(vm));
                break;
            case VM_CALL_SETUP_LINKLOCAL_ADDR:
                setup_linklocal_addr((struct Interface *)vm_pop(vm));
                break;
            case VM_CALL_SETUP_ALLROUTERS_MEMBERSHIP:
                setup_allrouters_membership((struct Interface *)vm_pop(vm));
                break;
            case VM_CALL_CHECK_ALLROUTERS_MEMBERSHIP:
                check_allrouters_membership((struct Interface *)vm_pop(vm));
                break;
            case VM_CALL_SET_INTERFACE_VAR:
                set_interface_var((const char *)vm_pop(vm), (const char *)vm_pop(vm), (const char *)vm_pop(vm), vm_pop(vm));
                break;
            case VM_CALL_SET_INTERFACE_LINKMTU:
                set_interface_linkmtu((const char *)vm_pop(vm), vm_pop(vm));
                break;
            case VM_CALL_SET_INTERFACE_CURHLIM:
                set_interface_curhlim((const char *)vm_pop(vm), vm_pop(vm));
                break;
            case VM_CALL_SET_INTERFACE_REACHTIME:
                set_interface_reachtime((const char *)vm_pop(vm), vm_pop(vm));
                break;
            case VM_CALL_SET_INTERFACE_RETRANSTIMER:
                set_interface_retranstimer((const char *)vm_pop(vm), vm_pop(vm));
                break;
            case VM_HALT:
                vm->halted = 1;
                break;
            default:
                // Handle unknown instruction
                break;
        }
    }
}

Instruction program[] = {
    VM_PUSH, (int)&iface,
    VM_CALL_SETUP_DEVICEINFO,
    VM_PUSH, (int)&iface,
    VM_CALL_SETUP_LINKLOCAL_ADDR,
    VM_PUSH, (int)&iface,
    VM_CALL_SETUP_ALLROUTERS_MEMBERSHIP,
    VM_PUSH, (int)&iface,
    VM_CALL_CHECK_ALLROUTERS_MEMBERSHIP,
    VM_HALT
};

int main() {
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, program);
    return 0;
}