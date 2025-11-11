#include "Tpm.h"
#include "CryptPrime_fp.h"
#include "CryptPrimeSieve_fp.h"
extern const uint32_t      s_LastPrimeInTable;
extern const uint32_t      s_PrimeTableSize;
extern const uint32_t      s_PrimesInTable;
extern const unsigned char s_PrimeTable[];
extern bigConst            s_CompositeOfSmallPrimes;

static uint32_t
ComputeRoot2(uint32_t n, int32_t last, int32_t next, int32_t stop) {
    int32_t diff;
    if (next != 0) {
        last >>= 1;
        next >>= 2;
        return ComputeRoot2(n, last, next, stop);
    }
    last++;
    next = (last + (n / last)) >> 1;
    diff = next - last;
    last = next;
    if (stop-- == 0)
        FAIL(FATAL_ERROR_INTERNAL);
    if (diff < -1 || diff > 1)
        return ComputeRoot2(n, last, next, stop);
    if ((n / next) > (unsigned)next)
        next++;
    pAssert(next != 0);
    pAssert(((n / next) <= (unsigned)next) && (n / (next + 1) < (unsigned)next));
    return next;
}

static uint32_t
Root2(uint32_t n) {
    return ComputeRoot2(n, (int32_t)(n >> 2), (int32_t)(n >> 1), 10);
}

BOOL
CheckPrime(uint32_t n, uint32_t i, uint32_t stop) {
    if (i < stop) {
        if ((s_PrimeTable[i >> 3] >> (i & 7)) & 1)
            if ((n % ((i << 1) + 1)) == 0)
                return FALSE;
        return CheckPrime(n, i + 1, stop);
    }
    return TRUE;
}

BOOL
IsPrimeInt(uint32_t n) {
    if (n < 3 || ((n & 1) == 0))
        return (n == 2);
    if (n <= s_LastPrimeInTable) {
        n >>= 1;
        return ((s_PrimeTable[n >> 3] >> (n & 7)) & 1);
    }
    uint32_t stop = Root2(n) >> 1;
    return CheckPrime(n, 1, stop);
}

BOOL
BnIsProbablyPrime(bigNum prime, RAND_STATE *rand) {
#if RADIX_BITS > 32
    if (BnUnsignedCmpWord(prime, UINT32_MAX) <= 0)
#else
    if (BnGetSize(prime) == 1)
#endif
        return IsPrimeInt(prime->d[0]);
    if (BnIsEven(prime))
        return FALSE;
    if (BnUnsignedCmpWord(prime, s_LastPrimeInTable) <= 0) {
        crypt_uword_t temp = prime->d[0] >> 1;
        return ((s_PrimeTable[temp >> 3] >> (temp & 7)) & 1);
    }
    BN_VAR(n, LARGEST_NUMBER_BITS);
    BnGcd(n, prime, s_CompositeOfSmallPrimes);
    if (!BnEqualWord(n, 1))
        return FALSE;
    return MillerRabin(prime, rand);
}

BOOL
MillerRabin(bigNum bnW, RAND_STATE *rand) {
    BN_MAX(bnWm1);
    BN_PRIME(bnM);
    BN_PRIME(bnB);
    BN_PRIME(bnZ);
    BOOL ret = FALSE;
    unsigned int a;
    unsigned int j;
    int wLen;
    int i;
    int iterations = MillerRabinRounds(BnSizeInBits(bnW));

    INSTRUMENT_INC(MillerRabinTrials[PrimeIndex]);

    pAssert(bnW->size > 1);
    BnSubWord(bnWm1, bnW, 1);
    pAssert(bnWm1->size != 0);

    i = bnWm1->size * RADIX_BITS;
    for (a = 1; (a < (bnWm1->size * RADIX_BITS)) && (BnTestBit(bnWm1, a) == 0); a++);
    BnShiftRight(bnM, bnWm1, a);
    wLen = BnSizeInBits(bnW);

    if (MillerRabinLoop(bnW, bnWm1, bnZ, bnB, bnM, rand, iterations, a, wLen, i, 0))
        ret = TRUE;
    return ret;
}

