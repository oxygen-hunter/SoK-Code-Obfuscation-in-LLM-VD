#include <linux/stat.h>
#include <linux/sysctl.h>
#include <linux/slab.h>
#include <linux/cred.h>
#include <linux/hash.h>
#include <linux/user_namespace.h>

#define UCOUNTS_HASHTABLE_BITS 10
static struct hlist_head ucounts_hashtable[(1 << UCOUNTS_HASHTABLE_BITS)];
static DEFINE_SPINLOCK(ucounts_lock);

#define ucounts_hashfn(ns, uid)						\
	hash_long((unsigned long)__kuid_val(uid) + (unsigned long)(ns), \
		  UCOUNTS_HASHTABLE_BITS)
#define ucounts_hashentry(ns, uid)	\
	(ucounts_hashtable + ucounts_hashfn(ns, uid))

enum vm_instructions {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

struct vm {
    int stack[256];
    int sp;
    int pc;
    int registers[8];
    int memory[256];
};

void vm_init(struct vm *vm) {
    vm->sp = -1;
    vm->pc = 0;
}

void vm_run(struct vm *vm, int *program, int program_size) {
    while (vm->pc < program_size) {
        switch (program[vm->pc]) {
            case PUSH:
                vm->stack[++vm->sp] = program[++vm->pc];
                break;
            case POP:
                --vm->sp;
                break;
            case ADD:
                vm->stack[vm->sp - 1] += vm->stack[vm->sp];
                --vm->sp;
                break;
            case SUB:
                vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
                --vm->sp;
                break;
            case JMP:
                vm->pc = program[++vm->pc] - 1;
                break;
            case JZ:
                if (vm->stack[vm->sp--] == 0)
                    vm->pc = program[++vm->pc] - 1;
                else
                    ++vm->pc;
                break;
            case LOAD:
                vm->stack[++vm->sp] = vm->memory[program[++vm->pc]];
                break;
            case STORE:
                vm->memory[program[++vm->pc]] = vm->stack[vm->sp--];
                break;
            case CALL:
                vm->stack[++vm->sp] = ++vm->pc;
                vm->pc = program[vm->pc];
                break;
            case RET:
                vm->pc = vm->stack[vm->sp--];
                break;
            default:
                return;
        }
        vm->pc++;
    }
}

#ifdef CONFIG_SYSCTL
static int zero = 0;
static int int_max = INT_MAX;

static int program[] = {
    PUSH, 100,
    STORE, 0,
    LOAD, 0,
    PUSH, 1,
    ADD,
    STORE, 0,
    LOAD, 0,
    PUSH, 10,
    SUB,
    JZ, 20,
    JMP, 2,
    HALT
};

bool setup_userns_sysctls(struct user_namespace *ns) {
    struct vm my_vm;
    vm_init(&my_vm);
    vm_run(&my_vm, program, sizeof(program) / sizeof(int));
    return my_vm.memory[0] == 110;
}

void retire_userns_sysctls(struct user_namespace *ns) {
    // Simulated VM code for cleanup
}

#endif

static __init int user_namespace_sysctl_init(void) {
#ifdef CONFIG_SYSCTL
    static struct ctl_table_header *user_header;
    static struct ctl_table empty[1];
    user_header = register_sysctl("user", empty);
    kmemleak_ignore(user_header);
    BUG_ON(!user_header);
    BUG_ON(!setup_userns_sysctls(&init_user_ns));
#endif
    return 0;
}
subsys_initcall(user_namespace_sysctl_init);