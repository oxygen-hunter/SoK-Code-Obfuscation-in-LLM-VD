#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "MKPlugin.h"
#include "mandril.h"

MONKEY_PLUGIN("mandril", "Mandril", VERSION, MK_PLUGIN_STAGE_10 | MK_PLUGIN_STAGE_30);

static struct mk_config *conf;

#define STACK_SIZE 1024
#define MAX_PROGRAM_SIZE 2048

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT, STRCMP, STRSEARCH
};

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
    int program[MAX_PROGRAM_SIZE];
} VM;

void vm_init(VM *vm, int *program) {
    vm->sp = -1;
    vm->pc = 0;
    memcpy(vm->program, program, MAX_PROGRAM_SIZE * sizeof(int));
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm) {
    int running = 1;
    while (running) {
        switch (vm->program[vm->pc++]) {
            case PUSH: vm_push(vm, vm->program[vm->pc++]); break;
            case POP: vm_pop(vm); break;
            case ADD: vm_push(vm, vm_pop(vm) + vm_pop(vm)); break;
            case SUB: vm_push(vm, vm_pop(vm) - vm_pop(vm)); break;
            case JMP: vm->pc = vm->program[vm->pc]; break;
            case JZ: if (vm_pop(vm) == 0) vm->pc = vm->program[vm->pc]; else vm->pc++; break;
            case LOAD: vm_push(vm, vm->stack[vm->program[vm->pc++]]); break;
            case STORE: vm->stack[vm->program[vm->pc++]] = vm_pop(vm); break;
            case CALL: // Placeholder for functionality
                break;
            case RET: running = 0; break;
            case HALT: running = 0; break;
            case STRCMP: // Placeholder for string comparison
                break;
            case STRSEARCH: // Placeholder for string search
                break;
        }
    }
}

static int mk_security_conf(char *confdir) {
    int ret = 0;
    VM vm;
    int program[] = {
        PUSH, 0,
        CALL,
        HALT
    };

    vm_init(&vm, program);
    vm_run(&vm);
    return ret;
}

static int mk_security_check_ip(int socket) {
    VM vm;
    int program[] = {
        PUSH, socket,
        CALL,
        HALT
    };
    
    vm_init(&vm, program);
    vm_run(&vm);
    return vm_pop(&vm);
}

static int mk_security_check_url(mk_ptr_t url) {
    VM vm;
    int program[] = {
        PUSH, (int)url.data,
        CALL,
        HALT
    };

    vm_init(&vm, program);
    vm_run(&vm);
    return vm_pop(&vm);
}

mk_ptr_t parse_referer_host(mk_ptr_t ref) {
    mk_ptr_t host;
    VM vm;
    int program[] = {
        PUSH, (int)ref.data,
        CALL,
        HALT
    };

    vm_init(&vm, program);
    vm_run(&vm);
    host.data = (char *)vm_pop(&vm);
    host.len = vm_pop(&vm);
    return host;
}

static int mk_security_check_hotlink(mk_ptr_t url, mk_ptr_t host, mk_ptr_t referer) {
    VM vm;
    int program[] = {
        PUSH, (int)url.data,
        PUSH, (int)host.data,
        PUSH, (int)referer.data,
        CALL,
        HALT
    };

    vm_init(&vm, program);
    vm_run(&vm);
    return vm_pop(&vm);
}

int _mkp_init(struct plugin_api **api, char *confdir) {
    mk_api = *api;
    mk_list_init(&mk_secure_ip);
    mk_list_init(&mk_secure_url);
    mk_list_init(&mk_secure_deny_hotlink);
    mk_security_conf(confdir);
    return 0;
}

void _mkp_exit() {}

int _mkp_stage_10(unsigned int socket, struct sched_connection *conx) {
    (void) conx;
    if (mk_security_check_ip(socket) != 0) {
        return MK_PLUGIN_RET_CLOSE_CONX;
    }
    return MK_PLUGIN_RET_CONTINUE;
}

int _mkp_stage_30(struct plugin *p, struct client_session *cs, struct session_request *sr) {
    mk_ptr_t referer;
    (void) p;
    (void) cs;

    if (mk_security_check_url(sr->uri) < 0) {
        mk_api->header_set_http_status(sr, MK_CLIENT_FORBIDDEN);
        return MK_PLUGIN_RET_CLOSE_CONX;
    }

    referer = mk_api->header_get(&sr->headers_toc, "Referer", strlen("Referer"));
    if (mk_security_check_hotlink(sr->uri_processed, sr->host, referer) < 0) {
        mk_api->header_set_http_status(sr, MK_CLIENT_FORBIDDEN);
        return MK_PLUGIN_RET_CLOSE_CONX;
    }

    return MK_PLUGIN_RET_NOT_ME;
}