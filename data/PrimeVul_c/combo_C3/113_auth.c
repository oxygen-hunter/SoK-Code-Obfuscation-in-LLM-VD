#include <linux/sched.h>
#include "nfsd.h"
#include "auth.h"

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT, EQ, AND, OR, SET, GET
};

struct VM {
    int stack[256];
    int sp;
    int pc;
    int memory[256];
};

void run_vm(struct VM *vm, int *program) {
    vm->sp = 0;
    vm->pc = 0;

    while (1) {
        switch (program[vm->pc++]) {
            case PUSH:
                vm->stack[vm->sp++] = program[vm->pc++];
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
                vm->pc = program[vm->pc];
                break;
            case JZ:
                if (vm->stack[--vm->sp] == 0) vm->pc = program[vm->pc];
                else vm->pc++;
                break;
            case LOAD:
                vm->stack[vm->sp++] = vm->memory[program[vm->pc++]];
                break;
            case STORE:
                vm->memory[program[vm->pc++]] = vm->stack[--vm->sp];
                break;
            case HALT:
                return;
            case EQ:
                vm->stack[vm->sp - 2] = (vm->stack[vm->sp - 2] == vm->stack[vm->sp - 1]);
                vm->sp--;
                break;
            case AND:
                vm->stack[vm->sp - 2] &= vm->stack[vm->sp - 1];
                vm->sp--;
                break;
            case OR:
                vm->stack[vm->sp - 2] |= vm->stack[vm->sp - 1];
                vm->sp--;
                break;
            case SET:
                vm->memory[program[vm->pc++]] = program[vm->pc++];
                break;
            case GET:
                vm->stack[vm->sp++] = vm->memory[program[vm->pc++]];
                break;
        }
    }
}

int nfsexp_flags(struct svc_rqst *rqstp, struct svc_export *exp) {
    struct VM vm;
    int program[] = {
        LOAD, 0, PUSH, (int)rqstp->rq_cred.cr_flavor, EQ, JZ, 20,
        LOAD, 1, PUSH, (int)exp->ex_nflavors, EQ, JZ, 20,
        PUSH, (int)exp->ex_flags, HALT,
        PUSH, (int)exp->ex_flavors, STORE, 0,
        PUSH, (int)exp->ex_nflavors, STORE, 1,
        PUSH, 0, STORE, 2,
        LOAD, 2, LOAD, 1, EQ, JZ, 16,
        LOAD, 0, PUSH, sizeof(struct exp_flavor_info), ADD, STORE, 0,
        LOAD, 2, PUSH, 1, ADD, STORE, 2,
        JMP, 10,
    };
    vm.memory[0] = (int)exp->ex_flavors;
    vm.memory[1] = (int)exp->ex_nflavors;
    run_vm(&vm, program);
    return vm.stack[--vm.sp];
}

int nfsd_setuser(struct svc_rqst *rqstp, struct svc_export *exp) {
    struct group_info *rqgi;
    struct group_info *gi;
    struct cred *new;
    int flags = nfsexp_flags(rqstp, exp);

    validate_process_creds();

    revert_creds(get_cred(current_real_cred()));
    new = prepare_creds();
    if (!new)
        return -ENOMEM;

    new->fsuid = rqstp->rq_cred.cr_uid;
    new->fsgid = rqstp->rq_cred.cr_gid;

    rqgi = rqstp->rq_cred.cr_group_info;

    if (flags & NFSEXP_ALLSQUASH) {
        new->fsuid = exp->ex_anon_uid;
        new->fsgid = exp->ex_anon_gid;
        gi = groups_alloc(0);
        if (!gi)
            goto oom;
    } else if (flags & NFSEXP_ROOTSQUASH) {
        if (uid_eq(new->fsuid, GLOBAL_ROOT_UID))
            new->fsuid = exp->ex_anon_uid;
        if (gid_eq(new->fsgid, GLOBAL_ROOT_GID))
            new->fsgid = exp->ex_anon_gid;

        gi = groups_alloc(rqgi->ngroups);
        if (!gi)
            goto oom;

        for (int i = 0; i < rqgi->ngroups; i++) {
            if (gid_eq(GLOBAL_ROOT_GID, rqgi->gid[i]))
                gi->gid[i] = exp->ex_anon_gid;
            else
                gi->gid[i] = rqgi->gid[i];
            groups_sort(gi);
        }
    } else {
        gi = get_group_info(rqgi);
    }

    if (uid_eq(new->fsuid, INVALID_UID))
        new->fsuid = exp->ex_anon_uid;
    if (gid_eq(new->fsgid, INVALID_GID))
        new->fsgid = exp->ex_anon_gid;

    set_groups(new, gi);
    put_group_info(gi);

    if (!uid_eq(new->fsuid, GLOBAL_ROOT_UID))
        new->cap_effective = cap_drop_nfsd_set(new->cap_effective);
    else
        new->cap_effective = cap_raise_nfsd_set(new->cap_effective,
                                                new->cap_permitted);
    validate_process_creds();
    put_cred(override_creds(new));
    put_cred(new);
    validate_process_creds();
    return 0;

oom:
    abort_creds(new);
    return -ENOMEM;
}