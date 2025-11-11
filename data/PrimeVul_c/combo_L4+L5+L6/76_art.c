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

static MagickBooleanType
  WriteARTImage(const ImageInfo *,Image *,ExceptionInfo *);

static Image *ReadARTImageRecursive(const ImageInfo *image_info,ExceptionInfo *exception, Image *image, QuantumInfo *quantum_info, ssize_t y)
{
  if (y >= (ssize_t) image->rows)
    return image;

  register Quantum
    *magick_restrict q;

  const unsigned char
    *pixels;

  ssize_t
    count;

  q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
  if (q == (Quantum *) NULL)
    return image;
  pixels=(const unsigned char *) ReadBlobStream(image,
    GetQuantumExtent(image,quantum_info,GrayQuantum),
    GetQuantumPixels(quantum_info),&count);
  if (count != (ssize_t) GetQuantumExtent(image,quantum_info,GrayQuantum))
    ThrowReaderException(CorruptImageError,"UnableToReadImageData");
  (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
    GrayQuantum,pixels,exception);
  pixels=(const unsigned char *) ReadBlobStream(image,(size_t) (-(ssize_t)
    GetQuantumExtent(image,quantum_info,GrayQuantum)) & 0x01,GetQuantumPixels(quantum_info),&count);
  if (SyncAuthenticPixels(image,exception) == MagickFalse)
    return image;
  if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
    return image;

  return ReadARTImageRecursive(image_info, exception, image, quantum_info, y + 1);
}

static Image *ReadARTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  const unsigned char
    *pixels;

  Image
    *image;

  QuantumInfo
    *quantum_info;

  MagickBooleanType
    status;

  size_t
    length;

  ssize_t
    count;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  image->depth=1;
  image->endian=MSBEndian;
  (void) ReadBlobLSBShort(image);
  image->columns=(size_t) ReadBlobLSBShort(image);
  (void) ReadBlobLSBShort(image);
  image->rows=(size_t) ReadBlobLSBShort(image);
  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  SetImageColorspace(image,GRAYColorspace,exception);
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

  image = ReadARTImageRecursive(image_info, exception, image, quantum_info, 0);

  SetQuantumImageType(image,GrayQuantum);
  quantum_info=DestroyQuantumInfo(quantum_info);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

ModuleExport size_t RegisterARTImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("ART","ART","PFS: 1st Publisher Clip Art");
  entry->decoder=(DecodeImageHandler *) ReadARTImage;
  entry->encoder=(EncodeImageHandler *) WriteARTImage;
  entry->flags|=CoderRawSupportFlag;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

ModuleExport void UnregisterARTImage(void)
{
  (void) UnregisterMagickInfo("ART");
}

static MagickBooleanType WriteARTImageRecursive(const ImageInfo *image_info, Image *image, ExceptionInfo *exception, QuantumInfo *quantum_info, unsigned char *pixels, ssize_t y)
{
  if (y >= (ssize_t) image->rows)
    return MagickTrue;

  register const Quantum
    *p;

  size_t
    length;

  ssize_t
    count;

  p=GetVirtualPixels(image,0,y,image->columns,1,exception);
  if (p == (const Quantum *) NULL)
    return MagickFalse;
  length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
    GrayQuantum,pixels,exception);
  count=WriteBlob(image,length,pixels);
  if (count != (ssize_t) length)
    ThrowWriterException(CorruptImageError,"UnableToWriteImageData");
  count=WriteBlob(image,(size_t) (-(ssize_t) length) & 0x01,pixels);
  MagickBooleanType status = SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
    image->rows);
  if (status == MagickFalse)
    return MagickFalse;

  return WriteARTImageRecursive(image_info, image, exception, quantum_info, pixels, y + 1);
}

static MagickBooleanType WriteARTImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  QuantumInfo
    *quantum_info;

  unsigned char
    *pixels;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if ((image->columns > 65535UL) || (image->rows > 65535UL))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  (void) TransformImageColorspace(image,sRGBColorspace,exception);
  (void) SetImageType(image,BilevelType,exception);
  image->endian=MSBEndian;
  image->depth=1;
  (void) WriteBlobLSBShort(image,0);
  (void) WriteBlobLSBShort(image,(unsigned short) image->columns);
  (void) WriteBlobLSBShort(image,0);
  (void) WriteBlobLSBShort(image,(unsigned short) image->rows);
  quantum_info=AcquireQuantumInfo(image_info,image);
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);

  status = WriteARTImageRecursive(image_info, image, exception, quantum_info, pixels, 0);

  quantum_info=DestroyQuantumInfo(quantum_info);
  (void) CloseBlob(image);
  return(status);
}