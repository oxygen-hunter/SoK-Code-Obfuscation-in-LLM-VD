#include "MagickCore/studio.h"
#include "MagickCore/annotate.h"
#include "MagickCore/artifact.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/draw.h"
#include "MagickCore/draw-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/module.h"
#include "MagickCore/option.h"
#include "MagickCore/property.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility.h"

static Image *OX7B4DF339(const ImageInfo *OX9CF3B2D6,
  ExceptionInfo *OX4E2F86FF)
{
  char
    *OX45A4B1C7,
    OX9A2FAE4C[MagickPathExtent],
    *OX668DE1A7,
    *OX3E8D3E9F;

  const char
    *OX1A2F50E8,
    *OX1E5E2BDE;

  DrawInfo
    *OX4D4E3D1F;

  Image
    *OX7A1E9B5E;

  MagickBooleanType
    OX4C0B2C9B,
    OX7F6D5B2A;

  register ssize_t
    OX55A4D9A3;

  size_t
    OX8C2B3E9F,
    OX3D1F8A5E;

  TypeMetric
    OX5B7C4A9D;

  assert(OX9CF3B2D6 != (const ImageInfo *) NULL);
  assert(OX9CF3B2D6->signature == MagickCoreSignature);
  if (OX9CF3B2D6->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      OX9CF3B2D6->filename);
  assert(OX4E2F86FF != (ExceptionInfo *) NULL);
  assert(OX4E2F86FF->signature == MagickCoreSignature);
  OX7A1E9B5E=AcquireImage(OX9CF3B2D6,OX4E2F86FF);
  (void) ResetImagePage(OX7A1E9B5E,"0x0+0+0");

  OX1E5E2BDE=GetImageOption(OX9CF3B2D6,"filename");
  if (OX1E5E2BDE == (const char *) NULL)
    OX668DE1A7=InterpretImageProperties((ImageInfo *) OX9CF3B2D6,OX7A1E9B5E,
      OX9CF3B2D6->filename,OX4E2F86FF);
  else
    if (LocaleNCompare(OX1E5E2BDE,"caption:",8) == 0)
      OX668DE1A7=InterpretImageProperties((ImageInfo *) OX9CF3B2D6,OX7A1E9B5E,OX1E5E2BDE+8,
        OX4E2F86FF);
    else
      OX668DE1A7=InterpretImageProperties((ImageInfo *) OX9CF3B2D6,OX7A1E9B5E,OX1E5E2BDE,
        OX4E2F86FF);
  (void) SetImageProperty(OX7A1E9B5E,"caption",OX668DE1A7,OX4E2F86FF);
  OX668DE1A7=DestroyString(OX668DE1A7);
  OX45A4B1C7=ConstantString(GetImageProperty(OX7A1E9B5E,"caption",OX4E2F86FF));
  OX4D4E3D1F=CloneDrawInfo(OX9CF3B2D6,(DrawInfo *) NULL);
  (void) CloneString(&OX4D4E3D1F->text,OX45A4B1C7);
  OX1A2F50E8=GetImageOption(OX9CF3B2D6,"gravity");
  if (OX1A2F50E8 != (char *) NULL)
    OX4D4E3D1F->gravity=(GravityType) ParseCommandOption(MagickGravityOptions,
      MagickFalse,OX1A2F50E8);
  OX4C0B2C9B=MagickFalse;
  OX7F6D5B2A=MagickTrue;
  if (OX7A1E9B5E->columns == 0)
    {
      OX3E8D3E9F=AcquireString(OX45A4B1C7);
      OX55A4D9A3=FormatMagickCaption(OX7A1E9B5E,OX4D4E3D1F,OX4C0B2C9B,&OX5B7C4A9D,&OX3E8D3E9F,
        OX4E2F86FF);
      (void) CloneString(&OX4D4E3D1F->text,OX3E8D3E9F);
      OX3E8D3E9F=DestroyString(OX3E8D3E9F);
      (void) FormatLocaleString(OX9A2FAE4C,MagickPathExtent,"%+g%+g",
        -OX5B7C4A9D.bounds.x1,OX5B7C4A9D.ascent);
      if (OX4D4E3D1F->gravity == UndefinedGravity)
        (void) CloneString(&OX4D4E3D1F->geometry,OX9A2FAE4C);
      OX7F6D5B2A=GetMultilineTypeMetrics(OX7A1E9B5E,OX4D4E3D1F,&OX5B7C4A9D,OX4E2F86FF);
      OX3D1F8A5E=(size_t) floor(OX5B7C4A9D.width+OX4D4E3D1F->stroke_width+0.5);
      OX7A1E9B5E->columns=OX3D1F8A5E;
    }
  if (OX7A1E9B5E->rows == 0)
    {
      OX4C0B2C9B=MagickTrue;
      OX3E8D3E9F=AcquireString(OX45A4B1C7);
      OX55A4D9A3=FormatMagickCaption(OX7A1E9B5E,OX4D4E3D1F,OX4C0B2C9B,&OX5B7C4A9D,&OX3E8D3E9F,OX4E2F86FF);
      (void) CloneString(&OX4D4E3D1F->text,OX3E8D3E9F);
      OX3E8D3E9F=DestroyString(OX3E8D3E9F);
      (void) FormatLocaleString(OX9A2FAE4C,MagickPathExtent,"%+g%+g",
        -OX5B7C4A9D.bounds.x1,OX5B7C4A9D.ascent);
      if (OX4D4E3D1F->gravity == UndefinedGravity)
        (void) CloneString(&OX4D4E3D1F->geometry,OX9A2FAE4C);
      OX7F6D5B2A=GetMultilineTypeMetrics(OX7A1E9B5E,OX4D4E3D1F,&OX5B7C4A9D,OX4E2F86FF);
      OX7A1E9B5E->rows=(size_t) ((OX55A4D9A3+1)*(OX5B7C4A9D.ascent-OX5B7C4A9D.descent+
        OX4D4E3D1F->interline_spacing+OX4D4E3D1F->stroke_width)+0.5);
    }
  if (OX7F6D5B2A != MagickFalse)
    OX7F6D5B2A=SetImageExtent(OX7A1E9B5E,OX7A1E9B5E->columns,OX7A1E9B5E->rows,OX4E2F86FF);
  if (OX7F6D5B2A == MagickFalse)
    { 
      OX4D4E3D1F=DestroyDrawInfo(OX4D4E3D1F);
      return(DestroyImageList(OX7A1E9B5E));
    }
  if (SetImageBackgroundColor(OX7A1E9B5E,OX4E2F86FF) == MagickFalse)
    {
      OX4D4E3D1F=DestroyDrawInfo(OX4D4E3D1F);
      OX7A1E9B5E=DestroyImageList(OX7A1E9B5E);
      return((Image *) NULL);
    }
  if ((fabs(OX9CF3B2D6->pointsize) < MagickEpsilon) && (strlen(OX45A4B1C7) > 0))
    {
      double
        OX5C8D1B9A,
        OX2A1D3F4E;

      for ( ; ; OX4D4E3D1F->pointsize*=2.0)
      {
        OX3E8D3E9F=AcquireString(OX45A4B1C7);
        OX55A4D9A3=FormatMagickCaption(OX7A1E9B5E,OX4D4E3D1F,OX4C0B2C9B,&OX5B7C4A9D,&OX3E8D3E9F,
          OX4E2F86FF);
        (void) CloneString(&OX4D4E3D1F->text,OX3E8D3E9F);
        OX3E8D3E9F=DestroyString(OX3E8D3E9F);
        (void) FormatLocaleString(OX9A2FAE4C,MagickPathExtent,"%+g%+g",
          -OX5B7C4A9D.bounds.x1,OX5B7C4A9D.ascent);
        if (OX4D4E3D1F->gravity == UndefinedGravity)
          (void) CloneString(&OX4D4E3D1F->geometry,OX9A2FAE4C);
        OX7F6D5B2A=GetMultilineTypeMetrics(OX7A1E9B5E,OX4D4E3D1F,&OX5B7C4A9D,OX4E2F86FF);
        OX3D1F8A5E=(size_t) floor(OX5B7C4A9D.width+OX4D4E3D1F->stroke_width+0.5);
        OX8C2B3E9F=(size_t) floor(OX5B7C4A9D.height+OX4D4E3D1F->stroke_width+0.5);
        if ((OX7A1E9B5E->columns != 0) && (OX7A1E9B5E->rows != 0))
          {
            if ((OX3D1F8A5E >= OX7A1E9B5E->columns) && (OX8C2B3E9F >= OX7A1E9B5E->rows))
              break;
          }
        else
          if (((OX7A1E9B5E->columns != 0) && (OX3D1F8A5E >= OX7A1E9B5E->columns)) ||
              ((OX7A1E9B5E->rows != 0) && (OX8C2B3E9F >= OX7A1E9B5E->rows)))
            break;
      }
      OX5C8D1B9A=OX4D4E3D1F->pointsize;
      for (OX2A1D3F4E=1.0; (OX5C8D1B9A-OX2A1D3F4E) > 0.5; )
      {
        OX4D4E3D1F->pointsize=(OX2A1D3F4E+OX5C8D1B9A)/2.0;
        OX3E8D3E9F=AcquireString(OX45A4B1C7);
        OX55A4D9A3=FormatMagickCaption(OX7A1E9B5E,OX4D4E3D1F,OX4C0B2C9B,&OX5B7C4A9D,&OX3E8D3E9F,
          OX4E2F86FF);
        (void) CloneString(&OX4D4E3D1F->text,OX3E8D3E9F);
        OX3E8D3E9F=DestroyString(OX3E8D3E9F);
        (void) FormatLocaleString(OX9A2FAE4C,MagickPathExtent,"%+g%+g",
          -OX5B7C4A9D.bounds.x1,OX5B7C4A9D.ascent);
        if (OX4D4E3D1F->gravity == UndefinedGravity)
          (void) CloneString(&OX4D4E3D1F->geometry,OX9A2FAE4C);
        OX7F6D5B2A=GetMultilineTypeMetrics(OX7A1E9B5E,OX4D4E3D1F,&OX5B7C4A9D,OX4E2F86FF);
        OX3D1F8A5E=(size_t) floor(OX5B7C4A9D.width+OX4D4E3D1F->stroke_width+0.5);
        OX8C2B3E9F=(size_t) floor(OX5B7C4A9D.height+OX4D4E3D1F->stroke_width+0.5);
        if ((OX7A1E9B5E->columns != 0) && (OX7A1E9B5E->rows != 0))
          {
            if ((OX3D1F8A5E < OX7A1E9B5E->columns) && (OX8C2B3E9F < OX7A1E9B5E->rows))
              OX2A1D3F4E=OX4D4E3D1F->pointsize+0.5;
            else
              OX5C8D1B9A=OX4D4E3D1F->pointsize-0.5;
          }
        else
          if (((OX7A1E9B5E->columns != 0) && (OX3D1F8A5E < OX7A1E9B5E->columns)) ||
              ((OX7A1E9B5E->rows != 0) && (OX8C2B3E9F < OX7A1E9B5E->rows)))
            OX2A1D3F4E=OX4D4E3D1F->pointsize+0.5;
          else
            OX5C8D1B9A=OX4D4E3D1F->pointsize-0.5;
      }
      OX4D4E3D1F->pointsize=floor((OX2A1D3F4E+OX5C8D1B9A)/2.0-0.5);
    }
  OX55A4D9A3=FormatMagickCaption(OX7A1E9B5E,OX4D4E3D1F,OX4C0B2C9B,&OX5B7C4A9D,&OX45A4B1C7,OX4E2F86FF);
  (void) CloneString(&OX4D4E3D1F->text,OX45A4B1C7);
  (void) FormatLocaleString(OX9A2FAE4C,MagickPathExtent,"%+g%+g",MagickMax(
    OX4D4E3D1F->direction == RightToLeftDirection ? OX7A1E9B5E->columns-
    OX5B7C4A9D.bounds.x2 : -OX5B7C4A9D.bounds.x1,0.0),OX4D4E3D1F->gravity ==
    UndefinedGravity ? OX5B7C4A9D.ascent : 0.0);
  (void) CloneString(&OX4D4E3D1F->geometry,OX9A2FAE4C);
  OX7F6D5B2A=AnnotateImage(OX7A1E9B5E,OX4D4E3D1F,OX4E2F86FF);
  if (OX9CF3B2D6->pointsize == 0.0)
    { 
      char
        OX1F4D2E3B[MagickPathExtent];
      
      (void) FormatLocaleString(OX1F4D2E3B,MagickPathExtent,"%.20g",
        OX4D4E3D1F->pointsize);
      (void) SetImageProperty(OX7A1E9B5E,"caption:pointsize",OX1F4D2E3B,OX4E2F86FF);
    }
  OX4D4E3D1F=DestroyDrawInfo(OX4D4E3D1F);
  OX45A4B1C7=DestroyString(OX45A4B1C7);
  if (OX7F6D5B2A == MagickFalse)
    {
      OX7A1E9B5E=DestroyImageList(OX7A1E9B5E);
      return((Image *) NULL);
    }
  return(GetFirstImageInList(OX7A1E9B5E));
}

ModuleExport size_t OX3A4D6E1F(void)
{
  MagickInfo
    *OX6B2E5D1C;

  OX6B2E5D1C=AcquireMagickInfo("CAPTION","CAPTION","Caption");
  OX6B2E5D1C->decoder=(DecodeImageHandler *) OX7B4DF339;
  OX6B2E5D1C->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(OX6B2E5D1C);
  return(MagickImageCoderSignature);
}

ModuleExport void OX9E4A3F2B(void)
{
  (void) UnregisterMagickInfo("CAPTION");
}