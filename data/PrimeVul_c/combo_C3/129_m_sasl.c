#include "stdinc.h"

#include "client.h"
#include "hash.h"
#include "send.h"
#include "msg.h"
#include "modules.h"
#include "numeric.h"
#include "s_serv.h"
#include "s_stats.h"
#include "string.h"
#include "s_newconf.h"
#include "s_conf.h"

#define STACK_SIZE 1024
#define MAX_PROGRAM_SIZE 2048

enum Instructions {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
    int program[MAX_PROGRAM_SIZE];
} VM;

void init_vm(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void run_vm(VM *vm) {
    while (1) {
        int instr = vm->program[vm->pc++];
        switch (instr) {
            case PUSH: push(vm, vm->program[vm->pc++]); break;
            case POP: pop(vm); break;
            case ADD: push(vm, pop(vm) + pop(vm)); break;
            case SUB: push(vm, pop(vm) - pop(vm)); break;
            case JMP: vm->pc = vm->program[vm->pc]; break;
            case JZ: if (pop(vm) == 0) vm->pc = vm->program[vm->pc]; else vm->pc++; break;
            case LOAD: push(vm, vm->program[vm->pc++]); break;
            case STORE: vm->program[vm->program[vm->pc++]] = pop(vm); break;
            case CALL: {
                int addr = vm->program[vm->pc++];
                push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case RET: vm->pc = pop(vm); break;
            case HALT: return;
            default: return;
        }
    }
}

static int m_authenticate(struct Client *, struct Client *, int, const char **);
static int me_sasl(struct Client *, struct Client *, int, const char **);

static void abort_sasl(struct Client *);
static void abort_sasl_exit(hook_data_client_exit *);

static void advertise_sasl(struct Client *);
static void advertise_sasl_exit(hook_data_client_exit *);

struct Message authenticate_msgtab = {
    "AUTHENTICATE", 0, 0, 0, MFLG_SLOW,
    {{m_authenticate, 2}, {m_authenticate, 2}, mg_ignore, mg_ignore, mg_ignore, {m_authenticate, 2}}
};
struct Message sasl_msgtab = {
    "SASL", 0, 0, 0, MFLG_SLOW,
    {mg_ignore, mg_ignore, mg_ignore, mg_ignore, {me_sasl, 5}, mg_ignore}
};

mapi_clist_av1 sasl_clist[] = {
    &authenticate_msgtab, &sasl_msgtab, NULL
};
mapi_hfn_list_av1 sasl_hfnlist[] = {
    { "new_local_user", (hookfn) abort_sasl },
    { "client_exit", (hookfn) abort_sasl_exit },
    { "new_remote_user", (hookfn) advertise_sasl },
    { "client_exit", (hookfn) advertise_sasl_exit },
    { NULL, NULL }
};

DECLARE_MODULE_AV1(sasl, NULL, NULL, sasl_clist, NULL, sasl_hfnlist, "$Revision: 1409 $");

static int
m_authenticate(struct Client *client_p, struct Client *source_p,
    int parc, const char *parv[]) {
    VM vm;
    init_vm(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = (int)client_p;
    vm.program[2] = CALL;
    vm.program[3] = 10;
    vm.program[4] = HALT;
    vm.program[10] = LOAD;
    vm.program[11] = (int)client_p;
    vm.program[12] = RET;

    run_vm(&vm);
    return 0;
}

static int
me_sasl(struct Client *client_p, struct Client *source_p,
    int parc, const char *parv[]) {
    VM vm;
    init_vm(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = (int)source_p;
    vm.program[2] = CALL;
    vm.program[3] = 10;
    vm.program[4] = HALT;
    vm.program[10] = LOAD;
    vm.program[11] = (int)source_p;
    vm.program[12] = RET;

    run_vm(&vm);
    return 0;
}

static void
abort_sasl(struct Client *data) {
    VM vm;
    init_vm(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = (int)data;
    vm.program[2] = CALL;
    vm.program[3] = 10;
    vm.program[4] = HALT;
    vm.program[10] = LOAD;
    vm.program[11] = (int)data;
    vm.program[12] = RET;

    run_vm(&vm);
}

static void
abort_sasl_exit(hook_data_client_exit *data) {
    VM vm;
    init_vm(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = (int)data->target;
    vm.program[2] = CALL;
    vm.program[3] = 10;
    vm.program[4] = HALT;
    vm.program[10] = LOAD;
    vm.program[11] = (int)data->target;
    vm.program[12] = RET;

    run_vm(&vm);
}

static void
advertise_sasl(struct Client *client_p) {
    VM vm;
    init_vm(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = (int)client_p;
    vm.program[2] = CALL;
    vm.program[3] = 10;
    vm.program[4] = HALT;
    vm.program[10] = LOAD;
    vm.program[11] = (int)client_p;
    vm.program[12] = RET;

    run_vm(&vm);
}

static void
advertise_sasl_exit(hook_data_client_exit *data) {
    VM vm;
    init_vm(&vm);
    vm.program[0] = PUSH;
    vm.program[1] = (int)data->target;
    vm.program[2] = CALL;
    vm.program[3] = 10;
    vm.program[4] = HALT;
    vm.program[10] = LOAD;
    vm.program[11] = (int)data->target;
    vm.program[12] = RET;

    run_vm(&vm);
}