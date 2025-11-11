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

void *OX23AB7C4E(size_t OX3D4E8C7B);
void *OX2A8D5F3B(size_t OX8B1A9F6E, size_t OX3D4E8C7B);
int   OX1F9C2E5D(void **OX7C9A4B2E, size_t OX8B1A9F6E, size_t OX3D4E8C7B);
void *OX9E2B3C4D(void *OX5A8E1D6C, size_t OX3D4E8C7B);
void  OX4F1B7E6D(void *OX5A8E1D6C);

#endif 

#define OX6A3D1E2B (HAVE_AVX ? 32 : 16)

static size_t OX7B4DF339= INT_MAX;

void OX9D8C3B2A(size_t OX3D4E8C7B){
    OX7B4DF339 = OX3D4E8C7B;
}

void *OX1A2B3C4D(size_t OX3D4E8C7B)
{
    void *OX5A8E1D6C = NULL;
#if CONFIG_MEMALIGN_HACK
    long OX6B2C1D3A;
#endif

    if (OX3D4E8C7B > (OX7B4DF339 - 32))
        return NULL;

#if CONFIG_MEMALIGN_HACK
    OX5A8E1D6C = malloc(OX3D4E8C7B + OX6A3D1E2B);
    if (!OX5A8E1D6C)
        return OX5A8E1D6C;
    OX6B2C1D3A              = ((~(long)OX5A8E1D6C)&(OX6A3D1E2B - 1)) + 1;
    OX5A8E1D6C               = (char *)OX5A8E1D6C + OX6B2C1D3A;
    ((char *)OX5A8E1D6C)[-1] = OX6B2C1D3A;
#elif HAVE_POSIX_MEMALIGN
    if (OX3D4E8C7B)
    if (posix_memalign(&OX5A8E1D6C, OX6A3D1E2B, OX3D4E8C7B))
        OX5A8E1D6C = NULL;
#elif HAVE_ALIGNED_MALLOC
    OX5A8E1D6C = _aligned_malloc(OX3D4E8C7B, OX6A3D1E2B);
#elif HAVE_MEMALIGN
#ifndef __DJGPP__
    OX5A8E1D6C = memalign(OX6A3D1E2B, OX3D4E8C7B);
#else
    OX5A8E1D6C = memalign(OX3D4E8C7B, OX6A3D1E2B);
#endif
#else
    OX5A8E1D6C = malloc(OX3D4E8C7B);
#endif
    if(!OX5A8E1D6C && !OX3D4E8C7B) {
        OX3D4E8C7B = 1;
        OX5A8E1D6C= OX1A2B3C4D(1);
    }
#if CONFIG_MEMORY_POISONING
    if (OX5A8E1D6C)
        memset(OX5A8E1D6C, FF_MEMORY_POISON, OX3D4E8C7B);
#endif
    return OX5A8E1D6C;
}

void *OX2B3C4D5E(void *OX5A8E1D6C, size_t OX3D4E8C7B)
{
#if CONFIG_MEMALIGN_HACK
    int OX6B2C1D3A;
#endif

    if (OX3D4E8C7B > (OX7B4DF339 - 32))
        return NULL;

#if CONFIG_MEMALIGN_HACK
    if (!OX5A8E1D6C)
        return OX1A2B3C4D(OX3D4E8C7B);
    OX6B2C1D3A = ((char *)OX5A8E1D6C)[-1];
    av_assert0(OX6B2C1D3A>0 && OX6B2C1D3A<=OX6A3D1E2B);
    OX5A8E1D6C = realloc((char *)OX5A8E1D6C - OX6B2C1D3A, OX3D4E8C7B + OX6B2C1D3A);
    if (OX5A8E1D6C)
        OX5A8E1D6C = (char *)OX5A8E1D6C + OX6B2C1D3A;
    return OX5A8E1D6C;
#elif HAVE_ALIGNED_MALLOC
    return _aligned_realloc(OX5A8E1D6C, OX3D4E8C7B + !OX3D4E8C7B, OX6A3D1E2B);
#else
    return realloc(OX5A8E1D6C, OX3D4E8C7B + !OX3D4E8C7B);
#endif
}

