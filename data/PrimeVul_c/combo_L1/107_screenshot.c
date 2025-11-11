#include "MagickCore/studio.h"
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
#  if defined(__CYGWIN__)
#    include <windows.h>
#  else
#    include "MagickCore/nt-base-private.h"
#    include <wingdi.h>
#  ifndef DISPLAY_DEVICE_ACTIVE
#    define DISPLAY_DEVICE_ACTIVE    0x00000001
#  endif
#  endif
#endif
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/nt-feature.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/token.h"
#include "MagickCore/utility.h"
#include "MagickCore/xwindow.h"
#include "MagickCore/xwindow-private.h"

static Image *OX2F39A7F4(const ImageInfo *OX2E2B8FF3,
  ExceptionInfo *OX4A1C1B77)
{
  Image
    *OX1F2F3A7C;

  assert(OX2E2B8FF3->signature == MagickCoreSignature);
  if (OX2E2B8FF3->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      OX2E2B8FF3->filename);
  assert(OX4A1C1B77 != (ExceptionInfo *) NULL);
  assert(OX4A1C1B77->signature == MagickCoreSignature);
  OX1F2F3A7C=(Image *) NULL;
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  {
    BITMAPINFO
      OX1A7C4B23;

    DISPLAY_DEVICE
      OX3B2F6A2D;

    HBITMAP
      OX4F1A2B3C,
      OX3C6A1B2F;

    HDC
      OX3A1F2B4C,
      OX4B2C1A3F;

    Image
      *OX2C4B1F3A;

    int
      OX1B3A4F2C;

    MagickBooleanType
      OX4A2F3B1C;

    register Quantum
      *OX3C2B1A4F;

    register ssize_t
      OX1C4B2F3A;

    RGBTRIPLE
      *OX4B1C2A3F;

    ssize_t
      OX3A2B1C4F;

    assert(OX2E2B8FF3 != (const ImageInfo *) NULL);
    OX1B3A4F2C=0;
    OX3B2F6A2D.cb = sizeof(OX3B2F6A2D);
    OX1F2F3A7C=(Image *) NULL;
    while(EnumDisplayDevices(NULL,OX1B3A4F2C,&OX3B2F6A2D,0) && ++OX1B3A4F2C)
    {
      if ((OX3B2F6A2D.StateFlags & DISPLAY_DEVICE_ACTIVE) != DISPLAY_DEVICE_ACTIVE)
        continue;

      OX4B2C1A3F=CreateDC(OX3B2F6A2D.DeviceName,OX3B2F6A2D.DeviceName,NULL,NULL);
      if (OX4B2C1A3F == (HDC) NULL)
        ThrowReaderException(CoderError,"UnableToCreateDC");

      OX2C4B1F3A=AcquireImage(OX2E2B8FF3,OX4A1C1B77);
      OX2C4B1F3A->columns=(size_t) GetDeviceCaps(OX4B2C1A3F,HORZRES);
      OX2C4B1F3A->rows=(size_t) GetDeviceCaps(OX4B2C1A3F,VERTRES);
      OX2C4B1F3A->storage_class=DirectClass;
      OX4A2F3B1C=SetImageExtent(OX2C4B1F3A,OX2C4B1F3A->columns,OX2C4B1F3A->rows,OX4A1C1B77);
      if (OX4A2F3B1C == MagickFalse)
        return(DestroyImageList(OX1F2F3A7C));
      if (OX1F2F3A7C == (Image *) NULL)
        OX1F2F3A7C=OX2C4B1F3A;
      else
        AppendImageToList(&OX1F2F3A7C,OX2C4B1F3A);

      OX3A1F2B4C=CreateCompatibleDC(OX4B2C1A3F);
      if (OX3A1F2B4C == (HDC) NULL)
        {
          DeleteDC(OX4B2C1A3F);
          ThrowReaderException(CoderError,"UnableToCreateDC");
        }
      (void) ResetMagickMemory(&OX1A7C4B23,0,sizeof(BITMAPINFO));
      OX1A7C4B23.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
      OX1A7C4B23.bmiHeader.biWidth=(LONG) OX2C4B1F3A->columns;
      OX1A7C4B23.bmiHeader.biHeight=(-1)*(LONG) OX2C4B1F3A->rows;
      OX1A7C4B23.bmiHeader.biPlanes=1;
      OX1A7C4B23.bmiHeader.biBitCount=24;
      OX1A7C4B23.bmiHeader.biCompression=BI_RGB;
      OX4F1A2B3C=CreateDIBSection(OX4B2C1A3F,&OX1A7C4B23,DIB_RGB_COLORS,(void **) &OX4B1C2A3F,NULL,0);
      if (OX4F1A2B3C == (HBITMAP) NULL)
        {
          DeleteDC(OX4B2C1A3F);
          DeleteDC(OX3A1F2B4C);
          ThrowReaderException(CoderError,"UnableToCreateBitmap");
        }
      OX3C6A1B2F=(HBITMAP) SelectObject(OX3A1F2B4C,OX4F1A2B3C);
      if (OX3C6A1B2F == (HBITMAP) NULL)
        {
          DeleteDC(OX4B2C1A3F);
          DeleteDC(OX3A1F2B4C);
          DeleteObject(OX4F1A2B3C);
          ThrowReaderException(CoderError,"UnableToCreateBitmap");
        }
      BitBlt(OX3A1F2B4C,0,0,(int) OX2C4B1F3A->columns,(int) OX2C4B1F3A->rows,OX4B2C1A3F,0,0,
        SRCCOPY);
      (void) SelectObject(OX3A1F2B4C,OX3C6A1B2F);

      for (OX3A2B1C4F=0; OX3A2B1C4F < (ssize_t) OX2C4B1F3A->rows; OX3A2B1C4F++)
      {
        OX3C2B1A4F=QueueAuthenticPixels(OX2C4B1F3A,0,OX3A2B1C4F,OX2C4B1F3A->columns,1,OX4A1C1B77);
        if (OX3C2B1A4F == (Quantum *) NULL)
          break;
        for (OX1C4B2F3A=0; OX1C4B2F3A < (ssize_t) OX2C4B1F3A->columns; OX1C4B2F3A++)
        {
          SetPixelRed(OX1F2F3A7C,ScaleCharToQuantum(OX4B1C2A3F->rgbtRed),OX3C2B1A4F);
          SetPixelGreen(OX1F2F3A7C,ScaleCharToQuantum(OX4B1C2A3F->rgbtGreen),OX3C2B1A4F);
          SetPixelBlue(OX1F2F3A7C,ScaleCharToQuantum(OX4B1C2A3F->rgbtBlue),OX3C2B1A4F);
          SetPixelAlpha(OX1F2F3A7C,OpaqueAlpha,OX3C2B1A4F);
          OX4B1C2A3F++;
          OX3C2B1A4F+=GetPixelChannels(OX1F2F3A7C);
        }
        if (SyncAuthenticPixels(OX2C4B1F3A,OX4A1C1B77) == MagickFalse)
          break;
      }

      DeleteDC(OX4B2C1A3F);
      DeleteDC(OX3A1F2B4C);
      DeleteObject(OX4F1A2B3C);
    }
  }
#elif defined(MAGICKCORE_X11_DELEGATE)
  {
    const char
      *OX1F4A2B3C;

    XImportInfo
      OX3A4F2C1B;

    XGetImportInfo(&OX3A4F2C1B);
    OX1F4A2B3C=GetImageOption(OX2E2B8FF3,"x:screen");
    if (OX1F4A2B3C != (const char *) NULL)
      OX3A4F2C1B.screen=IsStringTrue(OX1F4A2B3C);
    OX1F4A2B3C=GetImageOption(OX2E2B8FF3,"x:silent");
    if (OX1F4A2B3C != (const char *) NULL)
      OX3A4F2C1B.silent=IsStringTrue(OX1F4A2B3C);
    OX1F2F3A7C=XImportImage(OX2E2B8FF3,&OX3A4F2C1B,OX4A1C1B77);
  }
#endif
  return(OX1F2F3A7C);
}

ModuleExport size_t OX1B4C3A2F(void)
{
  MagickInfo
    *OX4F3A2B1C;

  OX4F3A2B1C=AcquireMagickInfo("SCREENSHOT","SCREENSHOT","Screen shot");
  OX4F3A2B1C->decoder=(DecodeImageHandler *) OX2F39A7F4;
  OX4F3A2B1C->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(OX4F3A2B1C);
  return(MagickImageCoderSignature);
}

ModuleExport void OX3B2C1A4F(void)
{
  (void) UnregisterMagickInfo("SCREENSHOT");
}