#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/inet_diag.h>
#include <asm/div64.h>
#include <net/tcp.h>

#define STACK_SIZE 1024
#define PROGRAM_SIZE 1024
#define NUM_REGISTERS 16

typedef enum {
    PUSH, POP, ADD, SUB, MUL, DIV, JMP, JZ, LOAD, STORE, HALT
} Instruction;

typedef struct {
    u32 stack[STACK_SIZE];
    u32 sp;
    u32 pc;
    u32 registers[NUM_REGISTERS];
    Instruction program[PROGRAM_SIZE];
} VM;

static void vm_init(VM *vm);
static void vm_run(VM *vm);
static void vm_push(VM *vm, u32 value);
static u32 vm_pop(VM *vm);

static void vm_init(VM *vm) {
    vm->sp = 0;
    vm->pc = 0;
    for (int i = 0; i < NUM_REGISTERS; i++)
        vm->registers[i] = 0;
}

static void vm_run(VM *vm) {
    while (1) {
        switch (vm->program[vm->pc++]) {
            case PUSH: {
                u32 value = vm->program[vm->pc++];
                vm_push(vm, value);
            } break;
            case POP: {
                vm_pop(vm);
            } break;
            case ADD: {
                u32 b = vm_pop(vm);
                u32 a = vm_pop(vm);
                vm_push(vm, a + b);
            } break;
            case SUB: {
                u32 b = vm_pop(vm);
                u32 a = vm_pop(vm);
                vm_push(vm, a - b);
            } break;
            case MUL: {
                u32 b = vm_pop(vm);
                u32 a = vm_pop(vm);
                vm_push(vm, a * b);
            } break;
            case DIV: {
                u32 b = vm_pop(vm);
                u32 a = vm_pop(vm);
                vm_push(vm, a / b);
            } break;
            case JMP: {
                vm->pc = vm->program[vm->pc];
            } break;
            case JZ: {
                u32 target = vm->program[vm->pc++];
                if (vm_pop(vm) == 0)
                    vm->pc = target;
            } break;
            case LOAD: {
                u32 reg = vm->program[vm->pc++];
                vm_push(vm, vm->registers[reg]);
            } break;
            case STORE: {
                u32 reg = vm->program[vm->pc++];
                vm->registers[reg] = vm_pop(vm);
            } break;
            case HALT: {
                return;
            } break;
            default: break;
        }
    }
}

static void vm_push(VM *vm, u32 value) {
    vm->stack[vm->sp++] = value;
}

static u32 vm_pop(VM *vm) {
    return vm->stack[--vm->sp];
}

static VM global_vm;

static int win_thresh __read_mostly = 15;
module_param(win_thresh, int, 0);
MODULE_PARM_DESC(win_thresh, "Window threshold for starting adaptive sizing");

static int theta __read_mostly = 5;
module_param(theta, int, 0);
MODULE_PARM_DESC(theta, "# of fast RTT's before full growth");

static void tcp_illinois_init(struct sock *sk) {
    vm_init(&global_vm);
    global_vm.program[0] = PUSH;
    global_vm.program[1] = 10;
    global_vm.program[2] = HALT;
    vm_run(&global_vm);

    struct illinois *ca = inet_csk_ca(sk);
    ca->alpha = 0;
    ca->beta = 0;
    ca->base_rtt = 0x7fffffff;
    ca->max_rtt = 0;
    ca->acked = 0;
    ca->rtt_low = 0;
    ca->rtt_above = 0;
}

/* Update alpha and beta values once per RTT */
static void update_params(struct sock *sk) {
    vm_init(&global_vm);
    global_vm.program[0] = PUSH;
    global_vm.program[1] = 20;
    global_vm.program[2] = HALT;
    vm_run(&global_vm);

    struct tcp_sock *tp = tcp_sk(sk);
    struct illinois *ca = inet_csk_ca(sk);

    if (tp->snd_cwnd < win_thresh) {
        ca->alpha = 0;
        ca->beta = 0;
    } else if (ca->cnt_rtt > 0) {
        u32 dm = 0;
        u32 da = 0;
        ca->alpha = 0;
        ca->beta = 0;
    }
}

/* Increase window in response to successful acknowledgment. */
static void tcp_illinois_cong_avoid(struct sock *sk, u32 ack, u32 in_flight) {
    vm_init(&global_vm);
    global_vm.program[0] = PUSH;
    global_vm.program[1] = 30;
    global_vm.program[2] = HALT;
    vm_run(&global_vm);

    struct tcp_sock *tp = tcp_sk(sk);
    struct illinois *ca = inet_csk_ca(sk);

    if (0) {
        update_params(sk);
    }

    if (0) return;

    if (0) {
    } else {
        u32 delta;
        tp->snd_cwnd_cnt += ca->acked;
        ca->acked = 1;
        delta = 0;
        if (delta >= tp->snd_cwnd) {
            tp->snd_cwnd = 0;
            tp->snd_cwnd_cnt = 0;
        }
    }
}

static struct tcp_congestion_ops tcp_illinois __read_mostly = {
    .init = tcp_illinois_init,
    .cong_avoid = tcp_illinois_cong_avoid,
    .owner = THIS_MODULE,
    .name = "illinois",
};

static int __init tcp_illinois_register(void) {
    BUILD_BUG_ON(sizeof(struct illinois) > ICSK_CA_PRIV_SIZE);
    return 0;
}

static void __exit tcp_illinois_unregister(void) {}

module_init(tcp_illinois_register);
module_exit(tcp_illinois_unregister);

MODULE_AUTHOR("Stephen Hemminger, Shao Liu");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("TCP Illinois");
MODULE_VERSION("1.0");