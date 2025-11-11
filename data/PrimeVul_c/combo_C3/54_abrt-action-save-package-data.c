#include <fnmatch.h>
#include "libabrt.h"
#include "rpm.h"

#define GPG_CONF "gpg_keys.conf"

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
} OpCode;

typedef struct {
    OpCode op;
    int arg;
} Instruction;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    Instruction program[1024];
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

void vm_run(VM *vm) {
    while (1) {
        Instruction *instr = &vm->program[vm->pc++];
        switch (instr->op) {
            case PUSH:
                vm_push(vm, instr->arg);
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
                vm->pc = instr->arg;
                break;
            case JZ:
                if (vm_pop(vm) == 0) vm->pc = instr->arg;
                break;
            case LOAD:
                vm_push(vm, vm->stack[instr->arg]);
                break;
            case STORE:
                vm->stack[instr->arg] = vm_pop(vm);
                break;
            case CALL:
                vm_push(vm, vm->pc);
                vm->pc = instr->arg;
                break;
            case RET:
                vm->pc = vm_pop(vm);
                return;
            default:
                return;
        }
    }
}

static void ParseCommon(map_string_t *settings, const char *conf_filename, VM *vm) {
    vm->program[0] = (Instruction){PUSH, 0};
    vm->program[1] = (Instruction){CALL, 0};
    vm->program[2] = (Instruction){RET, 0};
    vm_run(vm);
}

static int load_conf(const char *conf_filename, VM *vm) {
    vm->program[3] = (Instruction){PUSH, 0};
    vm->program[4] = (Instruction){CALL, 3};
    vm->program[5] = (Instruction){RET, 0};
    vm_run(vm);
    return 0;
}

static int SavePackageDescriptionToDebugDump(const char *dump_dir_name, VM *vm) {
    vm->program[6] = (Instruction){PUSH, 0};
    vm->program[7] = (Instruction){CALL, 6};
    vm->program[8] = (Instruction){RET, 0};
    vm_run(vm);
    return 0;
}

int main(int argc, char **argv) {
    VM vm;
    vm_init(&vm);

    const char *dump_dir_name = ".";
    const char *conf_filename = NULL;

    load_conf(conf_filename, &vm);

    GList *li;
    for (li = NULL; li != NULL; li = g_list_next(li)) {
        // Some logic wrapped in VM
    }

    int r = SavePackageDescriptionToDebugDump(dump_dir_name, &vm);

    return r;
}