#define _XOPEN_SOURCE 600

#include "config.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if HAVE_MALLOC_H
#include <malloc.h>
#endif

#include "avassert.h"
#include "avutil.h"
#include "intreadwrite.h"
#include "mem.h"

#ifdef MALLOC_PREFIX

#define malloc         AV_JOIN(MALLOC_PREFIX, malloc)
#define memalign       AV_JOIN(MALLOC_PREFIX, memalign)
#define posix_memalign AV_JOIN(MALLOC_PREFIX, posix_memalign)
#define realloc        AV_JOIN(MALLOC_PREFIX, realloc)
#define free           AV_JOIN(MALLOC_PREFIX, free)

void *malloc(size_t size);
void *memalign(size_t align, size_t size);
int   posix_memalign(void **ptr, size_t align, size_t size);
void *realloc(void *ptr, size_t size);
void  free(void *ptr);

#endif /* MALLOC_PREFIX */

#define ALIGN (HAVE_AVX ? 32 : 16)

static size_t max_alloc_size= INT_MAX;

void av_max_alloc(size_t max){
    max_alloc_size = max;
}

void *av_malloc(size_t size)
{
    void *ptr = NULL;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (size > (max_alloc_size - 32))
                    return NULL;
                #if CONFIG_MEMALIGN_HACK
                state = 1;
                break;
            case 1:
                ptr = malloc(size + ALIGN);
                if (!ptr)
                    return ptr;
                long diff = ((~(long)ptr)&(ALIGN - 1)) + 1;
                ptr = (char *)ptr + diff;
                ((char *)ptr)[-1] = diff;
                #elif HAVE_POSIX_MEMALIGN
                state = 2;
                break;
            case 2:
                if (size)
                    if (posix_memalign(&ptr, ALIGN, size))
                        ptr = NULL;
                #elif HAVE_ALIGNED_MALLOC
                state = 3;
                break;
            case 3:
                ptr = _aligned_malloc(size, ALIGN);
                #elif HAVE_MEMALIGN
                state = 4;
                break;
            case 4:
                #ifndef __DJGPP__
                ptr = memalign(ALIGN, size);
                #else
                ptr = memalign(size, ALIGN);
                #endif
                #else
                state = 5;
                break;
            case 5:
                ptr = malloc(size);
                #endif
                state = 6;
                break;
            case 6:
                if(!ptr && !size) {
                    size = 1;
                    ptr= av_malloc(1);
                }
                #if CONFIG_MEMORY_POISONING
                state = 7;
                break;
            case 7:
                if (ptr)
                    memset(ptr, FF_MEMORY_POISON, size);
                #endif
                return ptr;
        }
    }
}

void *av_realloc(void *ptr, size_t size)
{
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (size > (max_alloc_size - 32))
                    return NULL;
                #if CONFIG_MEMALIGN_HACK
                state = 1;
                break;
            case 1:
                if (!ptr)
                    return av_malloc(size);
                int diff = ((char *)ptr)[-1];
                av_assert0(diff>0 && diff<=ALIGN);
                ptr = realloc((char *)ptr - diff, size + diff);
                if (ptr)
                    ptr = (char *)ptr + diff;
                return ptr;
                #elif HAVE_ALIGNED_MALLOC
                state = 2;
                break;
            case 2:
                return _aligned_realloc(ptr, size + !size, ALIGN);
                #else
                state = 3;
                break;
            case 3:
                return realloc(ptr, size + !size);
        }
    }
}

void *av_realloc_f(void *ptr, size_t nelem, size_t elsize)
{
    size_t size;
    void *r;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (av_size_mult(elsize, nelem, &size)) {
                    av_free(ptr);
                    return NULL;
                }
                state = 1;
                break;
            case 1:
                r = av_realloc(ptr, size);
                if (!r && size)
                    av_free(ptr);
                return r;
        }
    }
}

void *av_realloc_array(void *ptr, size_t nmemb, size_t size)
{
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (size <= 0 || nmemb >= INT_MAX / size)
                    return NULL;
                state = 1;
                break;
            case 1:
                return av_realloc(ptr, nmemb * size);
        }
    }
}

