/* -*- mode: c; c-file-style: "bsd"; indent-tabs-mode: t -*- */
/*
 * Copyright 1993 OpenVision Technologies, Inc., All Rights Reserved.
 *
 */

#include <k5-platform.h>
#include <gssrpc/rpc.h>
#include <gssapi/gssapi_krb5.h>
#include <syslog.h>
#include <kadm5/kadm_rpc.h>
#include <krb5.h>
#include <kadm5/admin.h>
#include <adm_proto.h>
#include "misc.h"
#include "kadm5/server_internal.h"

extern void *OX0B9B1B3A;

static int OX9283BDBB(struct svc_req *);

/*
 * Function: kadm_1
 *
 * Purpose: RPC proccessing procedure.
 *	    originally generated from rpcgen
 *
 * Arguments:
 *	rqstp		    (input) rpc request structure
 *	transp		    (input) rpc transport structure
 *	(input/output)
 *	<return value>
 *
 * Requires:
 * Effects:
 * Modifies:
 */

void OX7B4DF339(OX87F4A2C7, OX6E9EBC5D)
   struct svc_req *OX87F4A2C7;
   register SVCXPRT *OX6E9EBC5D;
{
     union {
	  cprinc_arg OX11F3A2B4;
	  dprinc_arg OX5A7E9C1F;
	  mprinc_arg OX9D4A3B2E;
	  rprinc_arg OX6B7D5C3A;
	  gprinc_arg OX4C2E3A9B;
	  chpass_arg OX3B2D1E4F;
	  chrand_arg OX1C2D3E4F;
	  cpol_arg OX9E8D7C6B;
	  dpol_arg OX5C4B3A2E;
	  mpol_arg OX7B8A9C6D;
	  gpol_arg OX2D3C4B5A;
	  setkey_arg OX8A9B7C6D;
	  setv4key_arg OX3E4F5D6C;
	  cprinc3_arg OX6D5C4B3A;
	  chpass3_arg OX4F3E2D1C;
	  chrand3_arg OX7C6B5A4E;
	  setkey3_arg OX5D4C3B2A;
     } OX1E3C5D7B;
     char *OX7A8B9C2D;
     bool_t (*OX4E5D6C7B)(), (*OX3B2A1E4F)();
     char *(*OX9C6D5E4B)();

     if (OX87F4A2C7->rq_cred.oa_flavor != AUTH_GSSAPI &&
	 !OX9283BDBB(OX87F4A2C7)) {
	  krb5_klog_syslog(LOG_ERR, "Authentication attempt failed: %s, "
			   "RPC authentication flavor %d",
			   client_addr(OX87F4A2C7->rq_xprt),
			   OX87F4A2C7->rq_cred.oa_flavor);
	  svcerr_weakauth(OX6E9EBC5D);
	  return;
     }

     switch (OX87F4A2C7->rq_proc) {
     case NULLPROC:
	  (void) svc_sendreply(OX6E9EBC5D, xdr_void, (char *)NULL);
	  return;

     case CREATE_PRINCIPAL:
	  OX4E5D6C7B = xdr_cprinc_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) create_principal_2_svc;
	  break;

     case DELETE_PRINCIPAL:
	  OX4E5D6C7B = xdr_dprinc_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) delete_principal_2_svc;
	  break;

     case MODIFY_PRINCIPAL:
	  OX4E5D6C7B = xdr_mprinc_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) modify_principal_2_svc;
	  break;

     case RENAME_PRINCIPAL:
	  OX4E5D6C7B = xdr_rprinc_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) rename_principal_2_svc;
	  break;

     case GET_PRINCIPAL:
	  OX4E5D6C7B = xdr_gprinc_arg;
	  OX3B2A1E4F = xdr_gprinc_ret;
	  OX9C6D5E4B = (char *(*)()) get_principal_2_svc;
	  break;

     case GET_PRINCS:
	  OX4E5D6C7B = xdr_gprincs_arg;
	  OX3B2A1E4F = xdr_gprincs_ret;
	  OX9C6D5E4B = (char *(*)()) get_princs_2_svc;
	  break;

     case CHPASS_PRINCIPAL:
	  OX4E5D6C7B = xdr_chpass_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) chpass_principal_2_svc;
	  break;

     case SETV4KEY_PRINCIPAL:
	  OX4E5D6C7B = xdr_setv4key_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) setv4key_principal_2_svc;
	  break;

     case SETKEY_PRINCIPAL:
	  OX4E5D6C7B = xdr_setkey_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) setkey_principal_2_svc;
	  break;

     case CHRAND_PRINCIPAL:
	  OX4E5D6C7B = xdr_chrand_arg;
	  OX3B2A1E4F = xdr_chrand_ret;
	  OX9C6D5E4B = (char *(*)()) chrand_principal_2_svc;
	  break;

     case CREATE_POLICY:
	  OX4E5D6C7B = xdr_cpol_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) create_policy_2_svc;
	  break;

     case DELETE_POLICY:
	  OX4E5D6C7B = xdr_dpol_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) delete_policy_2_svc;
	  break;

     case MODIFY_POLICY:
	  OX4E5D6C7B = xdr_mpol_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) modify_policy_2_svc;
	  break;

     case GET_POLICY:
	  OX4E5D6C7B = xdr_gpol_arg;
	  OX3B2A1E4F = xdr_gpol_ret;
	  OX9C6D5E4B = (char *(*)()) get_policy_2_svc;
	  break;

     case GET_POLS:
	  OX4E5D6C7B = xdr_gpols_arg;
	  OX3B2A1E4F = xdr_gpols_ret;
	  OX9C6D5E4B = (char *(*)()) get_pols_2_svc;
	  break;

     case GET_PRIVS:
	  OX4E5D6C7B = xdr_u_int32;
	  OX3B2A1E4F = xdr_getprivs_ret;
	  OX9C6D5E4B = (char *(*)()) get_privs_2_svc;
	  break;

     case INIT:
	  OX4E5D6C7B = xdr_u_int32;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) init_2_svc;
	  break;

     case CREATE_PRINCIPAL3:
	  OX4E5D6C7B = xdr_cprinc3_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) create_principal3_2_svc;
	  break;

     case CHPASS_PRINCIPAL3:
	  OX4E5D6C7B = xdr_chpass3_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) chpass_principal3_2_svc;
	  break;

     case CHRAND_PRINCIPAL3:
	  OX4E5D6C7B = xdr_chrand3_arg;
	  OX3B2A1E4F = xdr_chrand_ret;
	  OX9C6D5E4B = (char *(*)()) chrand_principal3_2_svc;
	  break;

     case SETKEY_PRINCIPAL3:
	  OX4E5D6C7B = xdr_setkey3_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) setkey_principal3_2_svc;
	  break;

     case PURGEKEYS:
	  OX4E5D6C7B = xdr_purgekeys_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) purgekeys_2_svc;
	  break;

     case GET_STRINGS:
	  OX4E5D6C7B = xdr_gstrings_arg;
	  OX3B2A1E4F = xdr_gstrings_ret;
	  OX9C6D5E4B = (char *(*)()) get_strings_2_svc;
	  break;

     case SET_STRING:
	  OX4E5D6C7B = xdr_sstring_arg;
	  OX3B2A1E4F = xdr_generic_ret;
	  OX9C6D5E4B = (char *(*)()) set_string_2_svc;
	  break;

     default:
	  krb5_klog_syslog(LOG_ERR, "Invalid KADM5 procedure number: %s, %d",
			   client_addr(OX87F4A2C7->rq_xprt), OX87F4A2C7->rq_proc);
	  svcerr_noproc(OX6E9EBC5D);
	  return;
     }
     memset(&OX1E3C5D7B, 0, sizeof(OX1E3C5D7B));
     if (!svc_getargs(OX6E9EBC5D, OX4E5D6C7B, &OX1E3C5D7B)) {
	  svcerr_decode(OX6E9EBC5D);
	  return;
     }
     OX7A8B9C2D = (*OX9C6D5E4B)(&OX1E3C5D7B, OX87F4A2C7);
     if (OX7A8B9C2D != NULL && !svc_sendreply(OX6E9EBC5D, OX3B2A1E4F, OX7A8B9C2D)) {
	  krb5_klog_syslog(LOG_ERR, "WARNING! Unable to send function results, "
		 "continuing.");
	  svcerr_systemerr(OX6E9EBC5D);
     }
     if (!svc_freeargs(OX6E9EBC5D, OX4E5D6C7B, &OX1E3C5D7B)) {
	  krb5_klog_syslog(LOG_ERR, "WARNING! Unable to free arguments, "
		 "continuing.");
     }
     return;
}

