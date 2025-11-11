#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

#ifdef ENABLE_ZLIB
#include <zlib.h>
#endif

#define PRINT(c) (isprint(c) ? c : '?')

typedef enum {
    OP_PUSH,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_JMP,
    OP_JZ,
    OP_LOAD,
    OP_STORE,
    OP_CALL,
    OP_RET,
    OP_END
} Opcode;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    unsigned char *program;
    int data[256];
} VM;

void init_vm(VM *vm, unsigned char *program) {
    memset(vm, 0, sizeof(VM));
    vm->program = program;
}

void push(VM *vm, int value) {
    vm->stack[vm->sp++] = value;
}

int pop(VM *vm) {
    return vm->stack[--vm->sp];
}

void run_vm(VM *vm) {
    int running = 1;
    while (running) {
        switch (vm->program[vm->pc++]) {
            case OP_PUSH: {
                int value = vm->program[vm->pc++];
                push(vm, value);
                break;
            }
            case OP_POP: {
                pop(vm);
                break;
            }
            case OP_ADD: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a + b);
                break;
            }
            case OP_SUB: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a - b);
                break;
            }
            case OP_JMP: {
                vm->pc = vm->program[vm->pc];
                break;
            }
            case OP_JZ: {
                int addr = vm->program[vm->pc++];
                if (pop(vm) == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case OP_LOAD: {
                int addr = vm->program[vm->pc++];
                push(vm, vm->data[addr]);
                break;
            }
            case OP_STORE: {
                int addr = vm->program[vm->pc++];
                vm->data[addr] = pop(vm);
                break;
            }
            case OP_CALL: {
                int addr = vm->program[vm->pc++];
                push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case OP_RET: {
                vm->pc = pop(vm);
                break;
            }
            case OP_END: {
                running = 0;
                break;
            }
        }
    }
}

#ifdef ENABLE_ZLIB
int uncompress_gzip(void* uncompressed, size_t *uncompressed_len, const void* compressed, size_t compressed_len) {
    z_stream stream;
    memset(&stream, 0, sizeof stream);
    stream.total_in = compressed_len;
    stream.avail_in = compressed_len;
    stream.total_out = *uncompressed_len;
    stream.avail_out = *uncompressed_len;
    stream.next_in = (Bytef *) compressed;
    stream.next_out = (Bytef *) uncompressed;

    if (Z_OK == inflateInit2(&stream, (15 + 32)) && Z_STREAM_END == inflate(&stream, Z_FINISH)) {
        *uncompressed_len = stream.total_out;
    } else {
        return -1;
    }
    inflateEnd(&stream);

    return 0;
}
#else
int uncompress_gzip(void* uncompressed, size_t *uncompressed_len, const void* compressed, size_t compressed_len) {
    return -1;
}
#endif

void dump_binary(void *buf, size_t buf_len) {
#ifdef _WIN32
    _setmode(fileno(stdout), _O_BINARY);
#endif
    fwrite(buf, 1, buf_len, stdout);
#ifdef _WIN32
    _setmode(fileno(stdout), _O_TEXT);
#endif
}

void decode_version(unsigned char *bcd, unsigned int *major, unsigned int *minor, unsigned int *fix) {
    *major = 0;
    *minor = 0;
    *fix = 0;

    if ((bcd[0]>>4) < 10 && ((bcd[0]&0xF) < 10) && ((bcd[1]>>4) < 10)) {
        *major = (bcd[0]>>4)*100 + (bcd[0]&0xF)*10 + (bcd[1]>>4);
    }
    if (((bcd[1]&0xF) < 10) && ((bcd[2]>>4) < 10) && ((bcd[2]&0xF) < 10)) {
        *minor = (bcd[1]&0xF)*100 + (bcd[2]>>4)*10 + (bcd[2]&0xF);
    }
    if ((bcd[3]>>4) < 10 && ((bcd[3]&0xF) < 10) && (bcd[4]>>4) < 10 && ((bcd[4]&0xF) < 10)) {
        *fix = (bcd[3]>>4)*1000 + (bcd[3]&0xF)*100 + (bcd[4]>>4)*10 + (bcd[4]&0xF);
    }
}

int main(int argc, char **argv) {
    unsigned char program[] = {
        OP_PUSH, 1,
        OP_PUSH, 2,
        OP_ADD,
        OP_END
    };

    VM vm;
    init_vm(&vm, program);
    run_vm(&vm);

    printf("Result: %d\n", pop(&vm));
    return 0;
}