#include <crypto/algapi.h>
#include <crypto/gf128mul.h>
#include <crypto/internal/hash.h>
#include <linux/crypto.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define GHASH_BLOCK_SIZE	16
#define GHASH_DIGEST_SIZE	16

enum {
    VM_PUSH, VM_POP, VM_ADD, VM_SUB, VM_JMP, VM_JZ, VM_LOAD, VM_STORE, VM_XOR, VM_GF128MUL, VM_MEMSET, VM_MEMCPY, VM_SETFLAG, VM_RET
};

typedef struct {
    int pc;
    int sp;
    unsigned char stack[256];
    unsigned char memory[256];
    unsigned int flags;
} VM;

void vm_run(VM *vm, unsigned char *program) {
    while (1) {
        switch (program[vm->pc++]) {
            case VM_PUSH:
                vm->stack[vm->sp++] = program[vm->pc++];
                break;
            case VM_POP:
                --vm->sp;
                break;
            case VM_ADD:
                vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1];
                --vm->sp;
                break;
            case VM_SUB:
                vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1];
                --vm->sp;
                break;
            case VM_JMP:
                vm->pc = program[vm->pc];
                break;
            case VM_JZ:
                if (vm->stack[--vm->sp] == 0)
                    vm->pc = program[vm->pc];
                else
                    vm->pc++;
                break;
            case VM_LOAD:
                vm->stack[vm->sp++] = vm->memory[program[vm->pc++]];
                break;
            case VM_STORE:
                vm->memory[program[vm->pc++]] = vm->stack[--vm->sp];
                break;
            case VM_XOR:
                vm->stack[vm->sp - 2] ^= vm->stack[vm->sp - 1];
                --vm->sp;
                break;
            case VM_GF128MUL:
                gf128mul_4k_lle((be128 *)&vm->memory[program[vm->pc]], (struct gf128mul_4k *)vm->stack[--vm->sp]);
                vm->pc += GHASH_BLOCK_SIZE;
                break;
            case VM_MEMSET:
                memset(&vm->memory[program[vm->pc]], 0, program[vm->pc + 1]);
                vm->pc += 2;
                break;
            case VM_MEMCPY:
                memcpy(&vm->memory[program[vm->pc]], &vm->memory[program[vm->pc + 1]], program[vm->pc + 2]);
                vm->pc += 3;
                break;
            case VM_SETFLAG:
                vm->flags = program[vm->pc++];
                break;
            case VM_RET:
                return;
        }
    }
}

struct ghash_ctx {
    struct gf128mul_4k *gf128;
};

struct ghash_desc_ctx {
    u8 buffer[GHASH_BLOCK_SIZE];
    u32 bytes;
};

static int ghash_init(struct shash_desc *desc) {
    VM vm = {0};
    unsigned char program[] = {VM_PUSH, 0, VM_LOAD, 1, VM_MEMSET, 2, GHASH_BLOCK_SIZE, VM_RET};
    vm.memory[1] = (unsigned char)(uintptr_t)shash_desc_ctx(desc);
    vm_run(&vm, program);
    return 0;
}

static int ghash_setkey(struct crypto_shash *tfm, const u8 *key, unsigned int keylen) {
    VM vm = {0};
    unsigned char program[] = {
        VM_PUSH, 0, VM_LOAD, 1, VM_LOAD, 2, VM_PUSH, GHASH_BLOCK_SIZE, VM_SUB, VM_JZ, 10,
        VM_LOAD, 3, VM_SETFLAG, CRYPTO_TFM_RES_BAD_KEY_LEN, VM_PUSH, -EINVAL, VM_RET,
        VM_LOAD, 4, VM_LOAD, 5, VM_JZ, 22, VM_PUSH, 6, VM_MEMSET, 7, 1, VM_PUSH, -ENOMEM, VM_RET,
        VM_LOAD, 8, VM_PUSH, 9, VM_STORE, 7, VM_RET
    };
    vm.memory[1] = (unsigned char)(uintptr_t)crypto_shash_ctx(tfm);
    vm.memory[2] = (unsigned char)keylen;
    vm.memory[3] = (unsigned char)(uintptr_t)tfm;
    vm.memory[4] = (unsigned char)(uintptr_t)crypto_shash_ctx(tfm);
    vm.memory[5] = (unsigned char)(uintptr_t)gf128mul_free_4k;
    vm.memory[6] = (unsigned char)(uintptr_t)gf128mul_init_4k_lle;
    vm.memory[7] = (unsigned char)(uintptr_t)gf128mul_init_4k_lle((be128 *)key);
    vm.memory[8] = (unsigned char)(uintptr_t)crypto_shash_ctx(tfm);
    vm.memory[9] = (unsigned char)(uintptr_t)gf128mul_init_4k_lle((be128 *)key);
    vm_run(&vm, program);
    return vm.stack[vm.sp - 1];
}

