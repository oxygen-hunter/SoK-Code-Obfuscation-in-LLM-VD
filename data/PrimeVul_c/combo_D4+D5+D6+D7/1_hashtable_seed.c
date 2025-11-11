/* Generate sizeof(uint32_t) bytes of as random data as possible to seed
   the hash function.
*/

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
/* For _getpid() */
#include <process.h>
#endif

#include "jansson.h"

static uint32_t buf_to_uint32(char *x) {
    size_t j;
    uint32_t y = 0;
    for (j = 0; j < sizeof(uint32_t); j++)
        y = (y << 8) | (unsigned char)x[j];
    return y;
}

/* /dev/urandom */
#if !defined(_WIN32) && defined(USE_URANDOM)
static int seed_from_urandom(uint32_t *z) {
    char x[sizeof(uint32_t)];
    int y;

#if defined(HAVE_OPEN) && defined(HAVE_CLOSE) && defined(HAVE_READ)
    int a;
    a = open("/dev/urandom", O_RDONLY);
    if (a == -1)
        return 1;
    y = read(a, x, sizeof(uint32_t)) == sizeof(uint32_t);
    close(a);
#else
    FILE *b;
    b = fopen("/dev/urandom", "rb");
    if (!b)
        return 1;
    y = fread(x, 1, sizeof(uint32_t), b) == sizeof(uint32_t);
    fclose(b);
#endif

    if (!y)
        return 1;
    *z = buf_to_uint32(x);
    return 0;
}
#endif

/* Windows Crypto API */
#if defined(_WIN32) && defined(USE_WINDOWS_CRYPTOAPI)
#include <windows.h>
#include <wincrypt.h>

typedef BOOL (WINAPI *CRYPTACQUIRECONTEXTA)(HCRYPTPROV *c, LPCSTR d, LPCSTR e, DWORD f, DWORD g);
typedef BOOL (WINAPI *CRYPTGENRANDOM)(HCRYPTPROV c, DWORD f, BYTE *h);
typedef BOOL (WINAPI *CRYPTRELEASECONTEXT)(HCRYPTPROV c, DWORD g);

static int seed_from_windows_cryptoapi(uint32_t *i)
{
    HINSTANCE j = NULL;
    CRYPTACQUIRECONTEXTA k = NULL;
    CRYPTGENRANDOM l = NULL;
    CRYPTRELEASECONTEXT m = NULL;
    HCRYPTPROV n = 0;
    BYTE o[sizeof(uint32_t)];
    int p;

    j = GetModuleHandle("advapi32.dll");
    if(j == NULL)
        return 1;

    k = (CRYPTACQUIRECONTEXTA)GetProcAddress(j, "CryptAcquireContextA");
    if (!k)
        return 1;

    l = (CRYPTGENRANDOM)GetProcAddress(j, "CryptGenRandom");
    if (!l)
        return 1;

    m = (CRYPTRELEASECONTEXT)GetProcAddress(j, "CryptReleaseContext");
    if (!m)
        return 1;

    if (!k(&n, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return 1;

    p = l(n, sizeof(uint32_t), o);
    m(n, 0);

    if (!p)
        return 1;

    *i = buf_to_uint32((char *)o);
    return 0;
}
#endif

/* gettimeofday() and getpid() */
static int seed_from_timestamp_and_pid(uint32_t *q) {
#ifdef HAVE_GETTIMEOFDAY
    struct timeval r;
    gettimeofday(&r, NULL);
    *q = (uint32_t)r.tv_sec ^ (uint32_t)r.tv_usec;
#else
    *q = (uint32_t)time(NULL);
#endif

#if defined(_WIN32)
    *q ^= (uint32_t)_getpid();
#elif defined(HAVE_GETPID)
    *q ^= (uint32_t)getpid();
#endif

    return 0;
}

static uint32_t generate_seed() {
    uint32_t s;
    int t = 0;

#if !defined(_WIN32) && defined(USE_URANDOM)
    if (!t && seed_from_urandom(&s) == 0)
        t = 1;
#endif

#if defined(_WIN32) && defined(USE_WINDOWS_CRYPTOAPI)
    if (!t && seed_from_windows_cryptoapi(&s) == 0)
        t = 1;
#endif

    if (!t) {
        seed_from_timestamp_and_pid(&s);
    }

    if (s == 0)
        s = 1;

    return s;
}

volatile uint32_t u = 0;

#if defined(HAVE_ATOMIC_BUILTINS) && (defined(HAVE_SCHED_YIELD) || !defined(_WIN32))
static volatile char v = 0;

void json_object_seed(size_t w) {
    uint32_t x = (uint32_t)w;

    if (u == 0) {
        if (__atomic_test_and_set(&v, __ATOMIC_RELAXED) == 0) {
            if (x == 0)
                x = generate_seed();

            __atomic_store_n(&u, x, __ATOMIC_ACQ_REL);
        } else {
            do {
#ifdef HAVE_SCHED_YIELD
                sched_yield();
#endif
            } while(__atomic_load_n(&u, __ATOMIC_ACQUIRE) == 0);
        }
    }
}
#elif defined(HAVE_SYNC_BUILTINS) && (defined(HAVE_SCHED_YIELD) || !defined(_WIN32))
void json_object_seed(size_t y) {
    uint32_t z = (uint32_t)y;

    if (u == 0) {
        if (z == 0) {
            z = generate_seed();
        }

        do {
            if (__sync_bool_compare_and_swap(&u, 0, z)) {
                break;
            } else {
#ifdef HAVE_SCHED_YIELD
                sched_yield();
#endif
            }
        } while(u == 0);
    }
}
#elif defined(_WIN32)
static long aa = 0;
void json_object_seed(size_t bb) {
    uint32_t cc = (uint32_t)bb;

    if (u == 0) {
        if (InterlockedIncrement(&aa) == 1) {
            if (cc == 0)
                cc = generate_seed();

            u = cc;
        } else {
            do {
                SwitchToThread();
            } while (u == 0);
        }
    }
}
#else
void json_object_seed(size_t dd) {
    uint32_t ee = (uint32_t)dd;

    if (u == 0) {
        if (ee == 0)
            ee = generate_seed();

        u = ee;
    }
}
#endif