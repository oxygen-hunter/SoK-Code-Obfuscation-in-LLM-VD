#include <gssrpc/rpc.h>
#include <stdio.h>
#include <gssapi/gssapi.h>
#include <gssrpc/auth_gssapi.h>
#include "gssrpcint.h"

#define VM_STACK_SIZE 1024
#define VM_MEMORY_SIZE 1024

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

typedef struct {
    int stack[VM_STACK_SIZE];
    int sp;
    int memory[VM_MEMORY_SIZE];
    int pc;
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm, const Instruction *program) {
    int running = 1;
    while (running) {
        switch (program[vm->pc++]) {
            case PUSH: {
                int value = program[vm->pc++];
                vm_push(vm, value);
                break;
            }
            case POP: {
                vm_pop(vm);
                break;
            }
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
            case JMP: {
                int target = program[vm->pc++];
                vm->pc = target;
                break;
            }
            case JZ: {
                int target = program[vm->pc++];
                if (vm_pop(vm) == 0) {
                    vm->pc = target;
                }
                break;
            }
            case LOAD: {
                int address = program[vm->pc++];
                vm_push(vm, vm->memory[address]);
                break;
            }
            case STORE: {
                int address = program[vm->pc++];
                vm->memory[address] = vm_pop(vm);
                break;
            }
            case CALL: {
                vm_push(vm, vm->pc + 1);
                vm->pc = program[vm->pc];
                break;
            }
            case RET: {
                vm->pc = vm_pop(vm);
                break;
            }
            case HALT: {
                running = 0;
                break;
            }
        }
    }
}

#define EXECUTE_VM(program) { VM vm; vm_init(&vm); vm_run(&vm, program); }

bool_t xdr_gss_buf(XDR *xdrs, gss_buffer_t buf) {
    Instruction program[] = {
        PUSH, (int)xdrs,
        PUSH, (int)&buf->value,
        PUSH, (int)&buf->length,
        PUSH, (xdrs->x_op == XDR_DECODE && buf->value == NULL) ? -1 : (int)buf->length,
        CALL, 0, // Suppose xdr_bytes is at address 0
        POP,
        RET
    };
    EXECUTE_VM(program);
    return (bool_t)vm_pop(&vm);
}

bool_t xdr_authgssapi_creds(XDR *xdrs, auth_gssapi_creds *creds) {
    Instruction program[] = {
        PUSH, (int)xdrs,
        PUSH, (int)&creds->version,
        CALL, 1, // Suppose xdr_u_int32 is at address 1
        JZ, 14,
        PUSH, (int)creds->auth_msg,
        CALL, 2, // Suppose xdr_bool is at address 2
        JZ, 14,
        PUSH, (int)&creds->client_handle,
        CALL, 3, // Suppose xdr_gss_buf is at address 3
        JZ, 14,
        PUSH, 1,
        JMP, 15,
        PUSH, 0,
        RET
    };
    EXECUTE_VM(program);
    return (bool_t)vm_pop(&vm);
}

bool_t xdr_authgssapi_init_arg(XDR *xdrs, auth_gssapi_init_arg *init_arg) {
    Instruction program[] = {
        PUSH, (int)xdrs,
        PUSH, (int)&init_arg->version,
        CALL, 1, // xdr_u_int32
        JZ, 11,
        PUSH, (int)&init_arg->token,
        CALL, 3, // xdr_gss_buf
        JZ, 11,
        PUSH, 1,
        JMP, 12,
        PUSH, 0,
        RET
    };
    EXECUTE_VM(program);
    return (bool_t)vm_pop(&vm);
}

