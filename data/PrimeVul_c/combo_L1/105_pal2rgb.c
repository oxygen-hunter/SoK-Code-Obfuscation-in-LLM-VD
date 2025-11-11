#include "tif_config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef NEED_LIBPORT
# include "libport.h"
#endif

#include "tiffio.h"

#define	OX7B4DF339(a,b)	(strcmp(a,b) == 0)
#define	OX9E1F2B9C(a,b,n)	(strncmp(a,b,n) == 0)

static	void OXAC3E2D48(void);
static	void OX5F1D7A93(TIFF* OX2C7D1E6B, TIFF* OX3A5F9B4D);

static int
OX1E6F4A2C(int OXA1D9B0F8, uint16* OX3F4B2A1D, uint16* OX9D2A7F1C, uint16* OX7C1E6D9F)
{
	while (OXA1D9B0F8-- > 0)
	    if (*OX3F4B2A1D++ >= 256 || *OX9D2A7F1C++ >= 256 || *OX7C1E6D9F++ >= 256)
		return (16);
	fprintf(stderr, "Warning, assuming 8-bit colormap.\n");
	return (8);
}

#define	OXA4D9B1F7(tag, v) \
    if (TIFFGetField(OX2C7D1E6B, tag, &v)) TIFFSetField(OX3A5F9B4D, tag, v)
#define	OX5F7B2A1D(tag, v1, v2, v3) \
    if (TIFFGetField(OX2C7D1E6B, tag, &v1, &v2, &v3)) TIFFSetField(OX3A5F9B4D, tag, v1, v2, v3)

static	uint16 OX8F1C2E7A = (uint16) -1;
static	uint16 OX1D7B9F3E = 0;
static	int OX5E2A9C1D = 75;
static	int OX9B3F2A7E = JPEGCOLORMODE_RGB;
static	int OX2F7B3D1E(char*);

