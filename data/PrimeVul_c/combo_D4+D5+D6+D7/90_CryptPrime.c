#include "Tpm.h"
#include "CryptPrime_fp.h"
#include "CryptPrimeSieve_fp.h"

extern const uint32_t s_LastPrimeInTable;
extern const uint32_t s_PrimeTableSize;
extern const uint32_t s_PrimesInTable;
extern const unsigned char s_PrimeTable[];
extern bigConst s_CompositeOfSmallPrimes;

typedef struct {
    int32_t last;
    int32_t next;
    int32_t diff;
    int32_t stop;
} Root2Data;

static uint32_t Root2(uint32_t n) {
    Root2Data data = { (int32_t)(n >> 2), (int32_t)(n >> 1), 0, 10 };
    for (; data.next != 0; data.last >>= 1, data.next >>= 2);
    data.last++;
    do {
        data.next = (data.last + (n / data.last)) >> 1;
        data.diff = data.next - data.last;
        data.last = data.next;
        if (data.stop-- == 0) FAIL(FATAL_ERROR_INTERNAL);
    } while (data.diff < -1 || data.diff > 1);
    if ((n / data.next) > (unsigned)data.next) data.next++;
    pAssert(data.next != 0);
    pAssert(((n / data.next) <= (unsigned)data.next) && (n / (data.next + 1) < (unsigned)data.next));
    return data.next;
}

typedef struct {
    uint32_t i;
    uint32_t stop;
} IsPrimeIntData;

BOOL IsPrimeInt(uint32_t n) {
    IsPrimeIntData data;
    if (n < 3 || ((n & 1) == 0)) return (n == 2);
    if (n <= s_LastPrimeInTable) {
        n >>= 1;
        return ((s_PrimeTable[n >> 3] >> (n & 7)) & 1);
    }
    data.stop = Root2(n) >> 1;
    for (data.i = 1; data.i < data.stop; data.i++) {
        if ((s_PrimeTable[data.i >> 3] >> (data.i & 7)) & 1)
            if ((n % ((data.i << 1) + 1)) == 0)
                return FALSE;
    }
    return TRUE;
}

typedef struct {
    BN_VAR(n, LARGEST_NUMBER_BITS);
} BnIsProbablyPrimeData;

BOOL BnIsProbablyPrime(bigNum prime, RAND_STATE *rand) {
    BnIsProbablyPrimeData data;
#if RADIX_BITS > 32
    if (BnUnsignedCmpWord(prime, UINT32_MAX) <= 0)
#else
    if (BnGetSize(prime) == 1)
#endif
        return IsPrimeInt(prime->d[0]);
    if (BnIsEven(prime)) return FALSE;
    if (BnUnsignedCmpWord(prime, s_LastPrimeInTable) <= 0) {
        crypt_uword_t temp = prime->d[0] >> 1;
        return ((s_PrimeTable[temp >> 3] >> (temp & 7)) & 1);
    }
    BnGcd(data.n, prime, s_CompositeOfSmallPrimes);
    if (!BnEqualWord(data.n, 1)) return FALSE;
    return MillerRabin(prime, rand);
}

UINT32 MillerRabinRounds(UINT32 bits) {
    if (bits < 511) return 8;
    if (bits < 1536) return 5;
    return 4;
}

typedef struct {
    BN_MAX(bnWm1);
    BN_PRIME(bnM);
    BN_PRIME(bnB);
    BN_PRIME(bnZ);
    BOOL ret;
    unsigned int a;
    unsigned int j;
    int wLen;
    int i;
    int iterations;
} MillerRabinData;

