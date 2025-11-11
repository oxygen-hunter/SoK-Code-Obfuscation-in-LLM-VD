#define _GNU_SOURCE
#include "lxclock.h"
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

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

	free(strings);
}
#else
static pthread_mutex_t thread_mutex = PTHREAD_MUTEX_INITIALIZER;

static inline void dump_stacktrace(void) {;}
#endif

typedef enum {
	INS_NOP,
	INS_PUSH,
	INS_POP,
	INS_ADD,
	INS_SUB,
	INS_JMP,
	INS_JZ,
	INS_LOAD,
	INS_STORE,
	INS_CALL_LOCK_MUTEX,
	INS_CALL_UNLOCK_MUTEX,
	INS_CALL_LXCLOCK_NAME,
	INS_CALL_LXC_NEW_UNNAMED_SEM,
	INS_CALL_LXC_NEWLOCK,
	INS_CALL_LXCLOCK,
	INS_CALL_LXCUNLOCK,
	INS_CALL_LXC_PUTLOCK,
	INS_CALL_PROCESS_LOCK,
	INS_CALL_PROCESS_UNLOCK,
	INS_CALL_CONTAINER_MEM_LOCK,
	INS_CALL_CONTAINER_MEM_UNLOCK,
	INS_CALL_CONTAINER_DISK_LOCK,
	INS_CALL_CONTAINER_DISK_UNLOCK,
	INS_HALT
} Instruction;

#define STACK_SIZE 1024

typedef struct {
	int stack[STACK_SIZE];
	int sp;
	int pc;
	Instruction instructions[STACK_SIZE];
} VM;

static void vm_init(VM *vm) {
	vm->sp = -1;
	vm->pc = 0;
}

static void vm_push(VM *vm, int value) {
	vm->stack[++vm->sp] = value;
}

static int vm_pop(VM *vm) {
	return vm->stack[vm->sp--];
}

static void vm_execute(VM *vm) {
	int running = 1;
	while (running) {
		Instruction ins = vm->instructions[vm->pc++];
		switch (ins) {
			case INS_NOP:
				break;
			case INS_PUSH: {
				int value = vm->instructions[vm->pc++];
				vm_push(vm, value);
				break;
			}
			case INS_POP:
				vm_pop(vm);
				break;
			case INS_ADD: {
				int b = vm_pop(vm);
				int a = vm_pop(vm);
				vm_push(vm, a + b);
				break;
			}
			case INS_SUB: {
				int b = vm_pop(vm);
				int a = vm_pop(vm);
				vm_push(vm, a - b);
				break;
			}
			case INS_JMP: {
				int addr = vm->instructions[vm->pc++];
				vm->pc = addr;
				break;
			}
			case INS_JZ: {
				int addr = vm->instructions[vm->pc++];
				int value = vm_pop(vm);
				if (value == 0) {
					vm->pc = addr;
				}
				break;
			}
			case INS_LOAD: {
				int addr = vm->instructions[vm->pc++];
				vm_push(vm, vm->stack[addr]);
				break;
			}
			case INS_STORE: {
				int addr = vm->instructions[vm->pc++];
				vm->stack[addr] = vm_pop(vm);
				break;
			}
			case INS_CALL_LOCK_MUTEX:
				lock_mutex(&thread_mutex);
				break;
			case INS_CALL_UNLOCK_MUTEX:
				unlock_mutex(&thread_mutex);
				break;
			case INS_CALL_LXCLOCK_NAME: {
				const char *p = (const char *)vm_pop(vm);
				const char *n = (const char *)vm_pop(vm);
				char *result = lxclock_name(p, n);
				vm_push(vm, (int)result);
				break;
			}
			case INS_CALL_LXC_NEW_UNNAMED_SEM: {
				sem_t *sem = lxc_new_unnamed_sem();
				vm_push(vm, (int)sem);
				break;
			}
			case INS_CALL_LXC_NEWLOCK: {
				const char *lxcpath = (const char *)vm_pop(vm);
				const char *name = (const char *)vm_pop(vm);
				struct lxc_lock *lock = lxc_newlock(lxcpath, name);
				vm_push(vm, (int)lock);
				break;
			}
			case INS_CALL_LXCLOCK: {
				struct lxc_lock *lock = (struct lxc_lock *)vm_pop(vm);
				int timeout = vm_pop(vm);
				int result = lxclock(lock, timeout);
				vm_push(vm, result);
				break;
			}
			case INS_CALL_LXCUNLOCK: {
				struct lxc_lock *lock = (struct lxc_lock *)vm_pop(vm);
				int result = lxcunlock(lock);
				vm_push(vm, result);
				break;
			}
			case INS_CALL_LXC_PUTLOCK: {
				struct lxc_lock *lock = (struct lxc_lock *)vm_pop(vm);
				lxc_putlock(lock);
				break;
			}
			case INS_CALL_PROCESS_LOCK:
				process_lock();
				break;
			case INS_CALL_PROCESS_UNLOCK:
				process_unlock();
				break;
			case INS_CALL_CONTAINER_MEM_LOCK: {
				struct lxc_container *c = (struct lxc_container *)vm_pop(vm);
				int result = container_mem_lock(c);
				vm_push(vm, result);
				break;
			}
			case INS_CALL_CONTAINER_MEM_UNLOCK: {
				struct lxc_container *c = (struct lxc_container *)vm_pop(vm);
				container_mem_unlock(c);
				break;
			}
			case INS_CALL_CONTAINER_DISK_LOCK: {
				struct lxc_container *c = (struct lxc_container *)vm_pop(vm);
				int result = container_disk_lock(c);
				vm_push(vm, result);
				break;
			}
			case INS_CALL_CONTAINER_DISK_UNLOCK: {
				struct lxc_container *c = (struct lxc_container *)vm_pop(vm);
				container_disk_unlock(c);
				break;
			}
			case INS_HALT:
				running = 0;
				break;
			default:
				fprintf(stderr, "Unknown instruction: %d\n", ins);
				exit(1);
		}
	}
}

int main() {
	VM vm;
	vm_init(&vm);

	vm.instructions[0] = INS_CALL_PROCESS_LOCK;
	vm.instructions[1] = INS_CALL_PROCESS_UNLOCK;
	vm.instructions[2] = INS_HALT;

	vm_execute(&vm);
	return 0;
}