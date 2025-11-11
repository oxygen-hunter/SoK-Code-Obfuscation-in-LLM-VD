#include <config.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include "internal.h"

enum Instructions {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

typedef struct {
    int stack[256];
    int sp;
    int pc;
    int code[1024];
    bool running;
} VM;

void vm_init(VM *vm, int *program, int program_size) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = true;
    for (int i = 0; i < program_size; i++) {
        vm->code[i] = program[i];
    }
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_execute(VM *vm) {
    while (vm->running) {
        switch (vm->code[vm->pc++]) {
            case PUSH:
                vm_push(vm, vm->code[vm->pc++]);
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
                vm->pc = vm->code[vm->pc];
                break;
            case JZ:
                if (vm_pop(vm) == 0) vm->pc = vm->code[vm->pc];
                else vm->pc++;
                break;
            case CALL:
                vm_push(vm, vm->pc + 1);
                vm->pc = vm->code[vm->pc];
                break;
            case RET:
                vm->pc = vm_pop(vm);
                break;
            case HALT:
                vm->running = false;
                break;
        }
    }
}

void nbd_internal_free_option(struct nbd_handle *h) {
    VM vm;
    int program[] = {PUSH, (int)(h->opt_current == NBD_OPT_LIST), JZ, 9, LOAD, (int)&h->opt_cb.fn.list, CALL, 0, HALT, 
                     PUSH, (int)(h->opt_current == NBD_OPT_LIST_META_CONTEXT), JZ, 19, LOAD, (int)&h->opt_cb.fn.context, CALL, 0, HALT,
                     LOAD, (int)&h->opt_cb.completion, CALL, 0, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
}

int nbd_unlocked_set_opt_mode(struct nbd_handle *h, bool value) {
    VM vm;
    int program[] = {LOAD, (int)&h->opt_mode, PUSH, value, STORE, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}

int nbd_unlocked_get_opt_mode(struct nbd_handle *h) {
    VM vm;
    int program[] = {LOAD, (int)&h->opt_mode, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return h->opt_mode;
}

static int wait_for_option(struct nbd_handle *h) {
    VM vm;
    int program[] = {CALL, 0, PUSH, -1, CALL, 1, JZ, 0, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}

static int go_complete(void *opaque, int *err) {
    VM vm;
    int program[] = {LOAD, (int)opaque, LOAD, (int)err, STORE, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}

int nbd_unlocked_opt_go(struct nbd_handle *h) {
    int err;
    nbd_completion_callback c = { .callback = go_complete, .user_data = &err };
    VM vm;
    int program[] = {CALL, 0, PUSH, -1, JZ, 14, CALL, 1, JZ, 10, LOAD, (int)&err, JZ, 13, CALL, 2, PUSH, -1, RET, HALT,
                     LOAD, (int)&err, JZ, 13, CALL, 3, PUSH, -1, RET, HALT, RET, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}

int nbd_unlocked_opt_info(struct nbd_handle *h) {
    int err;
    nbd_completion_callback c = { .callback = go_complete, .user_data = &err };
    VM vm;
    int program[] = {CALL, 0, PUSH, -1, JZ, 14, CALL, 1, JZ, 10, LOAD, (int)&err, JZ, 13, CALL, 2, PUSH, -1, RET, HALT,
                     LOAD, (int)&err, JZ, 13, CALL, 3, PUSH, -1, RET, HALT, RET, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}

int nbd_unlocked_opt_abort(struct nbd_handle *h) {
    VM vm;
    int program[] = {CALL, 0, PUSH, -1, JZ, 8, CALL, 1, RET, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}

int nbd_unlocked_opt_list(struct nbd_handle *h, nbd_list_callback *list) {
    struct list_helper s = { .list = *list };
    nbd_list_callback l = { .callback = list_visitor, .user_data = &s };
    nbd_completion_callback c = { .callback = list_complete, .user_data = &s };
    VM vm;
    int program[] = {CALL, 0, PUSH, -1, JZ, 12, CALL, 1, JZ, 8, LOAD, (int)&s.err, JZ, 11, CALL, 2, PUSH, -1, RET, HALT,
                     LOAD, (int)&s.err, JZ, 11, CALL, 3, PUSH, -1, RET, HALT, RET, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return s.count;
}

int nbd_unlocked_opt_list_meta_context(struct nbd_handle *h, nbd_context_callback *context) {
    struct context_helper s = { .context = *context };
    nbd_context_callback l = { .callback = context_visitor, .user_data = &s };
    nbd_completion_callback c = { .callback = context_complete, .user_data = &s };
    VM vm;
    int program[] = {CALL, 0, PUSH, -1, JZ, 12, CALL, 1, JZ, 8, LOAD, (int)&s.err, JZ, 11, CALL, 2, PUSH, -1, RET, HALT,
                     LOAD, (int)&s.err, JZ, 11, CALL, 3, PUSH, -1, RET, HALT, RET, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return s.count;
}

int nbd_unlocked_aio_opt_go(struct nbd_handle *h, nbd_completion_callback *complete) {
    VM vm;
    int program[] = {LOAD, (int)&h->opt_current, PUSH, NBD_OPT_GO, STORE,
                     LOAD, (int)&h->opt_cb.completion, LOAD, (int)complete, STORE,
                     CALL, 0, PUSH, -1, JZ, 8, CALL, 1, RET, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}

int nbd_unlocked_aio_opt_info(struct nbd_handle *h, nbd_completion_callback *complete) {
    VM vm;
    int program[] = {LOAD, (int)&h->opt_current, PUSH, NBD_OPT_INFO, STORE,
                     LOAD, (int)&h->opt_cb.completion, LOAD, (int)complete, STORE,
                     CALL, 0, PUSH, -1, JZ, 8, CALL, 1, RET, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}

int nbd_unlocked_aio_opt_abort(struct nbd_handle *h) {
    VM vm;
    int program[] = {LOAD, (int)&h->opt_current, PUSH, NBD_OPT_ABORT, STORE,
                     CALL, 0, PUSH, -1, JZ, 8, CALL, 1, RET, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}

int nbd_unlocked_aio_opt_list(struct nbd_handle *h, nbd_list_callback *list, nbd_completion_callback *complete) {
    VM vm;
    int program[] = {LOAD, (int)&h->opt_cb.fn.list, LOAD, (int)list, STORE,
                     LOAD, (int)&h->opt_cb.completion, LOAD, (int)complete, STORE,
                     LOAD, (int)&h->opt_current, PUSH, NBD_OPT_LIST, STORE,
                     CALL, 0, PUSH, -1, JZ, 8, CALL, 1, RET, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}

int nbd_unlocked_aio_opt_list_meta_context(struct nbd_handle *h, nbd_context_callback *context, nbd_completion_callback *complete) {
    VM vm;
    int program[] = {LOAD, (int)&h->opt_cb.fn.context, LOAD, (int)context, STORE,
                     LOAD, (int)&h->opt_cb.completion, LOAD, (int)complete, STORE,
                     LOAD, (int)&h->opt_current, PUSH, NBD_OPT_LIST_META_CONTEXT, STORE,
                     CALL, 0, PUSH, -1, JZ, 8, CALL, 1, RET, HALT};
    vm_init(&vm, program, sizeof(program) / sizeof(int));
    vm_execute(&vm);
    return 0;
}