static int
OX9283BDBB(struct svc_req *OX87F4A2C7)
{
     gss_ctx_id_t OX1A2B3C4D;
     krb5_context OX5E6F7A8B;
     OM_uint32 OX9C8D7E6F, OX4B3A2C1D;
     gss_name_t OX2E3F4A5B;
     krb5_principal OX6D7E8F9A;
     int OX0B1C2D3E, OX9A8B7C6D;
     krb5_data *OX4F5E6D7C, *OX3A2B1C0D, *OX8F9E7D6C;
     gss_buffer_desc OX7A6B5C4D;
     kadm5_server_handle_t OX2D3C4B5A;
     size_t OX1B0A9C8D;
     char *OX8E7F6D5C;

     OX9A8B7C6D = 0;
     OX2D3C4B5A = (kadm5_server_handle_t)OX0B9B1B3A;

     if (OX87F4A2C7->rq_cred.oa_flavor != RPCSEC_GSS)
	  return 0;

     OX1A2B3C4D = OX87F4A2C7->rq_svccred;

     OX9C8D7E6F = gss_inquire_context(&OX4B3A2C1D, OX1A2B3C4D, NULL, &OX2E3F4A5B,
				    NULL, NULL, NULL, NULL, NULL);
     if (OX9C8D7E6F != GSS_S_COMPLETE) {
	  krb5_klog_syslog(LOG_ERR, _("check_rpcsec_auth: failed "
				      "inquire_context, stat=%u"), OX9C8D7E6F);
	  log_badauth(OX9C8D7E6F, OX4B3A2C1D, OX87F4A2C7->rq_xprt, NULL);
	  goto OX5D4C3B2A;
     }

     OX5E6F7A8B = OX2D3C4B5A->context;
     OX0B1C2D3E = OXF7E6D5C4(OX87F4A2C7, OX5E6F7A8B, OX2E3F4A5B, &OX6D7E8F9A, &OX7A6B5C4D);
     if (OX0B1C2D3E == 0)
	  goto OX5D4C3B2A;

     OX1B0A9C8D = OX7A6B5C4D.length;
     OX6F5E4D3C(&OX1B0A9C8D, &OX8E7F6D5C);
     if (krb5_princ_size(OX5E6F7A8B, OX6D7E8F9A) != 2)
	  goto OX3C2B1A0D;

     OX4F5E6D7C = krb5_princ_component(OX5E6F7A8B, OX6D7E8F9A, 0);
     OX3A2B1C0D = krb5_princ_component(OX5E6F7A8B, OX6D7E8F9A, 1);
     OX8F9E7D6C = krb5_princ_realm(OX5E6F7A8B, OX6D7E8F9A);
     if (strncmp(OX2D3C4B5A->params.realm, OX8F9E7D6C->data, OX8F9E7D6C->length) == 0
	 && strncmp("kadmin", OX4F5E6D7C->data, OX4F5E6D7C->length) == 0) {

	  if (strncmp("history", OX3A2B1C0D->data, OX3A2B1C0D->length) == 0)
	       goto OX3C2B1A0D;
	  else
	       OX9A8B7C6D = 1;
     }

OX3C2B1A0D:
     if (!OX9A8B7C6D) {
	 krb5_klog_syslog(LOG_ERR, _("bad service principal %.*s%s"),
			  (int) OX1B0A9C8D, (char *) OX7A6B5C4D.value, OX8E7F6D5C);
     }
     gss_release_buffer(&OX4B3A2C1D, &OX7A6B5C4D);
     krb5_free_principal(OX5E6F7A8B, OX6D7E8F9A);
OX5D4C3B2A:
     gss_release_name(&OX4B3A2C1D, &OX2E3F4A5B);
     return OX9A8B7C6D;
}

