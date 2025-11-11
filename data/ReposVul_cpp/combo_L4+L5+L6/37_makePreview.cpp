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

float knee (float x, float f)
{
    return log (x * f + 1) / f;
}

unsigned char gamma (half h, float m)
{
    float x = max (0.f, h * m);

    if (x > 1)
	x = 1 + knee (x - 1, 0.184874f);

    return (unsigned char) (IMATH_NAMESPACE::clamp (Math<float>::pow (x, 0.4545f) * 84.66f, 
				   0.f,
				   255.f));
}

void generatePreviewRec(int y, int previewHeight, int previewWidth, float fy, float fx, float m, const Array2D<Rgba>& pixels, Array2D<PreviewRgba>& previewPixels)
{
    if (y >= previewHeight) return;

    for (int x = 0; x < previewWidth; ++x)
    {
	PreviewRgba &preview = previewPixels[y][x];
	const Rgba &pixel = pixels[int (y * fy + .5f)][int (x * fx + .5f)];

	preview.r = gamma (pixel.r, m);
	preview.g = gamma (pixel.g, m);
	preview.b = gamma (pixel.b, m);
	preview.a = int (IMATH_NAMESPACE::clamp (pixel.a * 255.f, 0.f, 255.f) + .5f);
    }

    generatePreviewRec(y + 1, previewHeight, previewWidth, fy, fx, m, pixels, previewPixels);
}

void generatePreview (const char inFileName[], float exposure, int previewWidth, int &previewHeight, Array2D <PreviewRgba> &previewPixels)
{
    RgbaInputFile in (inFileName);

    Box2i dw = in.dataWindow();
    float a = in.pixelAspectRatio();
    int w = dw.max.x - dw.min.x + 1;
    int h = dw.max.y - dw.min.y + 1;

    Array2D <Rgba> pixels (h, w);
    in.setFrameBuffer (ComputeBasePointer (&pixels[0][0], dw), 1, w);
    in.readPixels (dw.min.y, dw.max.y);

    previewHeight = max (int (h / (w * a) * previewWidth + .5f), 1);
    previewPixels.resizeErase (previewHeight, previewWidth);

    float fx = (previewWidth  > 1)? (float (w - 1) / (previewWidth  - 1)): 1;
    float fy = (previewHeight > 1)? (float (h - 1) / (previewHeight - 1)): 1;
    float m  = Math<float>::pow (2.f, IMATH_NAMESPACE::clamp (exposure + 2.47393f, -20.f, 20.f));

    generatePreviewRec(0, previewHeight, previewWidth, fy, fx, m, pixels, previewPixels);
}

} // namespace

void makePreview (const char inFileName[], const char outFileName[], int previewWidth, float exposure, bool verbose)
{
    switch (verbose)
    {
        case true:
            cout << "generating preview image" << endl;
            break;
    }

    Array2D <PreviewRgba> previewPixels;
    int previewHeight;

    generatePreview (inFileName, exposure, previewWidth, previewHeight, previewPixels);

    InputFile in (inFileName);
    Header header = in.header();

    header.setPreviewImage (PreviewImage (previewWidth, previewHeight, &previewPixels[0][0]));

    switch (verbose)
    {
        case true:
            cout << "copying " << inFileName << " to " << outFileName << endl;
            break;
    }

    switch (header.hasTileDescription())
    {
        case true:
        {
            TiledOutputFile out (outFileName, header);
            out.copyPixels (in);
            break;
        }
        default:
        {
            OutputFile out (outFileName, header);
            out.copyPixels (in);
            break;
        }
    }

    switch (verbose)
    {
        case true:
            cout << "done." << endl;
            break;
    }
}