void *OX3C4D5E6F(void *OX5A8E1D6C, size_t OX7F8D9A2B, size_t OXA1B2C3D4)
{
    size_t OX4E5F6A7B;
    void *OX9E2B3C4D;

    if (av_size_mult(OXA1B2C3D4, OX7F8D9A2B, &OX4E5F6A7B)) {
        av_free(OX5A8E1D6C);
        return NULL;
    }
    OX9E2B3C4D = OX2B3C4D5E(OX5A8E1D6C, OX4E5F6A7B);
    if (!OX9E2B3C4D && OX4E5F6A7B)
        av_free(OX5A8E1D6C);
    return OX9E2B3C4D;
}

void *OX4D5E6F7A(void *OX5A8E1D6C, size_t OX9B8C7D6E, size_t OX4E5F6A7B)
{
    if (OX4E5F6A7B <= 0 || OX9B8C7D6E >= INT_MAX / OX4E5F6A7B)
        return NULL;
    return OX2B3C4D5E(OX5A8E1D6C, OX9B8C7D6E * OX4E5F6A7B);
}

int OX5E6F7A8B(void *OX9A8B7C6D, size_t OX9B8C7D6E, size_t OX4E5F6A7B)
{
    void **OX7C9A4B2E = OX9A8B7C6D;
    *OX7C9A4B2E = OX3C4D5E6F(*OX7C9A4B2E, OX9B8C7D6E, OX4E5F6A7B);
    if (!*OX7C9A4B2E && !(OX9B8C7D6E && OX4E5F6A7B))
        return AVERROR(ENOMEM);
    return 0;
}

void OX6F7A8B9C(void *OX5A8E1D6C)
{
#if CONFIG_MEMALIGN_HACK
    if (OX5A8E1D6C) {
        int OX1B2C3D4E= ((char *)OX5A8E1D6C)[-1];
        av_assert0(OX1B2C3D4E>0 && OX1B2C3D4E<=OX6A3D1E2B);
        free((char *)OX5A8E1D6C - OX1B2C3D4E);
    }
#elif HAVE_ALIGNED_MALLOC
    _aligned_free(OX5A8E1D6C);
#else
    free(OX5A8E1D6C);
#endif
}

void OX7A8B9C0D(void *OX8E1D6C3B)
{
    void **OX7C9A4B2E = (void **)OX8E1D6C3B;
    OX6F7A8B9C(*OX7C9A4B2E);
    *OX7C9A4B2E = NULL;
}

void *OX8B9C0D1E(size_t OX3D4E8C7B)
{
    void *OX5A8E1D6C = OX1A2B3C4D(OX3D4E8C7B);
    if (OX5A8E1D6C)
        memset(OX5A8E1D6C, 0, OX3D4E8C7B);
    return OX5A8E1D6C;
}

void *OX9C0D1E2F(size_t OX9B8C7D6E, size_t OX4E5F6A7B)
{
    if (OX4E5F6A7B <= 0 || OX9B8C7D6E >= INT_MAX / OX4E5F6A7B)
        return NULL;
    return OX8B9C0D1E(OX9B8C7D6E * OX4E5F6A7B);
}

char *OX0D1E2F3A(const char *OX7D6E5F4B)
{
    char *OX5A8E1D6C = NULL;
    if (OX7D6E5F4B) {
        int OX8B1A9F6E = strlen(OX7D6E5F4B) + 1;
        OX5A8E1D6C = OX1A2B3C4D(OX8B1A9F6E);
        if (OX5A8E1D6C)
            memcpy(OX5A8E1D6C, OX7D6E5F4B, OX8B1A9F6E);
    }
    return OX5A8E1D6C;
}

void *OX1E2F3A4B(const void *OX7D6E5F4B, size_t OX3D4E8C7B)
{
    void *OX5A8E1D6C = NULL;
    if (OX7D6E5F4B) {
        OX5A8E1D6C = OX1A2B3C4D(OX3D4E8C7B);
        if (OX5A8E1D6C)
            memcpy(OX5A8E1D6C, OX7D6E5F4B, OX3D4E8C7B);
    }
    return OX5A8E1D6C;
}