static int ghash_update(struct shash_desc *desc, const u8 *src, unsigned int srclen) {
    VM vm = {0};
    unsigned char program[] = {
        VM_PUSH, 0, VM_LOAD, 1, VM_LOAD, 2, VM_LOAD, 3, VM_LOAD, 4,
        VM_LOAD, 5, VM_PUSH, GHASH_BLOCK_SIZE, VM_SUB, VM_JZ, 14,
        VM_PUSH, 6, VM_MEMCPY, 7, 8, 9, VM_GF128MUL, 10, VM_JMP, 30,
        VM_PUSH, 11, VM_XOR, 12, VM_GF128MUL, 13, VM_JMP, 30,
        VM_PUSH, 14, VM_STORE, 15, VM_JMP, 30,
        VM_RET
    };
    vm.memory[1] = (unsigned char)(uintptr_t)shash_desc_ctx(desc);
    vm.memory[2] = (unsigned char)(uintptr_t)src;
    vm.memory[3] = (unsigned char)(uintptr_t)crypto_shash_ctx(desc->tfm);
    vm.memory[4] = (unsigned char)(uintptr_t)shash_desc_ctx(desc);
    vm.memory[5] = (unsigned char)(uintptr_t)src;
    vm.memory[6] = (unsigned char)(uintptr_t)min(srclen, ((struct ghash_desc_ctx *)shash_desc_ctx(desc))->bytes);
    vm.memory[7] = (unsigned char)(uintptr_t)dctx->buffer + (GHASH_BLOCK_SIZE - ((struct ghash_desc_ctx *)shash_desc_ctx(desc))->bytes);
    vm.memory[8] = (unsigned char)(uintptr_t)src;
    vm.memory[9] = (unsigned char)(uintptr_t)gf128mul_4k_lle;
    vm.memory[10] = (unsigned char)(uintptr_t)gf128mul_4k_lle;
    vm.memory[11] = (unsigned char)(uintptr_t)dctx->buffer;
    vm.memory[12] = (unsigned char)(uintptr_t)src;
    vm.memory[13] = (unsigned char)(uintptr_t)gf128mul_4k_lle;
    vm.memory[14] = (unsigned char)(uintptr_t)dctx->bytes = GHASH_BLOCK_SIZE - srclen;
    vm.memory[15] = (unsigned char)(uintptr_t)src;
    vm_run(&vm, program);
    return vm.stack[vm.sp - 1];
}

static void ghash_flush(struct ghash_ctx *ctx, struct ghash_desc_ctx *dctx) {
    VM vm = {0};
    unsigned char program[] = {
        VM_PUSH, 0, VM_LOAD, 1, VM_LOAD, 2, VM_LOAD, 3, VM_JZ, 12,
        VM_PUSH, 4, VM_XOR, 5, VM_PUSH, 6, VM_GF128MUL, 3,
        VM_STORE, 7, VM_RET
    };
    vm.memory[1] = (unsigned char)(uintptr_t)dctx;
    vm.memory[2] = (unsigned char)(uintptr_t)ctx;
    vm.memory[3] = (unsigned char)(uintptr_t)dctx->bytes;
    vm.memory[4] = (unsigned char)(uintptr_t)dctx->buffer + (GHASH_BLOCK_SIZE - dctx->bytes);
    vm.memory[5] = (unsigned char)(uintptr_t)dctx;
    vm.memory[6] = (unsigned char)(uintptr_t)gf128mul_4k_lle;
    vm.memory[7] = (unsigned char)(uintptr_t)dctx->bytes = 0;
    vm_run(&vm, program);
}

static int ghash_final(struct shash_desc *desc, u8 *dst) {
    VM vm = {0};
    unsigned char program[] = {
        VM_PUSH, 0, VM_LOAD, 1, VM_LOAD, 2, VM_LOAD, 3,
        VM_MEMCPY, 4, GHASH_BLOCK_SIZE, VM_RET
    };
    vm.memory[1] = (unsigned char)(uintptr_t)shash_desc_ctx(desc);
    vm.memory[2] = (unsigned char)(uintptr_t)crypto_shash_ctx(desc->tfm);
    vm.memory[3] = (unsigned char)(uintptr_t)dctx->buffer;
    vm.memory[4] = (unsigned char)(uintptr_t)dst;
    vm_run(&vm, program);
    return 0;
}

static void ghash_exit_tfm(struct crypto_tfm *tfm) {
    VM vm = {0};
    unsigned char program[] = {
        VM_PUSH, 0, VM_LOAD, 1, VM_JZ, 6,
        VM_PUSH, 2, VM_MEMSET, 3, 1, VM_RET
    };
    vm.memory[1] = (unsigned char)(uintptr_t)crypto_tfm_ctx(tfm);
    vm.memory[2] = (unsigned char)(uintptr_t)gf128mul_free_4k;
    vm.memory[3] = (unsigned char)(uintptr_t)crypto_tfm_ctx(tfm);
    vm_run(&vm, program);
}

static struct shash_alg ghash_alg = {
    .digestsize	= GHASH_DIGEST_SIZE,
    .init		= ghash_init,
    .update		= ghash_update,
    .final		= ghash_final,
    .setkey		= ghash_setkey,
    .descsize	= sizeof(struct ghash_desc_ctx),
    .base		= {
        .cra_name		= "ghash",
        .cra_driver_name	= "ghash-generic",
        .cra_priority		= 100,
        .cra_flags		= CRYPTO_ALG_TYPE_SHASH,
        .cra_blocksize		= GHASH_BLOCK_SIZE,
        .cra_ctxsize		= sizeof(struct ghash_ctx),
        .cra_module		= THIS_MODULE,
        .cra_list		= LIST_HEAD_INIT(ghash_alg.base.cra_list),
        .cra_exit		= ghash_exit_tfm,
    },
};

static int __init ghash_mod_init(void) {
    return crypto_register_shash(&ghash_alg);
}

static void __exit ghash_mod_exit(void) {
    crypto_unregister_shash(&ghash_alg);
}

module_init(ghash_mod_init);
module_exit(ghash_mod_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("GHASH Message Digest Algorithm");
MODULE_ALIAS("ghash");