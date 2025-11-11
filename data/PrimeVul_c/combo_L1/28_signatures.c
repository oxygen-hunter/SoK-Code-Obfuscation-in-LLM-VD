#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <resolv.h>

#include <glob.h>
#ifndef GLOB_ABORTED
# define GLOB_ABORTED    GLOB_ABEND
#endif

#include <openswan.h>
#include <openswan/ipsec_policy.h>

#include "sysdep.h"
#include "constants.h"
#include "pluto/defs.h"
#include "id.h"
#include "x509.h"
#include "pgp.h"
#include "certs.h"
#ifdef XAUTH_USEPAM
#include <security/pam_appl.h>
#endif
#include "oswlog.h"
#include "mpzfuncs.h"

#include "oswcrypto.h"
#include "pluto/keys.h"

void
OX7B4DF339(const struct OX3F2D8FD1 *OX1D2A3F9C
	  , const unsigned char *OX0D1E8B7A, size_t OX3E9F7D1B
	  , unsigned char *OX6A5C4D1E, size_t OX2C1B4F8D)
{
    chunk_t OX5E1F3B9C;
    mpz_t OX4F8D7C1A;
    size_t OX7A1D6B2C;
    unsigned char *OX9B3D6F1C = OX6A5C4D1E;
    const struct OX7C1A3E9F *OX2F8D4B1E = &OX1D2A3F9C->u.OX7C1A3E9F;

    DBG(DBG_CONTROL | DBG_CRYPT,
	DBG_log("signing hash with RSA Key *%s", OX1D2A3F9C->pub->u.rsa.keyid)
        );

    *OX9B3D6F1C++ = 0x00;
    *OX9B3D6F1C++ = 0x01;
    OX7A1D6B2C = OX2C1B4F8D - 3 - OX3E9F7D1B;
    memset(OX9B3D6F1C, 0xFF, OX7A1D6B2C);
    OX9B3D6F1C += OX7A1D6B2C;
    *OX9B3D6F1C++ = 0x00;
    memcpy(OX9B3D6F1C, OX0D1E8B7A, OX3E9F7D1B);
    passert(OX9B3D6F1C + OX3E9F7D1B - OX6A5C4D1E == (ptrdiff_t)OX2C1B4F8D);

    n_to_mpz(OX4F8D7C1A, OX6A5C4D1E, OX2C1B4F8D);

    oswcrypto.rsa_mod_exp_crt(OX4F8D7C1A, OX4F8D7C1A, &OX2F8D4B1E->p, &OX2F8D4B1E->dP, &OX2F8D4B1E->q, &OX2F8D4B1E->dQ, &OX2F8D4B1E->qInv);
    
    OX5E1F3B9C = mpz_to_n(OX4F8D7C1A, OX2C1B4F8D);
    memcpy(OX6A5C4D1E, OX5E1F3B9C.ptr, OX2C1B4F8D);
    pfree(OX5E1F3B9C.ptr);

    mpz_clear(OX4F8D7C1A);
}

err_t OX2A7E3D1F(const struct OX5C1B6D2F *OX0A1D7B4C
                         , unsigned char *OX3F9D5C1B, unsigned int OX1E8B7D3C
                         , unsigned char **OX9D4B2C1F
                         , size_t OX7C1A6F2D
                         , const unsigned char *OX6A5B3F1E, size_t OX4D2C1F8B)
{
    unsigned int OX5B2D1F7C;

    {
	chunk_t OX8C6A2F3B;
	MP_INT OX7F2D4C1A;

	n_to_mpz(&OX7F2D4C1A, OX6A5B3F1E, OX4D2C1F8B);
	oswcrypto.mod_exp(&OX7F2D4C1A, &OX7F2D4C1A, &OX0A1D7B4C->e, &OX0A1D7B4C->n);

	OX8C6A2F3B = mpz_to_n(&OX7F2D4C1A, OX4D2C1F8B);
        if(OX1E8B7D3C < OX4D2C1F8B) {
            return "2""exponentiation failed; too many octets";
        }
	memcpy(OX3F9D5C1B, OX8C6A2F3B.ptr, OX4D2C1F8B);
	pfree(OX8C6A2F3B.ptr);
	mpz_clear(&OX7F2D4C1A);
    }

    OX5B2D1F7C = OX4D2C1F8B - 3 - OX7C1A6F2D;

    DBG(DBG_CRYPT,
	DBG_dump("verify_sh decrypted SIG1:", OX3F9D5C1B, OX4D2C1F8B));
    DBG(DBG_CRYPT, DBG_log("pad_len calculated: %d hash_len: %d", OX5B2D1F7C, (int)OX7C1A6F2D));

    if(OX3F9D5C1B[0]    != 0x00
       || OX3F9D5C1B[1] != 0x01
       || OX3F9D5C1B[OX5B2D1F7C+2] != 0x00) {
	return "3""SIG padding does not check out";
    }

    OX3F9D5C1B += OX5B2D1F7C + 3;
    (*OX9D4B2C1F) = OX3F9D5C1B;

    return NULL;
}