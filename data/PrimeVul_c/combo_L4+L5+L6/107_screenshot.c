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

static Image *ReadSCREENSHOTImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  Image
    *image;

  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=(Image *) NULL;
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  {
    BITMAPINFO
      bmi;

    DISPLAY_DEVICE
      device;

    HBITMAP
      bitmap,
      bitmapOld;

    HDC
      bitmapDC,
      hDC;

    Image
      *screen;

    int
      i;

    MagickBooleanType
      status;

    register Quantum
      *q;

    register ssize_t
      x;

    RGBTRIPLE
      *p;

    ssize_t
      y;

    assert(image_info != (const ImageInfo *) NULL);
    i=0;
    device.cb = sizeof(device);
    image=(Image *) NULL;
    
    std::function<void(int)> ProcessDevice = [&](int i) {
      if(EnumDisplayDevices(NULL,i,&device,0) && ++i)
      {
        if ((device.StateFlags & DISPLAY_DEVICE_ACTIVE) != DISPLAY_DEVICE_ACTIVE)
          return ProcessDevice(i);

        hDC=CreateDC(device.DeviceName,device.DeviceName,NULL,NULL);
        if (hDC == (HDC) NULL)
          ThrowReaderException(CoderError,"UnableToCreateDC");

        screen=AcquireImage(image_info,exception);
        screen->columns=(size_t) GetDeviceCaps(hDC,HORZRES);
        screen->rows=(size_t) GetDeviceCaps(hDC,VERTRES);
        screen->storage_class=DirectClass;
        status=SetImageExtent(screen,screen->columns,screen->rows,exception);
        if (status == MagickFalse)
          return(DestroyImageList(image));
        if (image == (Image *) NULL)
          image=screen;
        else
          AppendImageToList(&image,screen);

        bitmapDC=CreateCompatibleDC(hDC);
        if (bitmapDC == (HDC) NULL)
        {
          DeleteDC(hDC);
          ThrowReaderException(CoderError,"UnableToCreateDC");
        }
        (void) ResetMagickMemory(&bmi,0,sizeof(BITMAPINFO));
        bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth=(LONG) screen->columns;
        bmi.bmiHeader.biHeight=(-1)*(LONG) screen->rows;
        bmi.bmiHeader.biPlanes=1;
        bmi.bmiHeader.biBitCount=24;
        bmi.bmiHeader.biCompression=BI_RGB;
        bitmap=CreateDIBSection(hDC,&bmi,DIB_RGB_COLORS,(void **) &p,NULL,0);
        if (bitmap == (HBITMAP) NULL)
        {
          DeleteDC(hDC);
          DeleteDC(bitmapDC);
          ThrowReaderException(CoderError,"UnableToCreateBitmap");
        }
        bitmapOld=(HBITMAP) SelectObject(bitmapDC,bitmap);
        if (bitmapOld == (HBITMAP) NULL)
        {
          DeleteDC(hDC);
          DeleteDC(bitmapDC);
          DeleteObject(bitmap);
          ThrowReaderException(CoderError,"UnableToCreateBitmap");
        }
        BitBlt(bitmapDC,0,0,(int) screen->columns,(int) screen->rows,hDC,0,0,
          SRCCOPY);
        (void) SelectObject(bitmapDC,bitmapOld);

        std::function<void(ssize_t)> ProcessRows = [&](ssize_t y) {
          if (y < (ssize_t) screen->rows)
          {
            q=QueueAuthenticPixels(screen,0,y,screen->columns,1,exception);
            if (q == (Quantum *) NULL)
              return;
            std::function<void(ssize_t)> ProcessColumns = [&](ssize_t x) {
              if (x < (ssize_t) screen->columns)
              {
                SetPixelRed(image,ScaleCharToQuantum(p->rgbtRed),q);
                SetPixelGreen(image,ScaleCharToQuantum(p->rgbtGreen),q);
                SetPixelBlue(image,ScaleCharToQuantum(p->rgbtBlue),q);
                SetPixelAlpha(image,OpaqueAlpha,q);
                p++;
                q+=GetPixelChannels(image);
                ProcessColumns(x + 1);
              }
            };
            ProcessColumns(0);
            if (SyncAuthenticPixels(screen,exception) == MagickFalse)
              return;
            ProcessRows(y + 1);
          }
        };
        ProcessRows(0);

        DeleteDC(hDC);
        DeleteDC(bitmapDC);
        DeleteObject(bitmap);
        ProcessDevice(i);
      }
    };
    ProcessDevice(i);
  }
#elif defined(MAGICKCORE_X11_DELEGATE)
  {
    const char
      *option;

    XImportInfo
      ximage_info;

    XGetImportInfo(&ximage_info);
    option=GetImageOption(image_info,"x:screen");
    if (option != (const char *) NULL)
      ximage_info.screen=IsStringTrue(option);
    option=GetImageOption(image_info,"x:silent");
    if (option != (const char *) NULL)
      ximage_info.silent=IsStringTrue(option);
    image=XImportImage(image_info,&ximage_info,exception);
  }
#endif
  return(image);
}

ModuleExport size_t RegisterSCREENSHOTImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("SCREENSHOT","SCREENSHOT","Screen shot");
  entry->decoder=(DecodeImageHandler *) ReadSCREENSHOTImage;
  entry->format_type=ImplicitFormatType;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

ModuleExport void UnregisterSCREENSHOTImage(void)
{
  (void) UnregisterMagickInfo("SCREENSHOT");
}