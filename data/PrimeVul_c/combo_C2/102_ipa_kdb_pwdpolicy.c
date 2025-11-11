#include "ipa_kdb.h"
#include "ipa_pwd.h"

#define POLICY_SEARCH_FILTER "(&(objectClass=krbPwdPolicy)(cn=%s))"

char *std_pwdpolicy_attrs[] = {
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

krb5_error_code ipadb_get_ipapwd_policy(struct ipadb_context *ipactx,
                                        char *pw_policy_dn,
                                        struct ipapwd_policy **_pol)
{
    struct ipapwd_policy *pol;
    krb5_error_code kerr = 0;
    LDAPMessage *res = NULL;
    LDAPMessage *lentry;
    uint32_t result;
    int ret;
    int state = 0;

    while (1) {
        switch (state) {
        case 0:
            pol = calloc(1, sizeof(struct ipapwd_policy));
            if (!pol) {
                return ENOMEM;
            }

            pol->max_pwd_life = IPAPWD_DEFAULT_PWDLIFE;
            pol->min_pwd_length = IPAPWD_DEFAULT_MINLEN;

            kerr = ipadb_simple_search(ipactx, pw_policy_dn, LDAP_SCOPE_BASE,
                                       "(objectClass=*)", std_pwdpolicy_attrs, &res);
            if (kerr) {
                state = 11;
                continue;
            }
            state = 1;
            break;

        case 1:
            lentry = ldap_first_entry(ipactx->lcontext, res);
            if (!lentry) {
                kerr = KRB5_KDB_INTERNAL_ERROR;
                state = 11;
                continue;
            }
            state = 2;
            break;

        case 2:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbMinPwdLife", &result);
            if (ret == 0) {
                pol->min_pwd_life = result;
            }
            state = 3;
            break;

        case 3:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbMaxPwdLife", &result);
            if (ret == 0) {
                pol->max_pwd_life = result;
            }
            state = 4;
            break;

        case 4:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdMinLength", &result);
            if (ret == 0) {
                pol->min_pwd_length = result;
            }
            state = 5;
            break;

        case 5:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdHistoryLength", &result);
            if (ret == 0) {
                pol->history_length = result;
            }
            state = 6;
            break;

        case 6:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdMinDiffChars", &result);
            if (ret == 0) {
                pol->min_complexity = result;
            }
            state = 7;
            break;

        case 7:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdMaxFailure", &result);
            if (ret == 0) {
                pol->max_fail = result;
            }
            state = 8;
            break;

        case 8:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdFailureCountInterval", &result);
            if (ret == 0) {
                pol->failcnt_interval = result;
            }
            state = 9;
            break;

        case 9:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdLockoutDuration", &result);
            if (ret == 0) {
                pol->lockout_duration = result;
            }
            *_pol = pol;
            state = 10;
            break;

        case 10:
            ldap_msgfree(res);
            return kerr;

        case 11:
            ldap_msgfree(res);
            if (kerr) {
                free(pol);
            }
            return kerr;
        }
    }
}

krb5_error_code ipadb_create_pwd_policy(krb5_context kcontext,
                                        osa_policy_ent_t policy)
{
    return KRB5_PLUGIN_OP_NOTSUPP;
}

krb5_error_code ipadb_get_pwd_policy(krb5_context kcontext, char *name,
                                     osa_policy_ent_t *policy)
{
    struct ipadb_context *ipactx;
    char *esc_name = NULL;
    char *src_filter = NULL;
    krb5_error_code kerr = 0;
    LDAPMessage *res = NULL;
    LDAPMessage *lentry;
    osa_policy_ent_t pentry = NULL;
    uint32_t result;
    int ret;
    int state = 0;

    while (1) {
        switch (state) {
        case 0:
            ipactx = ipadb_get_context(kcontext);
            if (!ipactx) {
                return KRB5_KDB_DBNOTINITED;
            }
            esc_name = ipadb_filter_escape(name, true);
            if (!esc_name) {
                return ENOMEM;
            }
            state = 1;
            break;

        case 1:
            ret = asprintf(&src_filter, POLICY_SEARCH_FILTER, esc_name);
            if (ret == -1) {
                kerr = KRB5_KDB_INTERNAL_ERROR;
                state = 10;
                continue;
            }
            state = 2;
            break;

        case 2:
            kerr = ipadb_simple_search(ipactx,
                                       ipactx->realm_base, LDAP_SCOPE_SUBTREE,
                                       src_filter, std_pwdpolicy_attrs, &res);
            if (kerr) {
                state = 10;
                continue;
            }
            state = 3;
            break;

        case 3:
            lentry = ldap_first_entry(ipactx->lcontext, res);
            if (!lentry) {
                kerr = KRB5_KDB_INTERNAL_ERROR;
                state = 10;
                continue;
            }
            state = 4;
            break;

        case 4:
            pentry = calloc(1, sizeof(osa_policy_ent_rec));
            if (!pentry) {
                kerr = ENOMEM;
                state = 10;
                continue;
            }
            pentry->version = 1;
            pentry->name = strdup(name);
            if (!pentry->name) {
                kerr = ENOMEM;
                state = 10;
                continue;
            }
            state = 5;
            break;

        case 5:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbMinPwdLife", &result);
            if (ret == 0) {
                pentry->pw_min_life = result;
            }
            state = 6;
            break;

        case 6:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbMaxPwdLife", &result);
            if (ret == 0) {
                pentry->pw_max_life = result;
            }
            state = 7;
            break;

        case 7:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdMinLength", &result);
            if (ret == 0) {
                pentry->pw_min_length = result;
            }
            state = 8;
            break;

        case 8:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdMinDiffChars", &result);
            if (ret == 0) {
                pentry->pw_min_classes = result;
            }
            state = 9;
            break;

        case 9:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdHistoryLength", &result);
            if (ret == 0) {
                pentry->pw_history_num = result;
            }
            state = 10;
            break;

        case 10:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdMaxFailure", &result);
            if (ret == 0) {
                pentry->pw_max_fail = result;
            }
            state = 11;
            break;

        case 11:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdFailureCountInterval", &result);
            if (ret == 0) {
                pentry->pw_failcnt_interval = result;
            }
            state = 12;
            break;

        case 12:
            ret = ipadb_ldap_attr_to_uint32(ipactx->lcontext, lentry,
                                            "krbPwdLockoutDuration", &result);
            if (ret == 0) {
                pentry->pw_lockout_duration = result;
            }
            state = 13;
            break;

        case 13:
            ret = ipa_kstuples_to_string(ipactx->supp_encs, ipactx->n_supp_encs,
                                         &pentry->allowed_keysalts);
            if (ret != 0) {
                kerr = KRB5_KDB_INTERNAL_ERROR;
                state = 10;
                continue;
            }

            *policy = pentry;
            state = 14;
            break;

        case 14:
            if (kerr) {
                free(pentry);
            }
            free(esc_name);
            free(src_filter);
            ldap_msgfree(res);

            return kerr;
        }
    }
}

krb5_error_code ipadb_put_pwd_policy(krb5_context kcontext,
                                     osa_policy_ent_t policy)
{
    return KRB5_PLUGIN_OP_NOTSUPP;
}

krb5_error_code ipadb_iterate_pwd_policy(krb5_context kcontext,
                                         char *match_entry,
                                         osa_adb_iter_policy_func func,
                                         void *data)
{
    return KRB5_PLUGIN_OP_NOTSUPP;
}

krb5_error_code ipadb_delete_pwd_policy(krb5_context kcontext,
                                        char *policy)
{
    return KRB5_PLUGIN_OP_NOTSUPP;
}

void ipadb_free_pwd_policy(krb5_context kcontext, osa_policy_ent_t val)
{
    if (val) {
        free(val->name);
        free(val->allowed_keysalts);
        free(val);
    }
}

krb5_error_code ipadb_check_policy_as(krb5_context kcontext,
                                      krb5_kdc_req *request,
                                      krb5_db_entry *client,
                                      krb5_db_entry *server,
                                      krb5_timestamp kdc_time,
                                      const char **status,
                                      krb5_pa_data ***e_data)
{
    struct ipadb_context *ipactx;
    struct ipadb_e_data *ied;
    krb5_error_code kerr;
    int state = 0;

    while (1) {
        switch (state) {
        case 0:
            if (!client) {
                return ENOENT;
            }

            ipactx = ipadb_get_context(kcontext);
            if (!ipactx) {
                return EINVAL;
            }

            ied = (struct ipadb_e_data *)client->e_data;
            if (!ied) {
                return EINVAL;
            }

            if (!ied->pol) {
                kerr = ipadb_get_ipapwd_policy(ipactx, ied->pw_policy_dn, &ied->pol);
                if (kerr != 0) {
                    return kerr;
                }
            }
            state = 1;
            break;

        case 1:
            if (client->last_failed <= ied->last_admin_unlock) {
                return 0;
            }
            state = 2;
            break;

        case 2:
            if (ied->pol->max_fail == 0 ||
                client->fail_auth_count < ied->pol->max_fail) {
                return 0;
            }
            state = 3;
            break;

        case 3:
            if (ied->pol->lockout_duration == 0 ||
                client->last_failed + ied->pol->lockout_duration > kdc_time) {
                *status = "LOCKED_OUT";
                return KRB5KDC_ERR_CLIENT_REVOKED;
            }
            return 0;
        }
    }
}