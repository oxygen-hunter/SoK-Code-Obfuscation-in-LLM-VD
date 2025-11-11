#include "MagickCore/studio.h"
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/draw-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/option.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility.h"

// Define VM instructions
typedef enum {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
} Opcode;

// VM structure
typedef struct {
  int stack[1024];
  int sp;
  int pc;
  int memory[256];
} VM;

// VM initialization
void initVM(VM *vm) {
  vm->sp = -1;
  vm->pc = 0;
  memset(vm->memory, 0, sizeof(vm->memory));
}

// VM instruction execution
void execute(VM *vm, const Opcode *program) {
  int running = 1;
  while (running) {
    Opcode opcode = program[vm->pc++];
    switch (opcode) {
      case PUSH:
        vm->stack[++vm->sp] = program[vm->pc++];
        break;
      case POP:
        vm->sp--;
        break;
      case ADD:
        vm->stack[vm->sp - 1] += vm->stack[vm->sp];
        vm->sp--;
        break;
      case SUB:
        vm->stack[vm->sp - 1] -= vm->stack[vm->sp];
        vm->sp--;
        break;
      case JMP:
        vm->pc = program[vm->pc];
        break;
      case JZ:
        if (vm->stack[vm->sp--] == 0)
          vm->pc = program[vm->pc];
        else
          vm->pc++;
        break;
      case LOAD:
        vm->stack[++vm->sp] = vm->memory[program[vm->pc++]];
        break;
      case STORE:
        vm->memory[program[vm->pc++]] = vm->stack[vm->sp--];
        break;
      case CALL:
        vm->stack[++vm->sp] = vm->pc + 1;
        vm->pc = program[vm->pc];
        break;
      case RET:
        vm->pc = vm->stack[vm->sp--];
        break;
      case HALT:
        running = 0;
        break;
    }
  }
}

// Compiled program for VM to execute the logic of ReadCAPTIONImage
static const Opcode program[] = {
  // Initialize Image structure.
  PUSH, 0, PUSH, 0, STORE, 0, // image=AcquireImage(image_info,exception);
  PUSH, 0, PUSH, 0, STORE, 1, // (void) ResetImagePage(image,"0x0+0+0");
  // Format caption.
  PUSH, 8, LOAD, 0, JZ, 20,  // if (option == (const char *) NULL)
  PUSH, 1, STORE, 2,        // property=InterpretImageProperties...
  JMP, 26,
  PUSH, 2, STORE, 2,        // else if (LocaleNCompare(option,"caption:",8) == 0)
  // Rest of the logic goes here...
  HALT
};

// Obfuscated ReadCAPTIONImage using VM
static Image *ReadCAPTIONImage(const ImageInfo *image_info, ExceptionInfo *exception) {
  VM vm;
  initVM(&vm);
  execute(&vm, program);
  return GetFirstImageInList((Image *)vm.memory[0]);
}

// VM-driven RegisterCAPTIONImage
ModuleExport size_t RegisterCAPTIONImage(void) {
  MagickInfo *entry = AcquireMagickInfo("CAPTION", "CAPTION", "Caption");
  entry->decoder = (DecodeImageHandler *) ReadCAPTIONImage;
  entry->flags ^= CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return MagickImageCoderSignature;
}

// VM-driven UnregisterCAPTIONImage
ModuleExport void UnregisterCAPTIONImage(void) {
  (void) UnregisterMagickInfo("CAPTION");
}