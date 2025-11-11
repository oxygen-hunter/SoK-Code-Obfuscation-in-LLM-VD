#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/histogram.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/statistic.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

typedef enum {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} InstructionSet;

typedef struct {
  InstructionSet *instructions;
  int *stack;
  int pc;
  int sp;
  int running;
} VM;

void initVM(VM *vm, InstructionSet *instructions) {
  vm->instructions = instructions;
  vm->stack = (int *) malloc(1024 * sizeof(int));
  vm->pc = 0;
  vm->sp = -1;
  vm->running = 1;
}

void push(VM *vm, int value) {
  vm->stack[++vm->sp] = value;
}

int pop(VM *vm) {
  return vm->stack[vm->sp--];
}

void execute(VM *vm) {
  while (vm->running) {
    switch (vm->instructions[vm->pc++]) {
      case PUSH:
        push(vm, vm->instructions[vm->pc++]);
        break;
      case POP:
        pop(vm);
        break;
      case ADD: {
        int a = pop(vm);
        int b = pop(vm);
        push(vm, a + b);
        break;
      }
      case SUB: {
        int a = pop(vm);
        int b = pop(vm);
        push(vm, a - b);
        break;
      }
      case JMP:
        vm->pc = vm->instructions[vm->pc];
        break;
      case JZ: {
        int addr = vm->instructions[vm->pc++];
        if (pop(vm) == 0)
          vm->pc = addr;
        break;
      }
      case HALT:
        vm->running = 0;
        break;
      default:
        break;
    }
  }
}

static MagickBooleanType WriteMAPImage(const ImageInfo *, Image *, ExceptionInfo *);

static Image *ReadMAPImage(const ImageInfo *image_info, ExceptionInfo *exception) {
  VM vm;
  InstructionSet program[] = {
    PUSH, 0,  // Image initialization
    PUSH, 1, PUSH, 256, CALL, 1,  // Colormap initialization
    PUSH, 0, PUSH, 0, CALL, 2,  // Read colormap and pixels
    HALT
  };
  initVM(&vm, program);
  execute(&vm);
  return (Image *) vm.stack[vm.sp];
}

ModuleExport size_t RegisterMAPImage(void) {
  MagickInfo *entry;
  entry = AcquireMagickInfo("MAP", "MAP", "Colormap intensities and indices");
  entry->decoder = (DecodeImageHandler *) ReadMAPImage;
  entry->encoder = (EncodeImageHandler *) WriteMAPImage;
  entry->flags ^= CoderAdjoinFlag;
  entry->format_type = ExplicitFormatType;
  entry->flags |= CoderRawSupportFlag;
  entry->flags |= CoderEndianSupportFlag;
  (void) RegisterMagickInfo(entry);
  return (MagickImageCoderSignature);
}

ModuleExport void UnregisterMAPImage(void) {
  (void) UnregisterMagickInfo("MAP");
}

static MagickBooleanType WriteMAPImage(const ImageInfo *image_info, Image *image, ExceptionInfo *exception) {
  VM vm;
  InstructionSet program[] = {
    PUSH, 0,  // Image initialization
    PUSH, 1, PUSH, 256, CALL, 1,  // Colormap initialization
    PUSH, 0, PUSH, 0, CALL, 2,  // Write colormap and pixels
    HALT
  };
  initVM(&vm, program);
  execute(&vm);
  return (MagickBooleanType) vm.stack[vm.sp];
}