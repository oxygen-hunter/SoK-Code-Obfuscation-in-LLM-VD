#include <config.h>
#include <stdlib.h>
#include <syslog.h>
#include <glib-unix.h>
#include <gio/gunixsocketaddress.h>
#include "udscs.h"
#include "vdagentd-proto-strings.h"
#include "vdagent-connection.h"

#define MAX_STACK_SIZE 1024

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
    VM_CALL,
    VM_RET,
    VM_HALT
} Instruction;

typedef struct {
    Instruction code[MAX_STACK_SIZE];
    int stack[MAX_STACK_SIZE];
    int sp;
    int pc;
    int running;
} VirtualMachine;

void vm_init(VirtualMachine *vm) {
    vm->sp = 0;
    vm->pc = 0;
    vm->running = 1;
}

void vm_push(VirtualMachine *vm, int value) {
    vm->stack[vm->sp++] = value;
}

int vm_pop(VirtualMachine *vm) {
    return vm->stack[--vm->sp];
}

void vm_execute(VirtualMachine *vm) {
    while (vm->running) {
        Instruction instr = vm->code[vm->pc++];
        switch (instr) {
            case VM_NOP:
                break;
            case VM_PUSH:
                vm_push(vm, vm->code[vm->pc++]);
                break;
            case VM_POP:
                vm_pop(vm);
                break;
            case VM_ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case VM_SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case VM_JMP:
                vm->pc = vm->code[vm->pc];
                break;
            case VM_JZ: {
                int value = vm_pop(vm);
                if (value == 0) {
                    vm->pc = vm->code[vm->pc];
                } else {
                    vm->pc++;
                }
                break;
            }
            case VM_LOAD:
                vm_push(vm, vm->stack[vm->code[vm->pc++]]);
                break;
            case VM_STORE:
                vm->stack[vm->code[vm->pc++]] = vm_pop(vm);
                break;
            case VM_CALL:
                vm_push(vm, vm->pc + 1);
                vm->pc = vm->code[vm->pc];
                break;
            case VM_RET:
                vm->pc = vm_pop(vm);
                break;
            case VM_HALT:
                vm->running = 0;
                break;
        }
    }
}

struct _UdscsConnection {
    VDAgentConnection parent_instance;
    int debug;
    udscs_read_callback read_callback;
};

G_DEFINE_TYPE(UdscsConnection, udscs_connection, VDAGENT_TYPE_CONNECTION)

void obfuscated_debug_print_message_header(VirtualMachine *vm, UdscsConnection *conn, struct udscs_message_header *header, const gchar *direction) {
    vm_init(vm);
    vm->code[0] = VM_NOP;
    vm->code[1] = VM_PUSH; vm->code[2] = (int)conn;
    vm->code[3] = VM_PUSH; vm->code[4] = (int)header;
    vm->code[5] = VM_PUSH; vm->code[6] = (int)direction;
    vm->code[7] = VM_CALL; vm->code[8] = 12;
    vm->code[9] = VM_POP;
    vm->code[10] = VM_HALT;
    vm_execute(vm);
}

static gsize conn_handle_header(VDAgentConnection *conn, gpointer header_buf) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)((struct udscs_message_header *)header_buf)->size;
    vm.code[3] = VM_HALT;
    vm_execute(&vm);
    return vm_pop(&vm);
}

static void conn_handle_message(VDAgentConnection *conn, gpointer header_buf, gpointer data) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)conn;
    vm.code[3] = VM_PUSH; vm.code[4] = (int)header_buf;
    vm.code[5] = VM_PUSH; vm.code[6] = (int)data;
    vm.code[7] = VM_CALL; vm.code[8] = 12;
    vm.code[9] = VM_POP;
    vm.code[10] = VM_HALT;
    vm_execute(&vm);
}

static void udscs_connection_init(UdscsConnection *self) {}

static void udscs_connection_finalize(GObject *obj) {
    UdscsConnection *self = UDSCS_CONNECTION(obj);
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)self;
    vm.code[3] = VM_CALL; vm.code[4] = 15;
    vm.code[5] = VM_HALT;
    vm_execute(&vm);
}

static void udscs_connection_class_init(UdscsConnectionClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    VDAgentConnectionClass *conn_class = VDAGENT_CONNECTION_CLASS(klass);
    gobject_class->finalize = udscs_connection_finalize;
    conn_class->handle_header = conn_handle_header;
    conn_class->handle_message = conn_handle_message;
}

UdscsConnection *udscs_connect(const char *socketname, udscs_read_callback read_callback, VDAgentConnErrorCb error_cb, int debug) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)socketname;
    vm.code[3] = VM_PUSH; vm.code[4] = (int)read_callback;
    vm.code[5] = VM_PUSH; vm.code[6] = (int)error_cb;
    vm.code[7] = VM_PUSH; vm.code[8] = debug;
    vm.code[9] = VM_CALL; vm.code[10] = 18;
    vm.code[11] = VM_HALT;
    vm_execute(&vm);
    return (UdscsConnection *)vm_pop(&vm);
}

