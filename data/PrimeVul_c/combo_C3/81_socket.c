#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>
#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
static int wsa_init = 0;
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif
#include "socket.h"

#define RECV_TIMEOUT 20000

static int verbose = 0;

typedef enum {
    PUSH,
    POP,
    ADD,
    SUB,
    JMP,
    JZ,
    LOAD,
    STORE,
    CALL,
    RET,
    HALT
} Instruction;

typedef struct {
    Instruction *code;
    size_t pc;
    int stack[256];
    int sp;
} VM;

void vm_init(VM *vm, Instruction *code) {
    vm->code = code;
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
    int running = 1;
    while (running) {
        switch (vm->code[vm->pc++]) {
            case PUSH: {
                int value = vm->code[vm->pc++];
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
                int address = vm->code[vm->pc++];
                vm->pc = address;
                break;
            }
            case JZ: {
                int address = vm->code[vm->pc++];
                if (vm_pop(vm) == 0) {
                    vm->pc = address;
                }
                break;
            }
            case LOAD: {
                int index = vm->code[vm->pc++];
                vm_push(vm, vm->stack[index]);
                break;
            }
            case STORE: {
                int index = vm->code[vm->pc++];
                vm->stack[index] = vm_pop(vm);
                break;
            }
            case CALL: {
                int address = vm->code[vm->pc++];
                vm_push(vm, vm->pc);
                vm->pc = address;
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

#define FUNC_SOCKET_CREATE_UNIX 0
#define FUNC_SOCKET_CONNECT_UNIX 1
#define FUNC_SOCKET_CREATE 2
#define FUNC_SOCKET_CONNECT 3

Instruction code[] = {
    CALL, FUNC_SOCKET_CREATE_UNIX,
    CALL, FUNC_SOCKET_CONNECT_UNIX,
    CALL, FUNC_SOCKET_CREATE,
    CALL, FUNC_SOCKET_CONNECT,
    HALT,

    // socket_create_unix
    PUSH, 0, PUSH, 0, CALL, FUNC_SOCKET_CREATE_UNIX, RET,
    
    // socket_connect_unix
    PUSH, 1, PUSH, 0, CALL, FUNC_SOCKET_CONNECT_UNIX, RET,

    // socket_create
    PUSH, 2, PUSH, 0, CALL, FUNC_SOCKET_CREATE, RET,

    // socket_connect
    PUSH, 3, PUSH, 0, CALL, FUNC_SOCKET_CONNECT, RET,
};

void socket_set_verbose(int level) {
    verbose = level;
}

#ifndef WIN32
int socket_create_unix(const char *filename) {
    VM vm;
    vm_init(&vm, code);
    vm_run(&vm);
    return 0;
}

int socket_connect_unix(const char *filename) {
    VM vm;
    vm_init(&vm, code);
    vm_run(&vm);
    return 0;
}
#endif

int socket_create(uint16_t port) {
    VM vm;
    vm_init(&vm, code);
    vm_run(&vm);
    return 0;
}

int socket_connect(const char *addr, uint16_t port) {
    VM vm;
    vm_init(&vm, code);
    vm_run(&vm);
    return 0;
}