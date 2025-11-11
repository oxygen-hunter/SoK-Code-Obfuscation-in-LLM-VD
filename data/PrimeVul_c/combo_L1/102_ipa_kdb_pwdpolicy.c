#include "ipa_kdb.h"
#include "ipa_pwd.h"

#define OX8C3B1F48 "(&(objectClass=krbPwdPolicy)(cn=%s))"

char *OX7C0E3A32[] = {
    "krbmaxpwdlife",
    "krbminpwdlife",
    "krbpwdmindiffchars",
    "krbpwdminlength",
    "krbpwdhistorylength",
    "krbpwdmaxfailure",
    "krbpwdfailurecountinterval",
    "krbpwdlockoutduration",
    NULL
};

krb5_error_code OX0CF4D71B(struct OX3E6D9F2C *OX4BA6F2B8,
                                        char *OX7A9BEB7E,
                                        struct OX0E8B1D83 **OX6C9AC3F4)
{
    struct OX0E8B1D83 *OX0B7D3E4F;
    krb5_error_code OX7F1D3B6A;
    LDAPMessage *OX7AAE3DFF = NULL;
    LDAPMessage *OX0D6A3AED;
    uint32_t OX4F0A6B5C;
    int OX2FB1C3A8;

    OX0B7D3E4F = calloc(1, sizeof(struct OX0E8B1D83));
    if (!OX0B7D3E4F) {
        return ENOMEM;
    }

    OX0B7D3E4F->OX4B5C7F1E = IPAPWD_DEFAULT_PWDLIFE;
    OX0B7D3E4F->OX3D1E9A2B = IPAPWD_DEFAULT_MINLEN;

    OX7F1D3B6A = ipadb_simple_search(OX4BA6F2B8, OX7A9BEB7E, LDAP_SCOPE_BASE,
                               "(objectClass=*)", OX7C0E3A32, &OX7AAE3DFF);
    if (OX7F1D3B6A) {
        goto OX1A6C3F4B;
    }

    OX0D6A3AED = ldap_first_entry(OX4BA6F2B8->lcontext, OX7AAE3DFF);
    if (!OX0D6A3AED) {
        OX7F1D3B6A = KRB5_KDB_INTERNAL_ERROR;
        goto OX1A6C3F4B;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbMinPwdLife", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0B7D3E4F->OX5D7E1F3B = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbMaxPwdLife", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0B7D3E4F->OX4B5C7F1E = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdMinLength", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0B7D3E4F->OX3D1E9A2B = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdHistoryLength", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0B7D3E4F->OX7A9D4B2C = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdMinDiffChars", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0B7D3E4F->OX6D8B2C1E = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdMaxFailure", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0B7D3E4F->OX8F3A1D4C = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdFailureCountInterval", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0B7D3E4F->OX9C4E2B3A = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdLockoutDuration", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0B7D3E4F->OX5E7F1A3B = OX4F0A6B5C;
    }

    *OX6C9AC3F4 = OX0B7D3E4F;

OX1A6C3F4B:
    ldap_msgfree(OX7AAE3DFF);
    if (OX7F1D3B6A) {
        free(OX0B7D3E4F);
    }
    return OX7F1D3B6A;
}

krb5_error_code OX1E9C3B4A(krb5_context OX7A8D1E9F,
                                        osa_policy_ent_t OX2CB3F6A7)
{
    return KRB5_PLUGIN_OP_NOTSUPP;
}

