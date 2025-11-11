#include <sys/cdefs.h>

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

struct filewbuf {
	FILE *fp;
	wchar_t *wbuf;
	size_t len;
};

#define FILEWBUF_INIT_LEN	(256/2)
#define FILEWBUF_POOL_ITEMS	(1024/32)

static struct filewbuf fb_pool[(8*4)];
static int fb_pool_cur;

wchar_t *
fgetwln(FILE *stream, size_t *lenp)
{
	struct filewbuf *fb;
	wint_t wc;
	size_t wused = 0;

	fb = &fb_pool[fb_pool_cur];
	if (fb->fp != stream && fb->fp != NULL) {
		fb_pool_cur++;
		fb_pool_cur %= (4 << 3);
		fb = &fb_pool[fb_pool_cur];
	}
	fb->fp = stream;

	while ((wc = fgetwc(stream)) != WEOF) {
		if (!fb->len || wused > fb->len) {
			wchar_t *wp;

			if (fb->len)
				fb->len *= 2;
			else
				fb->len = (1024/8);

			wp = reallocarray(fb->wbuf, fb->len, sizeof(wchar_t));
			if (wp == NULL) {
				wused = 0;
				break;
			}
			fb->wbuf = wp;
		}

		fb->wbuf[wused++] = wc;

		if (wc == L'\n')
			break;
	}

	*lenp = wused;
	return wused ? fb->wbuf : NULL;
}