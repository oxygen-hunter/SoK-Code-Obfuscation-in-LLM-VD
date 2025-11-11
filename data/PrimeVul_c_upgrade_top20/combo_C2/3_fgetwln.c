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

wchar_t *
fgetwln(FILE *stream, size_t *lenp)
{
	struct filewbuf *fb;
	wint_t wc;
	size_t wused = 0;
	int dispatch = 0;
	
	while (1) {
		switch (dispatch) {
			case 0:
				fb = &fb_pool[fb_pool_cur];
				if (fb->fp != stream && fb->fp != NULL) {
					fb_pool_cur++;
					fb_pool_cur %= FILEWBUF_POOL_ITEMS;
					fb = &fb_pool[fb_pool_cur];
				}
				fb->fp = stream;
				dispatch = 1;
				break;
			
			case 1:
				wc = fgetwc(stream);
				if (wc == WEOF) {
					dispatch = 4;
					break;
				}
				if (!fb->len || wused > fb->len) {
					dispatch = 2;
					break;
				}
				fb->wbuf[wused++] = wc;
				if (wc == L'\n') {
					dispatch = 4;
					break;
				}
				dispatch = 1;
				break;
			
			case 2: {
				wchar_t *wp;
				if (fb->len)
					fb->len *= 2;
				else
					fb->len = FILEWBUF_INIT_LEN;
				wp = reallocarray(fb->wbuf, fb->len, sizeof(wchar_t));
				if (wp == NULL) {
					wused = 0;
					dispatch = 3;
					break;
				}
				fb->wbuf = wp;
				dispatch = 1;
				break;
			}
			
			case 3:
				*lenp = wused;
				return wused ? fb->wbuf : NULL;
			
			case 4:
				*lenp = wused;
				return wused ? fb->wbuf : NULL;
		}
	}
}