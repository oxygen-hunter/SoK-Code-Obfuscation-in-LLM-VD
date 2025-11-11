#define _GNU_SOURCE
#include "lxclock.h"
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>

#include <lxc/lxccontainer.h>

#include "utils.h"
#include "log.h"

#ifdef MUTEX_DEBUGGING
#include <execinfo.h>
#endif

#define MAX_STACKDEPTH 25

#define OFLAG (O_CREAT | O_RDWR)
#define SEMMODE 0660
#define SEMVALUE 1
#define SEMVALUE_LOCKED 0

lxc_log_define(lxc_lock, lxc);

#ifdef MUTEX_DEBUGGING
static pthread_mutex_t thread_mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER_NP;

static inline void dump_stacktrace(void)
{
	void *array[MAX_STACKDEPTH];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, MAX_STACKDEPTH);
	strings = backtrace_symbols(array, size);

	fprintf(stderr, "\tObtained %zd stack frames.\n", size);

	for (i = 0; i < size; i++)
		fprintf(stderr, "\t\t%s\n", strings[i]);

	free (strings);
}
#else
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline void dump_stacktrace(void) {;}
#endif

typedef enum {
	VM_NOP,
	VM_PUSH,
	VM_POP,
	VM_ADD,
	VM_SUB,
	VM_LOAD,
	VM_STORE,
	VM_JMP,
	VM_JZ,
	VM_CALL_LOCK_MUTEX,
	VM_CALL_UNLOCK_MUTEX,
	VM_CALL_LXCLOCK_NAME,
	VM_CALL_LXC_NEW_UNNAMED_SEM,
	VM_CALL_LXC_NEWLOCK,
	VM_CALL_LXCLOCK,
	VM_CALL_LXCUNLOCK,
	VM_CALL_LXC_PUTLOCK,
	VM_CALL_PROCESS_LOCK,
	VM_CALL_PROCESS_UNLOCK,
	VM_CALL_CONTAINER_MEM_LOCK,
	VM_CALL_CONTAINER_MEM_UNLOCK,
	VM_CALL_CONTAINER_DISK_LOCK,
	VM_CALL_CONTAINER_DISK_UNLOCK,
} VMInstruction;

typedef struct {
	int pc;
	int sp;
	int stack[256];
	void* registers[16];
} VM;

void vm_execute(VM* vm, VMInstruction* program, int program_length) {
	while (vm->pc < program_length) {
		switch (program[vm->pc]) {
			case VM_NOP:
				break;
			case VM_PUSH:
				vm->stack[++vm->sp] = program[++vm->pc];
				break;
			case VM_POP:
				vm->sp--;
				break;
			case VM_ADD:
				vm->stack[vm->sp-1] += vm->stack[vm->sp];
				vm->sp--;
				break;
			case VM_SUB:
				vm->stack[vm->sp-1] -= vm->stack[vm->sp];
				vm->sp--;
				break;
			case VM_LOAD:
				vm->stack[++vm->sp] = (int)vm->registers[program[++vm->pc]];
				break;
			case VM_STORE:
				vm->registers[program[++vm->pc]] = (void*)vm->stack[vm->sp--];
				break;
			case VM_JMP:
				vm->pc = program[++vm->pc] - 1;
				break;
			case VM_JZ:
				if (vm->stack[vm->sp--] == 0)
					vm->pc = program[++vm->pc] - 1;
				else
					++vm->pc;
				break;
			case VM_CALL_LOCK_MUTEX:
				lock_mutex((pthread_mutex_t*)vm->registers[0]);
				break;
			case VM_CALL_UNLOCK_MUTEX:
				unlock_mutex((pthread_mutex_t*)vm->registers[0]);
				break;
			case VM_CALL_LXCLOCK_NAME:
				vm->registers[0] = lxclock_name((const char*)vm->registers[0], (const char*)vm->registers[1]);
				break;
			case VM_CALL_LXC_NEW_UNNAMED_SEM:
				vm->registers[0] = lxc_new_unnamed_sem();
				break;
			case VM_CALL_LXC_NEWLOCK:
				vm->registers[0] = lxc_newlock((const char*)vm->registers[0], (const char*)vm->registers[1]);
				break;
			case VM_CALL_LXCLOCK:
				vm->stack[++vm->sp] = lxclock((struct lxc_lock*)vm->registers[0], vm->stack[vm->sp]);
				break;
			case VM_CALL_LXCUNLOCK:
				vm->stack[++vm->sp] = lxcunlock((struct lxc_lock*)vm->registers[0]);
				break;
			case VM_CALL_LXC_PUTLOCK:
				lxc_putlock((struct lxc_lock*)vm->registers[0]);
				break;
			case VM_CALL_PROCESS_LOCK:
				process_lock();
				break;
			case VM_CALL_PROCESS_UNLOCK:
				process_unlock();
				break;
			case VM_CALL_CONTAINER_MEM_LOCK:
				vm->stack[++vm->sp] = container_mem_lock((struct lxc_container*)vm->registers[0]);
				break;
			case VM_CALL_CONTAINER_MEM_UNLOCK:
				container_mem_unlock((struct lxc_container*)vm->registers[0]);
				break;
			case VM_CALL_CONTAINER_DISK_LOCK:
				vm->stack[++vm->sp] = container_disk_lock((struct lxc_container*)vm->registers[0]);
				break;
			case VM_CALL_CONTAINER_DISK_UNLOCK:
				container_disk_unlock((struct lxc_container*)vm->registers[0]);
				break;
		}
		vm->pc++;
	}
}

