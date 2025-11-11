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

static MagickBooleanType WriteARTImage(const ImageInfo *,Image *,ExceptionInfo *);

static Image *ReadARTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  const unsigned char *pixels;
  Image *image;
  QuantumInfo *quantum_info;
  MagickBooleanType status;
  size_t length;
  ssize_t count, y;

  int dispatch = 0;
  while (1)
  {
    switch (dispatch)
    {
      case 0:
        assert(image_info != (const ImageInfo *) NULL);
        assert(image_info->signature == MagickCoreSignature);
        if (image_info->debug != MagickFalse)
          (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image_info->filename);
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
        dispatch = 1;
        break;
      case 1:
        SetImageColorspace(image,GRAYColorspace,exception);
        quantum_info=AcquireQuantumInfo(image_info,image);
        if (quantum_info == (QuantumInfo *) NULL)
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
        length=GetQuantumExtent(image,quantum_info,GrayQuantum);
        y = 0;
        dispatch = 2;
        break;
      case 2:
        if (y < (ssize_t) image->rows)
        {
          register Quantum *magick_restrict q;
          q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
          if (q == (Quantum *) NULL)
          {
            dispatch = 4;
            break;
          }
          pixels=(const unsigned char *) ReadBlobStream(image,length,GetQuantumPixels(quantum_info),&count);
          if (count != (ssize_t) length)
            ThrowReaderException(CorruptImageError,"UnableToReadImageData");
          (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,GrayQuantum,pixels,exception);
          pixels=(const unsigned char *) ReadBlobStream(image,(size_t) (-(ssize_t) length) & 0x01,GetQuantumPixels(quantum_info),&count);
          if (SyncAuthenticPixels(image,exception) == MagickFalse)
          {
            dispatch = 4;
            break;
          }
          if (SetImageProgress(image,LoadImageTag,y,image->rows) == MagickFalse)
          {
            dispatch = 4;
            break;
          }
          y++;
          dispatch = 2;
        }
        else
        {
          dispatch = 3;
        }
        break;
      case 3:
        SetQuantumImageType(image,GrayQuantum);
        quantum_info=DestroyQuantumInfo(quantum_info);
        if (EOFBlob(image) != MagickFalse)
          ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",image->filename);
        (void) CloseBlob(image);
        return(GetFirstImageInList(image));
      case 4:
        quantum_info=DestroyQuantumInfo(quantum_info);
        (void) CloseBlob(image);
        return((Image *) NULL);
    }
  }
}

ModuleExport size_t RegisterARTImage(void)
{
  MagickInfo *entry;
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

static MagickBooleanType WriteARTImage(const ImageInfo *image_info,Image *image,ExceptionInfo *exception)
{
  MagickBooleanType status;
  QuantumInfo *quantum_info;
  register const Quantum *p;
  size_t length;
  ssize_t count, y;
  unsigned char *pixels;

  int dispatch = 0;
  while (1)
  {
    switch (dispatch)
    {
      case 0:
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
        y = 0;
        dispatch = 1;
        break;
      case 1:
        if (y < (ssize_t) image->rows)
        {
          p=GetVirtualPixels(image,0,y,image->columns,1,exception);
          if (p == (const Quantum *) NULL)
          {
            dispatch = 3;
            break;
          }
          length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,GrayQuantum,pixels,exception);
          count=WriteBlob(image,length,pixels);
          if (count != (ssize_t) length)
            ThrowWriterException(CorruptImageError,"UnableToWriteImageData");
          count=WriteBlob(image,(size_t) (-(ssize_t) length) & 0x01,pixels);
          status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,image->rows);
          if (status == MagickFalse)
          {
            dispatch = 3;
            break;
          }
          y++;
          dispatch = 1;
        }
        else
        {
          dispatch = 2;
        }
        break;
      case 2:
        quantum_info=DestroyQuantumInfo(quantum_info);
        (void) CloseBlob(image);
        return(status);
      case 3:
        quantum_info=DestroyQuantumInfo(quantum_info);
        (void) CloseBlob(image);
        return(MagickFalse);
    }
  }
}