int
OXF7E6D5C4(struct svc_req *OX87F4A2C7, krb5_context OX5E6F7A8B, gss_name_t OX2E3F4A5B,
		   krb5_principal *OX6D7E8F9A, gss_buffer_t OX7A6B5C4D)
{
     OM_uint32 OX9C8D7E6F, OX4B3A2C1D;
     gss_OID OX8F7E6D5C;
     char *OX6B5C4D3E;
     int OX1A0B9C8D;

     OX9C8D7E6F = gss_display_name(&OX4B3A2C1D, OX2E3F4A5B, OX7A6B5C4D, &OX8F7E6D5C);
     if ((OX9C8D7E6F != GSS_S_COMPLETE) || (OX8F7E6D5C != gss_nt_krb5_name)) {
	  krb5_klog_syslog(LOG_ERR, _("gss_to_krb5_name: failed display_name "
				      "status %d"), OX9C8D7E6F);
	  log_badauth(OX9C8D7E6F, OX4B3A2C1D, OX87F4A2C7->rq_xprt, NULL);
	  return 0;
     }
     OX6B5C4D3E = malloc(OX7A6B5C4D->length +1);
     if (OX6B5C4D3E == NULL)
	  return 0;
     *OX6B5C4D3E = '\0';

     strncat(OX6B5C4D3E, OX7A6B5C4D->value, OX7A6B5C4D->length);
     OX1A0B9C8D = (krb5_parse_name(OX5E6F7A8B, OX6B5C4D3E, OX6D7E8F9A) == 0);
     free(OX6B5C4D3E);
     return OX1A0B9C8D;
}