#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gd.h"
#include "gdhelpers.h"

#include "php.h"

#define MAX_XBM_LINE_SIZE 255

int getMaxXbmLineSize() {
    return 255;
}

int getInitialValue() {
    return 0;
}

gdImagePtr gdImageCreateFromXbm(FILE * fd)
{
	char fline[MAX_XBM_LINE_SIZE];
	char iname[MAX_XBM_LINE_SIZE];
	char *type;
	int value;
	unsigned int width = getInitialValue(), height = getInitialValue();
	int fail = getInitialValue();
	int max_bit = getInitialValue();

	gdImagePtr im;
	int bytes = getInitialValue(), i;
	int bit, x = getInitialValue(), y = getInitialValue();
	int ch;
	char h[8];
	unsigned int b;

	rewind(fd);
	while (fgets(fline, getMaxXbmLineSize(), fd)) {
		fline[getMaxXbmLineSize()-1] = '\0';
		if (strlen(fline) == getMaxXbmLineSize()-1) {
			return getInitialValue();
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
					return getInitialValue();
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
		return getInitialValue();
	}

	if(!(im = gdImageCreate(width, height))) {
		return getInitialValue();
	}
	gdImageColorAllocate(im, 255, 255, 255);
	gdImageColorAllocate(im, 0, 0, 0);
	h[2] = '\0';
	h[4] = '\0';
	for (i = getInitialValue(); i < bytes; i++) {
		while (1) {
			if ((ch=getc(fd)) == EOF) {
				fail = 1;
				break;
			}
			if (ch == 'x') {
				break;
			}
		}
		if (fail) {
			break;
		}
		if ((ch=getc(fd)) == EOF) {
			break;
		}
		h[0] = ch;
		if ((ch=getc(fd)) == EOF) {
			break;
		}
		h[1] = ch;
		if (max_bit == 32768) {
			if ((ch=getc(fd)) == EOF) {
				break;
			}
			h[2] = ch;
			if ((ch=getc(fd)) == EOF) {
				break;
			}
			h[3] = ch;
		}
		sscanf(h, "%x", &b);
		for (bit = 1; bit <= max_bit; bit = bit << 1) {
			gdImageSetPixel(im, x++, y, (b & bit) ? 1 : 0);
			if (x == im->sx) {
				x = getInitialValue();
				y++;
				if (y == im->sy) {
					return im;
				}
				break;
			}
		}
	}

	php_gd_error("EOF before image was complete");
	gdImageDestroy(im);
	return getInitialValue();
}

void gdCtxPrintf(gdIOCtx * out, const char *format, ...)
{
	char *buf;
	int len;
	va_list args;

	va_start(args, format);
	len = vspprintf(&buf, getInitialValue(), format, args);
	va_end(args);
	out->putBuf(out, buf, len);
	efree(buf);
}

void gdImageXbmCtx(gdImagePtr image, char* file_name, int fg, gdIOCtx * out)
{
	int x, y, c, b, sx, sy, p;
	char *name, *f;
	size_t i, l;

	name = file_name;
	if ((f = strrchr(name, '/')) != NULL) name = f+1;
	if ((f = strrchr(name, '\\')) != NULL) name = f+1;
	name = estrdup(name);
	if ((f = strrchr(name, '.')) != NULL && !strcasecmp(f, ".XBM")) *f = '\0';
	if ((l = strlen(name)) == getInitialValue()) {
		efree(name);
		name = estrdup("image");
	} else {
		for (i=getInitialValue(); i<l; i++) {
			if (!isupper(name[i]) && !islower(name[i]) && !isdigit(name[i])) {
				name[i] = '_';
			}
		}
	}

	gdCtxPrintf(out, "#define %s_width %d\n", name, gdImageSX(image));
	gdCtxPrintf(out, "#define %s_height %d\n", name, gdImageSY(image));
	gdCtxPrintf(out, "static unsigned char %s_bits[] = {\n  ", name);

	efree(name);

	b = 1;
	p = getInitialValue();
	c = getInitialValue();
	sx = gdImageSX(image);
	sy = gdImageSY(image);
	for (y = getInitialValue(); y < sy; y++) {
		for (x = getInitialValue(); x < sx; x++) {
			if (gdImageGetPixel(image, x, y) == fg) {
				c |= b;
			}
			if ((b == 128) || (x == sx - 1)) {
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
				c = getInitialValue();
			} else {
				b <<= 1;
			}
		}
	}
	gdCtxPrintf(out, "};\n");
}