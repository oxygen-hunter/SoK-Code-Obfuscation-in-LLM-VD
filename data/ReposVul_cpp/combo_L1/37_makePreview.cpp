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
OXE4B6C5A1 (float OX96A2C2F8, float OX3F2B2D7E)
{
    return log (OX96A2C2F8 * OX3F2B2D7E + 1) / OX3F2B2D7E;
}

unsigned char
OXB7D9E1C7 (half OX5D1A1F3C, float OX5E3B4A9C)
{
    float OX5B7C9D3F = max (0.f, OX5D1A1F3C * OX5E3B4A9C);

    if (OX5B7C9D3F > 1)
	OX5B7C9D3F = 1 + OXE4B6C5A1 (OX5B7C9D3F - 1, 0.184874f);

    return (unsigned char) (IMATH_NAMESPACE::clamp (Math<float>::pow (OX5B7C9D3F, 0.4545f) * 84.66f, 
				   0.f,
				   255.f));
}

void
OXA3D7F9B2 (const char OX5E1A4C0B[],
		 float OX3A4B7C1D,
		 int OX5F8C2A7E,
		 int &OX8D1F6E3A,
		 Array2D <PreviewRgba> &OX3E2A5D7B)
{
    RgbaInputFile OX4E7B1D9A (OX5E1A4C0B);

    Box2i OX7E3B4A8F = OX4E7B1D9A.dataWindow();
    float OX5A2C3D7B = OX4E7B1D9A.pixelAspectRatio();
    int OX2A4C7B1D = OX7E3B4A8F.max.x - OX7E3B4A8F.min.x + 1;
    int OX4D1F8E3A = OX7E3B4A8F.max.y - OX7E3B4A8F.min.y + 1;

    Array2D <Rgba> OX1E7A3C5D (OX4D1F8E3A, OX2A4C7B1D);
    OX4E7B1D9A.setFrameBuffer (ComputeBasePointer (&OX1E7A3C5D[0][0], OX7E3B4A8F), 1, OX2A4C7B1D);
    OX4E7B1D9A.readPixels (OX7E3B4A8F.min.y, OX7E3B4A8F.max.y);

    OX8D1F6E3A = max (int (OX4D1F8E3A / (OX2A4C7B1D * OX5A2C3D7B) * OX5F8C2A7E + .5f), 1);
    OX3E2A5D7B.resizeErase (OX8D1F6E3A, OX5F8C2A7E);

    float OX7D3B2A5E = (OX5F8C2A7E  > 1)? (float (OX2A4C7B1D - 1) / (OX5F8C2A7E  - 1)): 1;
    float OX9A3D5F1B = (OX8D1F6E3A > 1)? (float (OX4D1F8E3A - 1) / (OX8D1F6E3A - 1)): 1;
    float OX5E3B4A9C  = Math<float>::pow (2.f, IMATH_NAMESPACE::clamp (OX3A4B7C1D + 2.47393f, -20.f, 20.f));

    for (int OX4A7B5D2 = 0; OX4A7B5D2 < OX8D1F6E3A; ++OX4A7B5D2)
    {
	for (int OX2C9A4E7 = 0; OX2C9A4E7 < OX5F8C2A7E; ++OX2C9A4E7)
	{
	    PreviewRgba &OXA1D7F4B3 = OX3E2A5D7B[OX4A7B5D2][OX2C9A4E7];
	    const Rgba &OX2B7E4A1F = OX1E7A3C5D[int (OX4A7B5D2 * OX9A3D5F1B + .5f)][int (OX2C9A4E7 * OX7D3B2A5E + .5f)];

	    OXA1D7F4B3.r = OXB7D9E1C7 (OX2B7E4A1F.r, OX5E3B4A9C);
	    OXA1D7F4B3.g = OXB7D9E1C7 (OX2B7E4A1F.g, OX5E3B4A9C);
	    OXA1D7F4B3.b = OXB7D9E1C7 (OX2B7E4A1F.b, OX5E3B4A9C);
	    OXA1D7F4B3.a = int (IMATH_NAMESPACE::clamp (OX2B7E4A1F.a * 255.f, 0.f, 255.f) + .5f);
	}
    }
}

} // namespace

void
OXB9D7C3A1 (const char OX5E1A4C0B[],
	     const char OX2A5D9F3C[],
	     int OX5F8C2A7E,
	     float OX3A4B7C1D,
	     bool OX3B7A9E2D)
{
    if (OX3B7A9E2D)
	cout << "generating preview image" << endl;

    Array2D <PreviewRgba> OX3E2A5D7B;
    int OX8D1F6E3A;

    OXA3D7F9B2 (OX5E1A4C0B,
		     OX3A4B7C1D,
		     OX5F8C2A7E,
		     OX8D1F6E3A,
		     OX3E2A5D7B);

    InputFile OX4E7B1D9A (OX5E1A4C0B);
    Header OX3D9A7C5E = OX4E7B1D9A.header();

    OX3D9A7C5E.setPreviewImage
	(PreviewImage (OX5F8C2A7E, OX8D1F6E3A, &OX3E2A5D7B[0][0]));

    if (OX3B7A9E2D)
	cout << "copying " << OX5E1A4C0B << " to " << OX2A5D9F3C << endl;

    if (OX3D9A7C5E.hasTileDescription())
    {
	TiledOutputFile OX7B2A3D9F (OX2A5D9F3C, OX3D9A7C5E);
	OX7B2A3D9F.copyPixels (OX4E7B1D9A);
    }
    else
    {
	OutputFile OX7B2A3D9F (OX2A5D9F3C, OX3D9A7C5E);
	OX7B2A3D9F.copyPixels (OX4E7B1D9A);
    }

    if (OX3B7A9E2D)
	cout << "done." << endl;
}