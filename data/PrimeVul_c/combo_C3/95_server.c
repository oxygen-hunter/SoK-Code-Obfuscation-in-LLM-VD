#include "k5-int.h"
#include "port-sockets.h"
#include "com_err.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

#define STACK_SIZE 256
#define MEM_SIZE 256
#define PROGRAM_SIZE 512

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
    int mem[MEM_SIZE];
    int program[PROGRAM_SIZE];
} VM;

void vm_init(VM* vm) {
    vm->sp = 0;
    vm->pc = 0;
    for (int i = 0; i < MEM_SIZE; i++) vm->mem[i] = 0;
}

void vm_push(VM* vm, int value) {
    vm->stack[vm->sp++] = value;
}

int vm_pop(VM* vm) {
    return vm->stack[--vm->sp];
}

void vm_execute(VM* vm) {
    for (;;) {
        int opcode = vm->program[vm->pc++];
        switch (opcode) {
            case PUSH: {
                int value = vm->program[vm->pc++];
                vm_push(vm, value);
            } break;
            case POP: {
                vm_pop(vm);
            } break;
            case ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
            } break;
            case SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
            } break;
            case JMP: {
                int addr = vm->program[vm->pc++];
                vm->pc = addr;
            } break;
            case JZ: {
                int addr = vm->program[vm->pc++];
                if (vm_pop(vm) == 0) vm->pc = addr;
            } break;
            case LOAD: {
                int addr = vm->program[vm->pc++];
                vm_push(vm, vm->mem[addr]);
            } break;
            case STORE: {
                int addr = vm->program[vm->pc++];
                vm->mem[addr] = vm_pop(vm);
            } break;
            case CALL: {
                int addr = vm->program[vm->pc++];
                vm_push(vm, vm->pc);
                vm->pc = addr;
            } break;
            case RET: {
                vm->pc = vm_pop(vm);
            } break;
            case HALT: {
                return;
            } break;
        }
    }
}

void setup_program(VM* vm) {
    int p[] = {
        /* Initialization */
        PUSH, 0, STORE, 0, // sock = 0
        /* krb5_init_context */
        PUSH, 1, CALL, 100, // krb5_init_context
        JZ, 10, // if retval != 0, goto error
        /* krb5_read_message for pname_data */
        PUSH, 2, CALL, 110, // krb5_read_message
        JZ, 10, // if retval != 0, goto error
        /* krb5_read_message for tkt_data */
        PUSH, 3, CALL, 120, // krb5_read_message
        JZ, 10, // if retval != 0, goto error
        /* krb5_cc_default */
        PUSH, 4, CALL, 130, // krb5_cc_default
        JZ, 10, // if retval != 0, goto error
        /* krb5_cc_get_principal */
        PUSH, 5, CALL, 140, // krb5_cc_get_principal
        JZ, 10, // if retval != 0, goto error
        /* krb5_parse_name */
        PUSH, 6, CALL, 150, // krb5_parse_name
        JZ, 10, // if retval != 0, goto error
        /* krb5_get_credentials */
        PUSH, 7, CALL, 160, // krb5_get_credentials
        JZ, 10, // if retval != 0, goto error
        /* krb5_auth_con_init */
        PUSH, 8, CALL, 170, // krb5_auth_con_init
        JZ, 10, // if retval != 0, goto error
        /* krb5_auth_con_setflags */
        PUSH, 9, CALL, 180, // krb5_auth_con_setflags
        JZ, 10, // if retval != 0, goto error
        /* krb5_auth_con_genaddrs */
        PUSH, 10, CALL, 190, // krb5_auth_con_genaddrs
        JZ, 10, // if retval != 0, goto error
        /* krb5_mk_req_extended */
        PUSH, 11, CALL, 200, // krb5_mk_req_extended
        JZ, 10, // if retval != 0, goto error
        /* krb5_write_message */
        PUSH, 12, CALL, 210, // krb5_write_message
        JZ, 10, // if retval != 0, goto error
        /* krb5_mk_safe */
        PUSH, 13, CALL, 220, // krb5_mk_safe
        JZ, 10, // if retval != 0, goto error
        /* krb5_write_message */
        PUSH, 14, CALL, 230, // krb5_write_message
        JZ, 10, // if retval != 0, goto error
        /* Clean up */
        HALT, // End Program
        
        /* Error handling */
        10, HALT // Error: HALT
    };
    for (int i = 0; i < sizeof(p)/sizeof(p[0]); i++) {
        vm->program[i] = p[i];
    }
}

int main(int argc, char *argv[]) {
    VM vm;
    vm_init(&vm);
    setup_program(&vm);
    vm_execute(&vm);
    return 0;
}