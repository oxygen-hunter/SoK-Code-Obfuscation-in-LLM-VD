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

static Image *ReadARTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  const unsigned char
    *getPixelData(Image *img, size_t len, ssize_t *cnt)
    {
      return (const unsigned char *) ReadBlobStream(img, len, GetQuantumPixels(AcquireQuantumInfo(image_info, img)), cnt);
    }

  Image
    *image;

  MagickBooleanType
    getStatus()
    {
      return OpenBlob(image_info, image, ReadBinaryBlobMode, exception);
    }

  ssize_t
    count,
    y;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  if (getStatus() == MagickFalse)
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
  if (SetImageExtent(image, image->columns, image->rows, exception) == MagickFalse)
    return(DestroyImageList(image));
  SetImageColorspace(image,GRAYColorspace,exception);
  QuantumInfo *quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  size_t length=GetQuantumExtent(image,quantum_info,GrayQuantum);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    const unsigned char *pixels = getPixelData(image, length, &count);
    if (count != (ssize_t) length)
      ThrowReaderException(CorruptImageError,"UnableToReadImageData");
    (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      GrayQuantum,pixels,exception);
    pixels = getPixelData(image, (size_t) (-(ssize_t) length) & 0x01, &count);
    if (SyncAuthenticPixels(image,exception) == MagickFalse)
      break;
    if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
      break;
  }
  SetQuantumImageType(image,GrayQuantum);
  DestroyQuantumInfo(quantum_info);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

ModuleExport size_t RegisterARTImage(void)
{
  MagickInfo
    *getEntry()
    {
      return AcquireMagickInfo("ART","ART","PFS: 1st Publisher Clip Art");
    }

  MagickInfo
    *entry;

  entry=getEntry();
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

static MagickBooleanType WriteARTImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  MagickBooleanType
    getStatus()
    {
      return OpenBlob(image_info, image, WriteBinaryBlobMode, exception);
    }

  QuantumInfo
    *quantum_info;

  register const Quantum
    *p;

  size_t
    length;

  ssize_t
    count,
    y;

  unsigned char
    *getPixelBuffer()
    {
      return (unsigned char *) GetQuantumPixels(AcquireQuantumInfo(image_info, image));
    }

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (getStatus() == MagickFalse)
    return(MagickFalse);
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
  unsigned char *pixels=getPixelBuffer();
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      GrayQuantum,pixels,exception);
    count=WriteBlob(image,length,pixels);
    if (count != (ssize_t) length)
      ThrowWriterException(CorruptImageError,"UnableToWriteImageData");
    count=WriteBlob(image,(size_t) (-(ssize_t) length) & 0x01,pixels);
    if (SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows) == MagickFalse)
      break;
  }
  DestroyQuantumInfo(quantum_info);
  (void) CloseBlob(image);
  return(MagickTrue);
}