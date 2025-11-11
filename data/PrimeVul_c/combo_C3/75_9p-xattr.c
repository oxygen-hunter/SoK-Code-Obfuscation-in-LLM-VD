#include "qemu/osdep.h"
#include "9p.h"
#include "fsdev/file-op-9p.h"
#include "9p-xattr.h"
#include "9p-util.h"
#include "9p-local.h"

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
} Instruction;

typedef struct {
    Instruction instr;
    intptr_t operand;
} VMInstruction;

typedef struct {
    VMInstruction *instructions;
    size_t ip;
    intptr_t stack[256];
    size_t sp;
} VM;

void vm_init(VM *vm, VMInstruction *instructions) {
    vm->instructions = instructions;
    vm->ip = 0;
    vm->sp = 0;
}

intptr_t vm_run(VM *vm) {
    while (1) {
        VMInstruction *instr = &vm->instructions[vm->ip++];
        switch (instr->instr) {
            case PUSH:
                vm->stack[vm->sp++] = instr->operand;
                break;
            case POP:
                vm->sp--;
                break;
            case ADD:
                vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1];
                vm->sp--;
                break;
            case SUB:
                vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1];
                vm->sp--;
                break;
            case JMP:
                vm->ip = instr->operand;
                break;
            case JZ:
                if (vm->stack[--vm->sp] == 0) {
                    vm->ip = instr->operand;
                }
                break;
            case LOAD:
                vm->stack[vm->sp++] = ((intptr_t*)instr->operand)[vm->stack[--vm->sp]];
                break;
            case STORE:
                ((intptr_t*)instr->operand)[vm->stack[--vm->sp]] = vm->stack[--vm->sp];
                break;
            case CALL:
                vm->stack[vm->sp++] = vm->ip;
                vm->ip = instr->operand;
                break;
            case RET:
                vm->ip = vm->stack[--vm->sp];
                break;
        }
        if (instr->instr == RET) {
            break;
        }
    }
    return vm->stack[--vm->sp];
}

static XattrOperations *get_xattr_operations(XattrOperations **h, const char *name) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)h},
        {PUSH, 0},
        {CALL, 3},
        {RET, 0},
        {LOAD, (intptr_t)name},
        {CALL, 12},
        {JZ, 10},
        {LOAD, (intptr_t)&h[0]},
        {PUSH, 1},
        {ADD, 0},
        {JMP, 0},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (XattrOperations *)vm_run(&vm);
}

ssize_t v9fs_get_xattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)ctx->xops},
        {LOAD, (intptr_t)name},
        {CALL, 0},
        {JZ, 9},
        {LOAD, (intptr_t)&ctx},
        {LOAD, (intptr_t)path},
        {LOAD, (intptr_t)name},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)size},
        {CALL, 6},
        {RET, 0},
        {LOAD, (intptr_t)&errno},
        {PUSH, EOPNOTSUPP},
        {STORE, 0},
        {PUSH, -1},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

ssize_t pt_listxattr(FsContext *ctx, const char *path, char *name, void *value, size_t size) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)name},
        {CALL, 3},
        {PUSH, 1},
        {ADD, 0},
        {STORE, 0},
        {LOAD, (intptr_t)value},
        {JZ, 8},
        {LOAD, (intptr_t)size},
        {LOAD, (intptr_t)&errno},
        {PUSH, ERANGE},
        {STORE, 0},
        {PUSH, -1},
        {RET, 0},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)name},
        {LOAD, (intptr_t)&name_size},
        {CALL, 1},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

static ssize_t flistxattrat_nofollow(int dirfd, const char *filename, char *list, size_t size) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)"/proc/self/fd/%d/%s"},
        {LOAD, (intptr_t)&dirfd},
        {LOAD, (intptr_t)filename},
        {CALL, 2},
        {STORE, 0},
        {LOAD, (intptr_t)proc_path},
        {LOAD, (intptr_t)list},
        {LOAD, (intptr_t)size},
        {CALL, 4},
        {STORE, 1},
        {LOAD, (intptr_t)proc_path},
        {CALL, 5},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

