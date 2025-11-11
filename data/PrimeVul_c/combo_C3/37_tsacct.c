#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/tsacct_kern.h>
#include <linux/acct.h>
#include <linux/jiffies.h>

#define USEC_PER_TICK (USEC_PER_SEC/HZ)

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
};

struct VM {
    s64 stack[256];
    int sp;
    int pc;
    struct taskstats *stats;
    struct task_struct *tsk;
};

void execute_vm(struct VM *vm, int *program) {
    int running = 1;
    while (running) {
        switch (program[vm->pc]) {
            case PUSH:
                vm->stack[vm->sp++] = program[++vm->pc];
                break;
            case POP:
                --vm->sp;
                break;
            case ADD:
                vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1];
                --vm->sp;
                break;
            case SUB:
                vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1];
                --vm->sp;
                break;
            case JMP:
                vm->pc = program[++vm->pc] - 1;
                break;
            case JZ:
                if (vm->stack[--vm->sp] == 0) vm->pc = program[++vm->pc] - 1;
                else ++vm->pc;
                break;
            case LOAD:
                vm->stack[vm->sp++] = *(s64 *)program[++vm->pc];
                break;
            case STORE:
                *(s64 *)program[++vm->pc] = vm->stack[--vm->sp];
                break;
            case HALT:
                running = 0;
                break;
        }
        vm->pc++;
    }
}

void bacct_add_tsk(struct taskstats *stats, struct task_struct *tsk) {
    struct VM vm = { .sp = 0, .pc = 0, .stats = stats, .tsk = tsk };
    int program[] = {
        LOAD, (int)&xtime.tv_sec,
        STORE, (int)&stats->ac_btime,
        PUSH, (int)xtime.tv_sec,
        LOAD, (int)&tsk->start_time.tv_sec,
        SUB,
        STORE, (int)&stats->ac_btime,
        LOAD, (int)&tsk->exit_code,
        STORE, (int)&stats->ac_exitcode,
        LOAD, (int)&tsk->flags,
        PUSH, PF_FORKNOEXEC,
        ADD,
        JZ, 21,
        PUSH, AFORK,
        STORE, (int)&stats->ac_flag,
        HALT
    };
    execute_vm(&vm, program);
}

#ifdef CONFIG_TASK_XACCT

#define KB 1024
#define MB (1024 * KB)

void xacct_add_tsk(struct taskstats *stats, struct task_struct *p) {
    struct VM vm = { .sp = 0, .pc = 0, .stats = stats, .tsk = p };
    int program[] = {
        LOAD, (int)&p->acct_rss_mem1,
        STORE, (int)&stats->coremem,
        LOAD, (int)&p->acct_vm_mem1,
        STORE, (int)&stats->virtmem,
        LOAD, (int)&p->rchar,
        STORE, (int)&stats->read_char,
        LOAD, (int)&p->wchar,
        STORE, (int)&stats->write_char,
        LOAD, (int)&p->syscr,
        STORE, (int)&stats->read_syscalls,
        LOAD, (int)&p->syscw,
        STORE, (int)&stats->write_syscalls,
        HALT
    };
    execute_vm(&vm, program);
}

#undef KB
#undef MB

void acct_update_integrals(struct task_struct *tsk) {
    struct VM vm = { .sp = 0, .pc = 0, .tsk = tsk };
    int program[] = {
        PUSH, (int)tsk->stime,
        PUSH, (int)tsk->acct_stimexpd,
        SUB,
        STORE, (int)&tsk->acct_stimexpd,
        HALT
    };
    execute_vm(&vm, program);
}

void acct_clear_integrals(struct task_struct *tsk) {
    struct VM vm = { .sp = 0, .pc = 0, .tsk = tsk };
    int program[] = {
        PUSH, 0,
        STORE, (int)&tsk->acct_stimexpd,
        STORE, (int)&tsk->acct_rss_mem1,
        STORE, (int)&tsk->acct_vm_mem1,
        HALT
    };
    execute_vm(&vm, program);
}

#endif