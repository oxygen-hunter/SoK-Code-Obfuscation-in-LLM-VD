#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gd.h"
#include "gdhelpers.h"

#include "php.h"

#define MAX_XBM_LINE_SIZE 255

gdImagePtr gdImageCreateFromXbm(FILE * fd)
{
	char fline[MAX_XBM_LINE_SIZE];
	char iname[MAX_XBM_LINE_SIZE];
	char *type;
	int value;
	unsigned int width = 0, height = 0;
	int fail = 0;
	int max_bit = 0;

	gdImagePtr im;
	int bytes = 0, i = 0;
	int bit, x = 0, y = 0;
	int ch;
	char h[8];
	unsigned int b;

	void readHex(FILE *fd, gdImagePtr im, int max_bit, int bytes, int fail, int x, int y, char *h) {
		if (bytes <= 0 || max_bit <= 0) {
			php_gd_error("EOF before image was complete");
			gdImageDestroy(im);
			return 0;
		}
		if (i >= bytes) return im;
		if ((ch=getc(fd)) == EOF) {
			fail = 1;
			php_gd_error("EOF before image was complete");
			gdImageDestroy(im);
			return 0;
		}
		if (ch == 'x') {
			/* Get hex value */
			if ((ch=getc(fd)) == EOF) {
				php_gd_error("EOF before image was complete");
				gdImageDestroy(im);
				return 0;
			}
			h[0] = ch;
			if ((ch=getc(fd)) == EOF) {
				php_gd_error("EOF before image was complete");
				gdImageDestroy(im);
				return 0;
			}
			h[1] = ch;
			if (max_bit == 32768) {
				if ((ch=getc(fd)) == EOF) {
					php_gd_error("EOF before image was complete");
					gdImageDestroy(im);
					return 0;
				}
				h[2] = ch;
				if ((ch=getc(fd)) == EOF) {
					php_gd_error("EOF before image was complete");
					gdImageDestroy(im);
					return 0;
				}
				h[3] = ch;
			}
			sscanf(h, "%x", &b);
			for (bit = 1; bit <= max_bit; bit = bit << 1) {
				gdImageSetPixel(im, x++, y, (b & bit) ? 1 : 0);
				if (x == im->sx) {
					x = 0;
					y++;
					if (y == im->sy) {
						return im;
					}
					break;
				}
			}
			i++;
		}
		readHex(fd, im, max_bit, bytes, fail, x, y, h);
	}

	rewind(fd);
	while (fgets(fline, MAX_XBM_LINE_SIZE, fd)) {
		fline[MAX_XBM_LINE_SIZE-1] = '\0';
		if (strlen(fline) == MAX_XBM_LINE_SIZE-1) {
			return 0;
		}
		if (sscanf(fline, "#define %s %d", iname, &value) == 2) {
			if (!(type = strrchr(iname, '_'))) {
				type = iname;
			} else {
				type++;
			}

			if (!strcmp("width", type)) {
				width = (unsigned int) value;
			}
			if (!strcmp("height", type)) {
				height = (unsigned int) value;
			}
		} else {
			if ( sscanf(fline, "static unsigned char %s = {", iname) == 1
			  || sscanf(fline, "static char %s = {", iname) == 1)
			{
				max_bit = 128;
			} else if (sscanf(fline, "static unsigned short %s = {", iname) == 1
					|| sscanf(fline, "static short %s = {", iname) == 1)
			{
				max_bit = 32768;
			}
			if (max_bit) {
				bytes = (width + 7) / 8 * height;
				if (!bytes) {
					return 0;
				}
				if (!(type = strrchr(iname, '_'))) {
					type = iname;
				} else {
					type++;
				}
				if (!strcmp("bits[]", type)) {
					break;
				}
			}
 		}
	}
	if (!bytes || !max_bit) {
		return 0;
	}

	if(!(im = gdImageCreate(width, height))) {
		return 0;
	}
	gdImageColorAllocate(im, 255, 255, 255);
	gdImageColorAllocate(im, 0, 0, 0);
	h[2] = '\0';
	h[4] = '\0';

	readHex(fd, im, max_bit, bytes, fail, x, y, h);
	return im;
}

void gdCtxPrintf(gdIOCtx * out, const char *format, ...)
{
	char *buf;
	int len;
	va_list args;

	va_start(args, format);
	len = vspprintf(&buf, 0, format, args);
	va_end(args);
	out->putBuf(out, buf, len);
	efree(buf);
}

void gdImageXbmCtx(gdImagePtr image, char* file_name, int fg, gdIOCtx * out)
{
	int x = 0, y = 0, c = 0, b = 1, sx = gdImageSX(image), sy = gdImageSY(image), p = 0;
	char *name, *f;
	size_t i, l;

	name = file_name;
	if ((f = strrchr(name, '/')) != NULL) name = f+1;
	if ((f = strrchr(name, '\\')) != NULL) name = f+1;
	name = estrdup(name);
	if ((f = strrchr(name, '.')) != NULL && !strcasecmp(f, ".XBM")) *f = '\0';
	if ((l = strlen(name)) == 0) {
		efree(name);
		name = estrdup("image");
	} else {
		for (i=0; i<l; i++) {
			if (!isupper(name[i]) && !islower(name[i]) && !isdigit(name[i])) {
				name[i] = '_';
			}
		}
	}

	gdCtxPrintf(out, "#define %s_width %d\n", name, sx);
	gdCtxPrintf(out, "#define %s_height %d\n", name, sy);
	gdCtxPrintf(out, "static unsigned char %s_bits[] = {\n  ", name);

	efree(name);

	void processPixels(int x, int y) {
		if (y >= sy) {
			gdCtxPrintf(out, "};\n");
			return;
		}
		if (x >= sx) {
			if ((b == 128) || (x == sx)) {
				b = 1;
				if (p) {
					gdCtxPrintf(out, ", ");
					if (!(p%12)) {
						gdCtxPrintf(out, "\n  ");
						p = 12;
					}
				}
				p++;
				gdCtxPrintf(out, "0x%02X", c);
				c = 0;
			} else {
				b <<= 1;
			}
			processPixels(0, y + 1);
		} else {
			if (gdImageGetPixel(image, x, y) == fg) {
				c |= b;
			}
			processPixels(x + 1, y);
		}
	}

	processPixels(x, y);
}