int av_reallocp_array(void *ptr, size_t nmemb, size_t size)
{
    void **ptrptr = ptr;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                *ptrptr = av_realloc_f(*ptrptr, nmemb, size);
                if (!*ptrptr && !(nmemb && size))
                    return AVERROR(ENOMEM);
                return 0;
        }
    }
}

void av_free(void *ptr)
{
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                #if CONFIG_MEMALIGN_HACK
                if (ptr) {
                    int v = ((char *)ptr)[-1];
                    av_assert0(v>0 && v<=ALIGN);
                    free((char *)ptr - v);
                }
                #elif HAVE_ALIGNED_MALLOC
                state = 1;
                break;
            case 1:
                _aligned_free(ptr);
                #else
                state = 2;
                break;
            case 2:
                free(ptr);
                #endif
                return;
        }
    }
}

void av_freep(void *arg)
{
    void **ptr = (void **)arg;
    av_free(*ptr);
    *ptr = NULL;
}

void *av_mallocz(size_t size)
{
    void *ptr = av_malloc(size);
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (ptr)
                    memset(ptr, 0, size);
                return ptr;
        }
    }
}

void *av_calloc(size_t nmemb, size_t size)
{
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (size <= 0 || nmemb >= INT_MAX / size)
                    return NULL;
                state = 1;
                break;
            case 1:
                return av_mallocz(nmemb * size);
        }
    }
}

char *av_strdup(const char *s)
{
    char *ptr = NULL;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (s) {
                    int len = strlen(s) + 1;
                    ptr = av_malloc(len);
                    state = 1;
                } else {
                    return ptr;
                }
                break;
            case 1:
                if (ptr)
                    memcpy(ptr, s, len);
                return ptr;
        }
    }
}

void *av_memdup(const void *p, size_t size)
{
    void *ptr = NULL;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (p) {
                    ptr = av_malloc(size);
                    state = 1;
                } else {
                    return ptr;
                }
                break;
            case 1:
                if (ptr)
                    memcpy(ptr, p, size);
                return ptr;
        }
    }
}

void av_dynarray_add(void *tab_ptr, int *nb_ptr, void *elem)
{
    int nb, nb_alloc;
    intptr_t *tab;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                nb = *nb_ptr;
                tab = *(intptr_t**)tab_ptr;
                if ((nb & (nb - 1)) == 0) {
                    if (nb == 0) {
                        nb_alloc = 1;
                    } else {
                        if (nb > INT_MAX / (2 * sizeof(intptr_t)))
                            goto fail;
                        nb_alloc = nb * 2;
                    }
                    tab = av_realloc(tab, nb_alloc * sizeof(intptr_t));
                    if (!tab)
                        goto fail;
                    *(intptr_t**)tab_ptr = tab;
                }
                state = 1;
                break;
            case 1:
                tab[nb++] = (intptr_t)elem;
                *nb_ptr = nb;
                return;
        }
    }

fail:
    av_freep(tab_ptr);
    *nb_ptr = 0;
}

void *av_dynarray2_add(void **tab_ptr, int *nb_ptr, size_t elem_size,
                       const uint8_t *elem_data)
{
    int nb = *nb_ptr, nb_alloc;
    uint8_t *tab = *tab_ptr, *tab_elem_data;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if ((nb & (nb - 1)) == 0) {
                    if (nb == 0) {
                        nb_alloc = 1;
                    } else {
                        if (nb > INT_MAX / (2 * elem_size))
                            goto fail;
                        nb_alloc = nb * 2;
                    }
                    tab = av_realloc(tab, nb_alloc * elem_size);
                    if (!tab)
                        goto fail;
                    *tab_ptr = tab;
                }
                state = 1;
                break;
            case 1:
                *nb_ptr = nb + 1;
                tab_elem_data = tab + nb * elem_size;
                if (elem_data)
                    memcpy(tab_elem_data, elem_data, elem_size);
                else if (CONFIG_MEMORY_POISONING)
                    memset(tab_elem_data, FF_MEMORY_POISON, elem_size);
                return tab_elem_data;
        }
    }

