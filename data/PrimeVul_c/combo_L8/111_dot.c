#include <Python.h>
#include <gvc.h>
static GVC_t
  *graphic_context = (GVC_t *) NULL;

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
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
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
  if (option == (const char *) NULL)
    gvLayout(graphic_context,graph,(char *) "dot");
  else
    gvLayout(graphic_context,graph,(char *) option);
  gvRenderFilename(graphic_context,graph,(char *) "svg",read_info->filename);
  gvFreeLayout(graphic_context,graph);
  agclose(graph);
  Py_Initialize();
  PyRun_SimpleString("import sys\nsys.path.append('.')\n");
  PyObject *pModule = PyImport_ImportModule("read_image");
  PyObject *pFunc = PyObject_GetAttrString(pModule, "read_image");
  PyObject *pArgs = PyTuple_Pack(2, PyUnicode_FromString(read_info->filename), exception);
  PyObject *pValue = PyObject_CallObject(pFunc, pArgs);
  if (pValue == NULL) {
      PyErr_Print();
      return NULL;
  }
  image = (Image *) PyCapsule_GetPointer(pValue, "Image");
  Py_XDECREF(pValue);
  Py_XDECREF(pArgs);
  Py_XDECREF(pFunc);
  Py_XDECREF(pModule);
  Py_Finalize();
  (void) RelinquishUniqueFileResource(read_info->filename);
  read_info=DestroyImageInfo(read_info);
  if (image == (Image *) NULL)
    return((Image *) NULL);
  return(GetFirstImageInList(image));
}

ModuleExport size_t RegisterDOTImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("DOT","DOT","Graphviz");
  entry->decoder=(DecodeImageHandler *) ReadDOTImage;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("DOT","GV","Graphviz");
  entry->decoder=(DecodeImageHandler *) ReadDOTImage;
  entry->flags^=CoderBlobSupportFlag;
  (void) RegisterMagickInfo(entry);
  graphic_context=gvContext();
  return(MagickImageCoderSignature);
}

ModuleExport void UnregisterDOTImage(void)
{
  (void) UnregisterMagickInfo("GV");
  (void) UnregisterMagickInfo("DOT");
  if (graphic_context != (GVC_t *) NULL)
    {
      gvFreeContext(graphic_context);
      graphic_context=(GVC_t *) NULL;
    }
}