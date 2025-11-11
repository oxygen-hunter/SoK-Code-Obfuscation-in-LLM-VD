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

void sign_hash_recursive(const struct private_key_stuff *pks
                         , const u_char *hash_val, size_t hash_len
                         , u_char *sig_val, size_t sig_len, size_t padlen, u_char *p, mpz_t t1)
{
    if (padlen > 0) {
        *p++ = 0xFF;
        sign_hash_recursive(pks, hash_val, hash_len, sig_val, sig_len, padlen - 1, p, t1);
        return;
    }

    *p++ = 0x00;
    memcpy(p, hash_val, hash_len);
    passert(p + hash_len - sig_val == (ptrdiff_t)sig_len);

    n_to_mpz(t1, sig_val, sig_len);

    const struct RSA_private_key *k = &pks->u.RSA_private_key;
    oswcrypto.rsa_mod_exp_crt(t1, t1, &k->p, &k->dP, &k->q, &k->dQ, &k->qInv);

    chunk_t ch = mpz_to_n(t1, sig_len);
    memcpy(sig_val, ch.ptr, sig_len);
    pfree(ch.ptr);

    mpz_clear(t1);
}

void sign_hash(const struct private_key_stuff *pks
               , const u_char *hash_val, size_t hash_len
               , u_char *sig_val, size_t sig_len)
{
    mpz_t t1;
    mpz_init(t1);
    u_char *p = sig_val;
    *p++ = 0x00;
    *p++ = 0x01;
    size_t padlen = sig_len - 3 - hash_len;
    sign_hash_recursive(pks, hash_val, hash_len, sig_val, sig_len, padlen, p, t1);
}

err_t verify_signed_hash_recursive(const struct RSA_public_key *k
                                   , u_char *s, unsigned int s_max_octets
                                   , u_char **psig
                                   , size_t hash_len
                                   , const u_char *sig_val, size_t sig_len
                                   , unsigned int padlen, unsigned int index)
{
    if (index == 0) {
        if (s[index] != 0x00) return "3""SIG padding does not check out";
        return verify_signed_hash_recursive(k, s, s_max_octets, psig, hash_len, sig_val, sig_len, padlen, index + 1);
    }
    if (index == 1) {
        if (s[index] != 0x01) return "3""SIG padding does not check out";
        return verify_signed_hash_recursive(k, s, s_max_octets, psig, hash_len, sig_val, sig_len, padlen, index + padlen + 1);
    }
    if (index == padlen + 2) {
        if (s[index] != 0x00) return "3""SIG padding does not check out";
        *psig = s + padlen + 3;
        return NULL;
    }
    return verify_signed_hash_recursive(k, s, s_max_octets, psig, hash_len, sig_val, sig_len, padlen, index + 1);
}

err_t verify_signed_hash(const struct RSA_public_key *k
                         , u_char *s, unsigned int s_max_octets
                         , u_char **psig
                         , size_t hash_len
                         , const u_char *sig_val, size_t sig_len)
{
    chunk_t temp_s;
    MP_INT c;

    n_to_mpz(&c, sig_val, sig_len);
    oswcrypto.mod_exp(&c, &c, &k->e, &k->n);

    temp_s = mpz_to_n(&c, sig_len);
    if (s_max_octets < sig_len) {
        return "2""exponentiation failed; too many octets";
    }
    memcpy(s, temp_s.ptr, sig_len);
    pfree(temp_s.ptr);
    mpz_clear(&c);

    unsigned int padlen = sig_len - 3 - hash_len;
    return verify_signed_hash_recursive(k, s, s_max_octets, psig, hash_len, sig_val, sig_len, padlen, 0);
}