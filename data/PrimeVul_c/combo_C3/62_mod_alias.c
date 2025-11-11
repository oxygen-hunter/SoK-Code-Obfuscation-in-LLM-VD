#include "first.h"
#include "base.h"
#include "log.h"
#include "buffer.h"
#include "plugin.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    array *alias;
} plugin_config;

typedef struct {
    PLUGIN_DATA;
    plugin_config **config_storage;
    plugin_config conf;
} plugin_data;

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
} VM_Opcode;

typedef struct {
    VM_Opcode op;
    int arg;
} VM_Instruction;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int running;
} VM_State;

void vm_init(VM_State *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void vm_push(VM_State *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM_State *vm) {
    return vm->stack[vm->sp--];
}

void vm_dispatch(VM_State *vm, VM_Instruction *program) {
    while (vm->running) {
        VM_Instruction instr = program[vm->pc++];
        switch (instr.op) {
            case VM_NOP:
                break;
            case VM_PUSH:
                vm_push(vm, instr.arg);
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
                vm->pc = instr.arg;
                break;
            case VM_JZ:
                if (vm_pop(vm) == 0) vm->pc = instr.arg;
                break;
            case VM_LOAD:
                vm_push(vm, vm->stack[instr.arg]);
                break;
            case VM_STORE:
                vm->stack[instr.arg] = vm_pop(vm);
                break;
            case VM_CALL:
                vm_push(vm, vm->pc);
                vm->pc = instr.arg;
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

INIT_FUNC(mod_alias_init) {
    VM_State vm;
    vm_init(&vm);

    VM_Instruction program[] = {
        {VM_PUSH, sizeof(plugin_data)},
        {VM_CALL, 2},  // calloc
        {VM_RET, 0},
        {VM_HALT, 0}
    };

    vm_dispatch(&vm, program);

    return (plugin_data *)vm_pop(&vm);
}

FREE_FUNC(mod_alias_free) {
    VM_State vm;
    vm_init(&vm);

    VM_Instruction program[] = {
        {VM_LOAD, 0},  // p
        {VM_JZ, 8},    // if (!p) return HANDLER_GO_ON;
        {VM_LOAD, 1},  // p->config_storage
        {VM_JZ, 7},    // if (p->config_storage) { ... }
        {VM_LOAD, 2},  // i
        {VM_LOAD, 3},  // srv->config_context->used
        {VM_SUB, 0},
        {VM_JZ, 6},    // for loop
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
    };

    vm_dispatch(&vm, program);

    return HANDLER_GO_ON;
}

SETDEFAULTS_FUNC(mod_alias_set_defaults) {
    VM_State vm;
    vm_init(&vm);

    VM_Instruction program[] = {
        {VM_LOAD, 0},  // p
        {VM_JZ, 7},    // if (!p) return HANDLER_ERROR;
        {VM_CALL, 1},  // calloc
        {VM_STORE, 1}, // p->config_storage
        {VM_LOAD, 1},  // i
        {VM_LOAD, 2},  // srv->config_context->used
        {VM_SUB, 0},
        {VM_JZ, 6},    // for loop
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
    };

    vm_dispatch(&vm, program);

    return HANDLER_GO_ON;
}

#define PATCH(x) \
    p->conf.x = s->x;

static int mod_alias_patch_connection(server *srv, connection *con, plugin_data *p) {
    VM_State vm;
    vm_init(&vm);

    VM_Instruction program[] = {
        {VM_LOAD, 0},  // i
        {VM_LOAD, 1},  // srv->config_context->used
        {VM_SUB, 0},
        {VM_JZ, 6},    // for loop
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
    };

    vm_dispatch(&vm, program);

    return 0;
}

#undef PATCH

PHYSICALPATH_FUNC(mod_alias_physical_handler) {
    VM_State vm;
    vm_init(&vm);

    VM_Instruction program[] = {
        {VM_LOAD, 0},  // buffer_is_empty(con->physical.path)
        {VM_JZ, 8},    // if (buffer_is_empty(con->physical.path)) return HANDLER_GO_ON;
        {VM_CALL, 1},  // mod_alias_patch_connection
        {VM_LOAD, 1},  // basedir_len
        {VM_LOAD, 2},  // buffer_string_length(con->physical.basedir)
        {VM_SUB, 0},   // basedir_len -= (con->physical.basedir->ptr[basedir_len-1] == '/') ? 1 : 0;
        {VM_LOAD, 3},  // uri_len
        {VM_LOAD, 4},  // k
        {VM_SUB, 0},   // for loop
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
        {VM_HALT, 0},
    };

    vm_dispatch(&vm, program);

    return HANDLER_GO_ON;
}

int mod_alias_plugin_init(plugin *p) {
    VM_State vm;
    vm_init(&vm);

    VM_Instruction program[] = {
        {VM_LOAD, 0},  // p->version
        {VM_LOAD, 1},  // p->name
        {VM_LOAD, 2},  // p->init
        {VM_LOAD, 3},  // p->handle_physical
        {VM_LOAD, 4},  // p->set_defaults
        {VM_LOAD, 5},  // p->cleanup
        {VM_HALT, 0},
    };

    vm_dispatch(&vm, program);

    return 0;
}