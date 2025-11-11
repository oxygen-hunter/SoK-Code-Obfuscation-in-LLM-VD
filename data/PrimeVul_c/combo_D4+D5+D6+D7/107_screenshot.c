#include "MagickCore/studio.h"
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
#  if defined(__CYGWIN__)
#    include <windows.h>
#  else
     /* All MinGW needs ... */
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
  struct S {
    Image *image;
    int i;
  } data;
  
  data.image = NULL;
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
#if defined(MAGICKCORE_WINGDI32_DELEGATE)
  {
    BITMAPINFO
      bmi;

    DISPLAY_DEVICE
      device;

    struct T {
      HBITMAP bitmap;
      HBITMAP bitmapOld;
      HDC bitmapDC;
      HDC hDC;
    } handles;

    Image
      *screen;

    MagickBooleanType
      status;

    register Quantum
      *q;

    RGBTRIPLE
      *p;

    ssize_t
      y;

    assert(image_info != (const ImageInfo *) NULL);
    data.i=0;
    device.cb = sizeof(device);
    data.image=(Image *) NULL;
    while(EnumDisplayDevices(NULL,data.i,&device,0) && ++data.i)
    {
      if ((device.StateFlags & DISPLAY_DEVICE_ACTIVE) != DISPLAY_DEVICE_ACTIVE)
        continue;

      handles.hDC=CreateDC(device.DeviceName,device.DeviceName,NULL,NULL);
      if (handles.hDC == (HDC) NULL)
        ThrowReaderException(CoderError,"UnableToCreateDC");

      screen=AcquireImage(image_info,exception);
      screen->columns=(size_t) GetDeviceCaps(handles.hDC,HORZRES);
      screen->rows=(size_t) GetDeviceCaps(handles.hDC,VERTRES);
      screen->storage_class=DirectClass;
      status=SetImageExtent(screen,screen->columns,screen->rows,exception);
      if (status == MagickFalse)
        return(DestroyImageList(data.image));
      if (data.image == (Image *) NULL)
        data.image=screen;
      else
        AppendImageToList(&data.image,screen);

      handles.bitmapDC=CreateCompatibleDC(handles.hDC);
      if (handles.bitmapDC == (HDC) NULL)
        {
          DeleteDC(handles.hDC);
          ThrowReaderException(CoderError,"UnableToCreateDC");
        }
      (void) ResetMagickMemory(&bmi,0,sizeof(BITMAPINFO));
      bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
      bmi.bmiHeader.biWidth=(LONG) screen->columns;
      bmi.bmiHeader.biHeight=(-1)*(LONG) screen->rows;
      bmi.bmiHeader.biPlanes=1;
      bmi.bmiHeader.biBitCount=24;
      bmi.bmiHeader.biCompression=BI_RGB;
      handles.bitmap=CreateDIBSection(handles.hDC,&bmi,DIB_RGB_COLORS,(void **) &p,NULL,0);
      if (handles.bitmap == (HBITMAP) NULL)
        {
          DeleteDC(handles.hDC);
          DeleteDC(handles.bitmapDC);
          ThrowReaderException(CoderError,"UnableToCreateBitmap");
        }
      handles.bitmapOld=(HBITMAP) SelectObject(handles.bitmapDC,handles.bitmap);
      if (handles.bitmapOld == (HBITMAP) NULL)
        {
          DeleteDC(handles.hDC);
          DeleteDC(handles.bitmapDC);
          DeleteObject(handles.bitmap);
          ThrowReaderException(CoderError,"UnableToCreateBitmap");
        }
      BitBlt(handles.bitmapDC,0,0,(int) screen->columns,(int) screen->rows,handles.hDC,0,0,
        SRCCOPY);
      (void) SelectObject(handles.bitmapDC,handles.bitmapOld);

      for (y=0; y < (ssize_t) screen->rows; y++)
      {
        q=QueueAuthenticPixels(screen,0,y,screen->columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (ssize_t x=0; x < (ssize_t) screen->columns; x++)
        {
          SetPixelRed(data.image,ScaleCharToQuantum(p->rgbtRed),q);
          SetPixelGreen(data.image,ScaleCharToQuantum(p->rgbtGreen),q);
          SetPixelBlue(data.image,ScaleCharToQuantum(p->rgbtBlue),q);
          SetPixelAlpha(data.image,OpaqueAlpha,q);
          p++;
          q+=GetPixelChannels(data.image);
        }
        if (SyncAuthenticPixels(screen,exception) == MagickFalse)
          break;
      }

      DeleteDC(handles.hDC);
      DeleteDC(handles.bitmapDC);
      DeleteObject(handles.bitmap);
    }
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
    data.image=XImportImage(image_info,&ximage_info,exception);
  }
#endif
  return(data.image);
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