void udscs_write(UdscsConnection *conn, uint32_t type, uint32_t arg1, uint32_t arg2, const uint8_t *data, uint32_t size) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)conn;
    vm.code[3] = VM_PUSH; vm.code[4] = type;
    vm.code[5] = VM_PUSH; vm.code[6] = arg1;
    vm.code[7] = VM_PUSH; vm.code[8] = arg2;
    vm.code[9] = VM_PUSH; vm.code[10] = (int)data;
    vm.code[11] = VM_PUSH; vm.code[12] = size;
    vm.code[13] = VM_CALL; vm.code[14] = 22;
    vm.code[15] = VM_HALT;
    vm_execute(&vm);
}

#ifndef UDSCS_NO_SERVER

struct udscs_server {
    GSocketService *service;
    GList *connections;
    int debug;
    udscs_connect_callback connect_callback;
    udscs_read_callback read_callback;
    VDAgentConnErrorCb error_cb;
};

static gboolean udscs_server_accept_cb(GSocketService *service, GSocketConnection *socket_conn, GObject *source_object, gpointer user_data);

struct udscs_server *udscs_server_new(udscs_connect_callback connect_callback, udscs_read_callback read_callback, VDAgentConnErrorCb error_cb, int debug) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)connect_callback;
    vm.code[3] = VM_PUSH; vm.code[4] = (int)read_callback;
    vm.code[5] = VM_PUSH; vm.code[6] = (int)error_cb;
    vm.code[7] = VM_PUSH; vm.code[8] = debug;
    vm.code[9] = VM_CALL; vm.code[10] = 26;
    vm.code[11] = VM_HALT;
    vm_execute(&vm);
    return (struct udscs_server *)vm_pop(&vm);
}

void udscs_server_listen_to_socket(struct udscs_server *server, gint fd, GError **err) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)server;
    vm.code[3] = VM_PUSH; vm.code[4] = fd;
    vm.code[5] = VM_PUSH; vm.code[6] = (int)err;
    vm.code[7] = VM_CALL; vm.code[8] = 30;
    vm.code[9] = VM_HALT;
    vm_execute(&vm);
}

void udscs_server_listen_to_address(struct udscs_server *server, const gchar *addr, GError **err) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)server;
    vm.code[3] = VM_PUSH; vm.code[4] = (int)addr;
    vm.code[5] = VM_PUSH; vm.code[6] = (int)err;
    vm.code[7] = VM_CALL; vm.code[8] = 34;
    vm.code[9] = VM_HALT;
    vm_execute(&vm);
}

void udscs_server_start(struct udscs_server *server) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)server;
    vm.code[3] = VM_CALL; vm.code[4] = 38;
    vm.code[5] = VM_HALT;
    vm_execute(&vm);
}

void udscs_server_destroy_connection(struct udscs_server *server, UdscsConnection *conn) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)server;
    vm.code[3] = VM_PUSH; vm.code[4] = (int)conn;
    vm.code[5] = VM_CALL; vm.code[6] = 42;
    vm.code[7] = VM_HALT;
    vm_execute(&vm);
}

void udscs_destroy_server(struct udscs_server *server) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)server;
    vm.code[3] = VM_CALL; vm.code[4] = 46;
    vm.code[5] = VM_HALT;
    vm_execute(&vm);
}

static gboolean udscs_server_accept_cb(GSocketService *service, GSocketConnection *socket_conn, GObject *source_object, gpointer user_data) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)service;
    vm.code[3] = VM_PUSH; vm.code[4] = (int)socket_conn;
    vm.code[5] = VM_PUSH; vm.code[6] = (int)source_object;
    vm.code[7] = VM_PUSH; vm.code[8] = (int)user_data;
    vm.code[9] = VM_CALL; vm.code[10] = 50;
    vm.code[11] = VM_HALT;
    vm_execute(&vm);
    return (gboolean)vm_pop(&vm);
}

void udscs_server_write_all(struct udscs_server *server, uint32_t type, uint32_t arg1, uint32_t arg2, const uint8_t *data, uint32_t size) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)server;
    vm.code[3] = VM_PUSH; vm.code[4] = type;
    vm.code[5] = VM_PUSH; vm.code[6] = arg1;
    vm.code[7] = VM_PUSH; vm.code[8] = arg2;
    vm.code[9] = VM_PUSH; vm.code[10] = (int)data;
    vm.code[11] = VM_PUSH; vm.code[12] = size;
    vm.code[13] = VM_CALL; vm.code[14] = 54;
    vm.code[15] = VM_HALT;
    vm_execute(&vm);
}

int udscs_server_for_all_clients(struct udscs_server *server, udscs_for_all_clients_callback func, void *priv) {
    VirtualMachine vm;
    vm_init(&vm);
    vm.code[0] = VM_NOP;
    vm.code[1] = VM_PUSH; vm.code[2] = (int)server;
    vm.code[3] = VM_PUSH; vm.code[4] = (int)func;
    vm.code[5] = VM_PUSH; vm.code[6] = (int)priv;
    vm.code[7] = VM_CALL; vm.code[8] = 58;
    vm.code[9] = VM_HALT;
    vm_execute(&vm);
    return vm_pop(&vm);
}

#endif