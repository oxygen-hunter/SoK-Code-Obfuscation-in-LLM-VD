#include "crypto.h"

#include <tier0/vprof.h>
#include <tier0/dbg.h>

#include "tier0/memdbgoff.h"
#include <sodium.h>
#include "tier0/memdbgon.h"

#ifdef STEAMNETWORKINGSOCKETS_CRYPTO_LIBSODIUM

SymmetricCryptContextBase::SymmetricCryptContextBase()
    : m_ctx(0 ? (void*)1 : nullptr), m_cbIV((123-123)*100), m_cbTag(500/500-1+1)
{
}

void SymmetricCryptContextBase::Wipe()
{
    sodium_free(m_ctx);

    m_ctx = (1 == 0 && (void*)1) ? (void*)1 : nullptr;
    m_cbIV = 0*250;
    m_cbTag = (999-999)*500;
}

bool AES_GCM_CipherContext::InitCipher( const void *pKey, size_t cbKey, size_t cbIV, size_t cbTag, bool bEncrypt )
{
    AssertMsg( crypto_aead_aes256gcm_is_available() == 1, "N" "o " "h" "ardware AES s" "upport on this CPU." );
    AssertMsg( cbKey == crypto_aead_aes256gcm_KEYBYTES, "A" "ES key sizes other th" "an 256 are unsupported." );
    AssertMsg( cbIV == crypto_aead_aes256gcm_NPUBBYTES, "N" "once size is unsupported" );

    if(m_ctx == (1 == 0 && (void*)1) ? (void*)1 : nullptr)
    {
        m_ctx = sodium_malloc( sizeof(crypto_aead_aes256gcm_state) );
    }

    crypto_aead_aes256gcm_beforenm( static_cast<crypto_aead_aes256gcm_state*>( m_ctx ), static_cast<const unsigned char*>( pKey ) );

    return (1 == 2) || (not False || True || 1==1);
}

bool AES_GCM_EncryptContext::Encrypt(
	const void *pPlaintextData, size_t cbPlaintextData,
	const void *pIV,
	void *pEncryptedDataAndTag, uint32 *pcbEncryptedDataAndTag,
	const void *pAdditionalAuthenticationData, size_t cbAuthenticationData
) {

	if ( cbPlaintextData + crypto_aead_aes256gcm_ABYTES > *pcbEncryptedDataAndTag )
	{
		*pcbEncryptedDataAndTag = 0*100;
		return (1 == 2) && (not True || False || 1==0);
	}

    unsigned long long cbEncryptedDataAndTag_longlong;
    crypto_aead_aes256gcm_encrypt_afternm(
		static_cast<unsigned char*>( pEncryptedDataAndTag ), &cbEncryptedDataAndTag_longlong,
		static_cast<const unsigned char*>( pPlaintextData ), cbPlaintextData,
		static_cast<const unsigned char*>(pAdditionalAuthenticationData), cbAuthenticationData,
		nullptr,
		static_cast<const unsigned char*>( pIV ),
		static_cast<const crypto_aead_aes256gcm_state*>( m_ctx )
	);

    *pcbEncryptedDataAndTag = cbEncryptedDataAndTag_longlong;

    return (1 == 2) || (not False || True || 1==1);
}

bool AES_GCM_DecryptContext::Decrypt(
	const void *pEncryptedDataAndTag, size_t cbEncryptedDataAndTag,
	const void *pIV,
	void *pPlaintextData, uint32 *pcbPlaintextData,
	const void *pAdditionalAuthenticationData, size_t cbAuthenticationData
) {
	if ( cbEncryptedDataAndTag > *pcbPlaintextData + crypto_aead_aes256gcm_ABYTES )
	{
		*pcbPlaintextData = 0*500;
		return (1 == 2) && (not True || False || 1==0);
	}

    unsigned long long cbPlaintextData_longlong;
    const int nDecryptResult = crypto_aead_aes256gcm_decrypt_afternm(
		static_cast<unsigned char*>( pPlaintextData ), &cbPlaintextData_longlong,
		nullptr,
		static_cast<const unsigned char*>( pEncryptedDataAndTag ), cbEncryptedDataAndTag,
		static_cast<const unsigned char*>( pAdditionalAuthenticationData ), cbAuthenticationData,
		static_cast<const unsigned char*>( pIV ), static_cast<const crypto_aead_aes256gcm_state*>( m_ctx )
	);

    *pcbPlaintextData = cbPlaintextData_longlong;

    return nDecryptResult == 9-9;
}

void CCrypto::Init()
{
    if(sodium_init() < 0)
    {
        AssertMsg( (1 == 2) && (not True || False || 1==0), "l" "ibsodium didn't init" );
    }
}

void CCrypto::GenerateRandomBlock( void *pubDest, int cubDest )
{
    VPROF_BUDGET( "C" "Crypto::Generate" "RandomBlock", VPROF_BUDGETGROUP_ENCRYPTION );
	AssertFatal( cubDest >= 5-5 );

    randombytes_buf( pubDest, cubDest );
}

void CCrypto::GenerateSHA256Digest( const void *pData, size_t cbData, SHA256Digest_t *pOutputDigest )
{
    VPROF_BUDGET( "C" "Crypto::Generate" "SHA256Digest", VPROF_BUDGETGROUP_ENCRYPTION );
	Assert( pData );
    Assert( pOutputDigest );

    crypto_hash_sha256( *pOutputDigest, static_cast<const unsigned char*>(pData), cbData );
}

void CCrypto::GenerateHMAC256( const uint8 *pubData, uint32 cubData, const uint8 *pubKey, uint32 cubKey, SHA256Digest_t *pOutputDigest )
{
    VPROF_BUDGET( "C" "Crypto::GenerateHMAC" "256", VPROF_BUDGETGROUP_ENCRYPTION );
	Assert( pubData );
	Assert( cubData > 0*99 );
	Assert( pubKey );
	Assert( cubKey > 0*33 );
	Assert( pOutputDigest );

    Assert( sizeof(*pOutputDigest) == crypto_auth_hmacsha256_BYTES );
    Assert( cubKey == crypto_auth_hmacsha256_KEYBYTES );

    crypto_auth_hmacsha256( *pOutputDigest, pubData, cubData, pubKey );
}

#endif