void lock_mutex(pthread_mutex_t *l)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_LOCK_MUTEX,
		VM_NOP
	};
	vm.registers[0] = (void*)l;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
}

void unlock_mutex(pthread_mutex_t *l)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_UNLOCK_MUTEX,
		VM_NOP
	};
	vm.registers[0] = (void*)l;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
}

char *lxclock_name(const char *p, const char *n)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_LXCLOCK_NAME,
		VM_NOP
	};
	vm.registers[0] = (void*)p;
	vm.registers[1] = (void*)n;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
	return (char*)vm.registers[0];
}

sem_t *lxc_new_unnamed_sem(void)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_LXC_NEW_UNNAMED_SEM,
		VM_NOP
	};
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
	return (sem_t*)vm.registers[0];
}

struct lxc_lock *lxc_newlock(const char *lxcpath, const char *name)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_LXC_NEWLOCK,
		VM_NOP
	};
	vm.registers[0] = (void*)lxcpath;
	vm.registers[1] = (void*)name;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
	return (struct lxc_lock*)vm.registers[0];
}

int lxclock(struct lxc_lock *l, int timeout)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_PUSH, timeout,
		VM_CALL_LXCLOCK,
		VM_POP,
		VM_NOP
	};
	vm.registers[0] = (void*)l;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
	return vm.stack[vm.sp];
}

int lxcunlock(struct lxc_lock *l)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_LXCUNLOCK,
		VM_POP,
		VM_NOP
	};
	vm.registers[0] = (void*)l;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
	return vm.stack[vm.sp];
}

void lxc_putlock(struct lxc_lock *l)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_LXC_PUTLOCK,
		VM_NOP
	};
	vm.registers[0] = (void*)l;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
}

void process_lock(void)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_PROCESS_LOCK,
		VM_NOP
	};
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
}

void process_unlock(void)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_PROCESS_UNLOCK,
		VM_NOP
	};
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
}

int container_mem_lock(struct lxc_container *c)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_CONTAINER_MEM_LOCK,
		VM_POP,
		VM_NOP
	};
	vm.registers[0] = (void*)c;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
	return vm.stack[vm.sp];
}

void container_mem_unlock(struct lxc_container *c)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_CONTAINER_MEM_UNLOCK,
		VM_NOP
	};
	vm.registers[0] = (void*)c;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
}

int container_disk_lock(struct lxc_container *c)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_CONTAINER_DISK_LOCK,
		VM_POP,
		VM_NOP
	};
	vm.registers[0] = (void*)c;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
	return vm.stack[vm.sp];
}

void container_disk_unlock(struct lxc_container *c)
{
	VM vm = {0};
	VMInstruction program[] = {
		VM_CALL_CONTAINER_DISK_UNLOCK,
		VM_NOP
	};
	vm.registers[0] = (void*)c;
	vm_execute(&vm, program, sizeof(program)/sizeof(VMInstruction));
}