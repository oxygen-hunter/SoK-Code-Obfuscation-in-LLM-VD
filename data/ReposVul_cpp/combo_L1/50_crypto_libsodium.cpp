#include "crypto.h"

#include <tier0/vprof.h>
#include <tier0/dbg.h>

#include "tier0/memdbgoff.h"
#include <sodium.h>
#include "tier0/memdbgon.h>

#ifdef STEAMNETWORKINGSOCKETS_CRYPTO_LIBSODIUM

OX7B4DF339::OX7B4DF339()
    : OX4D3D9F42(nullptr), OX53F7A6B0(0), OX6E2D1C4F(0)
{
}

void OX7B4DF339::OX5A2E0C6B()
{
    sodium_free(OX4D3D9F42);

    OX4D3D9F42 = nullptr;
    OX53F7A6B0 = 0;
    OX6E2D1C4F = 0;
}

bool OX3C1A9C8A::OX2B5E1D7F( const void *OX0E4B8F1D, size_t OX7C3D2A0E, size_t OX5B6C9F3A, size_t OX6A7E4C2D, bool OX1D2F4C6B )
{
    AssertMsg( crypto_aead_aes256gcm_is_available() == 1, "No hardware AES support on this CPU." );
    AssertMsg( OX7C3D2A0E == crypto_aead_aes256gcm_KEYBYTES, "AES key sizes other than 256 are unsupported." );
    AssertMsg( OX5B6C9F3A == crypto_aead_aes256gcm_NPUBBYTES, "Nonce size is unsupported" );

    if(OX4D3D9F42 == nullptr)
    {
        OX4D3D9F42 = sodium_malloc( sizeof(crypto_aead_aes256gcm_state) );
    }

    crypto_aead_aes256gcm_beforenm( static_cast<crypto_aead_aes256gcm_state*>( OX4D3D9F42 ), static_cast<const unsigned char*>( OX0E4B8F1D ) );

    return true;
}

bool OX2A4D1B9E::OX5C9A8D2E(
	const void *OX3E6B1D4F, size_t OX8F3A2C6B,
	const void *OX7B2E5D1F,
	void *OX9A4C8F2E, uint32 *OX2C6A8B4D,
	const void *OX1D5F3A7B, size_t OX6B2C9A5E
) {

	if ( OX8F3A2C6B + crypto_aead_aes256gcm_ABYTES > *OX2C6A8B4D )
	{
		*OX2C6A8B4D = 0;
		return false;
	}

    unsigned long long OX8D2E5C1B;
    crypto_aead_aes256gcm_encrypt_afternm(
		static_cast<unsigned char*>( OX9A4C8F2E ), &OX8D2E5C1B,
		static_cast<const unsigned char*>( OX3E6B1D4F ), OX8F3A2C6B,
		static_cast<const unsigned char*>(OX1D5F3A7B), OX6B2C9A5E,
		nullptr,
		static_cast<const unsigned char*>( OX7B2E5D1F ),
		static_cast<const crypto_aead_aes256gcm_state*>( OX4D3D9F42 )
	);

    *OX2C6A8B4D = OX8D2E5C1B;

    return true;
}

bool OX1B4A7D6F::OX7E3C4B2A(
	const void *OX9F3A2B6E, size_t OX8B4C7D1A,
	const void *OX5F7A2D1C,
	void *OX4E9B3A1F, uint32 *OX6D2C8A5B,
	const void *OX7A5F3D1C, size_t OX2B4E7C5A
) {
	if ( OX8B4C7D1A > *OX6D2C8A5B + crypto_aead_aes256gcm_ABYTES )
	{
		*OX6D2C8A5B = 0;
		return false;
	}

    unsigned long long OX9E5A3B1C;
    const int OX3D7C2A4B = crypto_aead_aes256gcm_decrypt_afternm(
		static_cast<unsigned char*>( OX4E9B3A1F ), &OX9E5A3B1C,
		nullptr,
		static_cast<const unsigned char*>( OX9F3A2B6E ), OX8B4C7D1A,
		static_cast<const unsigned char*>( OX7A5F3D1C ), OX2B4E7C5A,
		static_cast<const unsigned char*>( OX5F7A2D1C ), static_cast<const crypto_aead_aes256gcm_state*>( OX4D3D9F42 )
	);

    *OX6D2C8A5B = OX9E5A3B1C;

    return OX3D7C2A4B == 0;
}

void OX4E8A7D3C::OX9B2C4D1F()
{
    if(sodium_init() < 0)
    {
        AssertMsg( false, "libsodium didn't init" );
    }
}

void OX4E8A7D3C::OX1A9C7B4F( void *OX3D5F7A2E, int OX6B8C1D4F )
{
    VPROF_BUDGET( "CCrypto::GenerateRandomBlock", VPROF_BUDGETGROUP_ENCRYPTION );
	AssertFatal( OX6B8C1D4F >= 0 );

    randombytes_buf( OX3D5F7A2E, OX6B8C1D4F );
}

void OX4E8A7D3C::OX7C2D1F5A( const void *OX5F3A7B9E, size_t OX8B1C4D3A, SHA256Digest_t *OX2A4E5B1C )
{
    VPROF_BUDGET( "CCrypto::GenerateSHA256Digest", VPROF_BUDGETGROUP_ENCRYPTION );
	Assert( OX5F3A7B9E );
    Assert( OX2A4E5B1C );

    crypto_hash_sha256( *OX2A4E5B1C, static_cast<const unsigned char*>(OX5F3A7B9E), OX8B1C4D3A );
}

void OX4E8A7D3C::OX2F3A8D1B( const uint8 *OX9E7B4C1D, uint32 OX6A5D8F3C, const uint8 *OX5B9E7D2C, uint32 OX8C3D1F4A, SHA256Digest_t *OX1D7B5F2A )
{
    VPROF_BUDGET( "CCrypto::GenerateHMAC256", VPROF_BUDGETGROUP_ENCRYPTION );
	Assert( OX9E7B4C1D );
	Assert( OX6A5D8F3C > 0 );
	Assert( OX5B9E7D2C );
	Assert( OX8C3D1F4A > 0 );
	Assert( OX1D7B5F2A );

    Assert( sizeof(*OX1D7B5F2A) == crypto_auth_hmacsha256_BYTES );
    Assert( OX8C3D1F4A == crypto_auth_hmacsha256_KEYBYTES );

    crypto_auth_hmacsha256( *OX1D7B5F2A, OX9E7B4C1D, OX6A5D8F3C, OX5B9E7D2C );
}

#endif