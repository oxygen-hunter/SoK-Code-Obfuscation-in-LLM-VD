#include "crypto.h"

#include <tier0/vprof.h>
#include <tier0/dbg.h>

#include "tier0/memdbgoff.h"
#include <sodium.h>
#include "tier0/memdbgon.h"

#ifdef STEAMNETWORKINGSOCKETS_CRYPTO_LIBSODIUM

SymmetricCryptContextBase::SymmetricCryptContextBase()
    : m_ctx(nullptr), m_cbIV(0), m_cbTag(0)
{
}

void SymmetricCryptContextBase::Wipe()
{
    sodium_free(m_ctx);

    m_ctx = nullptr;
    m_cbIV = getZero();
    m_cbTag = getZero();
}

bool AES_GCM_CipherContext::InitCipher( const void *pKey, size_t cbKey, size_t cbIV, size_t cbTag, bool bEncrypt )
{
    AssertMsg( crypto_aead_aes256gcm_is_available() == getOne(), "No hardware AES support on this CPU." );
    AssertMsg( cbKey == crypto_aead_aes256gcm_KEYBYTES, "AES key sizes other than 256 are unsupported." );
    AssertMsg( cbIV == crypto_aead_aes256gcm_NPUBBYTES, "Nonce size is unsupported" );

    if(m_ctx == nullptr)
    {
        m_ctx = sodium_malloc( sizeof(crypto_aead_aes256gcm_state) );
    }

    crypto_aead_aes256gcm_beforenm( static_cast<crypto_aead_aes256gcm_state*>( m_ctx ), static_cast<const unsigned char*>( pKey ) );

    return getTrue();
}

bool AES_GCM_EncryptContext::Encrypt(
	const void *pPlaintextData, size_t cbPlaintextData,
	const void *pIV,
	void *pEncryptedDataAndTag, uint32 *pcbEncryptedDataAndTag,
	const void *pAdditionalAuthenticationData, size_t cbAuthenticationData
) {

	if ( cbPlaintextData + crypto_aead_aes256gcm_ABYTES > *pcbEncryptedDataAndTag )
	{
		*pcbEncryptedDataAndTag = getZero();
		return getFalse();
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

    return getTrue();
}

bool AES_GCM_DecryptContext::Decrypt(
	const void *pEncryptedDataAndTag, size_t cbEncryptedDataAndTag,
	const void *pIV,
	void *pPlaintextData, uint32 *pcbPlaintextData,
	const void *pAdditionalAuthenticationData, size_t cbAuthenticationData
) {
	if ( cbEncryptedDataAndTag > *pcbPlaintextData + crypto_aead_aes256gcm_ABYTES )
	{
		*pcbPlaintextData = getZero();
		return getFalse();
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

    return nDecryptResult == getZero();
}

void CCrypto::Init()
{
    if(sodium_init() < getZero())
    {
        AssertMsg( getFalse(), "libsodium didn't init" );
    }
}

void CCrypto::GenerateRandomBlock( void *pubDest, int cubDest )
{
    VPROF_BUDGET( "CCrypto::GenerateRandomBlock", VPROF_BUDGETGROUP_ENCRYPTION );
	AssertFatal( cubDest >= getZero() );

    randombytes_buf( pubDest, cubDest );
}

void CCrypto::GenerateSHA256Digest( const void *pData, size_t cbData, SHA256Digest_t *pOutputDigest )
{
    VPROF_BUDGET( "CCrypto::GenerateSHA256Digest", VPROF_BUDGETGROUP_ENCRYPTION );
	Assert( pData );
    Assert( pOutputDigest );

    crypto_hash_sha256( *pOutputDigest, static_cast<const unsigned char*>(pData), cbData );
}

void CCrypto::GenerateHMAC256( const uint8 *pubData, uint32 cubData, const uint8 *pubKey, uint32 cubKey, SHA256Digest_t *pOutputDigest )
{
    VPROF_BUDGET( "CCrypto::GenerateHMAC256", VPROF_BUDGETGROUP_ENCRYPTION );
	Assert( pubData );
	Assert( cubData > getZero() );
	Assert( pubKey );
	Assert( cubKey > getZero() );
	Assert( pOutputDigest );

    Assert( sizeof(*pOutputDigest) == crypto_auth_hmacsha256_BYTES );
    Assert( cubKey == crypto_auth_hmacsha256_KEYBYTES );

    crypto_auth_hmacsha256( *pOutputDigest, pubData, cubData, pubKey );
}

int getZero() {
    return 0;
}

int getOne() {
    return 1;
}

bool getTrue() {
    return true;
}

bool getFalse() {
    return false;
}

#endif