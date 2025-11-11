#include <linux/signal.h>
#include <linux/personality.h>
#include <linux/kallsyms.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/hardirq.h>
#include <linux/kdebug.h>
#include <linux/module.h>
#include <linux/kexec.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/syscalls.h>

#include <asm/atomic.h>
#include <asm/traps.h>
#include <asm/stacktrace.h>
#include <asm/exception.h>
#include <asm/system_misc.h>

typedef struct {
    int sp;
    unsigned long stack[256];
    int pc;
    unsigned char *code;
} VM;

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, PRINTK, GETFS, SETFS, DUMP, RET
};

void vm_run(VM *vm) {
    int running = 1;
    int fs_saved = 0;
    mm_segment_t fs;
    while (running) {
        switch (vm->code[vm->pc++]) {
            case PUSH:
                vm->stack[vm->sp++] = *(unsigned long *)(vm->code + vm->pc);
                vm->pc += sizeof(unsigned long);
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
                vm->pc = *(int *)(vm->code + vm->pc);
                break;
            case JZ:
                if (vm->stack[--vm->sp] == 0) {
                    vm->pc = *(int *)(vm->code + vm->pc);
                } else {
                    vm->pc += sizeof(int);
                }
                break;
            case LOAD:
                vm->stack[vm->sp++] = *(unsigned long *)vm->stack[--vm->sp];
                break;
            case STORE:
                *(unsigned long *)vm->stack[--vm->sp] = vm->stack[--vm->sp];
                break;
            case PRINTK:
                printk("%s", (char *)vm->stack[--vm->sp]);
                break;
            case GETFS:
                fs = get_fs();
                vm->stack[vm->sp++] = (unsigned long)fs;
                break;
            case SETFS:
                fs_saved = vm->stack[--vm->sp];
                set_fs((mm_segment_t)fs_saved);
                break;
            case DUMP:
                printk("Dumping memory from 0x%lx to 0x%lx\n", vm->stack[vm->sp - 2], vm->stack[vm->sp - 1]);
                vm->sp -= 2;
                break;
            case RET:
                running = 0;
                break;
        }
    }
}

unsigned char code[] = {
    GETFS, PUSH, 0, PUSH, (unsigned long)"Kernel memory dump: ", PRINTK,
    PUSH, 0xFFFF0000, PUSH, 0xFFFF0010, DUMP,
    SETFS, RET
};

void execute_virtual_machine() {
    VM vm = {0, {0}, 0, code};
    vm_run(&vm);
}

void show_stack(struct task_struct *tsk, unsigned long *sp) {
    execute_virtual_machine();
}

void die(const char *str, struct pt_regs *regs, int err) {
    struct thread_info *thread = current_thread_info();
    int ret;
    oops_enter();
    raw_spin_lock_irq(&die_lock);
    console_verbose();
    bust_spinlocks(1);
    ret = __die(str, err, thread, regs);
    if (regs && kexec_should_crash(thread->task))
        crash_kexec(regs);
    bust_spinlocks(0);
    add_taint(TAINT_DIE, LOCKDEP_NOW_UNRELIABLE);
    raw_spin_unlock_irq(&die_lock);
    oops_exit();
    if (in_interrupt())
        panic("Fatal exception in interrupt");
    if (panic_on_oops)
        panic("Fatal exception");
    if (ret != NOTIFY_STOP)
        do_exit(SIGSEGV);
}

asmlinkage void __exception do_undefinstr(struct pt_regs *regs) {
    siginfo_t info;
    void __user *pc = (void __user *)instruction_pointer(regs);
#ifdef CONFIG_COMPAT
    if (compat_user_mode(regs) && aarch32_break_trap(regs) == 0)
        return;
#endif
    if (show_unhandled_signals && unhandled_signal(current, SIGILL) && printk_ratelimit()) {
        pr_info("%s[%d]: undefined instruction: pc=%p\n",
                current->comm, task_pid_nr(current), pc);
        dump_instr(KERN_INFO, regs);
    }
    info.si_signo = SIGILL;
    info.si_errno = 0;
    info.si_code  = ILL_ILLOPC;
    info.si_addr  = pc;
    arm64_notify_die("Oops - undefined instruction", regs, &info, 0);
}

long compat_arm_syscall(struct pt_regs *regs);

asmlinkage long do_ni_syscall(struct pt_regs *regs) {
#ifdef CONFIG_COMPAT
    long ret;
    if (is_compat_task()) {
        ret = compat_arm_syscall(regs);
        if (ret != -ENOSYS)
            return ret;
    }
#endif
    if (show_unhandled_signals && printk_ratelimit()) {
        pr_info("%s[%d]: syscall %d\n", current->comm,
                task_pid_nr(current), (int)regs->syscallno);
        dump_instr("", regs);
        if (user_mode(regs))
            __show_regs(regs);
    }
    return sys_ni_syscall();
}

asmlinkage void bad_mode(struct pt_regs *regs, int reason, unsigned int esr) {
    console_verbose();
    pr_crit("Bad mode in %s handler detected, code 0x%08x\n", handler[reason], esr);
    die("Oops - bad mode", regs, 0);
    local_irq_disable();
    panic("bad mode");
}

void __pte_error(const char *file, int line, unsigned long val) {
    printk("%s:%d: bad pte %016lx.\n", file, line, val);
}

void __pmd_error(const char *file, int line, unsigned long val) {
    printk("%s:%d: bad pmd %016lx.\n", file, line, val);
}

void __pgd_error(const char *file, int line, unsigned long val) {
    printk("%s:%d: bad pgd %016lx.\n", file, line, val);
}

void __init trap_init(void) {
    return;
}