#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "Xlibint.h"

#define PUSH 0
#define POP 1
#define ADD 2
#define SUB 3
#define JMP 4
#define JZ 5
#define LOAD 6
#define STORE 7
#define HALT 8

typedef struct {
    int pc;
    int stack[256];
    int sp;
    int memory[256];
} VM;

void runVM(VM *vm, int *program) {
    int running = 1;
    while (running) {
        switch (program[vm->pc]) {
            case PUSH:
                vm->stack[++vm->sp] = program[++vm->pc];
                break;
            case POP:
                --vm->sp;
                break;
            case ADD:
                vm->stack[vm->sp - 1] += vm->stack[vm->sp];
                --vm->sp;
                break;
            case SUB:
                vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
                --vm->sp;
                break;
            case JMP:
                vm->pc = program[++vm->pc] - 1;
                break;
            case JZ:
                if (vm->stack[vm->sp--] == 0) vm->pc = program[++vm->pc] - 1;
                else ++vm->pc;
                break;
            case LOAD:
                vm->stack[++vm->sp] = vm->memory[program[++vm->pc]];
                break;
            case STORE:
                vm->memory[program[++vm->pc]] = vm->stack[vm->sp--];
                break;
            case HALT:
                running = 0;
                break;
        }
        vm->pc++;
    }
}

char **XListExtensions(register Display *dpy, int *nextensions) {
    VM vm = {0};
    int program[] = {
        PUSH, (int)dpy,
        PUSH, (int)nextensions,
        CALL, 100, // call XListExtensions logic
        HALT,
        // XListExtensions logic
        100, LOAD, 0,
        PUSH, 0,
        STORE, 1,
        PUSH, 0,
        STORE, 2,
        LOCKDISPLAY,
        GETEMPTYREQ, LISTEXTENSIONS, 3,
        CALL, 200, // call XReply logic
        JZ, 150,
        PUSH, 0,
        STORE, 4,
        PUSH, 0,
        STORE, 5,
        PUSH, 0,
        STORE, 6,
        HALT,
        // XReply logic
        200, LOAD, 0,
        LOAD, 3,
        CALL, _XREPLY,
        HALT,
    };

    runVM(&vm, program);
    return (char **)vm.memory[4];
}

int XFreeExtensionList(char **list) {
    if (list != NULL) {
        Xfree(list[0] - 1);
        Xfree(list);
    }
    return 1;
}