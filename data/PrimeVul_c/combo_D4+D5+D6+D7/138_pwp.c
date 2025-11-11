#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/constitute.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/resource_.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

static MagickBooleanType IsPWP(const unsigned char *magick,const size_t length)
{
  unsigned char temp = magick[0];
  unsigned char temp2 = magick[1];
  unsigned char temp3 = magick[2];
  unsigned char temp4 = magick[3];
  unsigned char temp5 = magick[4];

  if (length < 5)
    return(MagickFalse);
  if (LocaleNCompare((char *) magick,"SFW95",5) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

static Image *ReadPWPImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  struct L {
    int unique_file;
    int c;
  } local;

  struct G {
    size_t filesize;
    size_t length;
    ssize_t count;
  } global;

  FILE *file;
  Image *image, *next_image, *pwp_image;
  ImageInfo *read_info;
  
  register Image *p;
  register ssize_t i;
  
  unsigned char magick[MaxTextExtent];
  
  char filename[MaxTextExtent];
  
  MagickBooleanType status;
  
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  pwp_image=image;
  memset(magick,0,sizeof(magick));
  global.count=ReadBlob(pwp_image,5,magick);
  if ((global.count != 5) || (LocaleNCompare((char *) magick,"SFW95",5) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  read_info=CloneImageInfo(image_info);
  (void) SetImageInfoProgressMonitor(read_info,(MagickProgressMonitor) NULL,
    (void *) NULL);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  local.unique_file=AcquireUniqueFileResource(filename);
  (void) FormatLocaleString(read_info->filename,MagickPathExtent,"sfw:%s",
    filename);
  for ( ; ; )
  {
    (void) memset(magick,0,sizeof(magick));
    for (local.c=ReadBlobByte(pwp_image); local.c != EOF; local.c=ReadBlobByte(pwp_image))
    {
      for (i=0; i < 17; i++)
        magick[i]=magick[i+1];
      magick[17]=(unsigned char) local.c;
      if (LocaleNCompare((char *) (magick+12),"SFW94A",6) == 0)
        break;
    }
    if (local.c == EOF)
      {
        (void) RelinquishUniqueFileResource(filename);
        read_info=DestroyImageInfo(read_info);
        ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
      }
    if (LocaleNCompare((char *) (magick+12),"SFW94A",6) != 0)
      {
        (void) RelinquishUniqueFileResource(filename);
        read_info=DestroyImageInfo(read_info);
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      }
    file=(FILE *) NULL;
    if (local.unique_file != -1)
      file=fdopen(local.unique_file,"wb");
    if ((local.unique_file == -1) || (file == (FILE *) NULL))
      {
        (void) RelinquishUniqueFileResource(filename);
        read_info=DestroyImageInfo(read_info);
        ThrowFileException(exception,FileOpenError,"UnableToWriteFile",
          image->filename);
        image=DestroyImageList(image);
        return((Image *) NULL);
      }
    global.length=fwrite("SFW94A",1,6,file);
    (void) global.length;
    global.filesize=65535UL*magick[2]+256L*magick[1]+magick[0];
    for (i=0; i < (ssize_t) global.filesize; i++)
    {
      local.c=ReadBlobByte(pwp_image);
      if (local.c == EOF)
        break;
      (void) fputc(local.c,file);
    }
    (void) fclose(file);
    if (local.c == EOF)
      {
        (void) RelinquishUniqueFileResource(filename);
        read_info=DestroyImageInfo(read_info);
        ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
      }
    next_image=ReadImage(read_info,exception);
    if (next_image == (Image *) NULL)
      break;
    (void) FormatLocaleString(next_image->filename,MaxTextExtent,
      "slide_%02ld.sfw",(long) next_image->scene);
    if (image == (Image *) NULL)
      image=next_image;
    else
      {
        for (p=image; p->next != (Image *) NULL; p=GetNextImageInList(p)) ;
        next_image->previous=p;
        next_image->scene=p->scene+1;
        p->next=next_image;
      }
    if (image_info->number_scenes != 0)
      if (next_image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    status=SetImageProgress(image,LoadImagesTag,TellBlob(pwp_image),
      GetBlobSize(pwp_image));
    if (status == MagickFalse)
      break;
  }
  if (local.unique_file != -1)
    (void) close(local.unique_file);
  (void) RelinquishUniqueFileResource(filename);
  read_info=DestroyImageInfo(read_info);
  if (image != (Image *) NULL)
    {
      if (EOFBlob(image) != MagickFalse)
        {
          char
            *message;

          message=GetExceptionMessage(errno);
          (void) ThrowMagickException(exception,GetMagickModule(),
            CorruptImageError,"UnexpectedEndOfFile","`%s': %s",image->filename,
            message);
          message=DestroyString(message);
        }
      (void) CloseBlob(image);
    }
  return(GetFirstImageInList(image));
}

ModuleExport size_t RegisterPWPImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("PWP");
  entry->decoder=(DecodeImageHandler *) ReadPWPImage;
  entry->magick=(IsImageFormatHandler *) IsPWP;
  entry->description=ConstantString("Seattle Film Works");
  entry->module=ConstantString("PWP");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

ModuleExport void UnregisterPWPImage(void)
{
  (void) UnregisterMagickInfo("PWP");
}