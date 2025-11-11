#include <linux/random.h>
#include <linux/string.h>
#include <linux/fscrypto.h>

#define STACK_SIZE 1024
#define PROGRAM_SIZE 1024

typedef struct {
    int stack[STACK_SIZE];
    int sp;
    int pc;
    int program[PROGRAM_SIZE];
} VM;

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

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

void vm_run(VM *vm) {
    int running = 1;
    while (running) {
        int instr = vm->program[vm->pc++];
        switch (instr) {
            case PUSH: vm_push(vm, vm->program[vm->pc++]); break;
            case POP: vm_pop(vm); break;
            case ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP: vm->pc = vm->program[vm->pc]; break;
            case JZ: {
                int addr = vm->program[vm->pc++];
                if (vm_pop(vm) == 0) vm->pc = addr;
                break;
            }
            case LOAD: {
                int addr = vm->program[vm->pc++];
                vm_push(vm, vm->stack[addr]);
                break;
            }
            case STORE: {
                int addr = vm->program[vm->pc++];
                vm->stack[addr] = vm_pop(vm);
                break;
            }
            case CALL: {
                int addr = vm->program[vm->pc++];
                vm_push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case RET: {
                vm->pc = vm_pop(vm);
                break;
            }
            default: running = 0; break;
        }
    }
}

void compile_fscrypt_process_policy(VM *vm) {
    vm->program[0] = PUSH;
    vm->program[1] = 0; // policy->version
    vm->program[2] = JZ;
    vm->program[3] = 7;
    vm->program[4] = PUSH;
    vm->program[5] = -22; // -EINVAL
    vm->program[6] = RET;

    // inode_has_encryption_context(inode)
    vm->program[7] = CALL;
    vm->program[8] = 10;
    vm->program[9] = JZ;
    vm->program[10] = 20;

    // create_encryption_context_from_policy(inode, policy)
    vm->program[11] = CALL;
    vm->program[12] = 15;
    vm->program[13] = RET;

    // is_encryption_context_consistent_with_policy(inode, policy)
    vm->program[20] = CALL;
    vm->program[21] = 25;
    vm->program[22] = JZ;
    vm->program[23] = 30;

    vm->program[24] = PUSH;
    vm->program[25] = 0; // return 0
    vm->program[26] = RET;

    // printk(KERN_WARNING)
    vm->program[30] = PUSH;
    vm->program[31] = -22; // return -EINVAL
    vm->program[32] = RET;
}

int fscrypt_process_policy(struct inode *inode,
                           const struct fscrypt_policy *policy) {
    VM vm;
    vm_init(&vm);
    compile_fscrypt_process_policy(&vm);
    vm_run(&vm);
    return vm_pop(&vm);
}

EXPORT_SYMBOL(fscrypt_process_policy);