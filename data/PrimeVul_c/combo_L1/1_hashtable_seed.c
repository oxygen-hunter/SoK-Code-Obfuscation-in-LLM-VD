#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <time.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if defined(_WIN32)
#include <process.h>
#endif

#include "jansson.h"

static uint32_t OX7B4DF339(char *OX3C4D8F12) {
    size_t OX1A2B3C4D;
    uint32_t OX5E6F7A8B = 0;

    for (OX1A2B3C4D = 0; OX1A2B3C4D < sizeof(uint32_t); OX1A2B3C4D++)
        OX5E6F7A8B = (OX5E6F7A8B << 8) | (unsigned char)OX3C4D8F12[OX1A2B3C4D];

    return OX5E6F7A8B;
}

#if !defined(_WIN32) && defined(USE_URANDOM)
static int OX9C8D7E6F(uint32_t *OX5E6F7A8B) {
    char OX3C4D8F12[sizeof(uint32_t)];
    int OX8B7A6E5D;

#if defined(HAVE_OPEN) && defined(HAVE_CLOSE) && defined(HAVE_READ)
    int OX1B2A3C4D;
    OX1B2A3C4D = open("/dev/urandom", O_RDONLY);
    if (OX1B2A3C4D == -1)
        return 1;

    OX8B7A6E5D = read(OX1B2A3C4D, OX3C4D8F12, sizeof(uint32_t)) == sizeof(uint32_t);
    close(OX1B2A3C4D);
#else
    FILE *OX1B2A3C4D;

    OX1B2A3C4D = fopen("/dev/urandom", "rb");
    if (!OX1B2A3C4D)
        return 1;

    OX8B7A6E5D = fread(OX3C4D8F12, 1, sizeof(uint32_t), OX1B2A3C4D) == sizeof(uint32_t);
    fclose(OX1B2A3C4D);
#endif

    if (!OX8B7A6E5D)
        return 1;

    *OX5E6F7A8B = OX7B4DF339(OX3C4D8F12);
    return 0;
}
#endif

#if defined(_WIN32) && defined(USE_WINDOWS_CRYPTOAPI)
#include <windows.h>
#include <wincrypt.h>

typedef BOOL (WINAPI *OX4D3C2B1A)(HCRYPTPROV *OX5A6B7C8D, LPCSTR OX9A8B7C6D, LPCSTR OX4C3D2E1F, DWORD OX1E2D3C4B, DWORD OX5B6C7D8E);
typedef BOOL (WINAPI *OX2B3C4D5E)(HCRYPTPROV OX5A6B7C8D, DWORD OX4E3D2C1B, BYTE *OX3C4D8F12);
typedef BOOL (WINAPI *OX6D7C8B9A)(HCRYPTPROV OX5A6B7C8D, DWORD OX8E7D6C5B);