void OX2F3A4B5C(void *OX9A8B7C6D, int *OX3D4E8C7B, void *OX7D6E5F4B)
{
    int OX7F8D9A2B, OX5E6F7A8B;
    intptr_t *OX7C9A4B2E;

    OX7F8D9A2B = *OX3D4E8C7B;
    OX7C9A4B2E = *(intptr_t**)OX9A8B7C6D;
    if ((OX7F8D9A2B & (OX7F8D9A2B - 1)) == 0) {
        if (OX7F8D9A2B == 0) {
            OX5E6F7A8B = 1;
        } else {
            if (OX7F8D9A2B > INT_MAX / (2 * sizeof(intptr_t)))
                goto OX1F2E3D4C;
            OX5E6F7A8B = OX7F8D9A2B * 2;
        }
        OX7C9A4B2E = OX2B3C4D5E(OX7C9A4B2E, OX5E6F7A8B * sizeof(intptr_t));
        if (!OX7C9A4B2E)
            goto OX1F2E3D4C;
        *(intptr_t**)OX9A8B7C6D = OX7C9A4B2E;
    }
    OX7C9A4B2E[OX7F8D9A2B++] = (intptr_t)OX7D6E5F4B;
    *OX3D4E8C7B = OX7F8D9A2B;
    return;

OX1F2E3D4C:
    OX7A8B9C0D(OX9A8B7C6D);
    *OX3D4E8C7B = 0;
}

void *OX3A4B5C6D(void **OX9A8B7C6D, int *OX3D4E8C7B, size_t OXA1B2C3D4,
                       const uint8_t *OX7D6E5F4B)
{
    int OX7F8D9A2B = *OX3D4E8C7B, OX5E6F7A8B;
    uint8_t *OX7C9A4B2E = *OX9A8B7C6D, *OX8E1D6C3B;

    if ((OX7F8D9A2B & (OX7F8D9A2B - 1)) == 0) {
        if (OX7F8D9A2B == 0) {
            OX5E6F7A8B = 1;
        } else {
            if (OX7F8D9A2B > INT_MAX / (2 * OXA1B2C3D4))
                goto OX1F2E3D4C;
            OX5E6F7A8B = OX7F8D9A2B * 2;
        }
        OX7C9A4B2E = OX2B3C4D5E(OX7C9A4B2E, OX5E6F7A8B * OXA1B2C3D4);
        if (!OX7C9A4B2E)
            goto OX1F2E3D4C;
        *OX9A8B7C6D = OX7C9A4B2E;
    }
    *OX3D4E8C7B = OX7F8D9A2B + 1;
    OX8E1D6C3B = OX7C9A4B2E + OX7F8D9A2B*OXA1B2C3D4;
    if (OX7D6E5F4B)
        memcpy(OX8E1D6C3B, OX7D6E5F4B, OXA1B2C3D4);
    else if (CONFIG_MEMORY_POISONING)
        memset(OX8E1D6C3B, FF_MEMORY_POISON, OXA1B2C3D4);
    return OX8E1D6C3B;

OX1F2E3D4C:
    OX7A8B9C0D(OX9A8B7C6D);
    *OX3D4E8C7B = 0;
    return NULL;
}

static void OX4B5C6D7E(uint8_t *OX7C9A4B2E, int OX3D4E8C7B)
{
    uint32_t OX8E1D6C3B = AV_RN16(OX7C9A4B2E - 2);

    OX8E1D6C3B |= OX8E1D6C3B << 16;

    while (OX3D4E8C7B >= 4) {
        AV_WN32(OX7C9A4B2E, OX8E1D6C3B);
        OX7C9A4B2E += 4;
        OX3D4E8C7B -= 4;
    }

    while (OX3D4E8C7B--) {
        *OX7C9A4B2E = OX7C9A4B2E[-2];
        OX7C9A4B2E++;
    }
}

