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
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, XOR, GF128MUL, MEMSET, MEMCPY, RETURN
};

struct ghash_ctx {
    struct gf128mul_4k *gf128;
};

struct ghash_desc_ctx {
    u8 buffer[GHASH_BLOCK_SIZE];
    u32 bytes;
};

struct VM {
    int stack[256];
    int sp;
    unsigned int pc;
    unsigned char *program;
    unsigned char memory[256];
};

void vm_init(struct VM *vm, unsigned char *program) {
    vm->sp = -1;
    vm->pc = 0;
    vm->program = program;
}

int vm_run(struct VM *vm) {
    unsigned char instr;
    while ((instr = vm->program[vm->pc++])) {
        switch (instr) {
            case PUSH:
                vm->stack[++vm->sp] = vm->program[vm->pc++];
                break;
            case POP:
                vm->sp--;
                break;
            case ADD:
                vm->stack[vm->sp - 1] = vm->stack[vm->sp - 1] + vm->stack[vm->sp];
                vm->sp--;
                break;
            case SUB:
                vm->stack[vm->sp - 1] = vm->stack[vm->sp - 1] - vm->stack[vm->sp];
                vm->sp--;
                break;
            case JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case JZ:
                if (vm->stack[vm->sp--] == 0)
                    vm->pc = vm->program[vm->pc];
                else
                    vm->pc++;
                break;
            case LOAD:
                vm->stack[++vm->sp] = vm->memory[vm->program[vm->pc++]];
                break;
            case STORE:
                vm->memory[vm->program[vm->pc++]] = vm->stack[vm->sp--];
                break;
            case XOR:
                vm->stack[vm->sp - 1] ^= vm->stack[vm->sp];
                vm->sp--;
                break;
            case GF128MUL:
                gf128mul_4k_lle((be128 *)&vm->memory[0], (struct gf128mul_4k *)vm->memory[1]);
                break;
            case MEMSET:
                memset(&vm->memory[0], vm->program[vm->pc++], vm->program[vm->pc++]);
                break;
            case MEMCPY:
                memcpy(&vm->memory[0], &vm->memory[vm->program[vm->pc++]], vm->program[vm->pc++]);
                break;
            case RETURN:
                return vm->stack[vm->sp];
        }
    }
    return 0;
}

static int ghash_vm_interpreter(struct shash_desc *desc, unsigned char *program) {
    struct ghash_desc_ctx *dctx = shash_desc_ctx(desc);
    struct ghash_ctx *ctx = crypto_shash_ctx(desc->tfm);
    struct VM vm;
    vm_init(&vm, program);
    vm.memory[0] = (unsigned char)dctx->buffer;
    vm.memory[1] = (unsigned char)ctx->gf128;
    return vm_run(&vm);
}

static unsigned char ghash_program_init[] = {
    PUSH, 0, MEMSET, 0, GHASH_BLOCK_SIZE, RETURN
};

static unsigned char ghash_program_setkey[] = {
    PUSH, GHASH_BLOCK_SIZE, LOAD, 0, SUB, PUSH, 0, JZ, 14, PUSH, CRYPTO_TFM_RES_BAD_KEY_LEN, STORE, 0, PUSH, -EINVAL, RETURN,
    LOAD, 1, JZ, 24, PUSH, 0, GF128MUL, STORE, 1, PUSH, -ENOMEM, RETURN, RETURN
};

static unsigned char ghash_program_update[] = {
    LOAD, 2, JZ, 30, LOAD, 3, PUSH, GHASH_BLOCK_SIZE, LOAD, 2, SUB, STORE, 4, LOAD, 2, SUB, STORE, 2, MEMCPY, 0, 4, GF128MUL, RETURN,
    RETURN
};

static unsigned char ghash_program_final[] = {
    PUSH, 0, MEMCPY, 0, GHASH_BLOCK_SIZE, RETURN
};

static int ghash_init(struct shash_desc *desc) {
    return ghash_vm_interpreter(desc, ghash_program_init);
}

static int ghash_setkey(struct crypto_shash *tfm, const u8 *key, unsigned int keylen) {
    return ghash_vm_interpreter((struct shash_desc *)tfm, ghash_program_setkey);
}

static int ghash_update(struct shash_desc *desc, const u8 *src, unsigned int srclen) {
    return ghash_vm_interpreter(desc, ghash_program_update);
}

static int ghash_final(struct shash_desc *desc, u8 *dst) {
    return ghash_vm_interpreter(desc, ghash_program_final);
}

static void ghash_exit_tfm(struct crypto_tfm *tfm) {
    struct ghash_ctx *ctx = crypto_tfm_ctx(tfm);
    if (ctx->gf128)
        gf128mul_free_4k(ctx->gf128);
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