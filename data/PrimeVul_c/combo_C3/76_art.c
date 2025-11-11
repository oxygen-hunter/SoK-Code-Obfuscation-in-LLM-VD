#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"

typedef enum {
  PUSH_CONST, LOAD, STORE, ADD, SUB, JMP, JZ, HALT, CALL, RET
} InstructionSet;

typedef struct {
  int pc;
  int stack[256];
  int sp;
  int variables[256];
} VM;

void runVM(VM *vm, int *program) {
  int running = 1;
  while (running) {
    switch (program[vm->pc]) {
      case PUSH_CONST:
        vm->stack[++vm->sp] = program[++vm->pc];
        break;
      case LOAD:
        vm->stack[++vm->sp] = vm->variables[program[++vm->pc]];
        break;
      case STORE:
        vm->variables[program[++vm->pc]] = vm->stack[vm->sp--];
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
        vm->pc = program[++vm->pc] - 1;
        break;
      case JZ:
        if (vm->stack[vm->sp--] == 0)
          vm->pc = program[++vm->pc] - 1;
        else
          vm->pc++;
        break;
      case HALT:
        running = 0;
        break;
      case CALL:
        vm->stack[++vm->sp] = vm->pc + 2;
        vm->pc = program[++vm->pc] - 1;
        break;
      case RET:
        vm->pc = vm->stack[vm->sp--] - 1;
        break;
    }
    vm->pc++;
  }
}

static Image *ReadARTImage(const ImageInfo *image_info,ExceptionInfo *exception) {
  int program[] = {
    PUSH_CONST, 0, STORE, 0,
    PUSH_CONST, 0, STORE, 1,
    PUSH_CONST, 0, STORE, 2,
    PUSH_CONST, 0, STORE, 3,
    PUSH_CONST, 0, STORE, 4,
    CALL, 9, HALT,
    // Function: InitImage
    PUSH_CONST, 1, STORE, 5,
    PUSH_CONST, 1, STORE, 6,
    PUSH_CONST, 1, STORE, 7,
    PUSH_CONST, 4, STORE, 8,
    RET
  };
  
  VM vm = {0, {0}, -1, {0}};
  runVM(&vm, program);
  
  Image *image = AcquireImage(image_info, exception);
  MagickBooleanType status = OpenBlob(image_info, image, ReadBinaryBlobMode, exception);
  if (status == MagickFalse) {
    image = DestroyImageList(image);
    return((Image *) NULL);
  }
  image->depth = 1;
  image->endian = MSBEndian;
  (void) ReadBlobLSBShort(image);
  image->columns = (size_t) ReadBlobLSBShort(image);
  (void) ReadBlobLSBShort(image);
  image->rows = (size_t) ReadBlobLSBShort(image);
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(CorruptImageError, "ImproperImageHeader");
  if (image_info->ping != MagickFalse) {
    (void) CloseBlob(image);
    return(GetFirstImageInList(image));
  }
  status = SetImageExtent(image, image->columns, image->rows, exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  SetImageColorspace(image, GRAYColorspace, exception);
  QuantumInfo *quantum_info = AcquireQuantumInfo(image_info, image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError, "MemoryAllocationFailed");
  size_t length = GetQuantumExtent(image, quantum_info, GrayQuantum);
  for (ssize_t y = 0; y < (ssize_t) image->rows; y++) {
    Quantum *magick_restrict q = QueueAuthenticPixels(image, 0, y, image->columns, 1, exception);
    if (q == (Quantum *) NULL)
      break;
    const unsigned char *pixels = (const unsigned char *) ReadBlobStream(image, length, GetQuantumPixels(quantum_info), &count);
    if (count != (ssize_t) length)
      ThrowReaderException(CorruptImageError, "UnableToReadImageData");
    (void) ImportQuantumPixels(image, (CacheView *) NULL, quantum_info, GrayQuantum, pixels, exception);
    pixels = (const unsigned char *) ReadBlobStream(image, (size_t) (-(ssize_t) length) & 0x01, GetQuantumPixels(quantum_info), &count);
    if (SyncAuthenticPixels(image, exception) == MagickFalse)
      break;
    if (SetImageProgress(image, LoadImageTag, y, image->rows) == MagickFalse)
      break;
  }
  SetQuantumImageType(image, GrayQuantum);
  quantum_info = DestroyQuantumInfo(quantum_info);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception, CorruptImageError, "UnexpectedEndOfFile", image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

ModuleExport size_t RegisterARTImage(void) {
  MagickInfo *entry;
  entry = AcquireMagickInfo("ART", "ART", "PFS: 1st Publisher Clip Art");
  entry->decoder = (DecodeImageHandler *) ReadARTImage;
  entry->encoder = (EncodeImageHandler *) WriteARTImage;
  entry->flags |= CoderRawSupportFlag;
  entry->flags ^= CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

ModuleExport void UnregisterARTImage(void) {
  (void) UnregisterMagickInfo("ART");
}

static MagickBooleanType WriteARTImage(const ImageInfo *image_info, Image *image, ExceptionInfo *exception) {
  MagickBooleanType status = OpenBlob(image_info, image, WriteBinaryBlobMode, exception);
  if (status == MagickFalse)
    return(status);
  if ((image->columns > 65535UL) || (image->rows > 65535UL))
    ThrowWriterException(ImageError, "WidthOrHeightExceedsLimit");
  (void) TransformImageColorspace(image, sRGBColorspace, exception);
  (void) SetImageType(image, BilevelType, exception);
  image->endian = MSBEndian;
  image->depth = 1;
  (void) WriteBlobLSBShort(image, 0);
  (void) WriteBlobLSBShort(image, (unsigned short) image->columns);
  (void) WriteBlobLSBShort(image, 0);
  (void) WriteBlobLSBShort(image, (unsigned short) image->rows);
  QuantumInfo *quantum_info = AcquireQuantumInfo(image_info, image);
  unsigned char *pixels = (unsigned char *) GetQuantumPixels(quantum_info);
  for (ssize_t y = 0; y < (ssize_t) image->rows; y++) {
    const Quantum *p = GetVirtualPixels(image, 0, y, image->columns, 1, exception);
    if (p == (const Quantum *) NULL)
      break;
    size_t length = ExportQuantumPixels(image, (CacheView *) NULL, quantum_info, GrayQuantum, pixels, exception);
    ssize_t count = WriteBlob(image, length, pixels);
    if (count != (ssize_t) length)
      ThrowWriterException(CorruptImageError, "UnableToWriteImageData");
    count = WriteBlob(image, (size_t) (-(ssize_t) length) & 0x01, pixels);
    status = SetImageProgress(image, SaveImageTag, (MagickOffsetType) y, image->rows);
    if (status == MagickFalse)
      break;
  }
  quantum_info = DestroyQuantumInfo(quantum_info);
  (void) CloseBlob(image);
  return(status);
}