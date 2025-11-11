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

static MagickBooleanType OX7B4DF339(const ImageInfo *,Image *,ExceptionInfo *);

static Image *OX5F8A6DAA(const ImageInfo *OX0D47E3D8,ExceptionInfo *OX4A7B5F9C)
{
  Image *OX242A1D80;
  MagickBooleanType OX6D5A8F6C;
  Quantum OX1C3F2B43;
  register ssize_t OX6A9B7D0D;
  register Quantum *OX7E1C5A3F;
  register ssize_t OX4B8F6E2D;
  register unsigned char *OX5C9E8F0A;
  size_t OX3B1D9E6C, OX9D3F2A7B, OX7E2C5B1A;
  ssize_t OX0D1F6B7C, OX1E7C5B8A;
  unsigned char *OX7D9A3F2E, *OX4A9E7F1C;

  assert(OX0D47E3D8 != (const ImageInfo *) NULL);
  assert(OX0D47E3D8->signature == MagickCoreSignature);
  if (OX0D47E3D8->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      OX0D47E3D8->filename);
  assert(OX4A7B5F9C != (ExceptionInfo *) NULL);
  assert(OX4A7B5F9C->signature == MagickCoreSignature);
  OX242A1D80=AcquireImage(OX0D47E3D8,OX4A7B5F9C);
  if ((OX242A1D80->columns == 0) || (OX242A1D80->rows == 0))
    ThrowReaderException(OptionError,"MustSpecifyImageSize");
  OX6D5A8F6C=OpenBlob(OX0D47E3D8,OX242A1D80,ReadBinaryBlobMode,OX4A7B5F9C);
  if (OX6D5A8F6C == MagickFalse)
    {
      OX242A1D80=DestroyImageList(OX242A1D80);
      return((Image *) NULL);
    }

  OX242A1D80->storage_class=PseudoClass;
  OX6D5A8F6C=AcquireImageColormap(OX242A1D80,(size_t)
    (OX242A1D80->offset != 0 ? OX242A1D80->offset : 256),OX4A7B5F9C);
  if (OX6D5A8F6C == MagickFalse)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  OX3B1D9E6C=GetImageQuantumDepth(OX242A1D80,MagickTrue);
  OX9D3F2A7B=(size_t) (OX3B1D9E6C/8);
  OX4A9E7F1C=(unsigned char *) AcquireQuantumMemory(OX242A1D80->columns,OX9D3F2A7B*
    sizeof(*OX4A9E7F1C));
  OX9D3F2A7B=(size_t) (OX242A1D80->colors > 256 ? 6UL : 3UL);
  OX7D9A3F2E=(unsigned char *) AcquireQuantumMemory(OX242A1D80->colors,OX9D3F2A7B*
    sizeof(*OX7D9A3F2E));
  if ((OX4A9E7F1C == (unsigned char *) NULL) ||
      (OX7D9A3F2E == (unsigned char *) NULL))
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");

  OX0D1F6B7C=ReadBlob(OX242A1D80,OX9D3F2A7B*OX242A1D80->colors,OX7D9A3F2E);
  if (OX0D1F6B7C != (ssize_t) (OX9D3F2A7B*OX242A1D80->colors))
    ThrowReaderException(CorruptImageError,"InsufficientImageDataInFile");
  OX5C9E8F0A=OX7D9A3F2E;
  if (OX242A1D80->depth <= 8)
    for (OX4B8F6E2D=0; OX4B8F6E2D < (ssize_t) OX242A1D80->colors; OX4B8F6E2D++)
    {
      OX242A1D80->colormap[OX4B8F6E2D].red=ScaleCharToQuantum(*OX5C9E8F0A++);
      OX242A1D80->colormap[OX4B8F6E2D].green=ScaleCharToQuantum(*OX5C9E8F0A++);
      OX242A1D80->colormap[OX4B8F6E2D].blue=ScaleCharToQuantum(*OX5C9E8F0A++);
    }
  else
    for (OX4B8F6E2D=0; OX4B8F6E2D < (ssize_t) OX242A1D80->colors; OX4B8F6E2D++)
    {
      OX7E2C5B1A=(*OX5C9E8F0A++ << 8);
      OX7E2C5B1A|=(*OX5C9E8F0A++);
      OX242A1D80->colormap[OX4B8F6E2D].red=(Quantum) OX7E2C5B1A;
      OX7E2C5B1A=(*OX5C9E8F0A++ << 8);
      OX7E2C5B1A|=(*OX5C9E8F0A++);
      OX242A1D80->colormap[OX4B8F6E2D].green=(Quantum) OX7E2C5B1A;
      OX7E2C5B1A=(*OX5C9E8F0A++ << 8);
      OX7E2C5B1A|=(*OX5C9E8F0A++);
      OX242A1D80->colormap[OX4B8F6E2D].blue=(Quantum) OX7E2C5B1A;
    }
  OX7D9A3F2E=(unsigned char *) RelinquishMagickMemory(OX7D9A3F2E);
  if (OX0D47E3D8->ping != MagickFalse)
    {
      (void) CloseBlob(OX242A1D80);
      return(GetFirstImageInList(OX242A1D80));
    }
  OX6D5A8F6C=SetImageExtent(OX242A1D80,OX242A1D80->columns,OX242A1D80->rows,OX4A7B5F9C);
  if (OX6D5A8F6C == MagickFalse)
    return(DestroyImageList(OX242A1D80));

  OX9D3F2A7B=(size_t) (OX3B1D9E6C/8);
  for (OX1E7C5B8A=0; OX1E7C5B8A < (ssize_t) OX242A1D80->rows; OX1E7C5B8A++)
  {
    OX5C9E8F0A=OX4A9E7F1C;
    OX7E1C5A3F=QueueAuthenticPixels(OX242A1D80,0,OX1E7C5B8A,OX242A1D80->columns,1,OX4A7B5F9C);
    if (OX7E1C5A3F == (Quantum *) NULL)
      break;
    OX0D1F6B7C=ReadBlob(OX242A1D80,(size_t) OX9D3F2A7B*OX242A1D80->columns,OX4A9E7F1C);
    if (OX0D1F6B7C != (ssize_t) (OX9D3F2A7B*OX242A1D80->columns))
      break;
    for (OX6A9B7D0D=0; OX6A9B7D0D < (ssize_t) OX242A1D80->columns; OX6A9B7D0D++)
    {
      OX1C3F2B43=ConstrainColormapIndex(OX242A1D80,*OX5C9E8F0A,OX4A7B5F9C);
      OX5C9E8F0A++;
      if (OX242A1D80->colors > 256)
        {
          OX1C3F2B43=ConstrainColormapIndex(OX242A1D80,((size_t) OX1C3F2B43 << 8)+(*OX5C9E8F0A),
            OX4A7B5F9C);
          OX5C9E8F0A++;
        }
      SetPixelIndex(OX242A1D80,OX1C3F2B43,OX7E1C5A3F);
      SetPixelViaPixelInfo(OX242A1D80,OX242A1D80->colormap+(ssize_t) OX1C3F2B43,OX7E1C5A3F);
      OX7E1C5A3F+=GetPixelChannels(OX242A1D80);
    }
    if (SyncAuthenticPixels(OX242A1D80,OX4A7B5F9C) == MagickFalse)
      break;
  }
  OX4A9E7F1C=(unsigned char *) RelinquishMagickMemory(OX4A9E7F1C);
  if (OX1E7C5B8A < (ssize_t) OX242A1D80->rows)
    ThrowFileException(OX4A7B5F9C,CorruptImageError,"UnexpectedEndOfFile",
      OX242A1D80->filename);
  (void) CloseBlob(OX242A1D80);
  return(GetFirstImageInList(OX242A1D80));
}

