#include <sys/cdefs.h>
#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

#define VM_STACK_SIZE 256
#define VM_PROGRAM_SIZE 512

typedef enum {
    OP_PUSH, OP_POP, OP_ADD, OP_SUB, OP_JMP, OP_JZ, OP_LOAD, OP_STORE, OP_HALT
} OpCode;

typedef struct {
    int stack[VM_STACK_SIZE];
    int sp;
    int pc;
    int program[VM_PROGRAM_SIZE];
} VM;

void vm_init(VM *vm, int *program) {
    vm->sp = -1;
    vm->pc = 0;
    for (int i = 0; i < VM_PROGRAM_SIZE; i++) {
        vm->program[i] = program[i];
    }
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
            case OP_PUSH:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case OP_POP:
                vm_pop(vm);
                break;
            case OP_ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case OP_SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case OP_JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case OP_JZ:
                if (vm_pop(vm) == 0) {
                    vm->pc = vm->program[vm->pc];
                } else {
                    vm->pc++;
                }
                break;
            case OP_LOAD:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case OP_STORE:
                vm->program[vm->program[vm->pc++]] = vm_pop(vm);
                break;
            case OP_HALT:
                running = 0;
                break;
        }
    }
}

struct filewbuf {
    FILE *fp;
    wchar_t *wbuf;
    size_t len;
};

#define FILEWBUF_INIT_LEN 128
#define FILEWBUF_POOL_ITEMS 32

static struct filewbuf fb_pool[FILEWBUF_POOL_ITEMS];
static int fb_pool_cur;

wchar_t *fgetwln(FILE *stream, size_t *lenp) {
    struct filewbuf *fb;
    wint_t wc;
    size_t wused = 0;

    fb = &fb_pool[fb_pool_cur];
    if (fb->fp != stream && fb->fp != NULL) {
        fb_pool_cur++;
        fb_pool_cur %= FILEWBUF_POOL_ITEMS;
        fb = &fb_pool[fb_pool_cur];
    }
    fb->fp = stream;

    int program[] = {
        OP_PUSH, (int)&fb->len,
        OP_LOAD, 0,
        OP_PUSH, 0,
        OP_STORE, 1,
        OP_LOAD, 0,
        OP_PUSH, 0,
        OP_ADD,
        OP_STORE, 0,
        OP_PUSH, 1,
        OP_HALT
    };

    VM vm;
    vm_init(&vm, program);
    vm_run(&vm);

    while ((wc = fgetwc(stream)) != WEOF) {
        if (!fb->len || wused > fb->len) {
            wchar_t *wp;

            if (fb->len)
                fb->len *= 2;
            else
                fb->len = FILEWBUF_INIT_LEN;

            wp = reallocarray(fb->wbuf, fb->len, sizeof(wchar_t));
            if (wp == NULL) {
                wused = 0;
                break;
            }
            fb->wbuf = wp;
        }

        fb->wbuf[wused++] = wc;

        if (wc == L'\n')
            break;
    }

    *lenp = wused;
    return wused ? fb->wbuf : NULL;
}