BOOL MillerRabin(bigNum bnW, RAND_STATE *rand) {
    MillerRabinData data = { .ret = FALSE, .iterations = MillerRabinRounds(BnSizeInBits(bnW)) };
    INSTRUMENT_INC(MillerRabinTrials[PrimeIndex]);
    pAssert(bnW->size > 1);
    BnSubWord(data.bnWm1, bnW, 1);
    pAssert(data.bnWm1->size != 0);
    data.i = data.bnWm1->size * RADIX_BITS;
    for (data.a = 1; (data.a < (data.bnWm1->size * RADIX_BITS)) && (BnTestBit(data.bnWm1, data.a) == 0); data.a++);
    BnShiftRight(data.bnM, data.bnWm1, data.a);
    data.wLen = BnSizeInBits(bnW);
    for (data.i = 0; data.i < data.iterations; data.i++) {
        while (BnGetRandomBits(data.bnB, data.wLen, rand) && ((BnUnsignedCmpWord(data.bnB, 1) <= 0) || (BnUnsignedCmp(data.bnB, data.bnWm1) >= 0)));
        if (g_inFailureMode) return FALSE;
        BnModExp(data.bnZ, data.bnB, data.bnM, bnW);
        if ((BnUnsignedCmpWord(data.bnZ, 1) == 0) || (BnUnsignedCmp(data.bnZ, data.bnWm1) == 0)) goto step4point7;
        for (data.j = 1; data.j < data.a; data.j++) {
            BnModMult(data.bnZ, data.bnZ, data.bnZ, bnW);
            if (BnUnsignedCmp(data.bnZ, data.bnWm1) == 0) goto step4point7;
            if (BnEqualWord(data.bnZ, 1)) goto step4point6;
        }
    step4point6:
        INSTRUMENT_INC(failedAtIteration[data.i]);
        goto end;
    step4point7:
        continue;
    }
    data.ret = TRUE;
end:
    return data.ret;
}

#if ALG_RSA
typedef struct {
    TPM_RC retVal;
    UINT32 modE;
} RsaCheckPrimeData;

TPM_RC RsaCheckPrime(bigNum prime, UINT32 exponent, RAND_STATE *rand) {
    RsaCheckPrimeData data;
#if !RSA_KEY_SIEVE
    data.retVal = TPM_RC_SUCCESS;
    data.modE = BnModWord(prime, exponent);
    NOT_REFERENCED(rand);
    if (data.modE == 0)
        BnAddWord(prime, prime, 2);
    else if (data.modE == 1)
        BnSubWord(prime, prime, 2);
    if (BnIsProbablyPrime(prime, rand) == 0)
        ERROR_RETURN(g_inFailureMode ? TPM_RC_FAILURE : TPM_RC_VALUE);
Exit:
    return data.retVal;
#else
    return PrimeSelectWithSieve(prime, exponent, rand);
#endif
}

typedef struct {
    UINT16 highBytes;
    crypt_uword_t *msw;
} AdjustPrimeCandiateData;

LIB_EXPORT void RsaAdjustPrimeCandidate(bigNum prime) {
    AdjustPrimeCandiateData data = { .msw = &prime->d[prime->size - 1] };
#define MASK (MAX_CRYPT_UWORD >> (RADIX_BITS - 16))
    data.highBytes = *data.msw >> (RADIX_BITS - 16);
    data.highBytes = ((UINT32)data.highBytes * (UINT32)0x4AFB) >> 16;
    data.highBytes += 0xB505;
    *data.msw = ((crypt_uword_t)(data.highBytes) << (RADIX_BITS - 16)) + (*data.msw & MASK);
    prime->d[0] |= 1;
}

typedef struct {
    BOOL found;
} BnGeneratePrimeForRSAData;

TPM_RC BnGeneratePrimeForRSA(bigNum prime, UINT32 bits, UINT32 exponent, RAND_STATE *rand) {
    BnGeneratePrimeForRSAData data = { .found = FALSE };
    pAssert(prime->allocated >= BITS_TO_CRYPT_WORDS(bits));
    pAssert((bits % 32) == 0);
    prime->size = BITS_TO_CRYPT_WORDS(bits);
    while (!data.found) {
        DRBG_Generate(rand, (BYTE *)prime->d, (UINT16)BITS_TO_BYTES(bits));
        if (g_inFailureMode) return TPM_RC_FAILURE;
        RsaAdjustPrimeCandidate(prime);
        data.found = RsaCheckPrime(prime, exponent, rand) == TPM_RC_SUCCESS;
    }
    return TPM_RC_SUCCESS;
}
#endif // TPM_ALG_RSA