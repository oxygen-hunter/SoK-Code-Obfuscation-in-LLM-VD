#include <linux/atomic.h>
#include <crypto/internal/rng.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/random.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/cryptouser.h>
#include <net/netlink.h>

#include "internal.h"

#define VM_STACK_SIZE 256
#define VM_PROGRAM_SIZE 512

typedef enum {
    VM_NOP,
    VM_PUSH,
    VM_POP,
    VM_ADD,
    VM_SUB,
    VM_JMP,
    VM_JZ,
    VM_LOAD,
    VM_STORE,
    VM_HALT
} VM_OpCode;

typedef struct {
    int stack[VM_STACK_SIZE];
    int sp;
    int program[VM_PROGRAM_SIZE];
    int pc;
} VM;

static void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

static void vm_push(VM *vm, int value) {
    if (vm->sp < VM_STACK_SIZE - 1) {
        vm->stack[++vm->sp] = value;
    }
}

static int vm_pop(VM *vm) {
    if (vm->sp >= 0) {
        return vm->stack[vm->sp--];
    }
    return 0;
}

static void vm_exec(VM *vm) {
    int running = 1;
    while (running) {
        switch (vm->program[vm->pc++]) {
            case VM_NOP:
                break;
            case VM_PUSH:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case VM_POP:
                vm_pop(vm);
                break;
            case VM_ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case VM_SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case VM_JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case VM_JZ: {
                int addr = vm->program[vm->pc++];
                if (vm_pop(vm) == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case VM_LOAD:
                vm_push(vm, vm->stack[vm->program[vm->pc++]]);
                break;
            case VM_STORE:
                vm->stack[vm->program[vm->pc++]] = vm_pop(vm);
                break;
            case VM_HALT:
                running = 0;
                break;
        }
    }
}

static void crypto_rng_vm_program(VM *vm, struct crypto_rng *tfm, const u8 *seed, unsigned int slen) {
    vm->program[0] = VM_PUSH;
    vm->program[1] = (int) tfm;
    vm->program[2] = VM_PUSH;
    vm->program[3] = (int) seed;
    vm->program[4] = VM_PUSH;
    vm->program[5] = slen;
    vm->program[6] = VM_STORE;
    vm->program[7] = 0;
    vm->program[8] = VM_HALT;
}

static DEFINE_MUTEX(crypto_default_rng_lock);
struct crypto_rng *crypto_default_rng;
EXPORT_SYMBOL_GPL(crypto_default_rng);
static int crypto_default_rng_refcnt;

int crypto_rng_reset(struct crypto_rng *tfm, const u8 *seed, unsigned int slen) {
    VM vm;
    vm_init(&vm);
    crypto_rng_vm_program(&vm, tfm, seed, slen);
    vm_exec(&vm);
    return vm.stack[0];
}
EXPORT_SYMBOL_GPL(crypto_rng_reset);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Random Number Generator");