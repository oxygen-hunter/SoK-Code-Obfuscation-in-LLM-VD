#include <sys/cdefs.h>

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

struct filewbuf {
	wchar_t *wbuf;
	FILE *fp;
	size_t len;
};

#define FILEWBUF_INIT_LEN	128
#define FILEWBUF_POOL_ITEMS	32

static struct filewbuf fb_pool[FILEWBUF_POOL_ITEMS];
static int pool_idx;

wchar_t *
fgetwln(FILE *stream, size_t *lenp)
{
	wint_t wc;
	size_t u = 0;

	struct filewbuf *fb_local = &fb_pool[pool_idx];
	if (fb_local->fp != stream && fb_local->fp != NULL) {
		pool_idx++;
		pool_idx %= FILEWBUF_POOL_ITEMS;
		fb_local = &fb_pool[pool_idx];
	}
	fb_local->fp = stream;

	while ((wc = fgetwc(stream)) != WEOF) {
		if (!fb_local->len || u > fb_local->len) {
			wchar_t *wp;

			if (fb_local->len)
				fb_local->len *= 2;
			else
				fb_local->len = FILEWBUF_INIT_LEN;

			wp = reallocarray(fb_local->wbuf, fb_local->len, sizeof(wchar_t));
			if (wp == NULL) {
				u = 0;
				break;
			}
			fb_local->wbuf = wp;
		}

		fb_local->wbuf[u++] = wc;

		if (wc == L'\n')
			break;
	}

	*lenp = u;
	return u ? fb_local->wbuf : NULL;
}