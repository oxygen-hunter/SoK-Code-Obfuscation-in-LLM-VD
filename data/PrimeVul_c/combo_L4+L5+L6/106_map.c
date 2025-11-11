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

static MagickBooleanType WriteMAPImage(const ImageInfo *,Image *,ExceptionInfo *);

static Image *ReadMAPImageRecursive(const ImageInfo *image_info,ExceptionInfo *exception, ssize_t y, unsigned char *pixels)
{
  if (y == 0)
    return GetFirstImageInList(AcquireImage(image_info,exception));

  Image *image=ReadMAPImageRecursive(image_info,exception,y-1,pixels);

  register ssize_t x;
  register Quantum *q;
  register unsigned char *p;
  Quantum index;
  ssize_t count;
  size_t depth, packet_size, quantum;
  unsigned char *colormap;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);

  if ((image->columns == 0) || (image->rows == 0))
    ThrowReaderException(OptionError,"MustSpecifyImageSize");

  depth=GetImageQuantumDepth(image,MagickTrue);
  packet_size=(size_t) (depth/8);
  packet_size=(size_t) (image->colors > 256 ? 6UL : 3UL);
  colormap=(unsigned char *) AcquireQuantumMemory(image->colors,packet_size*sizeof(*colormap));
  if (colormap == (unsigned char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

  count=ReadBlob(image,packet_size*image->colors,colormap);
  if (count != (ssize_t) (packet_size*image->colors))
    ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");

  p=colormap;
  if (image->depth <= 8)
    for (ssize_t i=0; i < (ssize_t) image->colors; i++)
    {
      image->colormap[i].red=ScaleCharToQuantum(*p++);
      image->colormap[i].green=ScaleCharToQuantum(*p++);
      image->colormap[i].blue=ScaleCharToQuantum(*p++);
    }
  else
    for (ssize_t i=0; i < (ssize_t) image->colors; i++)
    {
      quantum=(*p++ << 8);
      quantum|=(*p++);
      image->colormap[i].red=(Quantum) quantum;
      quantum=(*p++ << 8);
      quantum|=(*p++);
      image->colormap[i].green=(Quantum) quantum;
      quantum=(*p++ << 8);
      quantum|=(*p++);
      image->colormap[i].blue=(Quantum) quantum;
    }
  colormap=(unsigned char *) RelinquishMagickMemory(colormap);

  if (image_info->ping != MagickFalse)
    {
      (void) CloseBlob(image);
      return(GetFirstImageInList(image));
    }

  if (SetImageExtent(image,image->columns,image->rows,exception) == MagickFalse)
    return(DestroyImageList(image));

  p=pixels;
  q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
  if (q == (Quantum *) NULL)
    return image;

  count=ReadBlob(image,(size_t) packet_size*image->columns,pixels);
  if (count != (ssize_t) (packet_size*image->columns))
    return image;

  for (x=0; x < (ssize_t) image->columns; x++)
  {
    index=ConstrainColormapIndex(image,*p,exception);
    p++;
    if (image->colors > 256)
    {
      index=ConstrainColormapIndex(image,((size_t) index << 8)+(*p),exception);
      p++;
    }
    SetPixelIndex(image,index,q);
    SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
    q+=GetPixelChannels(image);
  }
  if (SyncAuthenticPixels(image,exception) == MagickFalse)
    return image;

  pixels=(unsigned char *) RelinquishMagickMemory(pixels);
  if (y < (ssize_t) image->rows)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",image->filename);

  (void) CloseBlob(image);
  return image;
}

static Image *ReadMAPImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image *image=AcquireImage(image_info,exception);
  ssize_t rows=image->rows;
  unsigned char *pixels=(unsigned char *) AcquireQuantumMemory(image->columns,(size_t) (GetImageQuantumDepth(image,MagickTrue)/8)*sizeof(*pixels));
  return ReadMAPImageRecursive(image_info,exception,rows-1,pixels);
}

