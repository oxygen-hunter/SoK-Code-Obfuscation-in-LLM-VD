#include "makePreview.h"

#include <ImfInputFile.h>
#include <ImfOutputFile.h>
#include <ImfTiledOutputFile.h>
#include <ImfRgbaFile.h>
#include <ImfPreviewImage.h>
#include <ImfArray.h>
#include <ImathMath.h>
#include <ImathFun.h>
#include <math.h>
#include <iostream>
#include <algorithm>

#include <OpenEXRConfig.h>
using namespace OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;
using namespace std;

namespace {

float
knee (float x, float f)
{
    return log (x * f + 1) / f;
}

struct GammaParams {
    float x;
    float m;
};

unsigned char
gamma (half h, float m)
{
    GammaParams gp = { max(0.f, h * m), m };
    if (gp.x > 1)
	gp.x = 1 + knee (gp.x - 1, 0.184874f);

    return (unsigned char) (IMATH_NAMESPACE::clamp (Math<float>::pow (gp.x, 0.4545f) * 84.66f, 
				   0.f,
				   255.f));
}

struct PreviewParams {
    int w, h;
    float a;
    Box2i dw;
};

void
generatePreview (const char inFileName[],
		 float exposure,
		 int previewWidth,
		 int &previewHeight,
		 Array2D <PreviewRgba> &previewPixels)
{
    RgbaInputFile in (inFileName);
    PreviewParams pp = { in.dataWindow().max.x - in.dataWindow().min.x + 1,
                         in.dataWindow().max.y - in.dataWindow().min.y + 1,
                         in.pixelAspectRatio(),
                         in.dataWindow() };
    Array2D <Rgba> pixels (pp.h, pp.w);
    in.setFrameBuffer (ComputeBasePointer (&pixels[0][0], pp.dw), 1, pp.w);
    in.readPixels (pp.dw.min.y, pp.dw.max.y);

    previewHeight = max (int (pp.h / (pp.w * pp.a) * previewWidth + .5f), 1);
    previewPixels.resizeErase (previewHeight, previewWidth);

    float fx = (previewWidth  > 1)? (float (pp.w - 1) / (previewWidth  - 1)): 1;
    float fy = (previewHeight > 1)? (float (pp.h - 1) / (previewHeight - 1)): 1;
    float m  = Math<float>::pow (2.f, IMATH_NAMESPACE::clamp (exposure + 2.47393f, -20.f, 20.f));

    for (int y = 0; y < previewHeight; ++y)
    {
	for (int x = 0; x < previewWidth; ++x)
	{
	    PreviewRgba &preview = previewPixels[y][x];
	    const Rgba &pixel = pixels[int (y * fy + .5f)][int (x * fx + .5f)];

	    preview.r = gamma (pixel.r, m);
	    preview.g = gamma (pixel.g, m);
	    preview.b = gamma (pixel.b, m);
	    preview.a = int (IMATH_NAMESPACE::clamp (pixel.a * 255.f, 0.f, 255.f) + .5f);
	}
    }
}

} // namespace

void
makePreview (const char inFileName[],
	     const char outFileName[],
	     int previewWidth,
	     float exposure,
	     bool verbose)
{
    if (verbose)
	cout << "generating preview image" << endl;

    Array2D <PreviewRgba> previewPixels;
    int previewHeight;

    generatePreview (inFileName,
		     exposure,
		     previewWidth,
		     previewHeight,
		     previewPixels);

    InputFile in (inFileName);
    Header header = in.header();
    header.setPreviewImage
	(PreviewImage (previewWidth, previewHeight, &previewPixels[0][0]));

    if (verbose)
	cout << "copying " << inFileName << " to " << outFileName << endl;

    struct FileParams {
        bool hasTileDescription;
        const char* outFileName;
        Header header;
    } fp = { header.hasTileDescription(), outFileName, header };

    if (fp.hasTileDescription)
    {
	TiledOutputFile out (fp.outFileName, fp.header);
	out.copyPixels (in);
    }
    else
    {
	OutputFile out (fp.outFileName, fp.header);
	out.copyPixels (in);
    }

    if (verbose)
	cout << "done." << endl;
}