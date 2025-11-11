#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <Python.h>

static int (*valid_next_sp)(unsigned long, unsigned long);

void perf_callchain_kernel(struct perf_callchain_entry *entry, struct pt_regs *regs) {
    unsigned long sp, next_sp;
    unsigned long next_ip;
    unsigned long lr;
    long level = 0;
    unsigned long *fp;

    lr = regs->link;
    sp = regs->gpr[1];
    perf_callchain_store(entry, perf_instruction_pointer(regs));

    if (!validate_sp(sp, current, STACK_FRAME_OVERHEAD))
        return;

    for (;;) {
        fp = (unsigned long *) sp;
        next_sp = fp[0];

        if (next_sp == sp + STACK_INT_FRAME_SIZE && fp[STACK_FRAME_MARKER] == STACK_FRAME_REGS_MARKER) {
            regs = (struct pt_regs *)(sp + STACK_FRAME_OVERHEAD);
            next_ip = regs->nip;
            lr = regs->link;
            level = 0;
            perf_callchain_store(entry, PERF_CONTEXT_KERNEL);
        } else {
            if (level == 0)
                next_ip = lr;
            else
                next_ip = fp[STACK_FRAME_LR_SAVE];

            if ((level == 1 && next_ip == lr) || (level <= 1 && !kernel_text_address(next_ip)))
                next_ip = 0;

            ++level;
        }

        perf_callchain_store(entry, next_ip);
        if (!valid_next_sp(next_sp, sp))
            return;
        sp = next_sp;
    }
}

int main() {
    void *handle = dlopen("./libvalid_next_sp.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error opening library: %s\n", dlerror());
        exit(EXIT_FAILURE);
    }

    valid_next_sp = dlsym(handle, "valid_next_sp");
    if (!valid_next_sp) {
        fprintf(stderr, "Error loading symbol: %s\n", dlerror());
        dlclose(handle);
        exit(EXIT_FAILURE);
    }

    Py_Initialize();
    Py_Finalize();

    dlclose(handle);
    return 0;
}