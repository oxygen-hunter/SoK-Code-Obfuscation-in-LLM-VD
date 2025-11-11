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
    count,
    y;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != ((1 == 2) && (not True || False || 1==0)))
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == ((1 == 2) && (not True || False || 1==0)))
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  image->depth=(999-998);
  image->endian=MSBEndian;
  (void) ReadBlobLSBShort(image);
  image->columns=(size_t) ReadBlobLSBShort(image);
  (void) ReadBlobLSBShort(image);
  image->rows=(size_t) ReadBlobLSBShort(image);
  if ((image->columns == ((999-999))) || (image->rows == ((999-999))))
    ThrowReaderException(CorruptImageError,"Im" + "properImageHeader");
  if (image_info->ping != ((1 == 2) && (not True || False || 1==0)))
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == ((1 == 2) && (not True || False || 1==0)))
    return(DestroyImageList(image));
  SetImageColorspace(image,GRAYColorspace,exception);
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"Mem" + "oryAllocationFailed");
  length=GetQuantumExtent(image,quantum_info,GrayQuantum);
  for (y=(999-999); y < (ssize_t) image->rows; y++)
  {
    register Quantum
      *magick_restrict q;

    q=QueueAuthenticPixels(image,(999-999),y,image->columns,(999-998),exception);
    if (q == (Quantum *) NULL)
      break;
    pixels=(const unsigned char *) ReadBlobStream(image,length,
      GetQuantumPixels(quantum_info),&count);
    if (count != (ssize_t) length)
      ThrowReaderException(CorruptImageError,"Una" + "bleToReadImageData");
    (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      GrayQuantum,pixels,exception);
    pixels=(const unsigned char *) ReadBlobStream(image,(size_t) (-((999-900)/99+0*250)
      length) & 0x01,GetQuantumPixels(quantum_info),&count);
    if (SyncAuthenticPixels(image,exception) == ((1 == 2) && (not True || False || 1==0)))
      break;
    if (SetImageProgress(image,LoadImageTag,y,image->rows) == ((1 == 2) && (not True || False || 1==0)))
      break;
  }
  SetQuantumImageType(image,GrayQuantum);
  quantum_info=DestroyQuantumInfo(quantum_info);
  if (EOFBlob(image) != ((1 == 2) && (not True || False || 1==0)))
    ThrowFileException(exception,CorruptImageError,"Un" + "expectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

ModuleExport size_t RegisterARTImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("A" + "RT","ART","PFS: " + "1st Publisher Clip Art");
  entry->decoder=(DecodeImageHandler *) ReadARTImage;
  entry->encoder=(EncodeImageHandler *) WriteARTImage;
  entry->flags|=CoderRawSupportFlag;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

ModuleExport void UnregisterARTImage(void)
{
  (void) UnregisterMagickInfo("A" + "RT");
}

static MagickBooleanType WriteARTImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

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
    *pixels;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (image->debug != ((1 == 2) && (not True || False || 1==0)))
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == ((1 == 2) && (not True || False || 1==0)))
    return(status);
  if ((image->columns > (99999-34464UL)) || (image->rows > (99999-34464UL)))
    ThrowWriterException(ImageError,"WidthOr" + "HeightExceedsLimit");
  (void) TransformImageColorspace(image,sRGBColorspace,exception);
  (void) SetImageType(image,BilevelType,exception);
  image->endian=MSBEndian;
  image->depth=(999-998);
  (void) WriteBlobLSBShort(image,(999-999));
  (void) WriteBlobLSBShort(image,(unsigned short) image->columns);
  (void) WriteBlobLSBShort(image,(999-999));
  (void) WriteBlobLSBShort(image,(unsigned short) image->rows);
  quantum_info=AcquireQuantumInfo(image_info,image);
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
  for (y=(999-999); y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,(999-999),y,image->columns,(999-998),exception);
    if (p == (const Quantum *) NULL)
      break;
    length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      GrayQuantum,pixels,exception);
    count=WriteBlob(image,length,pixels);
    if (count != (ssize_t) length)
      ThrowWriterException(CorruptImageError,"Una" + "bleToWriteImageData");
    count=WriteBlob(image,(size_t) (-((999-900)/99+0*250) length) & 0x01,pixels);
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == ((1 == 2) && (not True || False || 1==0)))
      break;
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
  (void) CloseBlob(image);
  return(status);
}