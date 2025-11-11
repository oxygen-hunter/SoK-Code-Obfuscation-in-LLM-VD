#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "gd.h"
#include "gdhelpers.h"
#include "php.h"

#define MAX_XBM_LINE_SIZE 255

enum OpCode {
    OP_PUSH, OP_POP, OP_ADD, OP_SUB, OP_JMP, OP_JZ, OP_LOAD, OP_STORE, OP_CALL, OP_RET, OP_HALT
};

typedef struct {
    int stack[1024];
    int sp;
    int pc;
    unsigned char *code;
    int running;
} VM;

void vm_init(VM *vm, unsigned char *code) {
    vm->sp = -1;
    vm->pc = 0;
    vm->code = code;
    vm->running = 1;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm) {
    while (vm->running) {
        switch (vm->code[vm->pc++]) {
            case OP_PUSH:
                vm_push(vm, vm->code[vm->pc++]);
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
                vm->pc = vm->code[vm->pc];
                break;
            case OP_JZ: {
                int addr = vm->code[vm->pc++];
                if (vm_pop(vm) == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case OP_LOAD:
                vm_push(vm, vm->stack[vm->code[vm->pc++]]);
                break;
            case OP_STORE:
                vm->stack[vm->code[vm->pc++]] = vm_pop(vm);
                break;
            case OP_CALL: {
                int addr = vm->code[vm->pc++];
                int ret = vm->pc;
                vm->pc = addr;
                vm_push(vm, ret);
                break;
            }
            case OP_RET:
                vm->pc = vm_pop(vm);
                break;
            case OP_HALT:
                vm->running = 0;
                break;
        }
    }
}

unsigned char imageCreateFromXbmCode[] = {
    OP_PUSH, 0,          // fd
    OP_CALL, 10,         // call rewind
    OP_PUSH, MAX_XBM_LINE_SIZE,
    OP_PUSH, 0,          // width
    OP_PUSH, 0,          // height
    OP_PUSH, 0,          // fail
    OP_PUSH, 0,          // max_bit
    OP_PUSH, 0,          // bytes
    OP_PUSH, 0,          // i
    OP_PUSH, 0,          // bit
    OP_PUSH, 0,          // x
    OP_PUSH, 0,          // y
    OP_PUSH, 0,          // ch
    OP_PUSH, 0,          // b
    OP_CALL, 20,         // call gdImageCreate
    OP_STORE, 0,         // im
    OP_LOAD, 0,
    OP_JZ, 80,           // if (!im) return 0;
    OP_CALL, 30,         // call gdImageColorAllocate
    OP_CALL, 30,         // call gdImageColorAllocate
    OP_PUSH, 0,
    OP_STORE, 1,         // h[2] = '\0'
    OP_PUSH, 0,
    OP_STORE, 2,         // h[4] = '\0'
    OP_PUSH, 0,
    OP_STORE, 3,         // i = 0
    OP_LOAD, 7,          // while (i < bytes) {
    OP_PUSH, 0,
    OP_CALL, 40,         // call getc
    OP_STORE, 4,         // ch
    OP_LOAD, 4,
    OP_PUSH, -1,
    OP_SUB,
    OP_JZ, 70,           // if (ch == EOF) break;
    OP_LOAD, 4,
    OP_PUSH, 'x',
    OP_SUB,
    OP_JZ, 60,           // if (ch == 'x') break;
    OP_JMP, 40,
    OP_LOAD, 3,
    OP_PUSH, 1,
    OP_ADD,
    OP_STORE, 3,         // i++
    OP_JMP, 20,
    OP_CALL, 50,         // call php_gd_error
    OP_LOAD, 0,
    OP_CALL, 60,         // call gdImageDestroy
    OP_PUSH, 0,
    OP_RET,              // return 0;
    OP_HALT
};

// Placeholder for calls to C functions
void placeholder_function() {}

gdImagePtr gdImageCreateFromXbm(FILE * fd) {
    VM vm;
    vm_init(&vm, imageCreateFromXbmCode);
    vm_run(&vm);
    return (gdImagePtr)vm_pop(&vm);
}

void gdCtxPrintf(gdIOCtx * out, const char *format, ...) {
    char *buf;
    int len;
    va_list args;
    va_start(args, format);
    len = vspprintf(&buf, 0, format, args);
    va_end(args);
    out->putBuf(out, buf, len);
    efree(buf);
}

void gdImageXbmCtx(gdImagePtr image, char* file_name, int fg, gdIOCtx * out) {
    int x, y, c, b, sx, sy, p;
    char *name, *f;
    size_t i, l;

    name = file_name;
    if ((f = strrchr(name, '/')) != NULL) name = f+1;
    if ((f = strrchr(name, '\\')) != NULL) name = f+1;
    name = estrdup(name);
    if ((f = strrchr(name, '.')) != NULL && !strcasecmp(f, ".XBM")) *f = '\0';
    if ((l = strlen(name)) == 0) {
        efree(name);
        name = estrdup("image");
    } else {
        for (i=0; i<l; i++) {
            if (!isupper(name[i]) && !islower(name[i]) && !isdigit(name[i])) {
                name[i] = '_';
            }
        }
    }

    gdCtxPrintf(out, "#define %s_width %d\n", name, gdImageSX(image));
    gdCtxPrintf(out, "#define %s_height %d\n", name, gdImageSY(image));
    gdCtxPrintf(out, "static unsigned char %s_bits[] = {\n  ", name);

    efree(name);

    b = 1;
    p = 0;
    c = 0;
    sx = gdImageSX(image);
    sy = gdImageSY(image);
    for (y = 0; y < sy; y++) {
        for (x = 0; x < sx; x++) {
            if (gdImageGetPixel(image, x, y) == fg) {
                c |= b;
            }
            if ((b == 128) || (x == sx - 1)) {
                b = 1;
                if (p) {
                    gdCtxPrintf(out, ", ");
                    if (!(p%12)) {
                        gdCtxPrintf(out, "\n  ");
                        p = 12;
                    }
                }
                p++;
                gdCtxPrintf(out, "0x%02X", c);
                c = 0;
            } else {
                b <<= 1;
            }
        }
    }
    gdCtxPrintf(out, "};\n");
}