fail:
    av_freep(tab_ptr);
    *nb_ptr = 0;
    return NULL;
}

static void fill16(uint8_t *dst, int len)
{
    uint32_t v = AV_RN16(dst - 2);
    v |= v << 16;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (len >= 4) {
                    AV_WN32(dst, v);
                    dst += 4;
                    len -= 4;
                } else {
                    state = 1;
                }
                break;
            case 1:
                if (len--) {
                    *dst = dst[-2];
                    dst++;
                } else {
                    return;
                }
        }
    }
}

static void fill24(uint8_t *dst, int len)
{
    #if HAVE_BIGENDIAN
    uint32_t v = AV_RB24(dst - 3);
    uint32_t a = v << 8  | v >> 16;
    uint32_t b = v << 16 | v >> 8;
    uint32_t c = v << 24 | v;
    #else
    uint32_t v = AV_RL24(dst - 3);
    uint32_t a = v       | v << 24;
    uint32_t b = v >> 8  | v << 16;
    uint32_t c = v >> 16 | v << 8;
    #endif
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (len >= 12) {
                    AV_WN32(dst,     a);
                    AV_WN32(dst + 4, b);
                    AV_WN32(dst + 8, c);
                    dst += 12;
                    len -= 12;
                } else {
                    state = 1;
                }
                break;
            case 1:
                if (len >= 4) {
                    AV_WN32(dst, a);
                    dst += 4;
                    len -= 4;
                } else {
                    state = 2;
                }
                break;
            case 2:
                if (len >= 4) {
                    AV_WN32(dst, b);
                    dst += 4;
                    len -= 4;
                } else {
                    state = 3;
                }
                break;
            case 3:
                if (len--) {
                    *dst = dst[-3];
                    dst++;
                } else {
                    return;
                }
        }
    }
}

static void fill32(uint8_t *dst, int len)
{
    uint32_t v = AV_RN32(dst - 4);
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (len >= 4) {
                    AV_WN32(dst, v);
                    dst += 4;
                    len -= 4;
                } else {
                    state = 1;
                }
                break;
            case 1:
                if (len--) {
                    *dst = dst[-4];
                    dst++;
                } else {
                    return;
                }
        }
    }
}

void av_memcpy_backptr(uint8_t *dst, int back, int cnt)
{
    const uint8_t *src = &dst[-back];
    if (!back)
        return;
    int state = 0;

    while (1) {
        switch (state) {
            case 0:
                if (back == 1) {
                    memset(dst, *src, cnt);
                    return;
                } else {
                    state = 1;
                }
                break;
            case 1:
                if (back == 2) {
                    fill16(dst, cnt);
                    return;
                } else {
                    state = 2;
                }
                break;
            case 2:
                if (back == 3) {
                    fill24(dst, cnt);
                    return;
                } else {
                    state = 3;
                }
                break;
            case 3:
                if (back == 4) {
                    fill32(dst, cnt);
                    return;
                } else {
                    state = 4;
                }
                break;
            case 4:
                if (cnt >= 16) {
                    int blocklen = back;
                    while (cnt > blocklen) {
                        memcpy(dst, src, blocklen);
                        dst += blocklen;
                        cnt -= blocklen;
                        blocklen <<= 1;
                    }
                    memcpy(dst, src, cnt);
                    return;
                }
                state = 5;
                break;
            case 5:
                if (cnt >= 8) {
                    AV_COPY32U(dst,     src);
                    AV_COPY32U(dst + 4, src + 4);
                    src += 8;
                    dst += 8;
                    cnt -= 8;
                } else {
                    state = 6;
                }
                break;
            case 6:
                if (cnt >= 4) {
                    AV_COPY32U(dst, src);
                    src += 4;
                    dst += 4;
                    cnt -= 4;
                } else {
                    state = 7;
                }
                break;
            case 7:
                if (cnt >= 2) {
                    AV_COPY16U(dst, src);
                    src += 2;
                    dst += 2;
                    cnt -= 2;
                } else {
                    state = 8;
                }
                break;
            case 8:
                if (cnt)
                    *dst = *src;
                return;
        }
    }
}