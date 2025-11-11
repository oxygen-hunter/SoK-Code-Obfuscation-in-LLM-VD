#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gd.h"
#include "gdhelpers.h"

#include "php.h"

#define OX1A7E9B8E 255

gdImagePtr OX7B4DF339(FILE * OX2C618E7F)
{
	char OX2E3D9F6B[OX1A7E9B8E];
	char OX5F8D6A1E[OX1A7E9B8E];
	char *OX6A5D3B92;
	int OX5A9E2B1D;
	unsigned int OX3D9C8E1F = 0, OX1B2F7C9D = 0;
	int OX3F1B6D7C = 0;
	int OX7A3C2E9F = 0;

	gdImagePtr OX6E8A4B1F;
	int OX4F9B3C2E = 0, OX7D2A1C3B;
	int OX5D4F1B8E, OX8F7D6B2E = 0, OX3E2C5A9F = 0;
	int OX1B6D7F4C;
	char OX4C3E2D1A[8];
	unsigned int OX9F6B3D7E;

	rewind(OX2C618E7F);
	while (fgets(OX2E3D9F6B, OX1A7E9B8E, OX2C618E7F)) {
		OX2E3D9F6B[OX1A7E9B8E-1] = '\0';
		if (strlen(OX2E3D9F6B) == OX1A7E9B8E-1) {
			return 0;
		}
		if (sscanf(OX2E3D9F6B, "#define %s %d", OX5F8D6A1E, &OX5A9E2B1D) == 2) {
			if (!(OX6A5D3B92 = strrchr(OX5F8D6A1E, '_'))) {
				OX6A5D3B92 = OX5F8D6A1E;
			} else {
				OX6A5D3B92++;
			}

			if (!strcmp("width", OX6A5D3B92)) {
				OX3D9C8E1F = (unsigned int) OX5A9E2B1D;
			}
			if (!strcmp("height", OX6A5D3B92)) {
				OX1B2F7C9D = (unsigned int) OX5A9E2B1D;
			}
		} else {
			if ( sscanf(OX2E3D9F6B, "static unsigned char %s = {", OX5F8D6A1E) == 1
			  || sscanf(OX2E3D9F6B, "static char %s = {", OX5F8D6A1E) == 1)
			{
				OX7A3C2E9F = 128;
			} else if (sscanf(OX2E3D9F6B, "static unsigned short %s = {", OX5F8D6A1E) == 1
					|| sscanf(OX2E3D9F6B, "static short %s = {", OX5F8D6A1E) == 1)
			{
				OX7A3C2E9F = 32768;
			}
			if (OX7A3C2E9F) {
				OX4F9B3C2E = (OX3D9C8E1F + 7) / 8 * OX1B2F7C9D;
				if (!OX4F9B3C2E) {
					return 0;
				}
				if (!(OX6A5D3B92 = strrchr(OX5F8D6A1E, '_'))) {
					OX6A5D3B92 = OX5F8D6A1E;
				} else {
					OX6A5D3B92++;
				}
				if (!strcmp("bits[]", OX6A5D3B92)) {
					break;
				}
			}
 		}
	}
	if (!OX4F9B3C2E || !OX7A3C2E9F) {
		return 0;
	}

	if(!(OX6E8A4B1F = gdImageCreate(OX3D9C8E1F, OX1B2F7C9D))) {
		return 0;
	}
	gdImageColorAllocate(OX6E8A4B1F, 255, 255, 255);
	gdImageColorAllocate(OX6E8A4B1F, 0, 0, 0);
	OX4C3E2D1A[2] = '\0';
	OX4C3E2D1A[4] = '\0';
	for (OX7D2A1C3B = 0; OX7D2A1C3B < OX4F9B3C2E; OX7D2A1C3B++) {
		while (1) {
			if ((OX1B6D7F4C=getc(OX2C618E7F)) == EOF) {
				OX3F1B6D7C = 1;
				break;
			}
			if (OX1B6D7F4C == 'x') {
				break;
			}
		}
		if (OX3F1B6D7C) {
			break;
		}
		if ((OX1B6D7F4C=getc(OX2C618E7F)) == EOF) {
			break;
		}
		OX4C3E2D1A[0] = OX1B6D7F4C;
		if ((OX1B6D7F4C=getc(OX2C618E7F)) == EOF) {
			break;
		}
		OX4C3E2D1A[1] = OX1B6D7F4C;
		if (OX7A3C2E9F == 32768) {
			if ((OX1B6D7F4C=getc(OX2C618E7F)) == EOF) {
				break;
			}
			OX4C3E2D1A[2] = OX1B6D7F4C;
			if ((OX1B6D7F4C=getc(OX2C618E7F)) == EOF) {
				break;
			}
			OX4C3E2D1A[3] = OX1B6D7F4C;
		}
		sscanf(OX4C3E2D1A, "%x", &OX9F6B3D7E);
		for (OX5D4F1B8E = 1; OX5D4F1B8E <= OX7A3C2E9F; OX5D4F1B8E = OX5D4F1B8E << 1) {
			gdImageSetPixel(OX6E8A4B1F, OX8F7D6B2E++, OX3E2C5A9F, (OX9F6B3D7E & OX5D4F1B8E) ? 1 : 0);
			if (OX8F7D6B2E == OX6E8A4B1F->sx) {
				OX8F7D6B2E = 0;
				OX3E2C5A9F++;
				if (OX3E2C5A9F == OX6E8A4B1F->sy) {
					return OX6E8A4B1F;
				}
				break;
			}
		}
	}

	php_gd_error("EOF before image was complete");
	gdImageDestroy(OX6E8A4B1F);
	return 0;
}

