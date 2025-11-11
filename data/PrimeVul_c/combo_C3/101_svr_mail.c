#include <pbs_config.h>   
#include "pbs_ifl.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list_link.h"
#include "attribute.h"
#include "server_limits.h"
#include "pbs_job.h"
#include "log.h"
#include "server.h"
#include "rpp.h"

extern void net_close(int);
extern void svr_format_job(FILE *, job *, char *, int, char *);
extern struct server server;
extern int LOGLEVEL;

typedef enum {
  VM_PUSH, VM_POP, VM_ADD, VM_SUB, VM_JMP, VM_JZ, VM_LOAD, VM_STORE, VM_EXEC
} Instruction;

typedef struct {
  int stack[256];
  int sp;
  int pc;
  Instruction program[1024];
  int data[256];
} VM;

void vm_init(VM *vm) {
  vm->sp = -1;
  vm->pc = 0;
}

void vm_execute(VM *vm) {
  while (1) {
    switch (vm->program[vm->pc++]) {
    case VM_PUSH:
      vm->stack[++vm->sp] = vm->program[vm->pc++];
      break;
    case VM_POP:
      --vm->sp;
      break;
    case VM_ADD:
      vm->stack[vm->sp - 1] += vm->stack[vm->sp];
      --vm->sp;
      break;
    case VM_SUB:
      vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
      --vm->sp;
      break;
    case VM_JMP:
      vm->pc = vm->program[vm->pc];
      break;
    case VM_JZ:
      if (vm->stack[vm->sp--] == 0) {
        vm->pc = vm->program[vm->pc];
      } else {
        vm->pc++;
      }
      break;
    case VM_LOAD:
      vm->stack[++vm->sp] = vm->data[vm->program[vm->pc++]];
      break;
    case VM_STORE:
      vm->data[vm->program[vm->pc++]] = vm->stack[vm->sp--];
      break;
    case VM_EXEC:
      return;
    }
  }
}

void svr_mailowner(job *pjob, int mailpoint, int force, char *text) {
  VM vm;
  vm_init(&vm);

  int i = 0;
  vm.program[i++] = VM_PUSH; vm.program[i++] = (int)pjob;
  vm.program[i++] = VM_PUSH; vm.program[i++] = mailpoint;
  vm.program[i++] = VM_PUSH; vm.program[i++] = force;
  vm.program[i++] = VM_PUSH; vm.program[i++] = (int)text;
  vm.program[i++] = VM_LOAD;
  vm.program[i++] = VM_EXEC;

  vm_execute(&vm);
}

void vm_load(VM *vm) {
  vm->program[0] = VM_PUSH;
  vm->program[1] = VM_POP;
  vm->program[2] = VM_STORE;
  vm->program[3] = VM_LOAD;
  vm->program[4] = VM_ADD;
  vm->program[5] = VM_SUB;
  vm->program[6] = VM_JMP;
  vm->program[7] = VM_JZ;
  vm->program[8] = VM_EXEC;
}