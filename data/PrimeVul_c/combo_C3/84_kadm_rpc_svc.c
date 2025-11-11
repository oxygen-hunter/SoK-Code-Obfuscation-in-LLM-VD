#include <k5-platform.h>
#include <gssrpc/rpc.h>
#include <gssapi/gssapi_krb5.h>
#include <syslog.h>
#include <kadm5/kadm_rpc.h>
#include <krb5.h>
#include <kadm5/admin.h>
#include <adm_proto.h>
#include "misc.h"
#include "kadm5/server_internal.h"

extern void *global_server_handle;

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
} Opcode;

typedef struct {
    Opcode op;
    int arg;
} Instruction;

typedef struct {
    Instruction instructions[1024];
    int stack[256];
    int pc;
    int sp;
} VM;

void vm_init(VM *vm) {
    vm->pc = 0;
    vm->sp = -1;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm) {
    while (vm->pc < 1024) {
        Instruction instr = vm->instructions[vm->pc++];
        switch (instr.op) {
            case PUSH:
                vm_push(vm, instr.arg);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP:
                vm->pc = instr.arg;
                break;
            case JZ:
                if (vm_pop(vm) == 0)
                    vm->pc = instr.arg;
                break;
            case LOAD:
                vm_push(vm, vm->stack[instr.arg]);
                break;
            case STORE:
                vm->stack[instr.arg] = vm_pop(vm);
                break;
            case CALL:
                vm_push(vm, vm->pc);
                vm->pc = instr.arg;
                break;
            case RET:
                vm->pc = vm_pop(vm);
                break;
        }
    }
}

void kadm_1_vm(struct svc_req *rqstp, register SVCXPRT *transp) {
    VM vm;
    vm_init(&vm);

    vm.instructions[0] = (Instruction){LOAD, 0}; // Load oa_flavor
    vm.instructions[1] = (Instruction){PUSH, AUTH_GSSAPI};
    vm.instructions[2] = (Instruction){SUB, 0};
    vm.instructions[3] = (Instruction){JZ, 10}; // Jump to check_rpcsec_auth
    vm.instructions[4] = (Instruction){CALL, 100}; // Call check_rpcsec_auth
    vm.instructions[5] = (Instruction){JZ, 120}; // Jump to auth error
    vm.instructions[6] = (Instruction){CALL, 200}; // Call process_rpc_request
    vm.instructions[7] = (Instruction){RET, 0};
    vm.instructions[10] = (Instruction){PUSH, 0}; // auth error
    vm.instructions[11] = (Instruction){CALL, 300}; // Call svcerr_weakauth
    vm.instructions[12] = (Instruction){RET, 0};

    vm_run(&vm);
}

int check_rpcsec_auth_vm(struct svc_req *rqstp) {
    VM vm;
    vm_init(&vm);

    vm.instructions[0] = (Instruction){LOAD, 0}; // Load oa_flavor
    vm.instructions[1] = (Instruction){PUSH, RPCSEC_GSS};
    vm.instructions[2] = (Instruction){SUB, 0};
    vm.instructions[3] = (Instruction){JZ, 10}; // Jump if not RPCSEC_GSS
    vm.instructions[4] = (Instruction){PUSH, 0};
    vm.instructions[5] = (Instruction){RET, 0};
    vm.instructions[10] = (Instruction){CALL, 400}; // Call gss_inquire_context
    vm.instructions[11] = (Instruction){CALL, 500}; // Call gss_to_krb5_name_1
    vm.instructions[12] = (Instruction){JZ, 20}; // Jump if failed
    vm.instructions[13] = (Instruction){PUSH, 1}; // success
    vm.instructions[14] = (Instruction){RET, 0};
    vm.instructions[20] = (Instruction){PUSH, 0}; // fail
    vm.instructions[21] = (Instruction){RET, 0};

    vm_run(&vm);

    return vm_pop(&vm);
}