void OX5C3A7F2E(gdIOCtx * OX4F6E2D1A, const char *OX1A2D7E3B, ...)
{
	char *OX6B9F1C3E;
	int OX3C2E5A7B;
	va_list OX4B7D6F1A;

	va_start(OX4B7D6F1A, OX1A2D7E3B);
	OX3C2E5A7B = vspprintf(&OX6B9F1C3E, 0, OX1A2D7E3B, OX4B7D6F1A);
	va_end(OX4B7D6F1A);
	OX4F6E2D1A->putBuf(OX4F6E2D1A, OX6B9F1C3E, OX3C2E5A7B);
	efree(OX6B9F1C3E);
}

void OX1E7B5C3F(gdImagePtr OX6E3A9F5B, char* OX4C1D7E9F, int OX2D3F8A7B, gdIOCtx * OX7F2C1A3E)
{
	int OX8F4D6B2A, OX3E9C5D1B, OX7B1F6A3E, OX2A7E9F4C, OX9B3D6C1E, OX1C7E5B3A, OX4D2E6A9F;
	char *OX5A1C3E7F, *OX7E9B6F4A;
	size_t OX6D7C3A5E, OX3F1B8A7D;

	OX5A1C3E7F = OX4C1D7E9F;
	if ((OX7E9B6F4A = strrchr(OX5A1C3E7F, '/')) != NULL) OX5A1C3E7F = OX7E9B6F4A+1;
	if ((OX7E9B6F4A = strrchr(OX5A1C3E7F, '\\')) != NULL) OX5A1C3E7F = OX7E9B6F4A+1;
	OX5A1C3E7F = estrdup(OX5A1C3E7F);
	if ((OX7E9B6F4A = strrchr(OX5A1C3E7F, '.')) != NULL && !strcasecmp(OX7E9B6F4A, ".XBM")) *OX7E9B6F4A = '\0';
	if ((OX3F1B8A7D = strlen(OX5A1C3E7F)) == 0) {
		efree(OX5A1C3E7F);
		OX5A1C3E7F = estrdup("image");
	} else {
		for (OX6D7C3A5E=0; OX6D7C3A5E<OX3F1B8A7D; OX6D7C3A5E++) {
			if (!isupper(OX5A1C3E7F[OX6D7C3A5E]) && !islower(OX5A1C3E7F[OX6D7C3A5E]) && !isdigit(OX5A1C3E7F[OX6D7C3A5E])) {
				OX5A1C3E7F[OX6D7C3A5E] = '_';
			}
		}
	}

	OX5C3A7F2E(OX7F2C1A3E, "#define %s_width %d\n", OX5A1C3E7F, gdImageSX(OX6E3A9F5B));
	OX5C3A7F2E(OX7F2C1A3E, "#define %s_height %d\n", OX5A1C3E7F, gdImageSY(OX6E3A9F5B));
	OX5C3A7F2E(OX7F2C1A3E, "static unsigned char %s_bits[] = {\n  ", OX5A1C3E7F);

	efree(OX5A1C3E7F);

	OX2A7E9F4C = 1;
	OX4D2E6A9F = 0;
	OX7B1F6A3E = 0;
	OX9B3D6C1E = gdImageSX(OX6E3A9F5B);
	OX1C7E5B3A = gdImageSY(OX6E3A9F5B);
	for (OX3E9C5D1B = 0; OX3E9C5D1B < OX1C7E5B3A; OX3E9C5D1B++) {
		for (OX8F4D6B2A = 0; OX8F4D6B2A < OX9B3D6C1E; OX8F4D6B2A++) {
			if (gdImageGetPixel(OX6E3A9F5B, OX8F4D6B2A, OX3E9C5D1B) == OX2D3F8A7B) {
				OX7B1F6A3E |= OX2A7E9F4C;
			}
			if ((OX2A7E9F4C == 128) || (OX8F4D6B2A == OX9B3D6C1E - 1)) {
				OX2A7E9F4C = 1;
				if (OX4D2E6A9F) {
					OX5C3A7F2E(OX7F2C1A3E, ", ");
					if (!(OX4D2E6A9F%12)) {
						OX5C3A7F2E(OX7F2C1A3E, "\n  ");
						OX4D2E6A9F = 12;
					}
				}
				OX4D2E6A9F++;
				OX5C3A7F2E(OX7F2C1A3E, "0x%02X", OX7B1F6A3E);
				OX7B1F6A3E = 0;
			} else {
				OX2A7E9F4C <<= 1;
			}
		}
	}
	OX5C3A7F2E(OX7F2C1A3E, "};\n");
}