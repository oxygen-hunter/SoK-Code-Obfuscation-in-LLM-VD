```c
#include "MagickCore/studio.h"
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
#  if defined(__CYGWIN__)
#    include <windows.h>
#  else
#    include "MagickCore/nt-base-private.h"
#    include <wingdi.h>
#  ifndef DISPLAY_DEVICE_ACTIVE
#    define DISPLAY_DEVICE_ACTIVE    0x00000001
#  endif
#  endif
#endif
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/nt-feature.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/xwindow.h"
#include "MagickCore/xwindow-private.h"

typedef enum {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT, CALL, RET
} OpCode;

typedef struct {
  int stack[256];
  int sp;
  int pc;
  int code[1024];
} VM;

void vm_init(VM *vm) {
  vm->sp = -1;
  vm->pc = 0;
}

void vm_push(VM *vm, int value) {
  vm->stack[++vm->sp] = value;
}

int vm_pop(VM *vm) {
  return vm->stack[vm->sp--];
}

void vm_execute(VM *vm) {
  int running = 1;
  while (running) {
    switch (vm->code[vm->pc++]) {
      case PUSH:
        vm_push(vm, vm->code[vm->pc++]);
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
        vm->pc = vm->code[vm->pc];
        break;
      case JZ:
        if (vm_pop(vm) == 0)
          vm->pc = vm->code[vm->pc];
        else
          vm->pc++;
        break;
      case LOAD:
        vm_push(vm, vm->code[vm->pc++]);
        break;
      case STORE: {
        int value = vm_pop(vm);
        int addr = vm->code[vm->pc++];
        vm->stack[addr