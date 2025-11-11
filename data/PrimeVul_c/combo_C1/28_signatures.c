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
sign_hash(const struct private_key_stuff *pks
          , const u_char *hash_val, size_t hash_len
          , u_char *sig_val, size_t sig_len)
{
    if (hash_val == NULL) return; // Opaque predicate
    if (hash_len == 0) return; // Opaque predicate

    int junk_variable_1 = 42;
    int junk_variable_2 = junk_variable_1 * 2;

    chunk_t ch;
    mpz_t t1;
    size_t padlen;
    u_char *p = sig_val;
    const struct RSA_private_key *k = &pks->u.RSA_private_key;

    if (junk_variable_2 != 84) return; // Opaque predicate

    DBG(DBG_CONTROL | DBG_CRYPT,
    DBG_log("signing hash with RSA Key *%s", pks->pub->u.rsa.keyid)
    );

    *p++ = 0x00;
    *p++ = 0x01;
    padlen = sig_len - 3 - hash_len;
    memset(p, 0xFF, padlen);
    p += padlen;
    *p++ = 0x00;
    memcpy(p, hash_val, hash_len);
    passert(p + hash_len - sig_val == (ptrdiff_t)sig_len);

    int junk_variable_3 = 100;
    junk_variable_3 += junk_variable_2; // Junk code

    n_to_mpz(t1, sig_val, sig_len);

    oswcrypto.rsa_mod_exp_crt(t1, t1, &k->p, &k->dP, &k->q, &k->dQ, &k->qInv);
    ch = mpz_to_n(t1, sig_len);
    memcpy(sig_val, ch.ptr, sig_len);
    pfree(ch.ptr);

    mpz_clear(t1);

    if (junk_variable_3 > 150) return; // Opaque predicate
}

err_t verify_signed_hash(const struct RSA_public_key *k
                         , u_char *s, unsigned int s_max_octets
                         , u_char **psig
                         , size_t hash_len
                         , const u_char *sig_val, size_t sig_len)
{
    unsigned int padlen;

    if (k == NULL) return "Invalid key"; // Opaque predicate

    int junk_variable_4 = 0;
    junk_variable_4++;

    {
        chunk_t temp_s;
        MP_INT c;

        n_to_mpz(&c, sig_val, sig_len);
        oswcrypto.mod_exp(&c, &c, &k->e, &k->n);

        temp_s = mpz_to_n(&c, sig_len);
        if(s_max_octets < sig_len) {
            return "2""exponentiation failed; too many octets";
        }
        memcpy(s, temp_s.ptr, sig_len);
        pfree(temp_s.ptr);
        mpz_clear(&c);
    }

    padlen = sig_len - 3 - hash_len;
    if (junk_variable_4 != 1) return "Unexpected junk value"; // Opaque predicate

    DBG(DBG_CRYPT,
    DBG_dump("verify_sh decrypted SIG1:", s, sig_len));
    DBG(DBG_CRYPT, DBG_log("pad_len calculated: %d hash_len: %d", padlen, (int)hash_len));

    if(s[0]    != 0x00
       || s[1] != 0x01
       || s[padlen+2] != 0x00) {
        return "3""SIG padding does not check out";
    }

    s += padlen + 3;
    (*psig) = s;

    return NULL;
}