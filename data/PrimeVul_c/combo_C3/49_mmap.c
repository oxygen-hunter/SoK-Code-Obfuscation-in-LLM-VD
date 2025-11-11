#include <linux/personality.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/limits.h>
#include <linux/sched.h>
#include <asm/elf.h>

struct va_alignment __read_mostly va_align = {
    .flags = -1,
};

typedef enum {
    PUSH,
    POP,
    ADD,
    SUB,
    JMP,
    JZ,
    LOAD,
    STORE,
    HALT,
    CALL_FUNC
} InstructionSet;

#define STACK_SIZE 256
#define PROGRAM_SIZE 512

typedef struct {
    unsigned int stack[STACK_SIZE];
    unsigned int pc;
    int sp;
    unsigned int program[PROGRAM_SIZE];
    unsigned int memory[256];
} VirtualMachine;

static void vm_init(VirtualMachine *vm) {
    vm->pc = 0;
    vm->sp = -1;
}

static void vm_push(VirtualMachine *vm, unsigned int value) {
    if (vm->sp < STACK_SIZE - 1) {
        vm->stack[++vm->sp] = value;
    }
}

static unsigned int vm_pop(VirtualMachine *vm) {
    if (vm->sp >= 0) {
        return vm->stack[vm->sp--];
    }
    return 0;
}

static void vm_run(VirtualMachine *vm) {
    int running = 1;
    while (running) {
        switch (vm->program[vm->pc++]) {
            case PUSH: {
                unsigned int val = vm->program[vm->pc++];
                vm_push(vm, val);
                break;
            }
            case POP: {
                vm_pop(vm);
                break;
            }
            case ADD: {
                unsigned int a = vm_pop(vm);
                unsigned int b = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                unsigned int a = vm_pop(vm);
                unsigned int b = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP: {
                unsigned int addr = vm->program[vm->pc++];
                vm->pc = addr;
                break;
            }
            case JZ: {
                unsigned int addr = vm->program[vm->pc++];
                if (vm_pop(vm) == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case LOAD: {
                unsigned int addr = vm->program[vm->pc++];
                vm_push(vm, vm->memory[addr]);
                break;
            }
            case STORE: {
                unsigned int val = vm_pop(vm);
                unsigned int addr = vm->program[vm->pc++];
                vm->memory[addr] = val;
                break;
            }
            case CALL_FUNC: {
                unsigned int func = vm->program[vm->pc++];
                switch (func) {
                    case 0: // stack_maxrandom_size
                        vm_push(vm, stack_maxrandom_size());
                        break;
                    case 1: // mmap_is_legacy
                        vm_push(vm, mmap_is_legacy());
                        break;
                    case 2: // mmap_rnd
                        vm_push(vm, mmap_rnd());
                        break;
                    case 3: // mmap_base
                        vm_push(vm, mmap_base());
                        break;
                    case 4: // mmap_legacy_base
                        vm_push(vm, mmap_legacy_base());
                        break;
                }
                break;
            }
            case HALT: {
                running = 0;
                break;
            }
        }
    }
}

static unsigned int stack_maxrandom_size(void) {
    VirtualMachine vm;
    vm_init(&vm);
    unsigned int code[] = {
        CALL_FUNC, 0, HALT
    };
    memcpy(vm.program, code, sizeof(code));
    vm_run(&vm);
    return vm_pop(&vm);
}

static int mmap_is_legacy(void) {
    VirtualMachine vm;
    vm_init(&vm);
    unsigned int code[] = {
        CALL_FUNC, 1, HALT
    };
    memcpy(vm.program, code, sizeof(code));
    vm_run(&vm);
    return vm_pop(&vm);
}

static unsigned long mmap_rnd(void) {
    VirtualMachine vm;
    vm_init(&vm);
    unsigned int code[] = {
        CALL_FUNC, 2, HALT
    };
    memcpy(vm.program, code, sizeof(code));
    vm_run(&vm);
    return vm_pop(&vm);
}

static unsigned long mmap_base(void) {
    VirtualMachine vm;
    vm_init(&vm);
    unsigned int code[] = {
        CALL_FUNC, 3, HALT
    };
    memcpy(vm.program, code, sizeof(code));
    vm_run(&vm);
    return vm_pop(&vm);
}

static unsigned long mmap_legacy_base(void) {
    VirtualMachine vm;
    vm_init(&vm);
    unsigned int code[] = {
        CALL_FUNC, 4, HALT
    };
    memcpy(vm.program, code, sizeof(code));
    vm_run(&vm);
    return vm_pop(&vm);
}

void arch_pick_mmap_layout(struct mm_struct *mm) {
    mm->mmap_legacy_base = mmap_legacy_base();
    mm->mmap_base = mmap_base();

    if (mmap_is_legacy()) {
        mm->mmap_base = mm->mmap_legacy_base;
        mm->get_unmapped_area = arch_get_unmapped_area;
    } else {
        mm->get_unmapped_area = arch_get_unmapped_area_topdown;
    }
}