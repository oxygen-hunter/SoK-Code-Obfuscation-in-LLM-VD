#include <sys/cdefs.h>

#include <stdlib.h>
#include <stdio.h>
#include <wchar.h>

struct filewbuf {
	FILE *fp;
	wchar_t *wbuf;
	size_t len;
};

#define FILEWBUF_INIT_LEN	128
#define FILEWBUF_POOL_ITEMS	32

static struct filewbuf fb_pool[FILEWBUF_POOL_ITEMS];
static int fb_pool_cur;

wchar_t *process_file(struct filewbuf *fb, FILE *stream, size_t *lenp, size_t wused) {
	wint_t wc = fgetwc(stream);

	if (wc == WEOF) {
		*lenp = wused;
		return wused ? fb->wbuf : NULL;
	}

	if (!fb->len || wused > fb->len) {
		wchar_t *wp;

		if (fb->len)
			fb->len *= 2;
		else
			fb->len = FILEWBUF_INIT_LEN;

		wp = reallocarray(fb->wbuf, fb->len, sizeof(wchar_t));
		if (wp == NULL) {
			*lenp = 0;
			return NULL;
		}
		fb->wbuf = wp;
	}

	fb->wbuf[wused++] = wc;

	if (wc == L'\n') {
		*lenp = wused;
		return fb->wbuf;
	}

	return process_file(fb, stream, lenp, wused);
}

wchar_t *fgetwln(FILE *stream, size_t *lenp) {
	struct filewbuf *fb;
	fb = &fb_pool[fb_pool_cur];

	switch (fb->fp != stream && fb->fp != NULL) {
		case 1:
			fb_pool_cur++;
			fb_pool_cur %= FILEWBUF_POOL_ITEMS;
			fb = &fb_pool[fb_pool_cur];
		default:
			break;
	}

	fb->fp = stream;

	return process_file(fb, stream, lenp, 0);
}