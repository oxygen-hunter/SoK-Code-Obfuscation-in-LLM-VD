#include "virtgpu_drv.h"

enum VM_INSTRUCTIONS { 
    VM_HALT, VM_PUSH, VM_POP, VM_ADD, VM_SUB, VM_JMP, VM_JZ, VM_LOAD, VM_STORE, VM_CALL, VM_RET 
};

struct VM {
    int stack[256];
    int sp;
    int pc;
    int halt;
};

void vm_init(struct VM* vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->halt = 0;
}

void vm_push(struct VM* vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(struct VM* vm) {
    return vm->stack[vm->sp--];
}

void vm_execute(struct VM* vm, int* code) {
    while (!vm->halt) {
        switch (code[vm->pc++]) {
            case VM_HALT:
                vm->halt = 1;
                break;
            case VM_PUSH:
                vm_push(vm, code[vm->pc++]);
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
                vm->pc = code[vm->pc];
                break;
            case VM_JZ: {
                int addr = code[vm->pc++];
                if (vm_pop(vm) == 0) vm->pc = addr;
                break;
            }
            case VM_LOAD:
                vm_push(vm, vm->stack[vm_pop(vm)]);
                break;
            case VM_STORE:
                vm->stack[vm_pop(vm)] = vm_pop(vm);
                break;
            case VM_CALL: {
                int addr = code[vm->pc++];
                vm_push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case VM_RET:
                vm->pc = vm_pop(vm);
                break;
        }
    }
}

void virtio_gpu_ttm_bo_destroy(struct ttm_buffer_object *tbo) {
    int code[] = {
        VM_PUSH, (int)tbo,
        VM_CALL, 6,
        VM_HALT,

        // Destroy logic
        VM_POP,
        VM_PUSH, 0,
        VM_STORE,
        VM_RET
    };

    struct VM vm;
    vm_init(&vm);
    vm_execute(&vm, code);
}

void virtio_gpu_init_ttm_placement(struct virtio_gpu_object *vgbo, bool pinned) {
    int code[] = {
        VM_PUSH, (int)vgbo,
        VM_PUSH, pinned,
        VM_CALL, 6,
        VM_HALT,

        // Init placement logic
        VM_POP,
        VM_POP,
        VM_PUSH, 1,
        VM_PUSH, 0,
        VM_STORE,
        VM_PUSH, (int)&vgbo->placement_code,
        VM_STORE,
        VM_PUSH, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_STORE,
        VM_PUSH, TTM_PL_MASK_CACHING | TTM_PL_FLAG_TT,
        VM_STORE,
        VM_PUSH, 1,
        VM_STORE,
        VM_PUSH, 1,
        VM_STORE,
        VM_RET
    };

    struct VM vm;
    vm_init(&vm);
    vm_execute(&vm, code);
}

int virtio_gpu_object_create(struct virtio_gpu_device *vgdev, unsigned long size, bool kernel, bool pinned, struct virtio_gpu_object **bo_ptr) {
    int code[] = {
        VM_PUSH, (int)vgdev,
        VM_PUSH, size,
        VM_PUSH, kernel,
        VM_PUSH, pinned,
        VM_PUSH, (int)bo_ptr,
        VM_CALL, 6,
        VM_HALT,

        // Object creation logic
        VM_POP,  // bo_ptr
        VM_POP,  // pinned
        VM_POP,  // kernel
        VM_POP,  // size
        VM_POP,  // vgdev
        VM_PUSH, (int)NULL,
        VM_STORE,
        VM_PUSH, (int)sizeof(struct virtio_gpu_object),
        VM_PUSH, GFP_KERNEL,
        VM_ADD,
        VM_PUSH, (int)kzalloc,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_JZ, 19,
        VM_PUSH, -ENOMEM,
        VM_RET,
        VM_PUSH, (int)roundup,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, (int)drm_gem_object_init,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_JZ, 31,
        VM_RET,
        VM_PUSH, (int)virtio_gpu_init_ttm_placement,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, (int)ttm_bo_init,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_JZ, 45,
        VM_RET,
        VM_PUSH, 0,
        VM_STORE,
        VM_RET
    };

    struct VM vm;
    vm_init(&vm);
    vm_execute(&vm, code);

    return 0;
}

int virtio_gpu_object_kmap(struct virtio_gpu_object *bo, void **ptr) {
    int code[] = {
        VM_PUSH, (int)bo,
        VM_PUSH, (int)ptr,
        VM_CALL, 6,
        VM_HALT,

        // Kmap logic
        VM_POP,  // ptr
        VM_POP,  // bo
        VM_PUSH, 0,
        VM_STORE,
        VM_PUSH, (int)ttm_bo_kmap,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_JZ, 19,
        VM_RET,
        VM_PUSH, (int)ttm_kmap_obj_virtual,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_STORE,
        VM_RET
    };

    struct VM vm;
    vm_init(&vm);
    vm_execute(&vm, code);

    return 0;
}

int virtio_gpu_object_get_sg_table(struct virtio_gpu_device *qdev, struct virtio_gpu_object *bo) {
    int code[] = {
        VM_PUSH, (int)qdev,
        VM_PUSH, (int)bo,
        VM_CALL, 6,
        VM_HALT,

        // Get sg_table logic
        VM_POP,  // bo
        VM_POP,  // qdev
        VM_PUSH, 0,
        VM_STORE,
        VM_PUSH, (int)kmalloc,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_JZ, 19,
        VM_PUSH, -ENOMEM,
        VM_RET,
        VM_PUSH, (int)sg_alloc_table_from_pages,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_JZ, 31,
        VM_RET,
        VM_PUSH, 0,
        VM_RET
    };

    struct VM vm;
    vm_init(&vm);
    vm_execute(&vm, code);

    return -ENOMEM;
}

void virtio_gpu_object_free_sg_table(struct virtio_gpu_object *bo) {
    int code[] = {
        VM_PUSH, (int)bo,
        VM_CALL, 6,
        VM_HALT,

        // Free sg_table logic
        VM_POP,  // bo
        VM_PUSH, (int)sg_free_table,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_RET
    };

    struct VM vm;
    vm_init(&vm);
    vm_execute(&vm, code);
}

int virtio_gpu_object_wait(struct virtio_gpu_object *bo, bool no_wait) {
    int code[] = {
        VM_PUSH, (int)bo,
        VM_PUSH, no_wait,
        VM_CALL, 6,
        VM_HALT,

        // Wait logic
        VM_POP,  // no_wait
        VM_POP,  // bo
        VM_PUSH, (int)ttm_bo_reserve,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_JZ, 19,
        VM_RET,
        VM_PUSH, (int)ttm_bo_wait,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, (int)ttm_bo_unreserve,
        VM_CALL, 0,
        VM_STORE,
        VM_PUSH, 0,
        VM_RET
    };

    struct VM vm;
    vm_init(&vm);
    vm_execute(&vm, code);

    return 0;
}