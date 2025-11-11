// SPDX-License-Identifier: GPL-2.0
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
#include <linux/mm.h>
#include <linux/vmacache.h>
#include <asm/pgtable.h>

#ifdef CONFIG_MMU
#define VMACACHE_SHIFT PMD_SHIFT
#else
#define VMACACHE_SHIFT PAGE_SHIFT
#endif
#define VMACACHE_HASH(addr) ((addr >> VMACACHE_SHIFT) & VMACACHE_MASK)

enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, FLUSH_ALL, VALID_MM, UPDATE, FIND, FIND_EXACT, END
};

struct VM {
    int stack[256];
    int sp; 
    int pc;
    int instr[1024];
    struct task_struct *task;
    struct mm_struct *mm;
    unsigned long addr;
    struct vm_area_struct *newvma;
    struct vm_area_struct *result;
};

void vm_run(struct VM *vm) {
    while (1) {
        switch (vm->instr[vm->pc++]) {
            case PUSH:
                vm->stack[vm->sp++] = vm->instr[vm->pc++];
                break;
            case POP:
                vm->sp--;
                break;
            case ADD:
                vm->sp--;
                vm->stack[vm->sp - 1] += vm->stack[vm->sp];
                break;
            case SUB:
                vm->sp--;
                vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
                break;
            case JMP:
                vm->pc = vm->instr[vm->pc];
                break;
            case JZ:
                if (vm->stack[--vm->sp] == 0)
                    vm->pc = vm->instr[vm->pc];
                else
                    vm->pc++;
                break;
            case LOAD:
                vm->stack[vm->sp++] = vm->instr[vm->pc++];
                break;
            case STORE:
                vm->instr[vm->instr[vm->pc++]] = vm->stack[--vm->sp];
                break;
            case FLUSH_ALL:
                if (vm->mm) {
                    struct task_struct *g, *p;
                    count_vm_vmacache_event(VMACACHE_FULL_FLUSHES);
                    if (atomic_read(&vm->mm->mm_users) == 1)
                        break;
                    rcu_read_lock();
                    for_each_process_thread(g, p) {
                        if (vm->mm == p->mm)
                            vmacache_flush(p);
                    }
                    rcu_read_unlock();
                }
                break;
            case VALID_MM:
                vm->stack[vm->sp++] = (current->mm == vm->mm && !(current->flags & PF_KTHREAD));
                break;
            case UPDATE:
                if (vmacache_valid_mm(vm->newvma->vm_mm))
                    current->vmacache.vmas[VMACACHE_HASH(vm->addr)] = vm->newvma;
                break;
            case FIND:
                vm->result = NULL;
                count_vm_vmacache_event(VMACACHE_FIND_CALLS);
                if (!vmacache_valid(vm->mm))
                    break;
                for (int i = 0, idx = VMACACHE_HASH(vm->addr); i < VMACACHE_SIZE; ++i) {
                    struct vm_area_struct *vma = current->vmacache.vmas[idx];
                    if (vma && vma->vm_start <= vm->addr && vma->vm_end > vm->addr) {
                        count_vm_vmacache_event(VMACACHE_FIND_HITS);
                        vm->result = vma;
                        break;
                    }
                    if (++idx == VMACACHE_SIZE)
                        idx = 0;
                }
                break;
            case FIND_EXACT:
#ifndef CONFIG_MMU
                vm->result = NULL;
                count_vm_vmacache_event(VMACACHE_FIND_CALLS);
                if (!vmacache_valid(vm->mm))
                    break;
                for (int i = 0, idx = VMACACHE_HASH(vm->addr); i < VMACACHE_SIZE; ++i) {
                    struct vm_area_struct *vma = current->vmacache.vmas[idx];
                    if (vma && vma->vm_start == vm->addr && vma->vm_end == vm->instr[vm->pc]) {
                        count_vm_vmacache_event(VMACACHE_FIND_HITS);
                        vm->result = vma;
                        break;
                    }
                    if (++idx == VMACACHE_SIZE)
                        idx = 0;
                }
#endif
                break;
            case END:
                return;
        }
    }
}

void execute_program(struct VM *vm, struct mm_struct *mm, unsigned long addr, struct vm_area_struct *newvma) {
    vm->mm = mm;
    vm->addr = addr;
    vm->newvma = newvma;
    vm->sp = 0;
    vm->pc = 0;
    vm_run(vm);
}

int main() {
    struct VM vm;
    vm.instr[0] = FLUSH_ALL;
    vm.instr[1] = VALID_MM;
    vm.instr[2] = UPDATE;
    vm.instr[3] = FIND;
    vm.instr[4] = FIND_EXACT;
    vm.instr[5] = END;

    struct mm_struct mm;
    unsigned long addr = 0;
    struct vm_area_struct newvma;

    execute_program(&vm, &mm, addr, &newvma);
    return 0;
}