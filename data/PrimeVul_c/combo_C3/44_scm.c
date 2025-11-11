#include <linux/module.h>
#include <linux/signal.h>
#include <linux/capability.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/stat.h>
#include <linux/socket.h>
#include <linux/file.h>
#include <linux/fcntl.h>
#include <linux/net.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/security.h>
#include <linux/pid_namespace.h>
#include <linux/pid.h>
#include <linux/nsproxy.h>
#include <linux/slab.h>

#include <asm/uaccess.h>

#include <net/protocol.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <net/compat.h>
#include <net/scm.h>
#include <net/cls_cgroup.h>

#define MAX_STACK_SIZE 256
#define MAX_PROGRAM_SIZE 512

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT, CMP, JNZ
} OpCode;

typedef struct {
    int stack[MAX_STACK_SIZE];
    int sp;
    int program[MAX_PROGRAM_SIZE];
    int pc;
    int running;
} VM;

void vm_init(VM *vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = 1;
}

void vm_push(VM *vm, int value) {
    if (vm->sp < MAX_STACK_SIZE - 1) {
        vm->stack[++(vm->sp)] = value;
    }
}

int vm_pop(VM *vm) {
    if (vm->sp >= 0) {
        return vm->stack[(vm->sp)--];
    }
    return 0;
}

void vm_execute(VM *vm) {
    while (vm->running) {
        switch (vm->program[vm->pc++]) {
            case PUSH:
                vm_push(vm, vm->program[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP:
                vm->pc = vm->program[vm->pc];
                break;
            case JZ: {
                int addr = vm->program[vm->pc++];
                int value = vm_pop(vm);
                if (value == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case JNZ: {
                int addr = vm->program[vm->pc++];
                int value = vm_pop(vm);
                if (value != 0) {
                    vm->pc = addr;
                }
                break;
            }
            case HALT:
                vm->running = 0;
                break;
            default:
                vm->running = 0;
                break;
        }
    }
}

int run_vm_program(int *program, int program_size) {
    VM vm;
    vm_init(&vm);
    for (int i = 0; i < program_size; ++i) {
        vm.program[i] = program[i];
    }
    vm_execute(&vm);
    return vm_pop(&vm);
}

int scm_check_creds(struct ucred *creds) {
    int program[] = {
        PUSH, task_tgid_vnr(current),
        PUSH, creds->pid,
        CMP, JZ, 11,
        PUSH, current->nsproxy->pid_ns->user_ns,
        PUSH, CAP_SYS_ADMIN,
        CALL, ns_capable,
        JNZ, 11,
        PUSH, -EPERM,
        JMP, 20,
        PUSH, current_cred(),
        LOAD, offsetof(struct cred, uid),
        CALL, make_kuid,
        PUSH, creds->uid,
        CMP, JNZ, 9,
        PUSH, -EINVAL,
        HALT
    };
    return run_vm_program(program, sizeof(program) / sizeof(int));
}

int scm_fp_copy(struct cmsghdr *cmsg, struct scm_fp_list **fplp) {
    int program[] = {
        PUSH, (int)CMSG_DATA(cmsg),
        LOAD, 0,
        ADD, 0,
        CMP, 0,
        HALT
    };
    return run_vm_program(program, sizeof(program) / sizeof(int));
}

int __scm_send(struct socket *sock, struct msghdr *msg, struct scm_cookie *p) {
    int program[] = {
        PUSH, (int)CMSG_FIRSTHDR(msg),
        CALL, scm_fp_copy,
        HALT
    };
    return run_vm_program(program, sizeof(program) / sizeof(int));
}