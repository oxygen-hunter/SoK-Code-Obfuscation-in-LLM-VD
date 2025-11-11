#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/eventpoll.h>
#include <linux/sem.h>
#include <linux/socket.h>
#include <linux/net.h>
#include <linux/ipc.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define MAX_STACK_SIZE 256

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
} Instruction;

typedef struct {
    int stack[MAX_STACK_SIZE];
    int pc;
    int sp;
} VM;

void initVM(VM *vm) {
    vm->pc = 0;
    vm->sp = -1;
}

void push(VM *vm, int val) {
    if (vm->sp < MAX_STACK_SIZE - 1) {
        vm->stack[++vm->sp] = val;
    }
}

int pop(VM *vm) {
    if (vm->sp >= 0) {
        return vm->stack[vm->sp--];
    }
    return 0;
}

void execute(VM *vm, int *program) {
    while (1) {
        switch (program[vm->pc++]) {
            case PUSH:
                push(vm, program[vm->pc++]);
                break;
            case POP:
                pop(vm);
                break;
            case ADD: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a + b);
                break;
            }
            case SUB: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a - b);
                break;
            }
            case JMP:
                vm->pc = program[vm->pc];
                break;
            case JZ: {
                int cond = pop(vm);
                if (cond == 0) {
                    vm->pc = program[vm->pc];
                } else {
                    vm->pc++;
                }
                break;
            }
            case LOAD:
                push(vm, program[program[vm->pc++]]);
                break;
            case STORE: {
                int val = pop(vm);
                program[program[vm->pc++]] = val;
                break;
            }
            case CALL: {
                int addr = program[vm->pc++];
                push(vm, vm->pc);
                vm->pc = addr;
                break;
            }
            case RET:
                vm->pc = pop(vm);
                break;
            default:
                return;
        }
    }
}

int runSysOabiStat64(const char *filename, struct oldabi_stat64 __user *statbuf) {
    int program[] = {
        PUSH, (int)filename,
        PUSH, (int)statbuf,
        CALL, 24,
        RET,
        // Function cp_oldabi_stat64()
        LOAD, 0,         // filename
        LOAD, 1,         // statbuf
        CALL, 50,        // vfs_stat
        JZ, 54,
        RET,
        // if (!error) call cp_oldabi_stat64
        LOAD, 1,         
        CALL, 60,
        RET
    };

    VM vm;
    initVM(&vm);
    execute(&vm, program);
    return pop(&vm);
}

int runSysOabiLstat64(const char *filename, struct oldabi_stat64 __user *statbuf) {
    int program[] = {
        PUSH, (int)filename,
        PUSH, (int)statbuf,
        CALL, 24,
        RET,
        // Function cp_oldabi_stat64()
        LOAD, 0,         // filename
        LOAD, 1,         // statbuf
        CALL, 50,        // vfs_lstat
        JZ, 54,
        RET,
        // if (!error) call cp_oldabi_stat64
        LOAD, 1,         
        CALL, 60,
        RET
    };

    VM vm;
    initVM(&vm);
    execute(&vm, program);
    return pop(&vm);
}

int runSysOabiFstat64(unsigned long fd, struct oldabi_stat64 __user *statbuf) {
    int program[] = {
        PUSH, (int)fd,
        PUSH, (int)statbuf,
        CALL, 24,
        RET,
        // Function cp_oldabi_stat64()
        LOAD, 0,         // fd
        LOAD, 1,         // statbuf
        CALL, 50,        // vfs_fstat
        JZ, 54,
        RET,
        // if (!error) call cp_oldabi_stat64
        LOAD, 1,         
        CALL, 60,
        RET
    };

    VM vm;
    initVM(&vm);
    execute(&vm, program);
    return pop(&vm);
}

struct oldabi_stat64 {
    unsigned long long st_dev;
    unsigned int    __pad1;
    unsigned long   __st_ino;
    unsigned int    st_mode;
    unsigned int    st_nlink;

    unsigned long   st_uid;
    unsigned long   st_gid;

    unsigned long long st_rdev;
    unsigned int    __pad2;

    long long   st_size;
    unsigned long   st_blksize;
    unsigned long long st_blocks;

    unsigned long   st_atime;
    unsigned long   st_atime_nsec;

    unsigned long   st_mtime;
    unsigned long   st_mtime_nsec;

    unsigned long   st_ctime;
    unsigned long   st_ctime_nsec;

    unsigned long long st_ino;
} __attribute__ ((packed,aligned(4)));

static long cp_oldabi_stat64(struct kstat *stat,
                             struct oldabi_stat64 __user *statbuf) {
    struct oldabi_stat64 tmp;
    tmp.st_dev = huge_encode_dev(stat->dev);
    tmp.__pad1 = 0;
    tmp.__st_ino = stat->ino;
    tmp.st_mode = stat->mode;
    tmp.st_nlink = stat->nlink;
    tmp.st_uid = stat->uid;
    tmp.st_gid = stat->gid;
    tmp.st_rdev = huge_encode_dev(stat->rdev);
    tmp.st_size = stat->size;
    tmp.st_blocks = stat->blocks;
    tmp.__pad2 = 0;
    tmp.st_blksize = stat->blksize;
    tmp.st_atime = stat->atime.tv_sec;
    tmp.st_atime_nsec = stat->atime.tv_nsec;
    tmp.st_mtime = stat->mtime.tv_sec;
    tmp.st_mtime_nsec = stat->mtime.tv_nsec;
    tmp.st_ctime = stat->ctime.tv_sec;
    tmp.st_ctime_nsec = stat->ctime.tv_nsec;
    tmp.st_ino = stat->ino;
    return copy_to_user(statbuf, &tmp, sizeof(tmp)) ? -EFAULT : 0;
}

asmlinkage long sys_oabi_stat64(const char __user * filename,
                                struct oldabi_stat64 __user * statbuf) {
    return runSysOabiStat64(filename, statbuf);
}

asmlinkage long sys_oabi_lstat64(const char __user * filename,
                                 struct oldabi_stat64 __user * statbuf) {
    return runSysOabiLstat64(filename, statbuf);
}

asmlinkage long sys_oabi_fstat64(unsigned long fd,
                                 struct oldabi_stat64 __user * statbuf) {
    return runSysOabiFstat64(fd, statbuf);
}

// Further syscalls can be implemented similarly...