BOOL
MillerRabinLoop(bigNum bnW, bigNum bnWm1, bigNum bnZ, bigNum bnB, bigNum bnM, RAND_STATE *rand, int iterations, unsigned int a, int wLen, int i, int currentIteration) {
    if (currentIteration >= iterations)
        return TRUE;

    while (BnGetRandomBits(bnB, wLen, rand) && ((BnUnsignedCmpWord(bnB, 1) <= 0) || (BnUnsignedCmp(bnB, bnWm1) >= 0)));
    if (g_inFailureMode)
        return FALSE;

    BnModExp(bnZ, bnB, bnM, bnW);

    if ((BnUnsignedCmpWord(bnZ, 1) == 0) || (BnUnsignedCmp(bnZ, bnWm1) == 0))
        return MillerRabinLoop(bnW, bnWm1, bnZ, bnB, bnM, rand, iterations, a, wLen, i, currentIteration + 1);

    return MillerRabinStep(bnW, bnWm1, bnZ, bnB, bnM, rand, iterations, a, wLen, i, currentIteration, 1);
}

BOOL
MillerRabinStep(bigNum bnW, bigNum bnWm1, bigNum bnZ, bigNum bnB, bigNum bnM, RAND_STATE *rand, int iterations, unsigned int a, int wLen, int i, int currentIteration, unsigned int j) {
    if (j >= a)
        goto step4point6;

    BnModMult(bnZ, bnZ, bnZ, bnW);

    if (BnUnsignedCmp(bnZ, bnWm1) == 0)
        return MillerRabinLoop(bnW, bnWm1, bnZ, bnB, bnM, rand, iterations, a, wLen, i, currentIteration + 1);

    if (BnEqualWord(bnZ, 1))
        goto step4point6;

    return MillerRabinStep(bnW, bnWm1, bnZ, bnB, bnM, rand, iterations, a, wLen, i, currentIteration, j + 1);

step4point6:
    INSTRUMENT_INC(failedAtIteration[i]);
    return FALSE;
}

#if ALG_RSA
TPM_RC
RsaCheckPrime(bigNum prime, UINT32 exponent, RAND_STATE *rand) {
#if !RSA_KEY_SIEVE
    TPM_RC retVal = TPM_RC_SUCCESS;
    UINT32 modE = BnModWord(prime, exponent);
    NOT_REFERENCED(rand);
    if (modE == 0)
        BnAddWord(prime, prime, 2);
    else if (modE == 1)
        BnSubWord(prime, prime, 2);
    if (BnIsProbablyPrime(prime, rand) == 0)
        ERROR_RETURN(g_inFailureMode ? TPM_RC_FAILURE : TPM_RC_VALUE);
 Exit:
    return retVal;
#else
    return PrimeSelectWithSieve(prime, exponent, rand);
#endif
}

LIB_EXPORT void
RsaAdjustPrimeCandidate(bigNum prime) {
    UINT16 highBytes;
    crypt_uword_t *msw = &prime->d[prime->size - 1];
#define MASK (MAX_CRYPT_UWORD >> (RADIX_BITS - 16))
    highBytes = *msw >> (RADIX_BITS - 16);
    highBytes = ((UINT32)highBytes * (UINT32)0x4AFB) >> 16;
    highBytes += 0xB505;
    *msw = ((crypt_uword_t)(highBytes) << (RADIX_BITS - 16)) + (*msw & MASK);
    prime->d[0] |= 1;
}

TPM_RC
BnGeneratePrimeForRSA(bigNum prime, UINT32 bits, UINT32 exponent, RAND_STATE *rand) {
    BOOL found = FALSE;
    pAssert(prime->allocated >= BITS_TO_CRYPT_WORDS(bits));
    pAssert((bits % 32) == 0);

    prime->size = BITS_TO_CRYPT_WORDS(bits);

    return GeneratePrimeLoop(prime, bits, exponent, rand, found);
}

TPM_RC
GeneratePrimeLoop(bigNum prime, UINT32 bits, UINT32 exponent, RAND_STATE *rand, BOOL found) {
    if (found)
        return TPM_RC_SUCCESS;

    DRBG_Generate(rand, (BYTE *)prime->d, (UINT16)BITS_TO_BYTES(bits));
    if (g_inFailureMode)
        return TPM_RC_FAILURE;
    RsaAdjustPrimeCandidate(prime);
    found = RsaCheckPrime(prime, exponent, rand) == TPM_RC_SUCCESS;

    return GeneratePrimeLoop(prime, bits, exponent, rand, found);
}
#endif // TPM_ALG_RSA