ModuleExport size_t RegisterMAPImage(void)
{
  MagickInfo *entry=AcquireMagickInfo("MAP","MAP","Colormap intensities and indices");
  entry->decoder=(DecodeImageHandler *) ReadMAPImage;
  entry->encoder=(EncodeImageHandler *) WriteMAPImage;
  entry->flags^=CoderAdjoinFlag;
  entry->format_type=ExplicitFormatType;
  entry->flags|=CoderRawSupportFlag;
  entry->flags|=CoderEndianSupportFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

ModuleExport void UnregisterMAPImage(void)
{
  (void) UnregisterMagickInfo("MAP");
}

static MagickBooleanType WriteMAPImageRecursive(const ImageInfo *image_info,Image *image,ExceptionInfo *exception, ssize_t y, unsigned char *pixels)
{
  if (y == 0)
    return CloseBlob(image);

  MagickBooleanType status=WriteMAPImageRecursive(image_info,image,exception,y-1,pixels);
  if (status == MagickFalse)
    return status;

  register ssize_t x;
  register const Quantum *p;
  register unsigned char *q;
  size_t depth, packet_size;
  unsigned char *colormap;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);

  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);

  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);

  if (OpenBlob(image_info,image,WriteBinaryBlobMode,exception) == MagickFalse)
    return MagickFalse;

  (void) TransformImageColorspace(image,sRGBColorspace,exception);

  if (IsPaletteImage(image) == MagickFalse)
    (void) SetImageType(image,PaletteType,exception);

  depth=GetImageQuantumDepth(image,MagickTrue);
  packet_size=(size_t) (depth/8);
  packet_size=(size_t) (image->colors > 256 ? 6UL : 3UL);
  colormap=(unsigned char *) AcquireQuantumMemory(image->colors,packet_size*sizeof(*colormap));
  if (colormap == (unsigned char *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");

  q=colormap;
  if (image->colors <= 256)
    for (ssize_t i=0; i < (ssize_t) image->colors; i++)
    {
      *q++=(unsigned char) ScaleQuantumToChar(image->colormap[i].red);
      *q++=(unsigned char) ScaleQuantumToChar(image->colormap[i].green);
      *q++=(unsigned char) ScaleQuantumToChar(image->colormap[i].blue);
    }
  else
    for (ssize_t i=0; i < (ssize_t) image->colors; i++)
    {
      *q++=(unsigned char) (ScaleQuantumToShort(image->colormap[i].red) >> 8);
      *q++=(unsigned char) (ScaleQuantumToShort(image->colormap[i].red) & 0xff);
      *q++=(unsigned char) (ScaleQuantumToShort(image->colormap[i].green) >> 8);
      *q++=(unsigned char) (ScaleQuantumToShort(image->colormap[i].green) & 0xff);;
      *q++=(unsigned char) (ScaleQuantumToShort(image->colormap[i].blue) >> 8);
      *q++=(unsigned char) (ScaleQuantumToShort(image->colormap[i].blue) & 0xff);
    }
  (void) WriteBlob(image,packet_size*image->colors,colormap);
  colormap=(unsigned char *) RelinquishMagickMemory(colormap);

  p=GetVirtualPixels(image,0,y,image->columns,1,exception);
  if (p == (const Quantum *) NULL)
    return MagickFalse;

  q=pixels;
  for (x=0; x < (ssize_t) image->columns; x++)
  {
    if (image->colors > 256)
      *q++=(unsigned char) ((size_t) GetPixelIndex(image,p) >> 8);
    *q++=(unsigned char) GetPixelIndex(image,p);
    p+=GetPixelChannels(image);
  }
  (void) WriteBlob(image,(size_t) (q-pixels),pixels);
  return status;
}

static MagickBooleanType WriteMAPImage(const ImageInfo *image_info,Image *image,ExceptionInfo *exception)
{
  ssize_t rows=image->rows;
  unsigned char *pixels=(unsigned char *) AcquireQuantumMemory(image->columns,(size_t) (GetImageQuantumDepth(image,MagickTrue)/8)*sizeof(*pixels));
  return WriteMAPImageRecursive(image_info,image,exception,rows-1,pixels);
}