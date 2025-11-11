#include <linux/module.h>
#include <linux/kprobes.h>
#include "trace.h"

static char __percpu *perf_trace_buf[PERF_NR_CONTEXTS];

typedef typeof(unsigned long [PERF_MAX_TRACE_SIZE / sizeof(unsigned long)])
	perf_trace_t;

static int	total_ref_count;

enum VM_OpCode {
    VM_NOP,
    VM_PUSH,
    VM_POP,
    VM_ADD,
    VM_SUB,
    VM_JMP,
    VM_JZ,
    VM_LOAD,
    VM_STORE,
    VM_CALL,
    VM_RET,
    VM_HALT,
};

typedef struct {
    int pc;
    long stack[256];
    int sp;
    long memory[256];
} VMState;

void vm_execute(VMState* vm, const int* program) {
    int running = 1;
    while (running) {
        int instr = program[vm->pc++];
        switch (instr) {
            case VM_NOP:
                break;
            case VM_PUSH:
                vm->stack[vm->sp++] = program[vm->pc++];
                break;
            case VM_POP:
                vm->sp--;
                break;
            case VM_ADD:
                vm->stack[vm->sp - 2] += vm->stack[vm->sp - 1];
                vm->sp--;
                break;
            case VM_SUB:
                vm->stack[vm->sp - 2] -= vm->stack[vm->sp - 1];
                vm->sp--;
                break;
            case VM_JMP:
                vm->pc = program[vm->pc];
                break;
            case VM_JZ:
                if (vm->stack[--vm->sp] == 0)
                    vm->pc = program[vm->pc];
                else
                    vm->pc++;
                break;
            case VM_LOAD:
                vm->stack[vm->sp++] = vm->memory[program[vm->pc++]];
                break;
            case VM_STORE:
                vm->memory[program[vm->pc++]] = vm->stack[--vm->sp];
                break;
            case VM_CALL:
                vm->memory[program[vm->pc++]](vm);
                break;
            case VM_RET:
                return;
            case VM_HALT:
                running = 0;
                break;
        }
    }
}

static int perf_trace_event_perm_vm(struct ftrace_event_call *tp_event,
                                    struct perf_event *p_event) {
    VMState vm = {0};
    const int program[] = {
        VM_PUSH, (int)(intptr_t)tp_event,
        VM_PUSH, (int)(intptr_t)p_event,
        VM_CALL, 0,
        VM_HALT
    };
    vm_execute(&vm, program);
    return vm.memory[0];
}

static int perf_trace_event_reg_vm(struct ftrace_event_call *tp_event,
                                   struct perf_event *p_event) {
    VMState vm = {0};
    const int program[] = {
        VM_PUSH, (int)(intptr_t)tp_event,
        VM_PUSH, (int)(intptr_t)p_event,
        VM_CALL, 1,
        VM_HALT
    };
    vm_execute(&vm, program);
    return vm.memory[0];
}

static void perf_trace_event_unreg_vm(struct perf_event *p_event) {
    VMState vm = {0};
    const int program[] = {
        VM_PUSH, (int)(intptr_t)p_event,
        VM_CALL, 2,
        VM_HALT
    };
    vm_execute(&vm, program);
}

static int perf_trace_event_open_vm(struct perf_event *p_event) {
    VMState vm = {0};
    const int program[] = {
        VM_PUSH, (int)(intptr_t)p_event,
        VM_CALL, 3,
        VM_HALT
    };
    vm_execute(&vm, program);
    return vm.memory[0];
}

static void perf_trace_event_close_vm(struct perf_event *p_event) {
    VMState vm = {0};
    const int program[] = {
        VM_PUSH, (int)(intptr_t)p_event,
        VM_CALL, 4,
        VM_HALT
    };
    vm_execute(&vm, program);
}

int perf_trace_init(struct perf_event *p_event) {
    struct ftrace_event_call *tp_event;
    int event_id = p_event->attr.config;
    int ret = -EINVAL;

    mutex_lock(&event_mutex);
    list_for_each_entry(tp_event, &ftrace_events, list) {
        if (tp_event->event.type == event_id &&
            tp_event->class && tp_event->class->reg &&
            try_module_get(tp_event->mod)) {
            ret = perf_trace_event_perm_vm(tp_event, p_event);
            if (ret) {
                module_put(tp_event->mod);
                break;
            }
            ret = perf_trace_event_reg_vm(tp_event, p_event);
            if (ret) {
                module_put(tp_event->mod);
                break;
            }
            ret = perf_trace_event_open_vm(p_event);
            if (ret) {
                perf_trace_event_unreg_vm(p_event);
                module_put(tp_event->mod);
                break;
            }
            break;
        }
    }
    mutex_unlock(&event_mutex);

    return ret;
}

void perf_trace_destroy(struct perf_event *p_event) {
    mutex_lock(&event_mutex);
    perf_trace_event_close_vm(p_event);
    perf_trace_event_unreg_vm(p_event);
    mutex_unlock(&event_mutex);
}