#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/bootmem.h>

#include "blk.h"

#define MAX_STACK_SIZE 256

typedef enum {
    VM_PUSH, VM_POP, VM_ADD, VM_SUB, VM_JMP, VM_JZ, VM_LOAD, VM_STORE,
    VM_CALL_CFQ_DTOR, VM_CALL_CFQ_EXIT, VM_CALL_PUT_IO_CONTEXT,
    VM_CALL_ALLOC_IO_CONTEXT, VM_CALL_EXIT_IO_CONTEXT,
    VM_CALL_CURRENT_IO_CONTEXT, VM_CALL_GET_IO_CONTEXT,
    VM_CALL_COPY_IO_CONTEXT, VM_RET
} VM_OPCODES;

typedef struct {
    int stack[MAX_STACK_SIZE];
    int sp;
    int pc;
} VM;

static struct kmem_cache *iocontext_cachep;

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

void vm_execute(VM *vm, int *program) {
    int running = 1;
    while (running) {
        switch (program[vm->pc++]) {
            case VM_PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case VM_POP:
                vm_pop(vm);
                break;
            case VM_ADD:
                vm_push(vm, vm_pop(vm) + vm_pop(vm));
                break;
            case VM_SUB:
                vm_push(vm, vm_pop(vm) - vm_pop(vm));
                break;
            case VM_JMP:
                vm->pc = program[vm->pc];
                break;
            case VM_JZ:
                if (vm_pop(vm) == 0)
                    vm->pc = program[vm->pc];
                else
                    vm->pc++;
                break;
            case VM_LOAD:
                vm_push(vm, program[vm->pc++]);
                break;
            case VM_STORE:
                program[vm->pc++] = vm_pop(vm);
                break;
            case VM_CALL_CFQ_DTOR:
                cfq_dtor((struct io_context *)vm_pop(vm));
                break;
            case VM_CALL_CFQ_EXIT:
                cfq_exit((struct io_context *)vm_pop(vm));
                break;
            case VM_CALL_PUT_IO_CONTEXT:
                vm_push(vm, put_io_context((struct io_context *)vm_pop(vm)));
                break;
            case VM_CALL_ALLOC_IO_CONTEXT:
                vm_push(vm, (int)alloc_io_context((gfp_t)vm_pop(vm), vm_pop(vm)));
                break;
            case VM_CALL_EXIT_IO_CONTEXT:
                exit_io_context();
                break;
            case VM_CALL_CURRENT_IO_CONTEXT:
                vm_push(vm, (int)current_io_context((gfp_t)vm_pop(vm), vm_pop(vm)));
                break;
            case VM_CALL_GET_IO_CONTEXT:
                vm_push(vm, (int)get_io_context((gfp_t)vm_pop(vm), vm_pop(vm)));
                break;
            case VM_CALL_COPY_IO_CONTEXT:
                copy_io_context((struct io_context **)vm_pop(vm), (struct io_context **)vm_pop(vm));
                break;
            case VM_RET:
                running = 0;
                break;
            default:
                running = 0;
                break;
        }
    }
}

static void cfq_dtor(struct io_context *ioc) {
    int program[] = {
        VM_PUSH, (int)ioc,
        VM_CALL_CFQ_DTOR,
        VM_RET
    };
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, program);
}

int put_io_context(struct io_context *ioc) {
    int program[] = {
        VM_PUSH, (int)ioc,
        VM_CALL_PUT_IO_CONTEXT,
        VM_RET
    };
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, program);
    return vm_pop(&vm);
}
EXPORT_SYMBOL(put_io_context);

static void cfq_exit(struct io_context *ioc) {
    int program[] = {
        VM_PUSH, (int)ioc,
        VM_CALL_CFQ_EXIT,
        VM_RET
    };
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, program);
}

void exit_io_context(void) {
    int program[] = {
        VM_CALL_EXIT_IO_CONTEXT,
        VM_RET
    };
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, program);
}

struct io_context *alloc_io_context(gfp_t gfp_flags, int node) {
    int program[] = {
        VM_PUSH, (int)node,
        VM_PUSH, (int)gfp_flags,
        VM_CALL_ALLOC_IO_CONTEXT,
        VM_RET
    };
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, program);
    return (struct io_context *)vm_pop(&vm);
}

struct io_context *current_io_context(gfp_t gfp_flags, int node) {
    int program[] = {
        VM_PUSH, (int)node,
        VM_PUSH, (int)gfp_flags,
        VM_CALL_CURRENT_IO_CONTEXT,
        VM_RET
    };
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, program);
    return (struct io_context *)vm_pop(&vm);
}

struct io_context *get_io_context(gfp_t gfp_flags, int node) {
    int program[] = {
        VM_PUSH, (int)node,
        VM_PUSH, (int)gfp_flags,
        VM_CALL_GET_IO_CONTEXT,
        VM_RET
    };
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, program);
    return (struct io_context *)vm_pop(&vm);
}
EXPORT_SYMBOL(get_io_context);

void copy_io_context(struct io_context **pdst, struct io_context **psrc) {
    int program[] = {
        VM_PUSH, (int)psrc,
        VM_PUSH, (int)pdst,
        VM_CALL_COPY_IO_CONTEXT,
        VM_RET
    };
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, program);
}
EXPORT_SYMBOL(copy_io_context);

static int __init blk_ioc_init(void) {
    int program[] = {
        VM_RET
    };
    iocontext_cachep = kmem_cache_create("blkdev_ioc",
            sizeof(struct io_context), 0, SLAB_PANIC, NULL);
    VM vm;
    vm_init(&vm);
    vm_execute(&vm, program);
    return 0;
}
subsys_initcall(blk_ioc_init);