ssize_t v9fs_list_xattr(FsContext *ctx, const char *path, void *value, size_t vsize) {
    VMInstruction program[] = {
        {PUSH, 0},
        {STORE, 0},
        {LOAD, (intptr_t)value},
        {STORE, 1},
        {LOAD, (intptr_t)path},
        {CALL, 3},
        {STORE, 2},
        {LOAD, (intptr_t)ctx},
        {LOAD, (intptr_t)dirpath},
        {CALL, 2},
        {STORE, 3},
        {LOAD, (intptr_t)dirfd},
        {PUSH, -1},
        {JZ, 26},
        {LOAD, (intptr_t)path},
        {CALL, 3},
        {STORE, 4},
        {LOAD, (intptr_t)dirfd},
        {LOAD, (intptr_t)name},
        {LOAD, (intptr_t)value},
        {PUSH, 0},
        {CALL, 1},
        {STORE, 5},
        {LOAD, (intptr_t)xattr_len},
        {PUSH, 0},
        {JZ, 24},
        {LOAD, (intptr_t)name},
        {CALL, 5},
        {LOAD, (intptr_t)dirfd},
        {CALL, 6},
        {LOAD, (intptr_t)xattr_len},
        {RET, 0},
        {LOAD, (intptr_t)orig_value},
        {CALL, 7},
        {STORE, 6},
        {LOAD, (intptr_t)dirfd},
        {LOAD, (intptr_t)name},
        {LOAD, (intptr_t)orig_value},
        {LOAD, (intptr_t)xattr_len},
        {CALL, 1},
        {STORE, 5},
        {LOAD, (intptr_t)name},
        {CALL, 5},
        {LOAD, (intptr_t)dirfd},
        {CALL, 6},
        {LOAD, (intptr_t)xattr_len},
        {PUSH, 0},
        {JZ, 64},
        {LOAD, (intptr_t)orig_value},
        {STORE, 7},
        {LOAD, (intptr_t)xattr_len},
        {LOAD, (intptr_t)parsed_len},
        {SUB, 0},
        {PUSH, 0},
        {JZ, 64},
        {LOAD, (intptr_t)ctx->xops},
        {LOAD, (intptr_t)orig_value},
        {CALL, 0},
        {JZ, 56},
        {LOAD, (intptr_t)value},
        {JZ, 53},
        {LOAD, (intptr_t)xops},
        {LOAD, (intptr_t)ctx},
        {LOAD, (intptr_t)path},
        {LOAD, (intptr_t)orig_value},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)vsize},
        {CALL, 6},
        {STORE, 0},
        {LOAD, (intptr_t)size},
        {PUSH, 0},
        {JZ, 56},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)size},
        {ADD, 0},
        {STORE, 1},
        {LOAD, (intptr_t)vsize},
        {LOAD, (intptr_t)size},
        {SUB, 0},
        {STORE, 2},
        {JMP, 56},
        {LOAD, (intptr_t)orig_value},
        {CALL, 3},
        {STORE, 8},
        {LOAD, (intptr_t)parsed_len},
        {LOAD, (intptr_t)attr_len},
        {ADD, 0},
        {STORE, 3},
        {LOAD, (intptr_t)orig_value},
        {LOAD, (intptr_t)attr_len},
        {ADD, 0},
        {STORE, 4},
        {LOAD, (intptr_t)value},
        {JZ, 60},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)ovalue},
        {SUB, 0},
        {STORE, 0},
        {JMP, 64},
        {LOAD, (intptr_t)orig_value_start},
        {CALL, 5},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

int v9fs_set_xattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)ctx->xops},
        {LOAD, (intptr_t)name},
        {CALL, 0},
        {JZ, 7},
        {LOAD, (intptr_t)xops},
        {LOAD, (intptr_t)ctx},
        {LOAD, (intptr_t)path},
        {LOAD, (intptr_t)name},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)size},
        {LOAD, (intptr_t)flags},
        {CALL, 6},
        {RET, 0},
        {LOAD, (intptr_t)&errno},
        {PUSH, EOPNOTSUPP},
        {STORE, 0},
        {PUSH, -1},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (int)vm_run(&vm);
}

int v9fs_remove_xattr(FsContext *ctx, const char *path, const char *name) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)ctx->xops},
        {LOAD, (intptr_t)name},
        {CALL, 0},
        {JZ, 6},
        {LOAD, (intptr_t)xops},
        {LOAD, (intptr_t)ctx},
        {LOAD, (intptr_t)path},
        {LOAD, (intptr_t)name},
        {CALL, 5},
        {RET, 0},
        {LOAD, (intptr_t)&errno},
        {PUSH, EOPNOTSUPP},
        {STORE, 0},
        {PUSH, -1},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (int)vm_run(&vm);
}

ssize_t local_getxattr_nofollow(FsContext *ctx, const char *path, const char *name, void *value, size_t size) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)path},
        {CALL, 3},
        {STORE, 0},
        {LOAD, (intptr_t)path},
        {CALL, 3},
        {STORE, 1},
        {LOAD, (intptr_t)ctx},
        {LOAD, (intptr_t)dirpath},
        {CALL, 2},
        {STORE, 2},
        {LOAD, (intptr_t)dirfd},
        {PUSH, -1},
        {JZ, 16},
        {LOAD, (intptr_t)dirfd},
        {LOAD, (intptr_t)filename},
        {LOAD, (intptr_t)name},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)size},
        {CALL, 1},
        {STORE, 3},
        {LOAD, (intptr_t)dirfd},
        {CALL, 6},
        {JMP, 18},
        {LOAD, (intptr_t)dirpath},
        {CALL, 5},
        {LOAD, (intptr_t)filename},
        {CALL, 5},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

ssize_t pt_getxattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)ctx},
        {LOAD, (intptr_t)path},
        {LOAD, (intptr_t)name},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)size},
        {CALL, 6},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

