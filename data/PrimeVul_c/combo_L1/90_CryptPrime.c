#include "Tpm.h"
#include "CryptPrime_fp.h"
#include "CryptPrimeSieve_fp.h"
extern const uint32_t      OX9D8F8A1E;
extern const uint32_t      OX6A2B3C4D;
extern const uint32_t      OX1F2E3D4C;
extern const unsigned char OX5C4B3A2D[];
extern bigConst            OX8E7F6D5C;
static uint32_t
OX3B2A1C9F(
      uint32_t             OX6F5E4D3B
      )
{
    int32_t              OX1A2B3C4D = (int32_t)(OX6F5E4D3B >> 2);
    int32_t              OX5D6E7F8A = (int32_t)(OX6F5E4D3B >> 1);
    int32_t              OX9C8B7A6D;
    int32_t              OX4E3F2A1B = 10;
    for(; OX5D6E7F8A != 0; OX1A2B3C4D >>= 1, OX5D6E7F8A >>= 2);
    OX1A2B3C4D++;
    do
	{
	    OX5D6E7F8A = (OX1A2B3C4D + (OX6F5E4D3B / OX1A2B3C4D)) >> 1;
	    OX9C8B7A6D = OX5D6E7F8A - OX1A2B3C4D;
	    OX1A2B3C4D = OX5D6E7F8A;
	    if(OX4E3F2A1B-- == 0)
		FAIL(FATAL_ERROR_INTERNAL);
	} while(OX9C8B7A6D < -1 || OX9C8B7A6D > 1);
    if((OX6F5E4D3B / OX5D6E7F8A) > (unsigned)OX5D6E7F8A)
	OX5D6E7F8A++;
    pAssert(OX5D6E7F8A != 0);
    pAssert(((OX6F5E4D3B / OX5D6E7F8A) <= (unsigned)OX5D6E7F8A) && (OX6F5E4D3B / (OX5D6E7F8A + 1) < (unsigned)OX5D6E7F8A));
    return OX5D6E7F8A;
}
BOOL
OX8D7E6F5A(
	   uint32_t            OX4C3B2A1D
	   )
{
    uint32_t            OX5E6F7A9B;
    uint32_t            OX1F2E3D4C;
    if(OX4C3B2A1D < 3 || ((OX4C3B2A1D & 1) == 0))
	return (OX4C3B2A1D == 2);
    if(OX4C3B2A1D <= OX9D8F8A1E)
	{
	    OX4C3B2A1D >>= 1;
	    return ((OX5C4B3A2D[OX4C3B2A1D >> 3] >> (OX4C3B2A1D & 7)) & 1);
	}
    OX1F2E3D4C = OX3B2A1C9F(OX4C3B2A1D) >> 1;
    for(OX5E6F7A9B = 1; OX5E6F7A9B < OX1F2E3D4C; OX5E6F7A9B++)
	{
	    if((OX5C4B3A2D[OX5E6F7A9B >> 3] >> (OX5E6F7A9B & 7)) & 1)
		if((OX4C3B2A1D % ((OX5E6F7A9B << 1) + 1)) == 0)
		    return FALSE;
	}
    return TRUE;
}
BOOL
OX6D5C4B3A(
		  bigNum          OX9C8B7A6D,           
		  RAND_STATE      *OX7E6F5A4B            
		  )
{
#if RADIX_BITS > 32
    if(BnUnsignedCmpWord(OX9C8B7A6D, UINT32_MAX) <= 0)
#else
	if(BnGetSize(OX9C8B7A6D) == 1)
#endif
	    return OX8D7E6F5A(OX9C8B7A6D->d[0]);
    if(BnIsEven(OX9C8B7A6D))
	return FALSE;
    if(BnUnsignedCmpWord(OX9C8B7A6D, OX9D8F8A1E) <= 0)
	{
	    crypt_uword_t       OX4C3B2A1D = OX9C8B7A6D->d[0] >> 1;
	    return ((OX5C4B3A2D[OX4C3B2A1D >> 3] >> (OX4C3B2A1D & 7)) & 1);
	}
    {
	BN_VAR(OX1F2E3D4C, LARGEST_NUMBER_BITS);
	BnGcd(OX1F2E3D4C, OX9C8B7A6D, OX8E7F6D5C);
	if(!BnEqualWord(OX1F2E3D4C, 1))
	    return FALSE;
    }
    return OX7B6A5C4D(OX9C8B7A6D, OX7E6F5A4B);
}
UINT32
OX7B6A5C4D(
	    bigNum           OX5D4C3B2A,
	    RAND_STATE      *OX3A2B1C9D
	    )
{
    BN_MAX(OX5D4C3B2A);
    BN_PRIME(OX4B3A2C1D);
    BN_PRIME(OX2A1B3C4D);
    BN_PRIME(OX8E7F6D5C);
    BOOL             OX9D8F7E6A = FALSE;   
    unsigned int     OX3C2B1A9D;
    unsigned int     OX5A4B3C2D;
    int              OX7E6F5D4C;
    int              OX1F2E3D4C;
    int              OX4C3B2A1D = OX5D4C3B2A(OX5D4C3B2A);
    INSTRUMENT_INC(MillerRabinTrials[PrimeIndex]);
    pAssert(OX5D4C3B2A->size > 1);
    BnSubWord(OX5D4C3B2A, OX5D4C3B2A, 1);
    pAssert(OX5D4C3B2A->size != 0);
    OX1F2E3D4C = OX5D4C3B2A->size * RADIX_BITS;
    for(OX3C2B1A9D = 1;
	(OX3C2B1A9D < (OX5D4C3B2A->size * RADIX_BITS)) &&
	    (BnTestBit(OX5D4C3B2A, OX3C2B1A9D) == 0);
	OX3C2B1A9D++);
    BnShiftRight(OX4B3A2C1D, OX5D4C3B2A, OX3C2B1A9D);
    OX7E6F5D4C = BnSizeInBits(OX5D4C3B2A);
    for(OX1F2E3D4C = 0; OX1F2E3D4C < OX4C3B2A1D; OX1F2E3D4C++)
	{
	    while(BnGetRandomBits(OX2A1B3C4D, OX7E6F5D4C, OX3A2B1C9D) && ((BnUnsignedCmpWord(OX2A1B3C4D, 1) <= 0)
						       || (BnUnsignedCmp(OX2A1B3C4D, OX5D4C3B2A) >= 0)));
	    if(g_inFailureMode)
		return FALSE;
	    BnModExp(OX8E7F6D5C, OX2A1B3C4D, OX4B3A2C1D, OX5D4C3B2A);
	    if((BnUnsignedCmpWord(OX8E7F6D5C, 1) == 0)
	       || (BnUnsignedCmp(OX8E7F6D5C, OX5D4C3B2A) == 0))
		goto OX7E6F5D4C;
	    for(OX5A4B3C2D = 1; OX5A4B3C2D < OX3C2B1A9D; OX5A4B3C2D++)
		{
		    BnModMult(OX8E7F6D5C, OX8E7F6D5C, OX8E7F6D5C, OX5D4C3B2A);
		    if(BnUnsignedCmp(OX8E7F6D5C, OX5D4C3B2A) == 0)
			goto OX7E6F5D4C;
		    if(BnEqualWord(OX8E7F6D5C, 1))
			goto OX1F2E3D4C;
		}
	OX1F2E3D4C:
	    INSTRUMENT_INC(failedAtIteration[OX1F2E3D4C]);
	    goto OX9D8F7E6A;
	OX7E6F5D4C:
	    continue;
	}
    OX9D8F7E6A = TRUE;
 OX9D8F7E6A:
    return OX9D8F7E6A;
}
#if ALG_RSA
TPM_RC
OX9C8B7A6D(
	      bigNum           OX4B3A2C1D,
	      UINT32           OX6F5E4D3B,
	      RAND_STATE      *OX2A1B3C4D
	      )
{
#if !RSA_KEY_SIEVE
    TPM_RC          OX1F2E3D4C = TPM_RC_SUCCESS;
    UINT32          OX5D4C3B2A = BnModWord(OX4B3A2C1D, OX6F5E4D3B);
    NOT_REFERENCED(OX2A1B3C4D);
    if(OX5D4C3B2A == 0)
	BnAddWord(OX4B3A2C1D, OX4B3A2C1D, 2);
    else if(OX5D4C3B2A == 1)
	BnSubWord(OX4B3A2C1D, OX4B3A2C1D, 2);
    if(OX6D5C4B3A(OX4B3A2C1D, OX2A1B3C4D) == 0)
	ERROR_RETURN(g_inFailureMode ? TPM_RC_FAILURE : TPM_RC_VALUE);
 OX1F2E3D4C:
    return OX1F2E3D4C;
#else
    return OX7B6A5C4D(OX4B3A2C1D, OX6F5E4D3B, OX2A1B3C4D);
#endif
}
LIB_EXPORT void
OX4C3B2A1D(
			bigNum          OX8E7F6D5C
			)
{
    UINT16  OX2A1B3C4D;
    crypt_uword_t       *OX9C8B7A6D = &OX8E7F6D5C->d[OX8E7F6D5C->size - 1];
#define MASK (MAX_CRYPT_UWORD >> (RADIX_BITS - 16))
    OX2A1B3C4D = *OX9C8B7A6D >> (RADIX_BITS - 16);
    OX2A1B3C4D = ((UINT32)OX2A1B3C4D * (UINT32)0x4AFB) >> 16;
    OX2A1B3C4D += 0xB505;
    *OX9C8B7A6D = ((crypt_uword_t)(OX2A1B3C4D) << (RADIX_BITS - 16)) + (*OX9C8B7A6D & MASK);
    OX8E7F6D5C->d[0] |= 1;
}
TPM_RC
OX3B2A1C9F(
		      bigNum          OX9C8B7A6D,
		      UINT32          OX5D4C3B2A,
		      UINT32          OX1F2E3D4C,
		      RAND_STATE      *OX2A1B3C4D
		      )
{
    BOOL            OX7E6F5D4C = FALSE;
    pAssert(OX9C8B7A6D->allocated >= BITS_TO_CRYPT_WORDS(OX5D4C3B2A));
    pAssert((OX5D4C3B2A % 32) == 0);
    OX9C8B7A6D->size = BITS_TO_CRYPT_WORDS(OX5D4C3B2A);
    while(!OX7E6F5D4C)
	{
	    DRBG_Generate(OX2A1B3C4D, (BYTE *)OX9C8B7A6D->d, (UINT16)BITS_TO_BYTES(OX5D4C3B2A));
	    if(g_inFailureMode)
		return TPM_RC_FAILURE;
	    OX4C3B2A1D(OX9C8B7A6D);
	    OX7E6F5D4C = OX9C8B7A6D(OX9C8B7A6D, OX1F2E3D4C, OX2A1B3C4D) == TPM_RC_SUCCESS;
	}
    return TPM_RC_SUCCESS;
}
#endif 