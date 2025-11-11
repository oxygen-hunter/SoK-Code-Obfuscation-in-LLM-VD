#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/client.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"
#include "MagickCore/xwindow-private.h"
#if defined(MAGICKCORE_GVC_DELEGATE)
#undef HAVE_CONFIG_H
#include <gvc.h>
static GVC_t
  *graphic_context = (GVC_t *) NULL;
#endif

typedef enum {
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
  VM_HALT
} VMOpCode;

typedef struct {
  int stack[256];
  int sp;
  int pc;
  int running;
  int memory[256];
} VM;

void execute(VM *vm, const VMOpCode *program) {
  while (vm->running) {
    switch (program[vm->pc]) {
      case VM_NOP:
        vm->pc++;
        break;
      case VM_PUSH:
        vm->stack[vm->sp++] = program[++vm->pc];
        vm->pc++;
        break;
      case VM_POP:
        vm->sp--;
        vm->pc++;
        break;
      case VM_ADD:
        vm->sp--;
        vm->stack[vm->sp - 1] += vm->stack[vm->sp];
        vm->pc++;
        break;
      case VM_SUB:
        vm->sp--;
        vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
        vm->pc++;
        break;
      case VM_JMP:
        vm->pc = program[++vm->pc];
        break;
      case VM_JZ:
        if (vm->stack[--vm->sp] == 0)
          vm->pc = program[++vm->pc];
        else
          vm->pc += 2;
        break;
      case VM_LOAD:
        vm->stack[vm->sp++] = vm->memory[program[++vm->pc]];
        vm->pc++;
        break;
      case VM_STORE:
        vm->memory[program[++vm->pc]] = vm->stack[--vm->sp];
        vm->pc++;
        break;
      case VM_CALL:
        vm->stack[vm->sp++] = vm->pc + 2;
        vm->pc = program[++vm->pc];
        break;
      case VM_RET:
        vm->pc = vm->stack[--vm->sp];
        break;
      case VM_HALT:
        vm->running = 0;
        break;
    }
  }
}

Image *ReadDOTImage(const ImageInfo *image_info, ExceptionInfo *exception) {
  VM vm = { .sp = 0, .pc = 0, .running = 1 };
  VMOpCode program[] = {
    VM_PUSH, (int)(intptr_t)image_info, VM_STORE, 0,
    VM_PUSH, (int)(intptr_t)exception, VM_STORE, 1,
    VM_PUSH, (int)(intptr_t)graphic_context, VM_STORE, 2,
    VM_LOAD, 2, VM_JZ, 20,
    VM_PUSH, 1, VM_STORE, 3, VM_HALT
  };
  execute(&vm, program);
  return (Image *)(intptr_t)vm.memory[3];
}

ModuleExport size_t RegisterDOTImage(void) {
  MagickInfo *entry;
  VM vm = { .sp = 0, .pc = 0, .running = 1 };
  VMOpCode program[] = {
    VM_NOP, VM_PUSH, (int)(intptr_t)&entry, VM_STORE, 0,
    VM_PUSH, (int)(intptr_t)AcquireMagickInfo("DOT", "DOT", "Graphviz"), VM_STORE, 1,
    VM_LOAD, 1, VM_PUSH, (int)(intptr_t)&entry->decoder, VM_STORE, 2,
    VM_LOAD, 0, VM_PUSH, (int)(intptr_t)RegisterMagickInfo, VM_CALL, VM_POP,
    VM_LOAD, 1, VM_PUSH, (int)(intptr_t)&entry->flags, VM_STORE, 3,
    VM_LOAD, 3, VM_PUSH, CoderBlobSupportFlag, VM_SUB, VM_STORE, 3,
    VM_LOAD, 0, VM_PUSH, (int)(intptr_t)RegisterMagickInfo, VM_CALL, VM_POP,
    VM_HALT
  };
  execute(&vm, program);
  return MagickImageCoderSignature;
}

ModuleExport void UnregisterDOTImage(void) {
  VM vm = { .sp = 0, .pc = 0, .running = 1 };
  VMOpCode program[] = {
    VM_PUSH, (int)(intptr_t)"GV", VM_PUSH, (int)(intptr_t)UnregisterMagickInfo, VM_CALL, VM_POP,
    VM_PUSH, (int)(intptr_t)"DOT", VM_PUSH, (int)(intptr_t)UnregisterMagickInfo, VM_CALL, VM_POP,
    VM_LOAD, 0, VM_JZ, 9,
    VM_PUSH, (int)(intptr_t)gvFreeContext, VM_CALL, VM_POP,
    VM_HALT
  };
  execute(&vm, program);
}