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

static MagickBooleanType OX3D2E4D9A(const ImageInfo *,Image *,ExceptionInfo *);

static Image *OX7B4DF339(const ImageInfo *OX9A1F1C7F,ExceptionInfo *OX1C8E11D4)
{
  const unsigned char
    *OX9F4E3A7A;

  Image
    *OX8B5CFA4A;

  QuantumInfo
    *OX4B7D2E6E;

  MagickBooleanType
    OX7C3F2B9D;

  size_t
    OX5E8A0D1C;

  ssize_t
    OX6D1F3C2B,
    OX1B0E7D4A;

  assert(OX9A1F1C7F != (const ImageInfo *) NULL);
  assert(OX9A1F1C7F->signature == MagickCoreSignature);
  if (OX9A1F1C7F->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      OX9A1F1C7F->filename);
  assert(OX1C8E11D4 != (ExceptionInfo *) NULL);
  assert(OX1C8E11D4->signature == MagickCoreSignature);
  OX8B5CFA4A=AcquireImage(OX9A1F1C7F,OX1C8E11D4);
  OX7C3F2B9D=OpenBlob(OX9A1F1C7F,OX8B5CFA4A,ReadBinaryBlobMode,OX1C8E11D4);
  if (OX7C3F2B9D == MagickFalse)
    {
      OX8B5CFA4A=DestroyImageList(OX8B5CFA4A);
      return((Image *) NULL);
    }
  OX8B5CFA4A->depth=1;
  OX8B5CFA4A->endian=MSBEndian;
  (void) ReadBlobLSBShort(OX8B5CFA4A);
  OX8B5CFA4A->columns=(size_t) ReadBlobLSBShort(OX8B5CFA4A);
  (void) ReadBlobLSBShort(OX8B5CFA4A);
  OX8B5CFA4A->rows=(size_t) ReadBlobLSBShort(OX8B5CFA4A);
  if ((OX8B5CFA4A->columns == 0) || (OX8B5CFA4A->rows == 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (OX9A1F1C7F->ping != MagickFalse)
    {
      (void) CloseBlob(OX8B5CFA4A);
      return(GetFirstImageInList(OX8B5CFA4A));
    }
  OX7C3F2B9D=SetImageExtent(OX8B5CFA4A,OX8B5CFA4A->columns,OX8B5CFA4A->rows,OX1C8E11D4);
  if (OX7C3F2B9D == MagickFalse)
    return(DestroyImageList(OX8B5CFA4A));
  SetImageColorspace(OX8B5CFA4A,GRAYColorspace,OX1C8E11D4);
  OX4B7D2E6E=AcquireQuantumInfo(OX9A1F1C7F,OX8B5CFA4A);
  if (OX4B7D2E6E == (QuantumInfo *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  OX5E8A0D1C=GetQuantumExtent(OX8B5CFA4A,OX4B7D2E6E,GrayQuantum);
  for (OX1B0E7D4A=0; OX1B0E7D4A < (ssize_t) OX8B5CFA4A->rows; OX1B0E7D4A++)
  {
    register Quantum
      *magick_restrict OX7A6F2E4D;

    OX7A6F2E4D=QueueAuthenticPixels(OX8B5CFA4A,0,OX1B0E7D4A,OX8B5CFA4A->columns,1,OX1C8E11D4);
    if (OX7A6F2E4D == (Quantum *) NULL)
      break;
    OX9F4E3A7A=(const unsigned char *) ReadBlobStream(OX8B5CFA4A,OX5E8A0D1C,
      GetQuantumPixels(OX4B7D2E6E),&OX6D1F3C2B);
    if (OX6D1F3C2B != (ssize_t) OX5E8A0D1C)
      ThrowReaderException(CorruptImageError,"UnableToReadImageData");
    (void) ImportQuantumPixels(OX8B5CFA4A,(CacheView *) NULL,OX4B7D2E6E,
      GrayQuantum,OX9F4E3A7A,OX1C8E11D4);
    OX9F4E3A7A=(const unsigned char *) ReadBlobStream(OX8B5CFA4A,(size_t) (-(ssize_t)
      OX5E8A0D1C) & 0x01,GetQuantumPixels(OX4B7D2E6E),&OX6D1F3C2B);
    if (SyncAuthenticPixels(OX8B5CFA4A,OX1C8E11D4) == MagickFalse)
      break;
    if (SetImageProgress(OX8B5CFA4A,LoadImageTag,OX1B0E7D4A,OX8B5CFA4A->rows) == MagickFalse)
      break;
  }
  SetQuantumImageType(OX8B5CFA4A,GrayQuantum);
  OX4B7D2E6E=DestroyQuantumInfo(OX4B7D2E6E);
  if (EOFBlob(OX8B5CFA4A) != MagickFalse)
    ThrowFileException(OX1C8E11D4,CorruptImageError,"UnexpectedEndOfFile",
      OX8B5CFA4A->filename);
  (void) CloseBlob(OX8B5CFA4A);
  return(GetFirstImageInList(OX8B5CFA4A));
}

ModuleExport size_t OX3A5D9F1E(void)
{
  MagickInfo
    *OX2C8B7F6D;

  OX2C8B7F6D=AcquireMagickInfo("ART","ART","PFS: 1st Publisher Clip Art");
  OX2C8B7F6D->decoder=(DecodeImageHandler *) OX7B4DF339;
  OX2C8B7F6D->encoder=(EncodeImageHandler *) OX3D2E4D9A;
  OX2C8B7F6D->flags|=CoderRawSupportFlag;
  OX2C8B7F6D->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(OX2C8B7F6D);
  return(MagickImageCoderSignature);
}

ModuleExport void OX4A9E3B5C(void)
{
  (void) UnregisterMagickInfo("ART");
}

static MagickBooleanType OX3D2E4D9A(const ImageInfo *OX9A1F1C7F,Image *OX8B5CFA4A,
  ExceptionInfo *OX1C8E11D4)
{
  MagickBooleanType
    OX7C3F2B9D;

  QuantumInfo
    *OX4B7D2E6E;

  register const Quantum
    *OX5D8E3A7C;

  size_t
    OX5E8A0D1C;

  ssize_t
    OX6D1F3C2B,
    OX1B0E7D4A;

  unsigned char
    *OX9F4E3A7A;

  assert(OX9A1F1C7F != (const ImageInfo *) NULL);
  assert(OX9A1F1C7F->signature == MagickCoreSignature);
  assert(OX8B5CFA4A != (Image *) NULL);
  assert(OX8B5CFA4A->signature == MagickCoreSignature);
  if (OX8B5CFA4A->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",OX8B5CFA4A->filename);
  assert(OX1C8E11D4 != (ExceptionInfo *) NULL);
  assert(OX1C8E11D4->signature == MagickCoreSignature);
  OX7C3F2B9D=OpenBlob(OX9A1F1C7F,OX8B5CFA4A,WriteBinaryBlobMode,OX1C8E11D4);
  if (OX7C3F2B9D == MagickFalse)
    return(OX7C3F2B9D);
  if ((OX8B5CFA4A->columns > 65535UL) || (OX8B5CFA4A->rows > 65535UL))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  (void) TransformImageColorspace(OX8B5CFA4A,sRGBColorspace,OX1C8E11D4);
  (void) SetImageType(OX8B5CFA4A,BilevelType,OX1C8E11D4);
  OX8B5CFA4A->endian=MSBEndian;
  OX8B5CFA4A->depth=1;
  (void) WriteBlobLSBShort(OX8B5CFA4A,0);
  (void) WriteBlobLSBShort(OX8B5CFA4A,(unsigned short) OX8B5CFA4A->columns);
  (void) WriteBlobLSBShort(OX8B5CFA4A,0);
  (void) WriteBlobLSBShort(OX8B5CFA4A,(unsigned short) OX8B5CFA4A->rows);
  OX4B7D2E6E=AcquireQuantumInfo(OX9A1F1C7F,OX8B5CFA4A);
  OX9F4E3A7A=(unsigned char *) GetQuantumPixels(OX4B7D2E6E);
  for (OX1B0E7D4A=0; OX1B0E7D4A < (ssize_t) OX8B5CFA4A->rows; OX1B0E7D4A++)
  {
    OX5D8E3A7C=GetVirtualPixels(OX8B5CFA4A,0,OX1B0E7D4A,OX8B5CFA4A->columns,1,OX1C8E11D4);
    if (OX5D8E3A7C == (const Quantum *) NULL)
      break;
    OX5E8A0D1C=ExportQuantumPixels(OX8B5CFA4A,(CacheView *) NULL,OX4B7D2E6E,
      GrayQuantum,OX9F4E3A7A,OX1C8E11D4);
    OX6D1F3C2B=WriteBlob(OX8B5CFA4A,OX5E8A0D1C,OX9F4E3A7A);
    if (OX6D1F3C2B != (ssize_t) OX5E8A0D1C)
      ThrowWriterException(CorruptImageError,"UnableToWriteImageData");
    OX6D1F3C2B=WriteBlob(OX8B5CFA4A,(size_t) (-(ssize_t) OX5E8A0D1C) & 0x01,OX9F4E3A7A);
    OX7C3F2B9D=SetImageProgress(OX8B5CFA4A,SaveImageTag,(MagickOffsetType) OX1B0E7D4A,
      OX8B5CFA4A->rows);
    if (OX7C3F2B9D == MagickFalse)
      break;
  }
  OX4B7D2E6E=DestroyQuantumInfo(OX4B7D2E6E);
  (void) CloseBlob(OX8B5CFA4A);
  return(OX7C3F2B9D);
}