static int OX8B7A6E5D(uint32_t *OX5E6F7A8B) {
    HINSTANCE OX9B8C7D6E = NULL;
    OX4D3C2B1A OX3A2B1C4D = NULL;
    OX2B3C4D5E OX6E5F4A2B = NULL;
    OX6D7C8B9A OX1C2B3D4F = NULL;
    HCRYPTPROV OX5A6B7C8D = 0;
    BYTE OX3C4D8F12[sizeof(uint32_t)];
    int OX7E6D5C4B;

    OX9B8C7D6E = GetModuleHandle("advapi32.dll");
    if(OX9B8C7D6E == NULL)
        return 1;

    OX3A2B1C4D = (OX4D3C2B1A)GetProcAddress(OX9B8C7D6E, "CryptAcquireContextA");
    if (!OX3A2B1C4D)
        return 1;

    OX6E5F4A2B = (OX2B3C4D5E)GetProcAddress(OX9B8C7D6E, "CryptGenRandom");
    if (!OX6E5F4A2B)
        return 1;

    OX1C2B3D4F = (OX6D7C8B9A)GetProcAddress(OX9B8C7D6E, "CryptReleaseContext");
    if (!OX1C2B3D4F)
        return 1;

    if (!OX3A2B1C4D(&OX5A6B7C8D, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return 1;

    OX7E6D5C4B = OX6E5F4A2B(OX5A6B7C8D, sizeof(uint32_t), OX3C4D8F12);
    OX1C2B3D4F(OX5A6B7C8D, 0);

    if (!OX7E6D5C4B)
        return 1;

    *OX5E6F7A8B = OX7B4DF339((char *)OX3C4D8F12);
    return 0;
}
#endif

static int OX2A3B4C5D(uint32_t *OX5E6F7A8B) {
#ifdef HAVE_GETTIMEOFDAY
    struct timeval OX6C7D8E9F;
    gettimeofday(&OX6C7D8E9F, NULL);
    *OX5E6F7A8B = (uint32_t)OX6C7D8E9F.tv_sec ^ (uint32_t)OX6C7D8E9F.tv_usec;
#else
    *OX5E6F7A8B = (uint32_t)time(NULL);
#endif

#if defined(_WIN32)
    *OX5E6F7A8B ^= (uint32_t)_getpid();
#elif defined(HAVE_GETPID)
    *OX5E6F7A8B ^= (uint32_t)getpid();
#endif

    return 0;
}

static uint32_t OX0F1E2D3C() {
    uint32_t OX5E6F7A8B;
    int OX8D7C6B5A = 0;

#if !defined(_WIN32) && defined(USE_URANDOM)
    if (!OX8D7C6B5A && OX9C8D7E6F(&OX5E6F7A8B) == 0)
        OX8D7C6B5A = 1;
#endif

#if defined(_WIN32) && defined(USE_WINDOWS_CRYPTOAPI)
    if (!OX8D7C6B5A && OX8B7A6E5D(&OX5E6F7A8B) == 0)
        OX8D7C6B5A = 1;
#endif

    if (!OX8D7C6B5A) {
        OX2A3B4C5D(&OX5E6F7A8B);
    }

    if (OX5E6F7A8B == 0)
        OX5E6F7A8B = 1;

    return OX5E6F7A8B;
}

volatile uint32_t OX3B2C1D0E = 0;

#if defined(HAVE_ATOMIC_BUILTINS) && (defined(HAVE_SCHED_YIELD) || !defined(_WIN32))
static volatile char OX7F8E9D6C = 0;

void OX9A8B7C6D(size_t OX4E3D2C1B) {
    uint32_t OX5C4B3A2D = (uint32_t)OX4E3D2C1B;

    if (OX3B2C1D0E == 0) {
        if (__atomic_test_and_set(&OX7F8E9D6C, __ATOMIC_RELAXED) == 0) {
            if (OX5C4B3A2D == 0)
                OX5C4B3A2D = OX0F1E2D3C();

            __atomic_store_n(&OX3B2C1D0E, OX5C4B3A2D, __ATOMIC_ACQ_REL);
        } else {
            do {
#ifdef HAVE_SCHED_YIELD
                sched_yield();
#endif
            } while(__atomic_load_n(&OX3B2C1D0E, __ATOMIC_ACQUIRE) == 0);
        }
    }
}
#elif defined(HAVE_SYNC_BUILTINS) && (defined(HAVE_SCHED_YIELD) || !defined(_WIN32))
void OX9A8B7C6D(size_t OX4E3D2C1B) {
    uint32_t OX5C4B3A2D = (uint32_t)OX4E3D2C1B;

    if (OX3B2C1D0E == 0) {
        if (OX5C4B3A2D == 0) {
            OX5C4B3A2D = OX0F1E2D3C();
        }

        do {
            if (__sync_bool_compare_and_swap(&OX3B2C1D0E, 0, OX5C4B3A2D)) {
                break;
            } else {
#ifdef HAVE_SCHED_YIELD
                sched_yield();
#endif
            }
        } while(OX3B2C1D0E == 0);
    }
}
#elif defined(_WIN32)
static long OX7F8E9D6C = 0;
void OX9A8B7C6D(size_t OX4E3D2C1B) {
    uint32_t OX5C4B3A2D = (uint32_t)OX4E3D2C1B;

    if (OX3B2C1D0E == 0) {
        if (InterlockedIncrement(&OX7F8E9D6C) == 1) {
            if (OX5C4B3A2D == 0)
                OX5C4B3A2D = OX0F1E2D3C();

            OX3B2C1D0E = OX5C4B3A2D;
        } else {
            do {
                SwitchToThread();
            } while (OX3B2C1D0E == 0);
        }
    }
}
#else
void OX9A8B7C6D(size_t OX4E3D2C1B) {
    uint32_t OX5C4B3A2D = (uint32_t)OX4E3D2C1B;

    if (OX3B2C1D0E == 0) {
        if (OX5C4B3A2D == 0)
            OX5C4B3A2D = OX0F1E2D3C();

        OX3B2C1D0E = OX5C4B3A2D;
    }
}
#endif