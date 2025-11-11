#include "yajl_encode.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Instruction;

typedef struct {
    Instruction instr;
    int operand;
} Bytecode;

typedef struct {
    int stack[256];
    int sp;
    Bytecode *code;
    int pc;
} VM;

void vm_init(VM *vm, Bytecode *code) {
    vm->sp = -1;
    vm->code = code;
    vm->pc = 0;
}

void vm_run(VM *vm) {
    while (1) {
        Bytecode *instr = &vm->code[vm->pc++];
        switch (instr->instr) {
            case PUSH:
                vm->stack[++vm->sp] = instr->operand;
                break;
            case POP:
                vm->sp--;
                break;
            case ADD:
                vm->stack[vm->sp - 1] += vm->stack[vm->sp];
                vm->sp--;
                break;
            case SUB:
                vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
                vm->sp--;
                break;
            case JMP:
                vm->pc = instr->operand;
                break;
            case JZ:
                if (vm->stack[vm->sp--] == 0) vm->pc = instr->operand;
                break;
            case LOAD:
                vm->stack[++vm->sp] = instr->operand;
                break;
            case STORE:
                break;
            case CALL:
                break;
            case RET:
                break;
            case HALT:
                return;
        }
    }
}

static void CharToHex(unsigned char c, char *hexBuf) {
    VM vm;
    Bytecode code[] = {
        {PUSH, c >> 4}, {CALL, 0}, {PUSH, c & 0x0F}, {CALL, 0}, {HALT, 0},
        {LOAD, '0'}, {ADD, 0}, {STORE, 0}, {RET, 0}
    };
    vm_init(&vm, code);
    vm_run(&vm);
    hexBuf[0] = (char)vm.stack[0];
    hexBuf[1] = (char)vm.stack[1];
}

void yajl_string_encode(yajl_buf buf, const unsigned char *str, unsigned int len, unsigned int htmlSafe) {
    yajl_string_encode2((const yajl_print_t)&yajl_buf_append, buf, str, len, htmlSafe);
}

void yajl_string_encode2(const yajl_print_t print, void *ctx, const unsigned char *str, unsigned int len, unsigned int htmlSafe) {
    unsigned int beg = 0;
    unsigned int end = 0;
    char hexBuf[7];
    hexBuf[0] = '\\'; hexBuf[1] = 'u'; hexBuf[2] = '0'; hexBuf[3] = '0';
    hexBuf[6] = 0;

    while (end < len) {
        const char *escaped = NULL;
        switch (str[end]) {
            case '\r': escaped = "\\r"; break;
            case '\n': escaped = "\\n"; break;
            case '\\': escaped = "\\\\"; break;
            case '"': escaped = "\\\""; break;
            case '\f': escaped = "\\f"; break;
            case '\b': escaped = "\\b"; break;
            case '\t': escaped = "\\t"; break;
            case '/':
                if (htmlSafe) {
                    escaped = "\\/";
                }
                break;
            default:
                if ((unsigned char)str[end] < 32) {
                    CharToHex(str[end], hexBuf + 4);
                    escaped = hexBuf;
                }
                break;
        }
        if (escaped != NULL) {
            print(ctx, (const char *)(str + beg), end - beg);
            print(ctx, escaped, (unsigned int)strlen(escaped));
            beg = ++end;
        } else {
            ++end;
        }
    }
    print(ctx, (const char *)(str + beg), end - beg);
}

static void hexToDigit(unsigned int *val, const unsigned char *hex) {
    unsigned int i;
    for (i = 0; i < 4; i++) {
        unsigned char c = hex[i];
        if (c >= 'A') c = (c & ~0x20) - 7;
        c -= '0';
        assert(!(c & 0xF0));
        *val = (*val << 4) | c;
    }
}

static void Utf32toUtf8(unsigned int codepoint, char *utf8Buf) {
    if (codepoint < 0x80) {
        utf8Buf[0] = (char)codepoint;
        utf8Buf[1] = 0;
    } else if (codepoint < 0x0800) {
        utf8Buf[0] = (char)((codepoint >> 6) | 0xC0);
        utf8Buf[1] = (char)((codepoint & 0x3F) | 0x80);
        utf8Buf[2] = 0;
    } else if (codepoint < 0x10000) {
        utf8Buf[0] = (char)((codepoint >> 12) | 0xE0);
        utf8Buf[1] = (char)(((codepoint >> 6) & 0x3F) | 0x80);
        utf8Buf[2] = (char)((codepoint & 0x3F) | 0x80);
        utf8Buf[3] = 0;
    } else if (codepoint < 0x200000) {
        utf8Buf[0] = (char)((codepoint >> 18) | 0xF0);
        utf8Buf[1] = (char)(((codepoint >> 12) & 0x3F) | 0x80);
        utf8Buf[2] = (char)(((codepoint >> 6) & 0x3F) | 0x80);
        utf8Buf[3] = (char)((codepoint & 0x3F) | 0x80);
        utf8Buf[4] = 0;
    } else {
        utf8Buf[0] = '?';
        utf8Buf[1] = 0;
    }
}

void yajl_string_decode(yajl_buf buf, const unsigned char *str, unsigned int len) {
    unsigned int beg = 0;
    unsigned int end = 0;

    while (end < len) {
        if (str[end] == '\\') {
            char utf8Buf[5];
            const char *unescaped = "?";
            yajl_buf_append(buf, str + beg, end - beg);
            switch (str[++end]) {
                case 'r': unescaped = "\r"; break;
                case 'n': unescaped = "\n"; break;
                case '\\': unescaped = "\\"; break;
                case '/': unescaped = "/"; break;
                case '"': unescaped = "\""; break;
                case 'f': unescaped = "\f"; break;
                case 'b': unescaped = "\b"; break;
                case 't': unescaped = "\t"; break;
                case 'u': {
                    unsigned int codepoint = 0;
                    hexToDigit(&codepoint, str + ++end);
                    end += 3;
                    if ((codepoint & 0xFC00) == 0xD800) {
                        end++;
                        if (str[end] == '\\' && str[end + 1] == 'u') {
                            unsigned int surrogate = 0;
                            hexToDigit(&surrogate, str + end + 2);
                            codepoint = (((codepoint & 0x3F) << 10) | ((((codepoint >> 6) & 0xF) + 1) << 16) | (surrogate & 0x3FF));
                            end += 5;
                        } else {
                            unescaped = "?";
                            break;
                        }
                    }
                    Utf32toUtf8(codepoint, utf8Buf);
                    unescaped = utf8Buf;

                    if (codepoint == 0) {
                        yajl_buf_append(buf, unescaped, 1);
                        beg = ++end;
                        continue;
                    }
                    break;
                }
                default:
                    assert("this should never happen" == NULL);
            }
            yajl_buf_append(buf, unescaped, (unsigned int)strlen(unescaped));
            beg = ++end;
        } else {
            end++;
        }
    }
    yajl_buf_append(buf, str + beg, end - beg);
}