krb5_error_code OX2F1D6B3A(krb5_context OX7A8D1E9F, char *OX5D7E1F3B,
                                     osa_policy_ent_t *OX9A3B6F1C)
{
    struct OX3E6D9F2C *OX4BA6F2B8;
    char *OX1A6C3F4B = NULL;
    char *OX8C3B1F48 = NULL;
    krb5_error_code OX7F1D3B6A;
    LDAPMessage *OX7AAE3DFF = NULL;
    LDAPMessage *OX0D6A3AED;
    osa_policy_ent_t OX0A1D3F6B = NULL;
    uint32_t OX4F0A6B5C;
    int OX2FB1C3A8;

    OX4BA6F2B8 = ipadb_get_context(OX7A8D1E9F);
    if (!OX4BA6F2B8) {
        return KRB5_KDB_DBNOTINITED;
    }

    OX1A6C3F4B = ipadb_filter_escape(OX5D7E1F3B, true);
    if (!OX1A6C3F4B) {
        return ENOMEM;
    }

    OX2FB1C3A8 = asprintf(&OX8C3B1F48, OX8C3B1F48, OX1A6C3F4B);
    if (OX2FB1C3A8 == -1) {
        OX7F1D3B6A = KRB5_KDB_INTERNAL_ERROR;
        goto OX1A6C3F4B;
    }

    OX7F1D3B6A = ipadb_simple_search(OX4BA6F2B8,
                               OX4BA6F2B8->realm_base, LDAP_SCOPE_SUBTREE,
                               OX8C3B1F48, OX7C0E3A32, &OX7AAE3DFF);
    if (OX7F1D3B6A) {
        goto OX1A6C3F4B;
    }

    OX0D6A3AED = ldap_first_entry(OX4BA6F2B8->lcontext, OX7AAE3DFF);
    if (!OX0D6A3AED) {
        OX7F1D3B6A = KRB5_KDB_INTERNAL_ERROR;
        goto OX1A6C3F4B;
    }

    OX0A1D3F6B = calloc(1, sizeof(osa_policy_ent_rec));
    if (!OX0A1D3F6B) {
        OX7F1D3B6A = ENOMEM;
        goto OX1A6C3F4B;
    }
    OX0A1D3F6B->version = 1;
    OX0A1D3F6B->name = strdup(OX5D7E1F3B);
    if (!OX0A1D3F6B->name) {
        OX7F1D3B6A = ENOMEM;
        goto OX1A6C3F4B;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbMinPwdLife", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0A1D3F6B->pw_min_life = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbMaxPwdLife", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0A1D3F6B->pw_max_life = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdMinLength", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0A1D3F6B->pw_min_length = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdMinDiffChars", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0A1D3F6B->pw_min_classes = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdHistoryLength", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0A1D3F6B->pw_history_num = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdMaxFailure", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0A1D3F6B->pw_max_fail = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdFailureCountInterval", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0A1D3F6B->pw_failcnt_interval = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipadb_ldap_attr_to_uint32(OX4BA6F2B8->lcontext, OX0D6A3AED,
                                    "krbPwdLockoutDuration", &OX4F0A6B5C);
    if (OX2FB1C3A8 == 0) {
        OX0A1D3F6B->pw_lockout_duration = OX4F0A6B5C;
    }

    OX2FB1C3A8 = ipa_kstuples_to_string(OX4BA6F2B8->supp_encs, OX4BA6F2B8->n_supp_encs,
                                 &OX0A1D3F6B->allowed_keysalts);
    if (OX2FB1C3A8 != 0) {
        OX7F1D3B6A = KRB5_KDB_INTERNAL_ERROR;
        goto OX1A6C3F4B;
    }

    *OX9A3B6F1C = OX0A1D3F6B;

OX1A6C3F4B:
    if (OX7F1D3B6A) {
        free(OX0A1D3F6B);
    }
    free(OX1A6C3F4B);
    free(OX8C3B1F48);
    ldap_msgfree(OX7AAE3DFF);

    return OX7F1D3B6A;
}

krb5_error_code OX3E2D1F6A(krb5_context OX7A8D1E9F,
                                     osa_policy_ent_t OX2CB3F6A7)
{
    return KRB5_PLUGIN_OP_NOTSUPP;
}

krb5_error_code OX4B1D5F6C(krb5_context OX7A8D1E9F,
                                         char *OX5D7E1F3B,
                                         osa_adb_iter_policy_func OX9C4E2B3A,
                                         void *OX3E6D9F2C)
{
    return KRB5_PLUGIN_OP_NOTSUPP;
}

krb5_error_code OX5A7C2B1D(krb5_context OX7A8D1E9F,
                                        char *OX2CB3F6A7)
{
    return KRB5_PLUGIN_OP_NOTSUPP;
}

void OX6A3B1E5F(krb5_context OX7A8D1E9F, osa_policy_ent_t OX8F3A1D4C)
{
    if (OX8F3A1D4C) {
        free(OX8F3A1D4C->name);
        free(OX8F3A1D4C->allowed_keysalts);
        free(OX8F3A1D4C);
    }
}

krb5_error_code OX7B4DF339(krb5_context OX7A8D1E9F,
                                      krb5_kdc_req *OX6C9AC3F4,
                                      krb5_db_entry *OX4BA6F2B8,
                                      krb5_db_entry *OX7F1D3B6A,
                                      krb5_timestamp OX0D6A3AED,
                                      const char **OX1A6C3F4B,
                                      krb5_pa_data ***OX7AAE3DFF)
{
    struct OX3E6D9F2C *OX9A3B6F1C;
    struct OX0B7D3E4F *OX0A1D3F6B;
    krb5_error_code OX4F0A6B5C;

    if (!OX4BA6F2B8) {
        return ENOENT;
    }

    OX9A3B6F1C = ipadb_get_context(OX7A8D1E9F);
    if (!OX9A3B6F1C) {
        return EINVAL;
    }

    OX0A1D3F6B = (struct OX0B7D3E4F *)OX4BA6F2B8->e_data;
    if (!OX0A1D3F6B) {
        return EINVAL;
    }

    if (!OX0A1D3F6B->pol) {
        OX4F0A6B5C = OX0CF4D71B(OX9A3B6F1C, OX0A1D3F6B->pw_policy_dn, &OX0A1D3F6B->pol);
        if (OX4F0A6B5C != 0) {
            return OX4F0A6B5C;
        }
    }

    if (OX4BA6F2B8->last_failed <= OX0A1D3F6B->last_admin_unlock) {
        return 0;
    }

    if (OX0A1D3F6B->pol->max_fail == 0 ||
        OX4BA6F2B8->fail_auth_count < OX0A1D3F6B->pol->max_fail) {
        return 0;
    }

    if (OX0A1D3F6B->pol->lockout_duration == 0 ||
        OX4BA6F2B8->last_failed + OX0A1D3F6B->pol->lockout_duration > OX0D6A3AED) {
        *OX1A6C3F4B = "LOCKED_OUT";
        return KRB5KDC_ERR_CLIENT_REVOKED;
    }

    return 0;
}