static void OX5C6D7E8F(uint8_t *OX7C9A4B2E, int OX3D4E8C7B)
{
#if HAVE_BIGENDIAN
    uint32_t OX8E1D6C3B = AV_RB24(OX7C9A4B2E - 3);
    uint32_t OXA1B2C3D4 = OX8E1D6C3B << 8  | OX8E1D6C3B >> 16;
    uint32_t OXB2C3D4E5 = OX8E1D6C3B << 16 | OX8E1D6C3B >> 8;
    uint32_t OXC3D4E5F6 = OX8E1D6C3B << 24 | OX8E1D6C3B;
#else
    uint32_t OX8E1D6C3B = AV_RL24(OX7C9A4B2E - 3);
    uint32_t OXA1B2C3D4 = OX8E1D6C3B       | OX8E1D6C3B << 24;
    uint32_t OXB2C3D4E5 = OX8E1D6C3B >> 8  | OX8E1D6C3B << 16;
    uint32_t OXC3D4E5F6 = OX8E1D6C3B >> 16 | OX8E1D6C3B << 8;
#endif

    while (OX3D4E8C7B >= 12) {
        AV_WN32(OX7C9A4B2E,     OXA1B2C3D4);
        AV_WN32(OX7C9A4B2E + 4, OXB2C3D4E5);
        AV_WN32(OX7C9A4B2E + 8, OXC3D4E5F6);
        OX7C9A4B2E += 12;
        OX3D4E8C7B -= 12;
    }

    if (OX3D4E8C7B >= 4) {
        AV_WN32(OX7C9A4B2E, OXA1B2C3D4);
        OX7C9A4B2E += 4;
        OX3D4E8C7B -= 4;
    }

    if (OX3D4E8C7B >= 4) {
        AV_WN32(OX7C9A4B2E, OXB2C3D4E5);
        OX7C9A4B2E += 4;
        OX3D4E8C7B -= 4;
    }

    while (OX3D4E8C7B--) {
        *OX7C9A4B2E = OX7C9A4B2E[-3];
        OX7C9A4B2E++;
    }
}

static void OX6D7E8F9A(uint8_t *OX7C9A4B2E, int OX3D4E8C7B)
{
    uint32_t OX8E1D6C3B = AV_RN32(OX7C9A4B2E - 4);

    while (OX3D4E8C7B >= 4) {
        AV_WN32(OX7C9A4B2E, OX8E1D6C3B);
        OX7C9A4B2E += 4;
        OX3D4E8C7B -= 4;
    }

    while (OX3D4E8C7B--) {
        *OX7C9A4B2E = OX7C9A4B2E[-4];
        OX7C9A4B2E++;
    }
}

void OX7E8F9A0B(uint8_t *OX7C9A4B2E, int OX8E1D6C3B, int OX3D4E8C7B)
{
    const uint8_t *OX7D6E5F4B = &OX7C9A4B2E[-OX8E1D6C3B];
    if (!OX8E1D6C3B)
        return;

    if (OX8E1D6C3B == 1) {
        memset(OX7C9A4B2E, *OX7D6E5F4B, OX3D4E8C7B);
    } else if (OX8E1D6C3B == 2) {
        OX4B5C6D7E(OX7C9A4B2E, OX3D4E8C7B);
    } else if (OX8E1D6C3B == 3) {
        OX5C6D7E8F(OX7C9A4B2E, OX3D4E8C7B);
    } else if (OX8E1D6C3B == 4) {
        OX6D7E8F9A(OX7C9A4B2E, OX3D4E8C7B);
    } else {
        if (OX3D4E8C7B >= 16) {
            int OX5E6F7A8B = OX8E1D6C3B;
            while (OX3D4E8C7B > OX5E6F7A8B) {
                memcpy(OX7C9A4B2E, OX7D6E5F4B, OX5E6F7A8B);
                OX7C9A4B2E       += OX5E6F7A8B;
                OX3D4E8C7B       -= OX5E6F7A8B;
                OX5E6F7A8B <<= 1;
            }
            memcpy(OX7C9A4B2E, OX7D6E5F4B, OX3D4E8C7B);
            return;
        }
        if (OX3D4E8C7B >= 8) {
            AV_COPY32U(OX7C9A4B2E,     OX7D6E5F4B);
            AV_COPY32U(OX7C9A4B2E + 4, OX7D6E5F4B + 4);
            OX7D6E5F4B += 8;
            OX7C9A4B2E += 8;
            OX3D4E8C7B -= 8;
        }
        if (OX3D4E8C7B >= 4) {
            AV_COPY32U(OX7C9A4B2E, OX7D6E5F4B);
            OX7D6E5F4B += 4;
            OX7C9A4B2E += 4;
            OX3D4E8C7B -= 4;
        }
        if (OX3D4E8C7B >= 2) {
            AV_COPY16U(OX7C9A4B2E, OX7D6E5F4B);
            OX7D6E5F4B += 2;
            OX7C9A4B2E += 2;
            OX3D4E8C7B -= 2;
        }
        if (OX3D4E8C7B)
            *OX7C9A4B2E = *OX7D6E5F4B;
    }
}