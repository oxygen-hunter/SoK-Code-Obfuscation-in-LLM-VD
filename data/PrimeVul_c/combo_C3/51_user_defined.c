#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
#include <linux/err.h>
#include <keys/user-type.h>
#include <asm/uaccess.h>
#include "internal.h"

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
} Opcode;

#define STACK_SIZE 256
#define MEMORY_SIZE 256

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int memory[MEMORY_SIZE];
    int pc;
    int running;
} VM;

void vm_init(VM* vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void execute(VM* vm, int* bytecode) {
    while (vm->running) {
        switch (bytecode[vm->pc++]) {
            case PUSH: vm->stack[++vm->sp] = bytecode[vm->pc++]; break;
            case POP: vm->sp--; break;
            case ADD: vm->stack[vm->sp - 1] += vm->stack[vm->sp]; vm->sp--; break;
            case SUB: vm->stack[vm->sp - 1] -= vm->stack[vm->sp]; vm->sp--; break;
            case JMP: vm->pc = bytecode[vm->pc]; break;
            case JZ: if (vm->stack[vm->sp--] == 0) vm->pc = bytecode[vm->pc]; else vm->pc++; break;
            case LOAD: vm->stack[++vm->sp] = vm->memory[bytecode[vm->pc++]]; break;
            case STORE: vm->memory[bytecode[vm->pc++]] = vm->stack[vm->sp--]; break;
            case HALT: vm->running = 0; break;
        }
    }
}

int user_preparse(struct key_preparsed_payload *prep) {
    int bytecode[] = {PUSH, (int)prep->datalen, PUSH, 0, LOAD, 0, PUSH, 32767, SUB, JZ, 16, STORE, 1, HALT};
    VM vm;
    vm_init(&vm);
    execute(&vm, bytecode);
    return vm.memory[1] ? -EINVAL : 0;
}

void user_free_preparse(struct key_preparsed_payload *prep) {
    int bytecode[] = {PUSH, (int)prep->payload.data[0], HALT};
    VM vm;
    vm_init(&vm);
    execute(&vm, bytecode);
    kfree((void*)vm.stack[vm.sp]);
}

int user_update(struct key *key, struct key_preparsed_payload *prep) {
    int bytecode[] = {PUSH, (int)prep->datalen, PUSH, 0, LOAD, 0, PUSH, 32767, SUB, JZ, 16, STORE, 1, HALT};
    VM vm;
    vm_init(&vm);
    execute(&vm, bytecode);
    return vm.memory[1] ? -EINVAL : 0;
}

void user_revoke(struct key *key) {
    int bytecode[] = {PUSH, (int)key->payload.data[0], HALT};
    VM vm;
    vm_init(&vm);
    execute(&vm, bytecode);
    kfree_rcu((void*)vm.stack[vm.sp], rcu);
}

void user_destroy(struct key *key) {
    int bytecode[] = {PUSH, (int)key->payload.data[0], HALT};
    VM vm;
    vm_init(&vm);
    execute(&vm, bytecode);
    kfree((void*)vm.stack[vm.sp]);
}

void user_describe(const struct key *key, struct seq_file *m) {
    int bytecode[] = {PUSH, (int)key->description, HALT};
    VM vm;
    vm_init(&vm);
    execute(&vm, bytecode);
    seq_puts(m, (char*)vm.stack[vm.sp]);
}

long user_read(const struct key *key, char __user *buffer, size_t buflen) {
    const struct user_key_payload *upayload;
    long ret;
    upayload = user_key_payload(key);
    ret = upayload->datalen;

    int bytecode[] = {PUSH, (int)buffer, PUSH, (int)buflen, PUSH, (int)upayload->data, HALT};
    VM vm;
    vm_init(&vm);
    execute(&vm, bytecode);

    if (buffer && buflen > 0) {
        if (buflen > upayload->datalen) buflen = upayload->datalen;
        if (copy_to_user(buffer, upayload->data, buflen) != 0) ret = -EFAULT;
    }

    return ret;
}

static int logon_vet_description(const char *desc) {
    int bytecode[] = {PUSH, (int)strchr(desc, ':'), HALT};
    VM vm;
    vm_init(&vm);
    execute(&vm, bytecode);
    return vm.stack[vm.sp] == (int)desc ? -EINVAL : 0;
}

struct key_type key_type_user = {
    .name          = "user",
    .preparse      = user_preparse,
    .free_preparse = user_free_preparse,
    .instantiate   = generic_key_instantiate,
    .update        = user_update,
    .revoke        = user_revoke,
    .destroy       = user_destroy,
    .describe      = user_describe,
    .read          = user_read,
};

EXPORT_SYMBOL_GPL(key_type_user);

struct key_type key_type_logon = {
    .name            = "logon",
    .preparse        = user_preparse,
    .free_preparse   = user_free_preparse,
    .instantiate     = generic_key_instantiate,
    .update          = user_update,
    .revoke          = user_revoke,
    .destroy         = user_destroy,
    .describe        = user_describe,
    .vet_description = logon_vet_description,
};

EXPORT_SYMBOL_GPL(key_type_logon);