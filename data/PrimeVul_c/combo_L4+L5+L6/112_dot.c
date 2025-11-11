#include "MagickCore/studio.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/client.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/option.h"
#include "MagickCore/resource_.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/utility.h"
#include "MagickCore/xwindow-private.h"
#if defined(MAGICKCORE_GVC_DELEGATE)
#undef HAVE_CONFIG_H
#include <gvc.h>
static GVC_t
  *graphic_context = (GVC_t *) NULL;
#endif
#if defined(MAGICKCORE_GVC_DELEGATE)
static Image *ReadDOTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  char
    command[MagickPathExtent];

  const char
    *option;

  graph_t
    *graph;

  Image
    *image;

  ImageInfo
    *read_info;

  MagickBooleanType
    status;

  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  assert(graphic_context != (GVC_t *) NULL);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    return((Image *) NULL);
  read_info=CloneImageInfo(image_info);
  SetImageInfoBlob(read_info,(void *) NULL,0);
  (void) CopyMagickString(read_info->magick,"SVG",MagickPathExtent);
  (void) AcquireUniqueFilename(read_info->filename);
  (void) FormatLocaleString(command,MagickPathExtent,"-Tsvg -o%s %s",
    read_info->filename,image_info->filename);
#if !defined(WITH_CGRAPH)
  graph=agread(GetBlobFileHandle(image));
#else
  graph=agread(GetBlobFileHandle(image),(Agdisc_t *) NULL);
#endif
  if (graph == (graph_t *) NULL)
    {
      (void) RelinquishUniqueFileResource(read_info->filename);
      return ((Image *) NULL);
    }
  option=GetImageOption(image_info,"dot:layout-engine");
  switch(option != (const char *) NULL)
  {
    case 0:
      gvLayout(graphic_context,graph,(char *) "dot");
      break;
    default:
      gvLayout(graphic_context,graph,(char *) option);
      break;
  }
  gvRenderFilename(graphic_context,graph,(char *) "svg",read_info->filename);
  gvFreeLayout(graphic_context,graph);
  agclose(graph);
  (void) CopyMagickString(read_info->magick,"SVG",MaxTextExtent);
  image=ReadImage(read_info,exception);
  (void) RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  return(GetFirstImageInList(image));
}
#endif
ModuleExport size_t RegisterDOTImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("DOT","DOT","Graphviz");
#if defined(MAGICKCORE_GVC_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadDOTImage;
#endif
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DOT","GV","Graphviz");
#if defined(MAGICKCORE_GVC_DELEGATE)
  entry->decoder=(DecodeImageHandler *) ReadDOTImage;
#endif
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
#if defined(MAGICKCORE_GVC_DELEGATE)
  graphic_context=gvContext();
#endif
  return(MagickImageCoderSignature);
}
ModuleExport void UnregisterDOTImage(void)
{
  (void) UnregisterMagickInfo("GV");
  (void) UnregisterMagickInfo("DOT");
#if defined(MAGICKCORE_GVC_DELEGATE)
  if (graphic_context != (GVC_t *) NULL)
    {
      gvFreeContext(graphic_context);
      graphic_context=(GVC_t *) NULL;
    }
#endif
}