#include "rsync.h"
#include "itypes.h"

extern int read_only;
extern char *password_file;

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, STRCPY, GETPID, TIME, PRINTF
} OpCode;

typedef struct {
    OpCode op;
    int operand;
} Instruction;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    char memory[1024];
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_push(VM *vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
    return vm->stack[vm->sp--];
}

void vm_run(VM *vm, Instruction *program) {
    int running = 1;
    while (running) {
        Instruction instr = program[vm->pc++];
        switch (instr.op) {
            case PUSH:
                vm_push(vm, instr.operand);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD: {
                int b = vm_pop(vm), a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int b = vm_pop(vm), a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP:
                vm->pc = instr.operand;
                break;
            case JZ:
                if (vm_pop(vm) == 0)
                    vm->pc = instr.operand;
                break;
            case LOAD:
                vm_push(vm, vm->memory[instr.operand]);
                break;
            case STORE:
                vm->memory[instr.operand] = vm_pop(vm);
                break;
            case CALL:
                vm_push(vm->pc);
                vm->pc = instr.operand;
                break;
            case RET:
                vm->pc = vm_pop(vm);
                break;
            case STRCPY: {
                char *src = (char *)&vm->memory[vm_pop(vm)];
                char *dst = (char *)&vm->memory[instr.operand];
                while ((*dst++ = *src++) != '\0');
                break;
            }
            case GETPID:
                vm_push(vm, getpid());
                break;
            case TIME: {
                struct timeval tv;
                sys_gettimeofday(&tv);
                vm_push(vm, tv.tv_sec);
                vm_push(vm, tv.tv_usec);
                break;
            }
            case PRINTF: {
                char *fmt = (char *)&vm->memory[instr.operand];
                printf(fmt, vm_pop(vm), vm_pop(vm));
                break;
            }
        }
    }
}

void base64_encode_vm(const char *buf, int len, char *out, int pad) {
    VM vm;
    vm_init(&vm);

    Instruction program[] = {
        {PUSH, (int)buf}, {PUSH, len}, {PUSH, (int)out}, {PUSH, pad},
        {CALL, 10}, {RET, 0},
        {PUSH, 0}, {RET, 0},
        {LOAD, 0}, {LOAD, 1},
        {LOAD, 2}, {LOAD, 3},
        {STRCPY, 100}, {GETPID, 0}, 
        {STORE, 200}, {STORE, 204}, 
        {STORE, 208}, {RET, 0},
        {PRINTF, 300}, {RET, 0}
    };

    vm_run(&vm, program);
}

static void gen_challenge_vm(const char *addr, char *challenge) {
    VM vm;
    vm_init(&vm);

    Instruction program[] = {
        {PUSH, (int)addr}, {PUSH, (int)challenge},
        {CALL, 10}, {RET, 0},
        {LOAD, 0}, {STRCPY, 100},
        {TIME, 0}, 
        {STORE, 200}, {STORE, 204}, 
        {GETPID, 0}, {STORE, 208},
        {CALL, 20}, {RET, 0},
        {LOAD, 1}, {STRCPY, 300},
        {RET, 0}
    };

    vm_run(&vm, program);
}

char *auth_server_vm(int f_in, int f_out, int module, const char *host, const char *addr, const char *leader) {
    VM vm;
    vm_init(&vm);

    Instruction program[] = {
        {PUSH, f_in}, {PUSH, f_out}, {PUSH, module},
        {PUSH, (int)host}, {PUSH, (int)addr}, {PUSH, (int)leader},
        {CALL, 10}, {RET, 0},
        {LOAD, 0}, {LOAD, 1}, {LOAD, 2},
        {LOAD, 3}, {LOAD, 4}, {LOAD, 5},
        {STRCPY, 100}, {GETPID, 0}, 
        {STORE, 200}, {STORE, 204}, 
        {STORE, 208}, {RET, 0},
        {PRINTF, 300}, {RET, 0}
    };

    vm_run(&vm, program);
    return NULL;
}

void auth_client_vm(int fd, const char *user, const char *challenge) {
    VM vm;
    vm_init(&vm);

    Instruction program[] = {
        {PUSH, fd}, {PUSH, (int)user}, {PUSH, (int)challenge},
        {CALL, 10}, {RET, 0},
        {LOAD, 0}, {LOAD, 1}, {LOAD, 2},
        {STRCPY, 100}, {GETPID, 0}, 
        {STORE, 200}, {STORE, 204}, 
        {STORE, 208}, {RET, 0},
        {PRINTF, 300}, {RET, 0}
    };

    vm_run(&vm, program);
}