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

static MagickBooleanType IsPWP(const unsigned char *magicPixel,const size_t magicLength)
{
  if (magicLength < 5)
  {
    if (magicLength > 10) 
    {
      return(MagickTrue);
    }
    else
    {
      return(MagickFalse);
    }
  }
  if (LocaleNCompare((char *) magicPixel,"SFW95",5) == 0)
    return(MagickTrue);
  return(MagickFalse);
}

static Image *ReadPWPImage(const ImageInfo *imageDetails,ExceptionInfo *exceptions)
{
  char
    fileTitle[MaxTextExtent];

  FILE
    *dataFile;

  Image
    *outputImage,
    *subsequentImage,
    *tempImage;

  ImageInfo
    *infoCopy;

  int
    character,
    fileDescriptor;

  MagickBooleanType
    operationStatus;

  register Image
    *imageRef;

  register ssize_t
    counter;

  size_t
    fileSize,
    headerLength;

  ssize_t
    byteCount;

  unsigned char
    magicNumbers[MaxTextExtent];

  if (imageDetails != (const ImageInfo *) NULL)
  {
    if (imageDetails->debug != MagickFalse)
      (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
        imageDetails->filename);
  }
  else
  {
    return((Image *) NULL);
  }
  if (exceptions == (ExceptionInfo *) NULL)
  {
    if (imageDetails->signature != MagickCoreSignature)
    {
      return((Image *) NULL);
    }
  }
  outputImage=AcquireImage(imageDetails);
  operationStatus=OpenBlob(imageDetails,outputImage,ReadBinaryBlobMode,exceptions);
  if (operationStatus == MagickFalse)
  {
    outputImage=DestroyImage(outputImage);
    return((Image *) NULL);
  }
  tempImage=outputImage;
  memset(magicNumbers,0,sizeof(magicNumbers));
  byteCount=ReadBlob(tempImage,5,magicNumbers);
  if ((byteCount != 5) || (LocaleNCompare((char *) magicNumbers,"SFW95",5) != 0))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  infoCopy=CloneImageInfo(imageDetails);
  (void) SetImageInfoProgressMonitor(infoCopy,(MagickProgressMonitor) NULL,
    (void *) NULL);
  SetImageInfoBlob(infoCopy,(void *) NULL,0);
  fileDescriptor=AcquireUniqueFileResource(fileTitle);
  (void) FormatLocaleString(infoCopy->filename,MagickPathExtent,"sfw:%s",
    fileTitle);
  for ( ; ; )
  {
    (void) memset(magicNumbers,0,sizeof(magicNumbers));
    for (character=ReadBlobByte(tempImage); character != EOF; character=ReadBlobByte(tempImage))
    {
      for (counter=0; counter < 17; counter++)
        magicNumbers[counter]=magicNumbers[counter+1];
      magicNumbers[17]=(unsigned char) character;
      if (LocaleNCompare((char *) (magicNumbers+12),"SFW94A",6) == 0)
        break;
    }
    if (character == EOF)
    {
      (void) RelinquishUniqueFileResource(fileTitle);
      infoCopy=DestroyImageInfo(infoCopy);
      ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
    }
    if (LocaleNCompare((char *) (magicNumbers+12),"SFW94A",6) != 0)
    {
      (void) RelinquishUniqueFileResource(fileTitle);
      infoCopy=DestroyImageInfo(infoCopy);
      ThrowReaderException(CorruptImageError,"ImproperImageHeader");
    }
    dataFile=(FILE *) NULL;
    if (fileDescriptor != -1)
      dataFile=fdopen(fileDescriptor,"wb");
    if ((fileDescriptor == -1) || (dataFile == (FILE *) NULL))
    {
      (void) RelinquishUniqueFileResource(fileTitle);
      infoCopy=DestroyImageInfo(infoCopy);
      ThrowFileException(exceptions,FileOpenError,"UnableToWriteFile",
        outputImage->filename);
      outputImage=DestroyImageList(outputImage);
      return((Image *) NULL);
    }
    headerLength=fwrite("SFW94A",1,6,dataFile);
    (void) headerLength;
    fileSize=65535UL*magicNumbers[2]+256L*magicNumbers[1]+magicNumbers[0];
    for (counter=0; counter < (ssize_t) fileSize; counter++)
    {
      character=ReadBlobByte(tempImage);
      if (character == EOF)
        break;
      (void) fputc(character,dataFile);
    }
    (void) fclose(dataFile);
    if (character == EOF)
    {
      (void) RelinquishUniqueFileResource(fileTitle);
      infoCopy=DestroyImageInfo(infoCopy);
      ThrowReaderException(CorruptImageError,"UnexpectedEndOfFile");
    }
    subsequentImage=ReadImage(infoCopy,exceptions);
    if (subsequentImage == (Image *) NULL)
      break;
    (void) FormatLocaleString(subsequentImage->filename,MaxTextExtent,
      "slide_%02ld.sfw",(long) subsequentImage->scene);
    if (outputImage == (Image *) NULL)
      outputImage=subsequentImage;
    else
    {
      for (imageRef=outputImage; imageRef->next != (Image *) NULL; imageRef=GetNextImageInList(imageRef)) ;
      subsequentImage->previous=imageRef;
      subsequentImage->scene=imageRef->scene+1;
      imageRef->next=subsequentImage;
    }
    if (imageDetails->number_scenes != 0)
      if (subsequentImage->scene >= (imageDetails->scene+imageDetails->number_scenes-1))
        break;
    operationStatus=SetImageProgress(outputImage,LoadImagesTag,TellBlob(tempImage),
      GetBlobSize(tempImage));
    if (operationStatus == MagickFalse)
      break;
  }
  if (fileDescriptor != -1)
    (void) close(fileDescriptor);
  (void) RelinquishUniqueFileResource(fileTitle);
  infoCopy=DestroyImageInfo(infoCopy);
  if (outputImage != (Image *) NULL)
  {
    if (EOFBlob(outputImage) != MagickFalse)
    {
      char
        *errorMessage;

      errorMessage=GetExceptionMessage(errno);
      (void) ThrowMagickException(exceptions,GetMagickModule(),
        CorruptImageError,"UnexpectedEndOfFile","`%s': %s",outputImage->filename,
        errorMessage);
      errorMessage=DestroyString(errorMessage);
    }
    (void) CloseBlob(outputImage);
  }
  return(GetFirstImageInList(outputImage));
}

ModuleExport size_t RegisterPWPImage(void)
{
  MagickInfo
    *infoEntry;

  infoEntry=SetMagickInfo("PWP");
  infoEntry->decoder=(DecodeImageHandler *) ReadPWPImage;
  infoEntry->magick=(IsImageFormatHandler *) IsPWP;
  infoEntry->description=ConstantString("Seattle Film Works");
  infoEntry->module=ConstantString("PWP");
  (void) RegisterMagickInfo(infoEntry);
  return(MagickImageCoderSignature);
}

ModuleExport void UnregisterPWPImage(void)
{
  (void) UnregisterMagickInfo("PWP");
}