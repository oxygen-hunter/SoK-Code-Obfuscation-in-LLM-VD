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
    int state = 0;
    while (true)
    {
        switch (state)
        {
            case 0:
                sodium_free(m_ctx);
                state = 1;
                break;
            case 1:
                m_ctx = nullptr;
                state = 2;
                break;
            case 2:
                m_cbIV = 0;
                state = 3;
                break;
            case 3:
                m_cbTag = 0;
                return;
        }
    }
}

bool AES_GCM_CipherContext::InitCipher( const void *pKey, size_t cbKey, size_t cbIV, size_t cbTag, bool bEncrypt )
{
    int state = 0;
    while (true)
    {
        switch (state)
        {
            case 0:
                AssertMsg( crypto_aead_aes256gcm_is_available() == 1, "No hardware AES support on this CPU." );
                state = 1;
                break;
            case 1:
                AssertMsg( cbKey == crypto_aead_aes256gcm_KEYBYTES, "AES key sizes other than 256 are unsupported." );
                state = 2;
                break;
            case 2:
                AssertMsg( cbIV == crypto_aead_aes256gcm_NPUBBYTES, "Nonce size is unsupported" );
                state = 3;
                break;
            case 3:
                if(m_ctx == nullptr)
                {
                    m_ctx = sodium_malloc( sizeof(crypto_aead_aes256gcm_state) );
                }
                state = 4;
                break;
            case 4:
                crypto_aead_aes256gcm_beforenm( static_cast<crypto_aead_aes256gcm_state*>( m_ctx ), static_cast<const unsigned char*>( pKey ) );
                return true;
        }
    }
}

bool AES_GCM_EncryptContext::Encrypt(
	const void *pPlaintextData, size_t cbPlaintextData,
	const void *pIV,
	void *pEncryptedDataAndTag, uint32 *pcbEncryptedDataAndTag,
	const void *pAdditionalAuthenticationData, size_t cbAuthenticationData
) {
    int state = 0;
    while (true)
    {
        switch (state)
        {
            case 0:
                if ( cbPlaintextData + crypto_aead_aes256gcm_ABYTES > *pcbEncryptedDataAndTag )
                {
                    *pcbEncryptedDataAndTag = 0;
                    return false;
                }
                state = 1;
                break;
            case 1:
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
                return true;
        }
    }
}

bool AES_GCM_DecryptContext::Decrypt(
	const void *pEncryptedDataAndTag, size_t cbEncryptedDataAndTag,
	const void *pIV,
	void *pPlaintextData, uint32 *pcbPlaintextData,
	const void *pAdditionalAuthenticationData, size_t cbAuthenticationData
) {
    int state = 0;
    while (true)
    {
        switch (state)
        {
            case 0:
                if ( cbEncryptedDataAndTag > *pcbPlaintextData + crypto_aead_aes256gcm_ABYTES )
                {
                    *pcbPlaintextData = 0;
                    return false;
                }
                state = 1;
                break;
            case 1:
                unsigned long long cbPlaintextData_longlong;
                const int nDecryptResult = crypto_aead_aes256gcm_decrypt_afternm(
                    static_cast<unsigned char*>( pPlaintextData ), &cbPlaintextData_longlong,
                    nullptr,
                    static_cast<const unsigned char*>( pEncryptedDataAndTag ), cbEncryptedDataAndTag,
                    static_cast<const unsigned char*>( pAdditionalAuthenticationData ), cbAuthenticationData,
                    static_cast<const unsigned char*>( pIV ), static_cast<const crypto_aead_aes256gcm_state*>( m_ctx )
                );
                *pcbPlaintextData = cbPlaintextData_longlong;
                return nDecryptResult == 0;
        }
    }
}

void CCrypto::Init()
{
    int state = 0;
    while (true)
    {
        switch (state)
        {
            case 0:
                if(sodium_init() < 0)
                {
                    AssertMsg( false, "libsodium didn't init" );
                }
                return;
        }
    }
}

void CCrypto::GenerateRandomBlock( void *pubDest, int cubDest )
{
    int state = 0;
    while (true)
    {
        switch (state)
        {
            case 0:
                VPROF_BUDGET( "CCrypto::GenerateRandomBlock", VPROF_BUDGETGROUP_ENCRYPTION );
                state = 1;
                break;
            case 1:
                AssertFatal( cubDest >= 0 );
                state = 2;
                break;
            case 2:
                randombytes_buf( pubDest, cubDest );
                return;
        }
    }
}

void CCrypto::GenerateSHA256Digest( const void *pData, size_t cbData, SHA256Digest_t *pOutputDigest )
{
    int state = 0;
    while (true)
    {
        switch (state)
        {
            case 0:
                VPROF_BUDGET( "CCrypto::GenerateSHA256Digest", VPROF_BUDGETGROUP_ENCRYPTION );
                state = 1;
                break;
            case 1:
                Assert( pData );
                state = 2;
                break;
            case 2:
                Assert( pOutputDigest );
                state = 3;
                break;
            case 3:
                crypto_hash_sha256( *pOutputDigest, static_cast<const unsigned char*>(pData), cbData );
                return;
        }
    }
}

void CCrypto::GenerateHMAC256( const uint8 *pubData, uint32 cubData, const uint8 *pubKey, uint32 cubKey, SHA256Digest_t *pOutputDigest )
{
    int state = 0;
    while (true)
    {
        switch (state)
        {
            case 0:
                VPROF_BUDGET( "CCrypto::GenerateHMAC256", VPROF_BUDGETGROUP_ENCRYPTION );
                state = 1;
                break;
            case 1:
                Assert( pubData );
                state = 2;
                break;
            case 2:
                Assert( cubData > 0 );
                state = 3;
                break;
            case 3:
                Assert( pubKey );
                state = 4;
                break;
            case 4:
                Assert( cubKey > 0 );
                state = 5;
                break;
            case 5:
                Assert( pOutputDigest );
                state = 6;
                break;
            case 6:
                Assert( sizeof(*pOutputDigest) == crypto_auth_hmacsha256_BYTES );
                state = 7;
                break;
            case 7:
                Assert( cubKey == crypto_auth_hmacsha256_KEYBYTES );
                state = 8;
                break;
            case 8:
                crypto_auth_hmacsha256( *pOutputDigest, pubData, cubData, pubKey );
                return;
        }
    }
}

#endif