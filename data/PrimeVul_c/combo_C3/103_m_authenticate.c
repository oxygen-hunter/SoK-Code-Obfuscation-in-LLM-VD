#include "config.h"
#include "client.h"
#include "ircd.h"
#include "ircd_features.h"
#include "ircd_log.h"
#include "ircd_reply.h"
#include "ircd_string.h"
#include "ircd_snprintf.h"
#include "msg.h"
#include "numeric.h"
#include "numnicks.h"
#include "random.h"
#include "send.h"
#include "s_misc.h"
#include "s_user.h"

#define STACK_SIZE 1024
#define PROGRAM_SIZE 1024

enum Instructions {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL
};

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int program[PROGRAM_SIZE];
    int pc;
    struct Client* cptr;
    struct Client* sptr;
    int parc;
    char** parv;
} VM;

void vm_init(VM* vm, struct Client* cptr, struct Client* sptr, int parc, char* parv[]) {
    vm->sp = -1;
    vm->pc = 0;
    vm->cptr = cptr;
    vm->sptr = sptr;
    vm->parc = parc;
    vm->parv = parv;
}

void vm_push(VM* vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM* vm) {
    return vm->stack[vm->sp--];
}

void vm_execute(VM* vm) {
    while (1) {
        switch (vm->program[vm->pc++]) {
            case PUSH:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD:
                vm_push(vm, vm_pop(vm) + vm_pop(vm));
                break;
            case SUB:
                vm_push(vm, vm_pop(vm) - vm_pop(vm));
                break;
            case JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case JZ:
                if (vm_pop(vm) == 0) {
                    vm->pc = vm->program[vm->pc];
                } else {
                    vm->pc++;
                }
                break;
            case LOAD:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case STORE:
                vm->program[vm->program[vm->pc++]] = vm_pop(vm);
                break;
            case CALL:
                if (vm->program[vm->pc] == 1) {
                    vm_push(vm, (int)cli_sockhost(vm->cptr));
                } else if (vm->program[vm->pc] == 2) {
                    vm_push(vm, (int)cli_sock_ip(vm->cptr));
                }
                vm->pc++;
                break;
            default:
                return;
        }
    }
}

int m_authenticate(struct Client* cptr, struct Client* sptr, int parc, char* parv[]) {
    VM vm;
    vm_init(&vm, cptr, sptr, parc, parv);

    int pc = 0;

    vm.program[pc++] = CALL; vm.program[pc++] = 1;
    vm.program[pc++] = CALL; vm.program[pc++] = 2;
    vm.program[pc++] = PUSH; vm.program[pc++] = (int)CapActive(cptr, CAP_SASL);
    vm.program[pc++] = JZ;   vm.program[pc++] = 30;
    vm.program[pc++] = PUSH; vm.program[pc++] = parc;
    vm.program[pc++] = PUSH; vm.program[pc++] = 2;
    vm.program[pc++] = SUB;
    vm.program[pc++] = JZ;   vm.program[pc++] = 60;
    vm.program[pc++] = PUSH; vm.program[pc++] = (int)strlen(parv[1]);
    vm.program[pc++] = PUSH; vm.program[pc++] = 400;
    vm.program[pc++] = SUB;
    vm.program[pc++] = JZ;   vm.program[pc++] = 90;
    vm.program[pc++] = PUSH; vm.program[pc++] = (int)IsSASLComplete(cptr);
    vm.program[pc++] = JZ;   vm.program[pc++] = 120;
    vm.program[pc++] = LOAD; vm.program[pc++] = need_more_params(cptr, "AUTHENTICATE");
    vm.program[pc++] = JMP;  vm.program[pc++] = 999;
    vm.program[pc++] = LOAD; vm.program[pc++] = send_reply(cptr, ERR_SASLTOOLONG);
    vm.program[pc++] = JMP;  vm.program[pc++] = 999;
    vm.program[pc++] = LOAD; vm.program[pc++] = send_reply(cptr, ERR_SASLALREADY);
    vm.program[pc++] = JMP;  vm.program[pc++] = 999;
    vm.program[pc++] = PUSH; vm.program[pc++] = 0;

    vm_execute(&vm);

    return vm_pop(&vm);
}

static void sasl_timeout_callback(struct Event* ev) {
    struct Client *cptr;
    assert(0 != ev_timer(ev));
    assert(0 != t_data(ev_timer(ev)));
    if (ev_type(ev) == ET_EXPIRE) {
        cptr = (struct Client*) t_data(ev_timer(ev));
        abort_sasl(cptr, 1);
    }
}