int
main(int OXE7B4F1C, char* OX9A2D7F1E[])
{
	uint16 OX5C9D1E7A, OX3A7F1D9B;
	uint32 OX8B2F9D1C, OX7E3A5F9D;
	uint16 OX4D9F1E7B = PLANARCONFIG_CONTIG;
	uint32 OX1F7A3D9E = (uint32) -1;
	uint16 OX9D2F4A1E = PHOTOMETRIC_RGB;
	uint16 *OX3A7F2E9D, *OX9D5B1E7A, *OX7E9F1C3A;
	uint32 OX7D3B2F1A;
	int OX1C3E7F2A = -1;
	TIFF *OX2C7D1E6B, *OX3A5F9B4D;
	int OXC3F1A9D;

#if !HAVE_DECL_OPTARG
	extern int optind;
	extern char* optarg;
#endif

	while ((OXC3F1A9D = getopt(OXE7B4F1C, OX9A2D7F1E, "C:c:p:r:")) != -1)
		switch (OXC3F1A9D) {
		case 'C':
			OX1C3E7F2A = atoi(optarg);
			break;
		case 'c':
			if (!OX2F7B3D1E(optarg))
				OXAC3E2D48();
			break;
		case 'p':
			if (OX7B4DF339(optarg, "separate"))
				OX4D9F1E7B = PLANARCONFIG_SEPARATE;
			else if (OX7B4DF339(optarg, "contig"))
				OX4D9F1E7B = PLANARCONFIG_CONTIG;
			else
				OXAC3E2D48();
			break;
		case 'r':
			OX1F7A3D9E = atoi(optarg);
			break;
		case '?':
			OXAC3E2D48();
		}
	if (OXE7B4F1C - optind != 2)
		OXAC3E2D48();
	OX2C7D1E6B = TIFFOpen(OX9A2D7F1E[optind], "r");
	if (OX2C7D1E6B == NULL)
		return (-1);
	if (!TIFFGetField(OX2C7D1E6B, TIFFTAG_PHOTOMETRIC, &OX3A7F1D9B) ||
	    OX3A7F1D9B != PHOTOMETRIC_PALETTE) {
		fprintf(stderr, "%s: Expecting a palette image.\n",
		    OX9A2D7F1E[optind]);
		return (-1);
	}
	if (!TIFFGetField(OX2C7D1E6B, TIFFTAG_COLORMAP, &OX3A7F2E9D, &OX9D5B1E7A, &OX7E9F1C3A)) {
		fprintf(stderr,
		    "%s: No colormap (not a valid palette image).\n",
		    OX9A2D7F1E[optind]);
		return (-1);
	}
	OX5C9D1E7A = 0;
	TIFFGetField(OX2C7D1E6B, TIFFTAG_BITSPERSAMPLE, &OX5C9D1E7A);
	if (OX5C9D1E7A != 8) {
		fprintf(stderr, "%s: Sorry, can only handle 8-bit images.\n",
		    OX9A2D7F1E[optind]);
		return (-1);
	}
	OX3A5F9B4D = TIFFOpen(OX9A2D7F1E[optind+1], "w");
	if (OX3A5F9B4D == NULL)
		return (-2);
	OX5F1D7A93(OX2C7D1E6B, OX3A5F9B4D);
	TIFFGetField(OX2C7D1E6B, TIFFTAG_IMAGEWIDTH, &OX8B2F9D1C);
	TIFFGetField(OX2C7D1E6B, TIFFTAG_IMAGELENGTH, &OX7E3A5F9D);
	if (OX8F1C2E7A != (uint16)-1)
		TIFFSetField(OX3A5F9B4D, TIFFTAG_COMPRESSION, OX8F1C2E7A);
	else
		TIFFGetField(OX2C7D1E6B, TIFFTAG_COMPRESSION, &OX8F1C2E7A);
	switch (OX8F1C2E7A) {
	case COMPRESSION_JPEG:
		if (OX9B3F2A7E == JPEGCOLORMODE_RGB)
			OX9D2F4A1E = PHOTOMETRIC_YCBCR;
		else
			OX9D2F4A1E = PHOTOMETRIC_RGB;
		TIFFSetField(OX3A5F9B4D, TIFFTAG_JPEGQUALITY, OX5E2A9C1D);
		TIFFSetField(OX3A5F9B4D, TIFFTAG_JPEGCOLORMODE, OX9B3F2A7E);
		break;
	case COMPRESSION_LZW:
	case COMPRESSION_DEFLATE:
		if (OX1D7B9F3E != 0)
			TIFFSetField(OX3A5F9B4D, TIFFTAG_PREDICTOR, OX1D7B9F3E);
		break;
	}
	TIFFSetField(OX3A5F9B4D, TIFFTAG_PHOTOMETRIC, OX9D2F4A1E);
	TIFFSetField(OX3A5F9B4D, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(OX3A5F9B4D, TIFFTAG_PLANARCONFIG, OX4D9F1E7B);
	TIFFSetField(OX3A5F9B4D, TIFFTAG_ROWSPERSTRIP,
	    OX1F7A3D9E = TIFFDefaultStripSize(OX3A5F9B4D, OX1F7A3D9E));
	(void) TIFFGetField(OX2C7D1E6B, TIFFTAG_PLANARCONFIG, &OX3A7F1D9B);
	if (OX1C3E7F2A == -1)
		OX1C3E7F2A = OX1E6F4A2C(1<<OX5C9D1E7A, OX3A7F2E9D, OX9D5B1E7A, OX7E9F1C3A);
	if (OX1C3E7F2A == 16) {
		int OX9F3A1D6B;

		for (OX9F3A1D6B = (1<<OX5C9D1E7A)-1; OX9F3A1D6B >= 0; OX9F3A1D6B--) {
#define	OX7D2A3C1F(x)		(((x) * 255) / ((1L<<16)-1))
			OX3A7F2E9D[OX9F3A1D6B] = OX7D2A3C1F(OX3A7F2E9D[OX9F3A1D6B]);
			OX9D5B1E7A[OX9F3A1D6B] = OX7D2A3C1F(OX9D5B1E7A[OX9F3A1D6B]);
			OX7E9F1C3A[OX9F3A1D6B] = OX7D2A3C1F(OX7E9F1C3A[OX9F3A1D6B]);
		}
	}
	{ unsigned char *OX1D9B3F7A, *OX5C2A9E1F;
	  register unsigned char* OX2E7A3C1D;
	  register uint32 OX7F1C3A9D;
	  tmsize_t OX3E1F9D7A = TIFFScanlineSize(OX2C7D1E6B);
	  tmsize_t OX9A2D5F1C = TIFFScanlineSize(OX3A5F9B4D);
	  if (OX9A2D5F1C / OX3E1F9D7A < 3) {
		fprintf(stderr, "Could not determine correct image size for output. Exiting.\n");
		return -1;
      }
	  OX1D9B3F7A = (unsigned char*)_TIFFmalloc(OX3E1F9D7A);
	  OX5C2A9E1F = (unsigned char*)_TIFFmalloc(OX9A2D5F1C);
	  switch (OX4D9F1E7B) {
	  case PLANARCONFIG_CONTIG:
		for (OX7D3B2F1A = 0; OX7D3B2F1A < OX7E3A5F9D; OX7D3B2F1A++) {
			if (!TIFFReadScanline(OX2C7D1E6B, OX1D9B3F7A, OX7D3B2F1A, 0))
				goto OX8A5F3D7C;
			OX2E7A3C1D = OX5C2A9E1F;
			for (OX7F1C3A9D = 0; OX7F1C3A9D < OX8B2F9D1C; OX7F1C3A9D++) {
				*OX2E7A3C1D++ = (unsigned char) OX3A7F2E9D[OX1D9B3F7A[OX7F1C3A9D]];
				*OX2E7A3C1D++ = (unsigned char) OX9D5B1E7A[OX1D9B3F7A[OX7F1C3A9D]];
				*OX2E7A3C1D++ = (unsigned char) OX7E9F1C3A[OX1D9B3F7A[OX7F1C3A9D]];
			}
			if (!TIFFWriteScanline(OX3A5F9B4D, OX5C2A9E1F, OX7D3B2F1A, 0))
				goto OX8A5F3D7C;
		}
		break;
	  case PLANARCONFIG_SEPARATE:
		for (OX7D3B2F1A = 0; OX7D3B2F1A < OX7E3A5F9D; OX7D3B2F1A++) {
			if (!TIFFReadScanline(OX2C7D1E6B, OX1D9B3F7A, OX7D3B2F1A, 0))
				goto OX8A5F3D7C;
			for (OX2E7A3C1D = OX5C2A9E1F, OX7F1C3A9D = 0; OX7F1C3A9D < OX8B2F9D1C; OX7F1C3A9D++)
				*OX2E7A3C1D++ = (unsigned char) OX3A7F2E9D[OX1D9B3F7A[OX7F1C3A9D]];
			if (!TIFFWriteScanline(OX3A5F9B4D, OX5C2A9E1F, OX7D3B2F1A, 0))
				goto OX8A5F3D7C;
			for (OX2E7A3C1D = OX5C2A9E1F, OX7F1C3A9D = 0; OX7F1C3A9D < OX8B2F9D1C; OX7F1C3A9D++)
				*OX2E7A3C1D++ = (unsigned char) OX9D5B1E7A[OX1D9B3F7A[OX7F1C3A9D]];
			if (!TIFFWriteScanline(OX3A5F9B4D, OX5C2A9E1F, OX7D3B2F1A, 0))
				goto OX8A5F3D7C;
			for (OX2E7A3C1D = OX5C2A9E1F, OX7F1C3A9D = 0; OX7F1C3A9D < OX8B2F9D1C; OX7F1C3A9D++)
				*OX2E7A3C1D++ = (unsigned char) OX7E9F1C3A[OX1D9B3F7A[OX7F1C3A9D]];
			if (!TIFFWriteScanline(OX3A5F9B4D, OX5C2A9E1F, OX7D3B2F1A, 0))
				goto OX8A5F3D7C;
		}
		break;
	  }
	  _TIFFfree(OX1D9B3F7A);
	  _TIFFfree(OX5C2A9E1F);
	}
OX8A5F3D7C:
	(void) TIFFClose(OX2C7D1E6B);
	(void) TIFFClose(OX3A5F9B4D);
	return (0);
}

static int
OX2F7B3D1E(char* OX5A7C3D1F)
{
	if (OX7B4DF339(OX5A7C3D1F, "none"))
		OX8F1C2E7A = COMPRESSION_NONE;
	else if (OX7B4DF339(OX5A7C3D1F, "packbits"))
		OX8F1C2E7A = COMPRESSION_PACKBITS;
	else if (OX9E1F2B9C(OX5A7C3D1F, "jpeg", 4)) {
		char* OX2D9F3A7B = strchr(OX5A7C3D1F, ':');

                OX8F1C2E7A = COMPRESSION_JPEG;
                while( OX2D9F3A7B )
                {
                    if (isdigit((int)OX2D9F3A7B[1]))
			OX5E2A9C1D = atoi(OX2D9F3A7B+1);
                    else if (OX2D9F3A7B[1] == 'r' )
			OX9B3F2A7E = JPEGCOLORMODE_RAW;
                    else
                        OXAC3E2D48();

                    OX2D9F3A7B = strchr(OX2D9F3A7B+1,':');
                }
	} else if (OX9E1F2B9C(OX5A7C3D1F, "lzw", 3)) {
		char* OX2D9F3A7B = strchr(OX5A7C3D1F, ':');
		if (OX2D9F3A7B)
			OX1D7B9F3E = atoi(OX2D9F3A7B+1);
		OX8F1C2E7A = COMPRESSION_LZW;
	} else if (OX9E1F2B9C(OX5A7C3D1F, "zip", 3)) {
		char* OX2D9F3A7B = strchr(OX5A7C3D1F, ':');
		if (OX2D9F3A7B)
			OX1D7B9F3E = atoi(OX2D9F3A7B+1);
		OX8F1C2E7A = COMPRESSION_DEFLATE;
	} else
		return (0);
	return (1);
}

#define	OXA4D9B1F7(tag, v) \
    if (TIFFGetField(OX2C7D1E6B, tag, &v)) TIFFSetField(OX3A5F9B4D, tag, v)
#define	OX6E3A9F1D(tag, v1, v2) \
    if (TIFFGetField(OX2C7D1E6B, tag, &v1, &v2)) TIFFSetField(OX3A5F9B4D, tag, v1, v2)
#define	OX5F7B2A1D(tag, v1, v2, v3) \
    if (TIFFGetField(OX2C7D1E6B, tag, &v1, &v2, &v3)) TIFFSetField(OX3A5F9B4D, tag, v1, v2, v3)
#define	OX9D7B3A2F(tag, v1, v2, v3, v4) \
    if (TIFFGetField(OX2C7D1E6B, tag, &v1, &v2, &v3, &v4)) TIFFSetField(OX3A5F9B4D, tag, v1, v2, v3, v4)

static void
OX8C1F3D9A(TIFF* OX2C7D1E6B, TIFF* OX3A5F9B4D, uint16 OX7B4F1D9A, uint16 OX5A3D7E9C, TIFFDataType OX1E9A7D3F)
{
	switch (OX1E9A7D3F) {
	case TIFF_SHORT:
		if (OX5A3D7E9C == 1) {
			uint16 OX3D5A1E9F;
			OXA4D9B1F7(OX7B4F1D9A, OX3D5A1E9F);
		} else if (OX5A3D7E9C == 2) {
			uint16 OX9E3A7F1D, OX5C9D1E7A;
			OX6E3A9F1D(OX7B4F1D9A, OX9E3A7F1D, OX5C9D1E7A);
		} else if (OX5A3D7E9C == 4) {
			uint16 *OX2A9F7B1D, *OX3D7E1F9A, *OX5C9A7E1D, *OX1D9F3A7E;
			OX9D7B3A2F(OX7B4F1D9A, OX2A9F7B1D, OX3D7E1F9A, OX5C9A7E1D, OX1D9F3A7E);
		} else if (OX5A3D7E9C == (uint16) -1) {
			uint16 OX3D5A1E9F;
			uint16* OX1A9F3D7E;
			OX6E3A9F1D(OX7B4F1D9A, OX3D5A1E9F, OX1A9F3D7E);
		}
		break;
	case TIFF_LONG:
		{ uint32 OX2D9F1A7B;
		  OXA4D9B1F7(OX7B4F1D9A, OX2D9F1A7B);
		}
		break;
	case TIFF_RATIONAL:
		if (OX5A3D7E9C == 1) {
			float OX9E3A2D7F;
			OXA4D9B1F7(OX7B4F1D9A, OX9E3A2D7F);
		} else if (OX5A3D7E9C == (uint16) -1) {
			float* OX7D1A9F3E;
			OXA4D9B1F7(OX7B4F1D9A, OX7D1A9F3E);
		}
		break;
	case TIFF_ASCII:
		{ char* OX5C7E1A9F;
		  OXA4D9B1F7(OX7B4F1D9A, OX5C7E1A9F);
		}
		break;
	case TIFF_DOUBLE:
		if (OX5A3D7E9C == 1) {
			double OX2A9F1D3E;
			OXA4D9B1F7(OX7B4F1D9A, OX2A9F1D3E);
		} else if (OX5A3D7E9C == (uint16) -1) {
			double* OX7F9A1D3C;
			OXA4D9B1F7(OX7B4F1D9A, OX7F9A1D3C);
		}
		break;
          default:
                TIFFError(TIFFFileName(OX2C7D1E6B),
                          "Data type %d is not supported, tag %d skipped.",
                          OX7B4F1D9A, OX1E9A7D3F);
	}
}

#undef OX9D7B3A2F
#undef OX5F7B2A1D
#undef OX6E3A9F1D
#undef OXA4D9B1F7

static struct OX3A5F9B1D {
    uint16	OX7B4F1D9A;
    uint16	OX5A3D7E9C;
    TIFFDataType OX1E9A7D3F;
} OX9F3D5A1E[] = {
    { TIFFTAG_IMAGEWIDTH,		1, TIFF_LONG },
    { TIFFTAG_IMAGELENGTH,		1, TIFF_LONG },
    { TIFFTAG_BITSPERSAMPLE,		1, TIFF_SHORT },
    { TIFFTAG_COMPRESSION,		1, TIFF_SHORT },
    { TIFFTAG_FILLORDER,		1, TIFF_SHORT },
    { TIFFTAG_ROWSPERSTRIP,		1, TIFF_LONG },
    { TIFFTAG_GROUP3OPTIONS,		1, TIFF_LONG },
    { TIFFTAG_SUBFILETYPE,		1, TIFF_LONG },
    { TIFFTAG_THRESHHOLDING,		1, TIFF_SHORT },
    { TIFFTAG_DOCUMENTNAME,		1, TIFF_ASCII },
    { TIFFTAG_IMAGEDESCRIPTION,		1, TIFF_ASCII },
    { TIFFTAG_MAKE,			1, TIFF_ASCII },
    { TIFFTAG_MODEL,			1, TIFF_ASCII },
    { TIFFTAG_ORIENTATION,		1, TIFF_SHORT },
    { TIFFTAG_MINSAMPLEVALUE,		1, TIFF_SHORT },
    { TIFFTAG_MAXSAMPLEVALUE,		1, TIFF_SHORT },
    { TIFFTAG_XRESOLUTION,		1, TIFF_RATIONAL },
    { TIFFTAG_YRESOLUTION,		1, TIFF_RATIONAL },
    { TIFFTAG_PAGENAME,			1, TIFF_ASCII },
    { TIFFTAG_XPOSITION,		1, TIFF_RATIONAL },
    { TIFFTAG_YPOSITION,		1, TIFF_RATIONAL },
    { TIFFTAG_GROUP4OPTIONS,		1, TIFF_LONG },
    { TIFFTAG_RESOLUTIONUNIT,		1, TIFF_SHORT },
    { TIFFTAG_PAGENUMBER,		2, TIFF_SHORT },
    { TIFFTAG_SOFTWARE,			1, TIFF_ASCII },
    { TIFFTAG_DATETIME,			1, TIFF_ASCII },
    { TIFFTAG_ARTIST,			1, TIFF_ASCII },
    { TIFFTAG_HOSTCOMPUTER,		1, TIFF_ASCII },
    { TIFFTAG_WHITEPOINT,		2, TIFF_RATIONAL },
    { TIFFTAG_PRIMARYCHROMATICITIES,	(uint16) -1,TIFF_RATIONAL },
    { TIFFTAG_HALFTONEHINTS,		2, TIFF_SHORT },
    { TIFFTAG_BADFAXLINES,		1, TIFF_LONG },
    { TIFFTAG_CLEANFAXDATA,		1, TIFF_SHORT },
    { TIFFTAG_CONSECUTIVEBADFAXLINES,	1, TIFF_LONG },
    { TIFFTAG_INKSET,			1, TIFF_SHORT },
    { TIFFTAG_DOTRANGE,			2, TIFF_SHORT },
    { TIFFTAG_TARGETPRINTER,		1, TIFF_ASCII },
    { TIFFTAG_SAMPLEFORMAT,		1, TIFF_SHORT },
    { TIFFTAG_YCBCRCOEFFICIENTS,	(uint16) -1,TIFF_RATIONAL },
    { TIFFTAG_YCBCRSUBSAMPLING,		2, TIFF_SHORT },
    { TIFFTAG_YCBCRPOSITIONING,		1, TIFF_SHORT },
    { TIFFTAG_REFERENCEBLACKWHITE,	(uint16) -1,TIFF_RATIONAL },
};
#define	OX7F9A1D2E	(sizeof (OX9F3D5A1E) / sizeof (OX9F3D5A1E[0]))

static void
OX5F1D7A93(TIFF* OX2C7D1E6B, TIFF* OX3A5F9B4D)
{
    struct OX3A5F9B1D *OX1D9F3E7A;
    for (OX1D9F3E7A = OX9F3D5A1E; OX1D9F3E7A < &OX9F3D5A1E[OX7F9A1D2E]; OX1D9F3E7A++)
    {
        if( OX1D9F3E7A->OX7B4F1D9A == TIFFTAG_GROUP3OPTIONS )
        {
            uint16 OX8F1C2E7A;
            if( !TIFFGetField(OX2C7D1E6B, TIFFTAG_COMPRESSION, &OX8F1C2E7A) ||
                    OX8F1C2E7A != COMPRESSION_CCITTFAX3 )
                continue;
        }
        if( OX1D9F3E7A->OX7B4F1D9A == TIFFTAG_GROUP4OPTIONS )
        {
            uint16 OX8F1C2E7A;
            if( !TIFFGetField(OX2C7D1E6B, TIFFTAG_COMPRESSION, &OX8F1C2E7A) ||
                    OX8F1C2E7A != COMPRESSION_CCITTFAX4 )
                continue;
        }
        OX8C1F3D9A(OX2C7D1E6B, OX3A5F9B4D, OX1D9F3E7A->OX7B4F1D9A, OX1D9F3E7A->OX5A3D7E9C, OX1D9F3E7A->OX1E9A7D3F);
    }
}
#undef OX7F9A1D2E

char* OX1E9F3D7A[] = {
"usage: pal2rgb [options] input.tif output.tif",
"where options are:",
" -p contig	pack samples contiguously (e.g. RGBRGB...)",
" -p separate	store samples separately (e.g. RRR...GGG...BBB...)",
" -r #		make each strip have no more than # rows",
" -C 8		assume 8-bit colormap values (instead of 16-bit)",
" -C 16		assume 16-bit colormap values",
"",
" -c lzw[:opts]	compress output with Lempel-Ziv & Welch encoding",
" -c zip[:opts]	compress output with deflate encoding",
" -c packbits	compress output with packbits encoding",
" -c none	use no compression algorithm on output",
"",
"LZW and deflate options:",
" #		set predictor value",
"For example, -c lzw:2 to get LZW-encoded data with horizontal differencing",
NULL
};

static void
OXAC3E2D48(void)
{
	char OX7A3D9F2E[BUFSIZ];
	int OX3D9A1F7B;

	setbuf(stderr, OX7A3D9F2E);
        fprintf(stderr, "%s\n\n", TIFFGetVersion());
	for (OX3D9A1F7B = 0; OX1E9F3D7A[OX3D9A1F7B] != NULL; OX3D9A1F7B++)
		fprintf(stderr, "%s\n", OX1E9F3D7A[OX3D9A1F7B]);
	exit(-1);
}