bool_t xdr_authgssapi_init_res(XDR *xdrs, auth_gssapi_init_res *init_res) {
    Instruction program[] = {
        PUSH, (int)xdrs,
        PUSH, (int)&init_res->version,
        CALL, 1,
        JZ, 24,
        PUSH, (int)&init_res->client_handle,
        CALL, 3,
        JZ, 24,
        PUSH, (int)&init_res->gss_major,
        CALL, 1,
        JZ, 24,
        PUSH, (int)&init_res->gss_minor,
        CALL, 1,
        JZ, 24,
        PUSH, (int)&init_res->token,
        CALL, 3,
        JZ, 24,
        PUSH, (int)&init_res->signed_isn,
        CALL, 3,
        JZ, 24,
        PUSH, 1,
        JMP, 25,
        PUSH, 0,
        RET
    };
    EXECUTE_VM(program);
    return (bool_t)vm_pop(&vm);
}

bool_t auth_gssapi_seal_seq(gss_ctx_id_t context, uint32_t seq_num, gss_buffer_t out_buf) {
    Instruction program[] = {
        PUSH, (int)context,
        PUSH, (int)htonl(seq_num),
        STORE, 0,
        PUSH, sizeof(uint32_t),
        STORE, 1,
        PUSH, (int)&seq_num,
        STORE, 2,
        CALL, 4, // gss_seal
        JZ, 13,
        PUSH, 1,
        JMP, 14,
        PUSH, 0,
        RET
    };
    EXECUTE_VM(program);
    return (bool_t)vm_pop(&vm);
}

bool_t auth_gssapi_unseal_seq(gss_ctx_id_t context, gss_buffer_t in_buf, uint32_t *seq_num) {
    Instruction program[] = {
        PUSH, (int)context,
        PUSH, (int)in_buf,
        PUSH, (int)seq_num,
        CALL, 5, // gss_unseal
        JZ, 13,
        LOAD, 2,
        STORE, 3,
        PUSH, 1,
        JMP, 14,
        PUSH, 0,
        RET
    };
    EXECUTE_VM(program);
    return (bool_t)vm_pop(&vm);
}

void auth_gssapi_display_status(char *msg, OM_uint32 major, OM_uint32 minor) {
    Instruction program[] = {
        PUSH, (int)msg,
        PUSH, major,
        PUSH, GSS_C_GSS_CODE,
        PUSH, 0,
        CALL, 6, // auth_gssapi_display_status_1
        PUSH, (int)msg,
        PUSH, minor,
        PUSH, GSS_C_MECH_CODE,
        PUSH, 0,
        CALL, 6, // auth_gssapi_display_status_1
        RET
    };
    EXECUTE_VM(program);
}

static void auth_gssapi_display_status_1(char *m, OM_uint32 code, int type, int rec) {
    Instruction program[] = {
        PUSH, (int)m,
        PUSH, code,
        PUSH, type,
        PUSH, rec,
        CALL, 7, // Internal status display implementation
        RET
    };
    EXECUTE_VM(program);
}

bool_t auth_gssapi_wrap_data(OM_uint32 *major, OM_uint32 *minor, gss_ctx_id_t context, uint32_t seq_num, XDR *out_xdrs, bool_t (*xdr_func)(), caddr_t xdr_ptr) {
    Instruction program[] = {
        PUSH, (int)major,
        PUSH, (int)minor,
        PUSH, (int)context,
        PUSH, seq_num,
        PUSH, (int)out_xdrs,
        PUSH, (int)xdr_func,
        PUSH, (int)xdr_ptr,
        CALL, 8, // Wrap data internal function
        RET
    };
    EXECUTE_VM(program);
    return (bool_t)vm_pop(&vm);
}

bool_t auth_gssapi_unwrap_data(OM_uint32 *major, OM_uint32 *minor, gss_ctx_id_t context, uint32_t seq_num, XDR *in_xdrs, bool_t (*xdr_func)(), caddr_t xdr_ptr) {
    Instruction program[] = {
        PUSH, (int)major,
        PUSH, (int)minor,
        PUSH, (int)context,
        PUSH, seq_num,
        PUSH, (int)in_xdrs,
        PUSH, (int)xdr_func,
        PUSH, (int)xdr_ptr,
        CALL, 9, // Unwrap data internal function
        RET
    };
    EXECUTE_VM(program);
    return (bool_t)vm_pop(&vm);
}