#include <config.h>
#include <stdio.h>
#include <ctype.h>

#include "ntp_fp.h"
#include "ntp.h"
#include "ntp_syslog.h"
#include "ntp_stdlib.h"

#ifdef OPENSSL
#include "openssl/objects.h"
#include "openssl/evp.h"
#endif

#define STACK_SIZE 1024

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, NEXTHASH, READKEYS, PRINTERR
} Instruction;

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
    int program[STACK_SIZE];
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_push(VM *vm, int value) {
    vm->stack[++(vm->sp)] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[(vm->sp)--];
}

void vm_load_program(VM *vm, int *program, int size) {
    for (int i = 0; i < size; i++) {
        vm->program[i] = program[i];
    }
}

void vm_run(VM *vm) {
    while (1) {
        switch (vm->program[vm->pc]) {
            case PUSH:
                vm->pc++;
                vm_push(vm, vm->program[vm->pc]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case JMP:
                vm->pc++;
                vm->pc = vm->program[vm->pc] - 1;
                break;
            case JZ:
                vm->pc++;
                if (vm_pop(vm) == 0) {
                    vm->pc = vm->program[vm->pc] - 1;
                }
                break;
            case LOAD:
                vm->pc++;
                vm_push(vm, vm->program[vm->pc]);
                break;
            case STORE:
                vm->pc++;
                vm->program[vm->program[vm->pc]] = vm_pop(vm);
                break;
            case NEXTHASH:
                vm_push(vm, (int)nexttok((char **)vm_pop(vm)));
                break;
            case READKEYS:
                vm_push(vm, (int)fopen((const char *)vm_pop(vm), "r"));
                break;
            case PRINTERR:
                msyslog(LOG_ERR, "authreadkeys: file %s: %m", (const char *)vm_pop(vm));
                break;
            default:
                return;
        }
        vm->pc++;
    }
}

static char *nexttok(char **str) {
    register char *cp;
    char *starttok;

    cp = *str;
    while (*cp == ' ' || *cp == '\t')
        cp++;
    starttok = cp;
    while (*cp != '\0' && *cp != '\n' && *cp != ' '
           && *cp != '\t' && *cp != '#')
        cp++;
    if (starttok == cp)
        return NULL;
    if (*cp == ' ' || *cp == '\t')
        *cp++ = '\0';
    else
        *cp = '\0';
    *str = cp;
    return starttok;
}

int authreadkeys(const char *file) {
    VM vm;
    vm_init(&vm);
    int program[] = {
        PUSH, (int)file, READKEYS, PRINTERR, JMP, 20,
        LOAD, 0, NEXTHASH, PUSH, 0, JZ, 19, JMP, 5,
        STORE, 1, JMP, 0
    };
    vm_load_program(&vm, program, sizeof(program) / sizeof(int));
    vm_run(&vm);
    return 1;
}