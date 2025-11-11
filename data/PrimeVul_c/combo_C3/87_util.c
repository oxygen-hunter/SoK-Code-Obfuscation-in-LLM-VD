#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef _WIN32
#include <termios.h>
#else
#include <conio.h>
#endif
#include <ctype.h>
#include "util.h"
#include "ui/notify.h"

#define STACK_SIZE 256
#define VM_PROGRAM_SIZE 1024

typedef enum {
    VM_PUSH,
    VM_POP,
    VM_ADD,
    VM_SUB,
    VM_JMP,
    VM_JZ,
    VM_LOAD,
    VM_STORE,
    VM_PRINT,
    VM_HALT
} VM_Opcode;

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
    int running;
    int program[VM_PROGRAM_SIZE];
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_execute(VM *vm) {
    while (vm->running) {
        int instr = vm->program[vm->pc++];
        switch (instr) {
            case VM_PUSH:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case VM_POP:
                vm_pop(vm);
                break;
            case VM_ADD:
                vm_push(vm, vm_pop(vm) + vm_pop(vm));
                break;
            case VM_SUB:
                {
                    int b = vm_pop(vm);
                    int a = vm_pop(vm);
                    vm_push(vm, a - b);
                }
                break;
            case VM_JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case VM_JZ:
                {
                    int addr = vm->program[vm->pc++];
                    if (vm_pop(vm) == 0) vm->pc = addr;
                }
                break;
            case VM_LOAD:
                // Implement if needed
                break;
            case VM_STORE:
                // Implement if needed
                break;
            case VM_PRINT:
                printf("%d\n", vm_pop(vm));
                break;
            case VM_HALT:
                vm->running = 0;
                break;
        }
    }
}

int is_string_valid_atr(const char *atr_str) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)atr_str;
    vm.program[2] = VM_CALL;
    vm.program[3] = VM_HALT;
    vm_execute(&vm);
    return vm_pop(&vm);
}

int util_connect_card_ex(sc_context_t *ctx, sc_card_t **cardp, const char *reader_id, int do_wait, int do_lock, int verbose) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)ctx;
    vm.program[2] = VM_PUSH;
    vm.program[3] = (int)cardp;
    vm.program[4] = VM_PUSH;
    vm.program[5] = (int)reader_id;
    vm.program[6] = VM_PUSH;
    vm.program[7] = do_wait;
    vm.program[8] = VM_PUSH;
    vm.program[9] = do_lock;
    vm.program[10] = VM_PUSH;
    vm.program[11] = verbose;
    vm.program[12] = VM_CALL;
    vm.program[13] = VM_HALT;
    vm_execute(&vm);
    return vm_pop(&vm);
}

int util_connect_card(sc_context_t *ctx, sc_card_t **cardp, const char *reader_id, int do_wait, int verbose) {
    return util_connect_card_ex(ctx, cardp, reader_id, do_wait, 1, verbose);
}

void util_print_binary(FILE *f, const u8 *buf, int count) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)f;
    vm.program[2] = VM_PUSH;
    vm.program[3] = (int)buf;
    vm.program[4] = VM_PUSH;
    vm.program[5] = count;
    vm.program[6] = VM_CALL;
    vm.program[7] = VM_HALT;
    vm_execute(&vm);
}

void util_hex_dump(FILE *f, const u8 *in, int len, const char *sep) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)f;
    vm.program[2] = VM_PUSH;
    vm.program[3] = (int)in;
    vm.program[4] = VM_PUSH;
    vm.program[5] = len;
    vm.program[6] = VM_PUSH;
    vm.program[7] = (int)sep;
    vm.program[8] = VM_CALL;
    vm.program[9] = VM_HALT;
    vm_execute(&vm);
}

void util_hex_dump_asc(FILE *f, const u8 *in, size_t count, int addr) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)f;
    vm.program[2] = VM_PUSH;
    vm.program[3] = (int)in;
    vm.program[4] = VM_PUSH;
    vm.program[5] = count;
    vm.program[6] = VM_PUSH;
    vm.program[7] = addr;
    vm.program[8] = VM_CALL;
    vm.program[9] = VM_HALT;
    vm_execute(&vm);
}

NORETURN void util_print_usage_and_die(const char *app_name, const struct option options[], const char *option_help[], const char *args) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)app_name;
    vm.program[2] = VM_PUSH;
    vm.program[3] = (int)options;
    vm.program[4] = VM_PUSH;
    vm.program[5] = (int)option_help;
    vm.program[6] = VM_PUSH;
    vm.program[7] = (int)args;
    vm.program[8] = VM_CALL;
    vm.program[9] = VM_HALT;
    vm_execute(&vm);
}

const char *util_acl_to_str(const sc_acl_entry_t *e) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)e;
    vm.program[2] = VM_CALL;
    vm.program[3] = VM_HALT;
    vm_execute(&vm);
    return (const char *)vm_pop(&vm);
}

NORETURN void util_fatal(const char *fmt, ...) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)fmt;
    vm.program[2] = VM_CALL;
    vm.program[3] = VM_HALT;
    vm_execute(&vm);
}

void util_error(const char *fmt, ...) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)fmt;
    vm.program[2] = VM_CALL;
    vm.program[3] = VM_HALT;
    vm_execute(&vm);
}

void util_warn(const char *fmt, ...) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)fmt;
    vm.program[2] = VM_CALL;
    vm.program[3] = VM_HALT;
    vm_execute(&vm);
}

int util_getpass(char **lineptr, size_t *len, FILE *stream) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)lineptr;
    vm.program[2] = VM_PUSH;
    vm.program[3] = (int)len;
    vm.program[4] = VM_PUSH;
    vm.program[5] = (int)stream;
    vm.program[6] = VM_CALL;
    vm.program[7] = VM_HALT;
    vm_execute(&vm);
    return vm_pop(&vm);
}

size_t util_get_pin(const char *input, const char **pin) {
    VM vm;
    vm_init(&vm);
    vm.program[0] = VM_PUSH;
    vm.program[1] = (int)input;
    vm.program[2] = VM_PUSH;
    vm.program[3] = (int)pin;
    vm.program[4] = VM_CALL;
    vm.program[5] = VM_HALT;
    vm_execute(&vm);
    return vm_pop(&vm);
}