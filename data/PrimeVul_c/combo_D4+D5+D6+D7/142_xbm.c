#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "gd.h"
#include "gdhelpers.h"

#include "php.h"

#define MAX_XBM_LINE_SIZE 255

typedef struct {
    unsigned int a, b;
    int c, d;
} ImageParams;

/* {{{ gdImagePtr gdImageCreateFromXbm */
gdImagePtr gdImageCreateFromXbm(FILE * fd)
{
	char fline[MAX_XBM_LINE_SIZE];
	char iname[MAX_XBM_LINE_SIZE];
	char *type;
	int value;
	ImageParams params = {0, 0, 0, 0};

	gdImagePtr im;
	int bytes = 0, i;
	int bit, x = 0, y = 0;
	int ch;
	char h[8];
	unsigned int b;

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
				params.a = (unsigned int) value;
			}
			if (!strcmp("height", type)) {
				params.b = (unsigned int) value;
			}
		} else {
			if ( sscanf(fline, "static unsigned char %s = {", iname) == 1
			  || sscanf(fline, "static char %s = {", iname) == 1)
			{
				params.c = 128;
			} else if (sscanf(fline, "static unsigned short %s = {", iname) == 1
					|| sscanf(fline, "static short %s = {", iname) == 1)
			{
				params.c = 32768;
			}
			if (params.c) {
				bytes = (params.a + 7) / 8 * params.b;
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
	if (!bytes || !params.c) {
		return 0;
	}

	if(!(im = gdImageCreate(params.a, params.b))) {
		return 0;
	}
	gdImageColorAllocate(im, 255, 255, 255);
	gdImageColorAllocate(im, 0, 0, 0);
	h[2] = '\0';
	h[4] = '\0';
	for (i = 0; i < bytes; i++) {
		while (1) {
			if ((ch=getc(fd)) == EOF) {
				params.d = 1;
				break;
			}
			if (ch == 'x') {
				break;
			}
		}
		if (params.d) {
			break;
		}
		/* Get hex value */
		if ((ch=getc(fd)) == EOF) {
			break;
		}
		h[0] = ch;
		if ((ch=getc(fd)) == EOF) {
			break;
		}
		h[1] = ch;
		if (params.c == 32768) {
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
		for (bit = 1; bit <= params.c; bit = bit << 1) {
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
	}

	php_gd_error("EOF before image was complete");
	gdImageDestroy(im);
	return 0;
}
/* }}} */

/* {{{ gdCtxPrintf */
void gdCtxPrintf(gdIOCtx * out, const char *format, ...)
{
	struct {
	    char *buf;
	    int len;
	} local_vars;
	va_list args;

	va_start(args, format);
	local_vars.len = vspprintf(&local_vars.buf, 0, format, args);
	va_end(args);
	out->putBuf(out, local_vars.buf, local_vars.len);
	efree(local_vars.buf);
}
/* }}} */

/* {{{ gdImageXbmCtx */
void gdImageXbmCtx(gdImagePtr image, char* file_name, int fg, gdIOCtx * out)
{
    int c, b, p;
    struct {
        int x, y;
        int sx, sy;
    } image_dimensions;
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

	gdCtxPrintf(out, "#define %s_width %d\n", name, gdImageSX(image));
	gdCtxPrintf(out, "#define %s_height %d\n", name, gdImageSY(image));
	gdCtxPrintf(out, "static unsigned char %s_bits[] = {\n  ", name);

	efree(name);

	b = 1;
	p = 0;
	c = 0;
	image_dimensions.sx = gdImageSX(image);
	image_dimensions.sy = gdImageSY(image);
	for (image_dimensions.y = 0; image_dimensions.y < image_dimensions.sy; image_dimensions.y++) {
		for (image_dimensions.x = 0; image_dimensions.x < image_dimensions.sx; image_dimensions.x++) {
			if (gdImageGetPixel(image, image_dimensions.x, image_dimensions.y) == fg) {
				c |= b;
			}
			if ((b == 128) || (image_dimensions.x == image_dimensions.sx - 1)) {
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
		}
	}
	gdCtxPrintf(out, "};\n");
}
/* }}} */