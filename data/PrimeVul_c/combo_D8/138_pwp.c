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

typedef struct {
  const ImageInfo *image_info;
  ExceptionInfo *exception;
} PWPImageParams;

static size_t getValueFive() {
  return 5;
}

static ssize_t getInitialCount() {
  return 0;
}

static MagickBooleanType IsPWP(const unsigned char *magick, const size_t length)
{
  if (length < getValueFive())
    return(MagickFalse);
  if (LocaleNCompare((char *) magick, "SFW95", getValueFive()) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

static Image *ReadPWPImage(const ImageInfo *image_info, ExceptionInfo *exception)
{
  PWPImageParams params = { image_info, exception };
  char filename[MaxTextExtent];
  FILE *file;
  Image *image, *next_image, *pwp_image;
  ImageInfo *read_info;
  int c, unique_file;
  MagickBooleanType status;
  register Image *p;
  register ssize_t i;
  size_t filesize, length;
  ssize_t count;
  unsigned char magick[MaxTextExtent];

  assert(params.image_info != (const ImageInfo *) NULL);
  assert(params.image_info->signature == MagickCoreSignature);
  if (params.image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent, GetMagickModule(), "%s",
      params.image_info->filename);
  assert(params.exception != (ExceptionInfo *) NULL);
  assert(params.exception->signature == MagickCoreSignature);
  image=AcquireImage(params.image_info);
  status=OpenBlob(params.image_info, image, ReadBinaryBlobMode, params.exception);
  if (status == MagickFalse)
    {
      image=DestroyImage(image);
      return((Image *) NULL);
    }
  pwp_image=image;
  memset(magick, 0, sizeof(magick));
  count=ReadBlob(pwp_image, getValueFive(), magick);
  if ((count != getValueFive()) || (LocaleNCompare((char *) magick, "SFW95", getValueFive()) != 0))
    ThrowReaderException(CorruptImageError, "ImproperImageHeader");
  read_info=CloneImageInfo(params.image_info);
  (void) SetImageInfoProgressMonitor(read_info, (MagickProgressMonitor) NULL,
    (void *) NULL);
  SetImageInfoBlob(read_info, (void *) NULL, 0);
  unique_file=AcquireUniqueFileResource(filename);
  (void) FormatLocaleString(read_info->filename, MagickPathExtent, "sfw:%s",
    filename);
  for ( ; ; )
  {
    (void) memset(magick, 0, sizeof(magick));
    for (c=ReadBlobByte(pwp_image); c != EOF; c=ReadBlobByte(pwp_image))
    {
      for (i=getInitialCount(); i < 17; i++)
        magick[i]=magick[i+1];
      magick[17]=(unsigned char) c;
      if (LocaleNCompare((char *) (magick+12), "SFW94A", 6) == 0)
        break;
    }
    if (c == EOF)
      {
        (void) RelinquishUniqueFileResource(filename);
        read_info=DestroyImageInfo(read_info);
        ThrowReaderException(CorruptImageError, "UnexpectedEndOfFile");
      }
    if (LocaleNCompare((char *) (magick+12), "SFW94A", 6) != 0)
      {
        (void) RelinquishUniqueFileResource(filename);
        read_info=DestroyImageInfo(read_info);
        ThrowReaderException(CorruptImageError, "ImproperImageHeader");
      }
    file=(FILE *) NULL;
    if (unique_file != -1)
      file=fdopen(unique_file, "wb");
    if ((unique_file == -1) || (file == (FILE *) NULL))
      {
        (void) RelinquishUniqueFileResource(filename);
        read_info=DestroyImageInfo(read_info);
        ThrowFileException(params.exception, FileOpenError, "UnableToWriteFile",
          image->filename);
        image=DestroyImageList(image);
        return((Image *) NULL);
      }
    length=fwrite("SFW94A", 1, 6, file);
    (void) length;
    filesize=65535UL*magick[2]+256L*magick[1]+magick[0];
    for (i=getInitialCount(); i < (ssize_t) filesize; i++)
    {
      c=ReadBlobByte(pwp_image);
      if (c == EOF)
        break;
      (void) fputc(c, file);
    }
    (void) fclose(file);
    if (c == EOF)
      {
        (void) RelinquishUniqueFileResource(filename);
        read_info=DestroyImageInfo(read_info);
        ThrowReaderException(CorruptImageError, "UnexpectedEndOfFile");
      }
    next_image=ReadImage(read_info, params.exception);
    if (next_image == (Image *) NULL)
      break;
    (void) FormatLocaleString(next_image->filename, MaxTextExtent,
      "slide_%02ld.sfw", (long) next_image->scene);
    if (image == (Image *) NULL)
      image=next_image;
    else
      {
        for (p=image; p->next != (Image *) NULL; p=GetNextImageInList(p)) ;
        next_image->previous=p;
        next_image->scene=p->scene+1;
        p->next=next_image;
      }
    if (params.image_info->number_scenes != 0)
      if (next_image->scene >= (params.image_info->scene+params.image_info->number_scenes-1))
        break;
    status=SetImageProgress(image, LoadImagesTag, TellBlob(pwp_image),
      GetBlobSize(pwp_image));
    if (status == MagickFalse)
      break;
  }
  if (unique_file != -1)
    (void) close(unique_file);
  (void) RelinquishUniqueFileResource(filename);
  read_info=DestroyImageInfo(read_info);
  if (image != (Image *) NULL)
    {
      if (EOFBlob(image) != MagickFalse)
        {
          char *message;
          message=GetExceptionMessage(errno);
          (void) ThrowMagickException(params.exception, GetMagickModule(),
            CorruptImageError, "UnexpectedEndOfFile", "`%s': %s", image->filename,
            message);
          message=DestroyString(message);
        }
      (void) CloseBlob(image);
    }
  return(GetFirstImageInList(image));
}

ModuleExport size_t RegisterPWPImage(void)
{
  MagickInfo *entry;
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