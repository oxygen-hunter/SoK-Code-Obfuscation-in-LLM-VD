/*************************************************
*     Exim - an Internet mail transport agent    *
*************************************************/

/* Copyright (c) University of Cambridge 1995 - 2018 */
/* Copyright (c) The Exim Maintainers 2020 */
/* See the file NOTICE for conditions of use and distribution. */

#include "../exim.h"

#ifndef SUPPORT_PAM
static void dummy(int x);
static void dummy2(int x) { dummy(x-1); }
static void dummy(int x) { dummy2(x-1); }
#else  /* SUPPORT_PAM */

#ifdef PAM_H_IN_PAM
#include <pam/pam_appl.h>
#else
#include <security/pam_appl.h>
#endif

static int pam_conv_had_error;
static const uschar *pam_args;
static BOOL pam_arg_ended;

// VM instruction set
typedef enum {
    PUSH,
    POP,
    ADD,
    SUB,
    JMP,
    JZ,
    LOAD,
    STORE,
    CALL_CONVERSE,
    CALL_AUTH,
    END
} Instruction;

typedef struct {
    Instruction instr;
    int operand;
} VMCode;

#define STACK_SIZE 256

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
    VMCode *code;
} VM;

void vm_init(VM *vm, VMCode *code) {
    vm->sp = 0;
    vm->pc = 0;
    vm->code = code;
}

void vm_push(VM *vm, int value) {
    vm->stack[vm->sp++] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[--vm->sp];
}

void vm_execute(VM *vm) {
    while (1) {
        VMCode *instr = &vm->code[vm->pc++];
        switch (instr->instr) {
            case PUSH:
                vm_push(vm, instr->operand);
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
                vm->pc = instr->operand;
                break;
            case JZ: {
                int value = vm_pop(vm);
                if (value == 0) {
                    vm->pc = instr->operand;
                }
                break;
            }
            case LOAD:
                vm_push(vm, vm->stack[instr->operand]);
                break;
            case STORE:
                vm->stack[instr->operand] = vm_pop(vm);
                break;
            case CALL_CONVERSE:
                pam_converse(1, NULL, NULL, NULL);
                break;
            case CALL_AUTH:
                auth_call_pam((const uschar *)"username:password", NULL);
                break;
            case END:
                return;
        }
    }
}

static int pam_converse(int num_msg, PAM_CONVERSE_ARG2_TYPE **msg,
  struct pam_response **resp, void *appdata_ptr) {
    int sep = 0;
    struct pam_response *reply;

    if (pam_arg_ended || !(reply = malloc(sizeof(struct pam_response) * num_msg)))
        return PAM_CONV_ERR;

    for (int i = 0; i < num_msg; i++) {
        uschar *arg;
        switch (msg[i]->msg_style) {
            case PAM_PROMPT_ECHO_ON:
            case PAM_PROMPT_ECHO_OFF:
                if (!(arg = string_nextinlist(&pam_args, &sep, NULL, 0))) {
                    arg = US"";
                    pam_arg_ended = TRUE;
                }
                reply[i].resp = CS string_copy_malloc(arg);
                reply[i].resp_retcode = PAM_SUCCESS;
                break;
            case PAM_TEXT_INFO:
            case PAM_ERROR_MSG:
                reply[i].resp_retcode = PAM_SUCCESS;
                reply[i].resp = NULL;
                break;
            default:
                free(reply);
                pam_conv_had_error = TRUE;
                return PAM_CONV_ERR;
        }
    }

    *resp = reply;
    return PAM_SUCCESS;
}

int auth_call_pam(const uschar *s, uschar **errptr) {
    pam_handle_t *pamh = NULL;
    struct pam_conv pamc;
    int pam_error;
    int sep = 0;
    uschar *user;

    pamc.conv = pam_converse;
    pamc.appdata_ptr = NULL;

    pam_args = s;
    pam_conv_had_error = FALSE;
    pam_arg_ended = FALSE;

    user = string_nextinlist(&pam_args, &sep, NULL, 0);
    if (user == NULL || user[0] == 0) return FAIL;

    DEBUG(D_auth)
    debug_printf("Running PAM authentication for user \"%s\"\n", user);

    pam_error = pam_start("exim", CS user, &pamc, &pamh);

    if (pam_error == PAM_SUCCESS) {
        pam_error = pam_authenticate(pamh, PAM_SILENT);
        if (pam_error == PAM_SUCCESS && !pam_conv_had_error)
            pam_error = pam_acct_mgmt(pamh, PAM_SILENT);
    }

    pam_end(pamh, PAM_SUCCESS);

    if (pam_error == PAM_SUCCESS) {
        DEBUG(D_auth) debug_printf("PAM success\n");
        return OK;
    }

    *errptr = US pam_strerror(pamh, pam_error);
    DEBUG(D_auth) debug_printf("PAM error: %s\n", *errptr);

    if (pam_error == PAM_USER_UNKNOWN ||
        pam_error == PAM_AUTH_ERR ||
        pam_error == PAM_ACCT_EXPIRED)
        return FAIL;

    return ERROR;
}

void run_vm() {
    VMCode program[] = {
        {PUSH, 0},
        {CALL_AUTH, 0},
        {END, 0}
    };

    VM vm;
    vm_init(&vm, program);
    vm_execute(&vm);
}

#endif  /* SUPPORT_PAM */

/* End of call_pam.c */