int fsetxattrat_nofollow(int dirfd, const char *filename, const char *name, void *value, size_t size, int flags) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)"/proc/self/fd/%d/%s"},
        {LOAD, (intptr_t)&dirfd},
        {LOAD, (intptr_t)filename},
        {CALL, 2},
        {STORE, 0},
        {LOAD, (intptr_t)proc_path},
        {LOAD, (intptr_t)name},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)size},
        {LOAD, (intptr_t)flags},
        {CALL, 4},
        {STORE, 1},
        {LOAD, (intptr_t)proc_path},
        {CALL, 5},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (int)vm_run(&vm);
}

ssize_t local_setxattr_nofollow(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)path},
        {CALL, 3},
        {STORE, 0},
        {LOAD, (intptr_t)path},
        {CALL, 3},
        {STORE, 1},
        {LOAD, (intptr_t)ctx},
        {LOAD, (intptr_t)dirpath},
        {CALL, 2},
        {STORE, 2},
        {LOAD, (intptr_t)dirfd},
        {PUSH, -1},
        {JZ, 16},
        {LOAD, (intptr_t)dirfd},
        {LOAD, (intptr_t)filename},
        {LOAD, (intptr_t)name},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)size},
        {LOAD, (intptr_t)flags},
        {CALL, 1},
        {STORE, 3},
        {LOAD, (intptr_t)dirfd},
        {CALL, 6},
        {JMP, 18},
        {LOAD, (intptr_t)dirpath},
        {CALL, 5},
        {LOAD, (intptr_t)filename},
        {CALL, 5},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

int pt_setxattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)ctx},
        {LOAD, (intptr_t)path},
        {LOAD, (intptr_t)name},
        {LOAD, (intptr_t)value},
        {LOAD, (intptr_t)size},
        {LOAD, (intptr_t)flags},
        {CALL, 6},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (int)vm_run(&vm);
}

static ssize_t fremovexattrat_nofollow(int dirfd, const char *filename, const char *name) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)"/proc/self/fd/%d/%s"},
        {LOAD, (intptr_t)&dirfd},
        {LOAD, (intptr_t)filename},
        {CALL, 2},
        {STORE, 0},
        {LOAD, (intptr_t)proc_path},
        {LOAD, (intptr_t)name},
        {CALL, 4},
        {STORE, 1},
        {LOAD, (intptr_t)proc_path},
        {CALL, 5},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

ssize_t local_removexattr_nofollow(FsContext *ctx, const char *path, const char *name) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)path},
        {CALL, 3},
        {STORE, 0},
        {LOAD, (intptr_t)path},
        {CALL, 3},
        {STORE, 1},
        {LOAD, (intptr_t)ctx},
        {LOAD, (intptr_t)dirpath},
        {CALL, 2},
        {STORE, 2},
        {LOAD, (intptr_t)dirfd},
        {PUSH, -1},
        {JZ, 16},
        {LOAD, (intptr_t)dirfd},
        {LOAD, (intptr_t)filename},
        {LOAD, (intptr_t)name},
        {CALL, 1},
        {STORE, 3},
        {LOAD, (intptr_t)dirfd},
        {CALL, 6},
        {JMP, 18},
        {LOAD, (intptr_t)dirpath},
        {CALL, 5},
        {LOAD, (intptr_t)filename},
        {CALL, 5},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

int pt_removexattr(FsContext *ctx, const char *path, const char *name) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)ctx},
        {LOAD, (intptr_t)path},
        {LOAD, (intptr_t)name},
        {CALL, 6},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (int)vm_run(&vm);
}

ssize_t notsup_getxattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)&errno},
        {PUSH, ENOTSUP},
        {STORE, 0},
        {PUSH, -1},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

int notsup_setxattr(FsContext *ctx, const char *path, const char *name, void *value, size_t size, int flags) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)&errno},
        {PUSH, ENOTSUP},
        {STORE, 0},
        {PUSH, -1},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (int)vm_run(&vm);
}

ssize_t notsup_listxattr(FsContext *ctx, const char *path, char *name, void *value, size_t size) {
    VMInstruction program[] = {
        {PUSH, 0},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (ssize_t)vm_run(&vm);
}

int notsup_removexattr(FsContext *ctx, const char *path, const char *name) {
    VMInstruction program[] = {
        {LOAD, (intptr_t)&errno},
        {PUSH, ENOTSUP},
        {STORE, 0},
        {PUSH, -1},
        {RET, 0}
    };
    VM vm;
    vm_init(&vm, program);
    return (int)vm_run(&vm);
}

XattrOperations *mapped_xattr_ops[] = {
    &mapped_user_xattr,
    &mapped_pacl_xattr,
    &mapped_dacl_xattr,
    NULL,
};

XattrOperations *passthrough_xattr_ops[] = {
    &passthrough_user_xattr,
    &passthrough_acl_xattr,
    NULL,
};

XattrOperations *none_xattr_ops[] = {
    &passthrough_user_xattr,
    &none_acl_xattr,
    NULL,
};