ModuleExport size_t OX1A3E9F6B(void)
{
  MagickInfo *OX7C4A5B6E;
  OX7C4A5B6E=AcquireMagickInfo("MAP","MAP","Colormap intensities and indices");
  OX7C4A5B6E->decoder=(DecodeImageHandler *) OX5F8A6DAA;
  OX7C4A5B6E->encoder=(EncodeImageHandler *) OX7B4DF339;
  OX7C4A5B6E->flags^=CoderAdjoinFlag;
  OX7C4A5B6E->format_type=ExplicitFormatType;
  OX7C4A5B6E->flags|=CoderRawSupportFlag;
  OX7C4A5B6E->flags|=CoderEndianSupportFlag;
  (void) RegisterMagickInfo(OX7C4A5B6E);
  return(MagickImageCoderSignature);
}

ModuleExport void OX3E5A9B7C(void)
{
  (void) UnregisterMagickInfo("MAP");
}

static MagickBooleanType OX7B4DF339(const ImageInfo *OX0D47E3D8,Image *OX242A1D80,
  ExceptionInfo *OX4A7B5F9C)
{
  MagickBooleanType OX6D5A8F6C;
  register const Quantum *OX5C9E8F0A;
  register ssize_t OX4B8F6E2D, OX6A9B7D0D;
  register unsigned char *OX7E1C5A3F;
  size_t OX3B1D9E6C, OX9D3F2A7B;
  ssize_t OX1E7C5B8A;
  unsigned char *OX7D9A3F2E, *OX4A9E7F1C;

  assert(OX0D47E3D8 != (const ImageInfo *) NULL);
  assert(OX0D47E3D8->signature == MagickCoreSignature);
  assert(OX242A1D80 != (Image *) NULL);
  assert(OX242A1D80->signature == MagickCoreSignature);
  if (OX242A1D80->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",OX242A1D80->filename);
  assert(OX4A7B5F9C != (ExceptionInfo *) NULL);
  assert(OX4A7B5F9C->signature == MagickCoreSignature);
  OX6D5A8F6C=OpenBlob(OX0D47E3D8,OX242A1D80,WriteBinaryBlobMode,OX4A7B5F9C);
  if (OX6D5A8F6C == MagickFalse)
    return(OX6D5A8F6C);
  (void) TransformImageColorspace(OX242A1D80,sRGBColorspace,OX4A7B5F9C);

  if (IsPaletteImage(OX242A1D80) == MagickFalse)
    (void) SetImageType(OX242A1D80,PaletteType,OX4A7B5F9C);
  OX3B1D9E6C=GetImageQuantumDepth(OX242A1D80,MagickTrue);
  OX9D3F2A7B=(size_t) (OX3B1D9E6C/8);
  OX4A9E7F1C=(unsigned char *) AcquireQuantumMemory(OX242A1D80->columns,OX9D3F2A7B*
    sizeof(*OX4A9E7F1C));
  OX9D3F2A7B=(size_t) (OX242A1D80->colors > 256 ? 6UL : 3UL);
  OX7D9A3F2E=(unsigned char *) AcquireQuantumMemory(OX242A1D80->colors,OX9D3F2A7B*
    sizeof(*OX7D9A3F2E));
  if ((OX4A9E7F1C == (unsigned char *) NULL) ||
      (OX7D9A3F2E == (unsigned char *) NULL))
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");

  OX7E1C5A3F=OX7D9A3F2E;
  OX7E1C5A3F=OX7D9A3F2E;
  if (OX242A1D80->colors <= 256)
    for (OX4B8F6E2D=0; OX4B8F6E2D < (ssize_t) OX242A1D80->colors; OX4B8F6E2D++)
    {
      *OX7E1C5A3F++=(unsigned char) ScaleQuantumToChar(OX242A1D80->colormap[OX4B8F6E2D].red);
      *OX7E1C5A3F++=(unsigned char) ScaleQuantumToChar(OX242A1D80->colormap[OX4B8F6E2D].green);
      *OX7E1C5A3F++=(unsigned char) ScaleQuantumToChar(OX242A1D80->colormap[OX4B8F6E2D].blue);
    }
  else
    for (OX4B8F6E2D=0; OX4B8F6E2D < (ssize_t) OX242A1D80->colors; OX4B8F6E2D++)
    {
      *OX7E1C5A3F++=(unsigned char) (ScaleQuantumToShort(OX242A1D80->colormap[OX4B8F6E2D].red) >> 8);
      *OX7E1C5A3F++=(unsigned char) (ScaleQuantumToShort(OX242A1D80->colormap[OX4B8F6E2D].red) & 0xff);
      *OX7E1C5A3F++=(unsigned char) (ScaleQuantumToShort(OX242A1D80->colormap[OX4B8F6E2D].green) >> 8);
      *OX7E1C5A3F++=(unsigned char) (ScaleQuantumToShort(OX242A1D80->colormap[OX4B8F6E2D].green) & 0xff);;
      *OX7E1C5A3F++=(unsigned char) (ScaleQuantumToShort(OX242A1D80->colormap[OX4B8F6E2D].blue) >> 8);
      *OX7E1C5A3F++=(unsigned char) (ScaleQuantumToShort(OX242A1D80->colormap[OX4B8F6E2D].blue) & 0xff);
    }
  (void) WriteBlob(OX242A1D80,OX9D3F2A7B*OX242A1D80->colors,OX7D9A3F2E);
  OX7D9A3F2E=(unsigned char *) RelinquishMagickMemory(OX7D9A3F2E);

  for (OX1E7C5B8A=0; OX1E7C5B8A < (ssize_t) OX242A1D80->rows; OX1E7C5B8A++)
  {
    OX5C9E8F0A=GetVirtualPixels(OX242A1D80,0,OX1E7C5B8A,OX242A1D80->columns,1,OX4A7B5F9C);
    if (OX5C9E8F0A == (const Quantum *) NULL)
      break;
    OX7E1C5A3F=OX4A9E7F1C;
    for (OX6A9B7D0D=0; OX6A9B7D0D < (ssize_t) OX242A1D80->columns; OX6A9B7D0D++)
    {
      if (OX242A1D80->colors > 256)
        *OX7E1C5A3F++=(unsigned char) ((size_t) GetPixelIndex(OX242A1D80,OX5C9E8F0A) >> 8);
      *OX7E1C5A3F++=(unsigned char) GetPixelIndex(OX242A1D80,OX5C9E8F0A);
      OX5C9E8F0A+=GetPixelChannels(OX242A1D80);
    }
    (void) WriteBlob(OX242A1D80,(size_t) (OX7E1C5A3F-OX4A9E7F1C),OX4A9E7F1C);
  }
  OX4A9E7F1C=(unsigned char *) RelinquishMagickMemory(OX4A9E7F1C);
  (void) CloseBlob(OX242A1D80);
  return(OX6D5A8F6C);
}