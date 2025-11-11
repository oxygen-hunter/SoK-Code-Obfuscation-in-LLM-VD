#include "Tpm.h"
#include "CryptPrime_fp.h"
#include "CryptPrimeSieve_fp.h"

extern const uint32_t      s_LastPrimeInTable;
extern const uint32_t      s_PrimeTableSize;
extern const uint32_t      s_PrimesInTable;
extern const unsigned char s_PrimeTable[];
extern bigConst            s_CompositeOfSmallPrimes;

static uint32_t inline __asm__ (
    "Root2: \n"
    "mov %1, %%eax \n"
    "shr $2, %%eax \n"
    "mov %%eax, %0 \n"
    "mov %1, %%eax \n"
    "shr $1, %%eax \n"
    "mov %%eax, %1 \n"
    "mov $10, %%ecx \n"
    "1: \n"
    "mov %0, %%eax \n"
    "mov %1, %%ebx \n"
    "xor %%edx, %%edx \n"
    "div %%ebx \n"
    "add %%eax, %0 \n"
    "shr $1, %0 \n"
    "sub %0, %1 \n"
    "mov %1, %%eax \n"
    "mov %%eax, %1 \n"
    "dec %%ecx \n"
    "jz 2f \n"
    "cmp $1, %1 \n"
    "jg 1b \n"
    "jl 1b \n"
    "2: \n"
    "cmp %0, %%eax \n"
    "jbe 3f \n"
    "inc %0 \n"
    "3: \n"
    : "=r" (n)
    : "r" (n)
    : "%eax", "%ebx", "%ecx", "%edx"
);

BOOL IsPrimeInt(uint32_t n) {
    uint32_t i;
    uint32_t stop;
    if(n < 3 || ((n & 1) == 0))
        return (n == 2);
    if(n <= s_LastPrimeInTable) {
        n >>= 1;
        return ((s_PrimeTable[n >> 3] >> (n & 7)) & 1);
    }
    stop = Root2(n) >> 1;
    for(i = 1; i < stop; i++) {
        if((s_PrimeTable[i >> 3] >> (i & 7)) & 1)
            if((n % ((i << 1) + 1)) == 0)
                return FALSE;
    }
    return TRUE;
}

BOOL BnIsProbablyPrime(bigNum prime, RAND_STATE *rand) {
#if RADIX_BITS > 32
    if(BnUnsignedCmpWord(prime, UINT32_MAX) <= 0)
#else
    if(BnGetSize(prime) == 1)
#endif
        return IsPrimeInt(prime->d[0]);
    if(BnIsEven(prime))
        return FALSE;
    if(BnUnsignedCmpWord(prime, s_LastPrimeInTable) <= 0) {
        crypt_uword_t temp = prime->d[0] >> 1;
        return ((s_PrimeTable[temp >> 3] >> (temp & 7)) & 1);
    }
    BN_VAR(n, LARGEST_NUMBER_BITS);
    BnGcd(n, prime, s_CompositeOfSmallPrimes);
    if(!BnEqualWord(n, 1))
        return FALSE;
    return MillerRabin(prime, rand);
}

UINT32 MillerRabinRounds(UINT32 bits) {
    if(bits < 511) return 8;
    if(bits < 1536) return 5;
    return 4;
}

BOOL MillerRabin(bigNum bnW, RAND_STATE *rand) {
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
    for(a = 1; (a < (bnWm1->size * RADIX_BITS)) && (BnTestBit(bnWm1, a) == 0); a++);
    BnShiftRight(bnM, bnWm1, a);
    wLen = BnSizeInBits(bnW);
    for(i = 0; i < iterations; i++) {
        while(BnGetRandomBits(bnB, wLen, rand) && ((BnUnsignedCmpWord(bnB, 1) <= 0)
                                                   || (BnUnsignedCmp(bnB, bnWm1) >= 0)));
        if(g_inFailureMode)
            return FALSE;
        
        BnModExp(bnZ, bnB, bnM, bnW);
        
        if((BnUnsignedCmpWord(bnZ, 1) == 0) || (BnUnsignedCmp(bnZ, bnWm1) == 0))
            goto step4point7;
        for(j = 1; j < a; j++) {
            BnModMult(bnZ, bnZ, bnZ, bnW);
            if(BnUnsignedCmp(bnZ, bnWm1) == 0)
                goto step4point7;
            if(BnEqualWord(bnZ, 1))
                goto step4point6;
        }
    step4point6:
        INSTRUMENT_INC(failedAtIteration[i]);
        goto end;
    step4point7:
        continue;
    }
    ret = TRUE;
 end:
    return ret;
}

#if ALG_RSA
TPM_RC RsaCheckPrime(bigNum prime, UINT32 exponent, RAND_STATE *rand) {
#if !RSA_KEY_SIEVE
    TPM_RC retVal = TPM_RC_SUCCESS;
    UINT32 modE = BnModWord(prime, exponent);
    NOT_REFERENCED(rand);
    if(modE == 0)
        BnAddWord(prime, prime, 2);
    else if(modE == 1)
        BnSubWord(prime, prime, 2);
    if(BnIsProbablyPrime(prime, rand) == 0)
        ERROR_RETURN(g_inFailureMode ? TPM_RC_FAILURE : TPM_RC_VALUE);
 Exit:
    return retVal;
#else
    return PrimeSelectWithSieve(prime, exponent, rand);
#endif
}

LIB_EXPORT void RsaAdjustPrimeCandidate(bigNum prime) {
    UINT16 highBytes;
    crypt_uword_t *msw = &prime->d[prime->size - 1];
#define MASK (MAX_CRYPT_UWORD >> (RADIX_BITS - 16))
    highBytes = *msw >> (RADIX_BITS - 16);
    highBytes = ((UINT32)highBytes * (UINT32)0x4AFB) >> 16;
    highBytes += 0xB505;
    *msw = ((crypt_uword_t)(highBytes) << (RADIX_BITS - 16)) + (*msw & MASK);
    prime->d[0] |= 1;
}

TPM_RC BnGeneratePrimeForRSA(bigNum prime, UINT32 bits, UINT32 exponent, RAND_STATE *rand) {
    BOOL found = FALSE;
    pAssert(prime->allocated >= BITS_TO_CRYPT_WORDS(bits));
    pAssert((bits % 32) == 0);
    
    prime->size = BITS_TO_CRYPT_WORDS(bits);
    
    while(!found) {
        DRBG_Generate(rand, (BYTE *)prime->d, (UINT16)BITS_TO_BYTES(bits));
        if(g_inFailureMode)
            return TPM_RC_FAILURE;
        RsaAdjustPrimeCandidate(prime);
        found = RsaCheckPrime(prime, exponent, rand) == TPM_RC_SUCCESS;
    }
    return TPM_RC_SUCCESS;
}
#endif