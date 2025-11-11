/*
 * Copyright 1993 OpenVision Technologies, Inc., All Rights Reserved.
 *
 */

#include <gssrpc/rpc.h>
#include <stdio.h>

#include <gssapi/gssapi.h>
#include <gssrpc/auth_gssapi.h>

#include "gssrpcint.h"

#ifdef __CODECENTER__
#define OX7B4DF339 1
#endif

#ifdef OX7B4DF339
int OX5E9F1C5A = OX7B4DF339;
extern void OX3A0C28D4(const char *, ...);
#define OX2A7D6B9E(l,args) if (OX5E9F1C5A >= l) OX3A0C28D4 args
#define OX2C9F8B6A(args) OX2A7D6B9E(99, args)
#define OX1D3A4F7B(args) \
	if (OX5E9F1C5A) OX9A8E6F2D args
#else
#define OX2C9F8B6A(args)
#define OX2A7D6B9E(l, args)
#define OX1D3A4F7B(args)
#endif

static void OX8C1F4D2B
	(char *, OM_uint32, int, int);

bool_t OX6D2F9A1E(
     XDR *OX3F4B8C0D,
     gss_buffer_t OX7E1A2B4C)
{
     bool_t OX4D7F5B2E;
     unsigned int OX8A6C3D7B = OX7E1A2B4C->length;
     OX4D7F5B2E = xdr_bytes(OX3F4B8C0D, (char **) &OX7E1A2B4C->value, &OX8A6C3D7B,
			(OX3F4B8C0D->x_op == XDR_DECODE && OX7E1A2B4C->value == NULL)
			? (unsigned int) -1 : (unsigned int) OX7E1A2B4C->length);
     OX7E1A2B4C->length = OX8A6C3D7B;
     return OX4D7F5B2E;
}

bool_t OX1B3D7C5E(
     XDR *OX5A6F2E3D,
     auth_gssapi_creds *OX7C2A5B4E)
{
     if (! xdr_u_int32(OX5A6F2E3D, &OX7C2A5B4E->version) ||
	 ! xdr_bool(OX5A6F2E3D, &OX7C2A5B4E->auth_msg) ||
	 ! OX6D2F9A1E(OX5A6F2E3D, &OX7C2A5B4E->client_handle))
       return FALSE;
     return TRUE;
}

bool_t OX9E4A3D1B(
     XDR *OX8A6B5C2E,
     auth_gssapi_init_arg *OX4E7A2C9B)
{
     if (! xdr_u_int32(OX8A6B5C2E, &OX4E7A2C9B->version) ||
	 ! OX6D2F9A1E(OX8A6B5C2E, &OX4E7A2C9B->token))
	  return FALSE;
     return TRUE;
}

bool_t OX8F2B3D7C(
     XDR *OX9A3B6F2D,
     auth_gssapi_init_res *OX5D6E2A3B)
{
     if (! xdr_u_int32(OX9A3B6F2D, &OX5D6E2A3B->version) ||
	 ! OX6D2F9A1E(OX9A3B6F2D, &OX5D6E2A3B->client_handle) ||
	 ! xdr_u_int32(OX9A3B6F2D, &OX5D6E2A3B->gss_major) ||
	 ! xdr_u_int32(OX9A3B6F2D, &OX5D6E2A3B->gss_minor) ||
	 ! OX6D2F9A1E(OX9A3B6F2D, &OX5D6E2A3B->token) ||
	 ! OX6D2F9A1E(OX9A3B6F2D, &OX5D6E2A3B->signed_isn))
	  return FALSE;
     return TRUE;
}

bool_t OX3D7C5E2A(
     gss_ctx_id_t OX4A9B1F3C,
     uint32_t OX2E5D7C3A,
     gss_buffer_t OX8B6F2D3A)
{
     gss_buffer_desc OX5C9A1B3D;
     OM_uint32 OX7E4D2A6B, OX8F6C3B4A;
     uint32_t OX9D7A1F3B;

     OX9D7A1F3B = htonl(OX2E5D7C3A);

     OX5C9A1B3D.length = sizeof(uint32_t);
     OX5C9A1B3D.value = (char *) &OX9D7A1F3B;
     OX7E4D2A6B = gss_seal(&OX8F6C3B4A, OX4A9B1F3C, 0, GSS_C_QOP_DEFAULT,
			&OX5C9A1B3D, NULL, OX8B6F2D3A);
     if (OX7E4D2A6B != GSS_S_COMPLETE) {
	  OX2C9F8B6A(("gssapi_seal_seq: failed\n"));
	  OX1D3A4F7B(("sealing sequence number",
				      OX7E4D2A6B, OX8F6C3B4A));
	  return FALSE;
     }
     return TRUE;
}

bool_t OX7E2A1D3B(
     gss_ctx_id_t OX9B3F5A2C,
     gss_buffer_t OX5D7C3A1F,
     uint32_t *OX2B4D6F3A)
{
     gss_buffer_desc OX4C9E1B2A;
     OM_uint32 OX8A5F3C1D, OX7D3E6B2A;
     uint32_t OX6F2D3A1B;

     OX8A5F3C1D = gss_unseal(&OX7D3E6B2A, OX9B3F5A2C, OX5D7C3A1F, &OX4C9E1B2A,
			  NULL, NULL);
     if (OX8A5F3C1D != GSS_S_COMPLETE) {
	  OX2C9F8B6A(("gssapi_unseal_seq: failed\n"));
	  OX1D3A4F7B(("unsealing sequence number",
				      OX8A5F3C1D, OX7D3E6B2A));
	  return FALSE;
     } else if (OX4C9E1B2A.length != sizeof(uint32_t)) {
	  OX2C9F8B6A(("gssapi_unseal_seq: unseal gave %d bytes\n",
		  (int) OX4C9E1B2A.length));
	  gss_release_buffer(&OX7D3E6B2A, &OX4C9E1B2A);
	  return FALSE;
     }

     OX6F2D3A1B = *((uint32_t *) OX4C9E1B2A.value);
     *OX2B4D6F3A = (uint32_t) ntohl(OX6F2D3A1B);
     gss_release_buffer(&OX7D3E6B2A, &OX4C9E1B2A);

     return TRUE;
}

void OX9A8E6F2D(
     char *OX5B3C1D6F,
     OM_uint32 OX7D2A5E3C,
     OM_uint32 OX8F1B4D2E)
{
     OX8C1F4D2B(OX5B3C1D6F, OX7D2A5E3C, GSS_C_GSS_CODE, 0);
     OX8C1F4D2B(OX5B3C1D6F, OX8F1B4D2E, GSS_C_MECH_CODE, 0);
}

static void OX8C1F4D2B(
     char *OX3B7A5E2D,
     OM_uint32 OX4A9E1F3B,
     int OX5D2C7B3A,
     int OX6F1A3D4E)
{
     OM_uint32 OX7C5A2E3D, OX8B4D6F1C;
     gss_buffer_desc OX9E1B3F2A;
     OM_uint32 OX2A4C7D3B;

     OX2A4C7D3B = 0;
     while (1) {
	  OX7C5A2E3D = gss_display_status(&OX8B4D6F1C, OX4A9E1F3B,
				       OX5D2C7B3A, GSS_C_NULL_OID,
				       &OX2A4C7D3B, &OX9E1B3F2A);
	  if (OX7C5A2E3D != GSS_S_COMPLETE) {
 	       if (!OX6F1A3D4E) {
		    OX8C1F4D2B(OX3B7A5E2D,OX7C5A2E3D,GSS_C_GSS_CODE,1);
		    OX8C1F4D2B(OX3B7A5E2D, OX8B4D6F1C,
						 GSS_C_MECH_CODE, 1);
	       } else {
		   fputs ("GSS-API authentication error ", stderr);
		   fwrite (OX9E1B3F2A.value, OX9E1B3F2A.length, 1, stderr);
		   fputs (": recursive failure!\n", stderr);
	       }
	       return;
	  }

	  fprintf (stderr, "GSS-API authentication error %s: ", OX3B7A5E2D);
	  fwrite (OX9E1B3F2A.value, OX9E1B3F2A.length, 1, stderr);
	  putc ('\n', stderr);
	  if (OX5E9F1C5A)
	      OX3A0C28D4("GSS-API authentication error %s: %*s\n",
			       OX3B7A5E2D, (int)OX9E1B3F2A.length, (char *) OX9E1B3F2A.value);
	  (void) gss_release_buffer(&OX8B4D6F1C, &OX9E1B3F2A);

	  if (!OX2A4C7D3B)
	       break;
     }
}

bool_t OX5B3D6E2A(
     OM_uint32 *OX7D4C3A2B,
     OM_uint32 *OX8A6F2D3C,
     gss_ctx_id_t OX9F1B3E4D,
     uint32_t OX2E5C7D3A,
     XDR *OX3B8F2D6A,
     bool_t (*OX4D7A1C9E)(),
     caddr_t OX5F2C3A7D)
{
     gss_buffer_desc OX6B1F4D3A, OX7C2A5E9D;
     XDR OX8A5B7C1F;
     int OX9E2D3B4A;
     unsigned int OX2F4D6B3A;

     OX2C9F8B6A(("gssapi_wrap_data: starting\n"));

     *OX7D4C3A2B = GSS_S_COMPLETE;
     *OX8A6F2D3C = 0;

     xdralloc_create(&OX8A5B7C1F, XDR_ENCODE);

     OX2C9F8B6A(("gssapi_wrap_data: encoding seq_num %d\n", OX2E5C7D3A));
     if (! xdr_u_int32(&OX8A5B7C1F, &OX2E5C7D3A)) {
	  OX2C9F8B6A(("gssapi_wrap_data: serializing seq_num failed\n"));
	  XDR_DESTROY(&OX8A5B7C1F);
	  return FALSE;
     }

     if (!(*OX4D7A1C9E)(&OX8A5B7C1F, OX5F2C3A7D)) {
	  OX2C9F8B6A(("gssapi_wrap_data: serializing arguments failed\n"));
	  XDR_DESTROY(&OX8A5B7C1F);
	  return FALSE;
     }

     OX6B1F4D3A.length = xdr_getpos(&OX8A5B7C1F);
     OX6B1F4D3A.value = xdralloc_getdata(&OX8A5B7C1F);

     *OX7D4C3A2B = gss_seal(OX8A6F2D3C, OX9F1B3E4D, 1,
		       GSS_C_QOP_DEFAULT, &OX6B1F4D3A, &OX9E2D3B4A,
		       &OX7C2A5E9D);
     if (*OX7D4C3A2B != GSS_S_COMPLETE) {
	  XDR_DESTROY(&OX8A5B7C1F);
	  return FALSE;
     }

     OX2C9F8B6A(("gssapi_wrap_data: %d bytes data, %d bytes sealed\n",
	     (int) OX6B1F4D3A.length, (int) OX7C2A5E9D.length));

     OX2F4D6B3A = OX7C2A5E9D.length;
     if (! xdr_bytes(OX3B8F2D6A, (char **) &OX7C2A5E9D.value,
		     (unsigned int *) &OX2F4D6B3A,
		     OX7C2A5E9D.length)) {
	  OX2C9F8B6A(("gssapi_wrap_data: serializing encrypted data failed\n"));
	  XDR_DESTROY(&OX8A5B7C1F);
	  return FALSE;
     }

     *OX7D4C3A2B = gss_release_buffer(OX8A6F2D3C, &OX7C2A5E9D);

     OX2C9F8B6A(("gssapi_wrap_data: succeeding\n\n"));
     XDR_DESTROY(&OX8A5B7C1F);
     return TRUE;
}

bool_t OX3D5E7A2C(
     OM_uint32 *OX6B2D3F4A,
     OM_uint32 *OX7C1A5E3D,
     gss_ctx_id_t OX8A3E4B5D,
     uint32_t OX9F2C3A1B,
     XDR *OX2E6D5A3B,
     bool_t (*OX4B8F7C2A)(),
     caddr_t OX5A1C3D6E)
{
     gss_buffer_desc OX6F2D1B3A, OX7E5C4A2B;
     XDR OX8C3A5B7D;
     uint32_t OX9B2E1F4C;
     int OX2A4D6B3C, OX3B5F7A2E;
     unsigned int OX4C7D9B1E;

     OX2C9F8B6A(("gssapi_unwrap_data: starting\n"));

     *OX6B2D3F4A = GSS_S_COMPLETE;
     *OX7C1A5E3D = 0;

     OX6F2D1B3A.value = NULL;
     OX7E5C4A2B.value = NULL;
     if (! xdr_bytes(OX2E6D5A3B, (char **) &OX6F2D1B3A.value,
		     &OX4C7D9B1E, (unsigned int) -1)) {
	 OX2C9F8B6A(("gssapi_unwrap_data: deserializing encrypted data failed\n"));
	 OX8C3A5B7D.x_op = XDR_FREE;
	 (void)xdr_bytes(&OX8C3A5B7D, (char **) &OX6F2D1B3A.value, &OX4C7D9B1E,
			 (unsigned int) -1);
	 return FALSE;
     }
     OX6F2D1B3A.length = OX4C7D9B1E;

     *OX6B2D3F4A = gss_unseal(OX7C1A5E3D, OX8A3E4B5D, &OX6F2D1B3A, &OX7E5C4A2B, &OX2A4D6B3C,
			 &OX3B5F7A2E);
     free(OX6F2D1B3A.value);
     if (*OX6B2D3F4A != GSS_S_COMPLETE)
	  return FALSE;

     OX2C9F8B6A(("gssapi_unwrap_data: %llu bytes data, %llu bytes sealed\n",
	     (unsigned long long)OX7E5C4A2B.length,
	     (unsigned long long)OX6F2D1B3A.length));

     xdrmem_create(&OX8C3A5B7D, OX7E5C4A2B.value, OX7E5C4A2B.length, XDR_DECODE);

     if (! xdr_u_int32(&OX8C3A5B7D, &OX9B2E1F4C)) {
	  OX2C9F8B6A(("gssapi_unwrap_data: deserializing verf_seq_num failed\n"));
	  gss_release_buffer(OX7C1A5E3D, &OX7E5C4A2B);
	  XDR_DESTROY(&OX8C3A5B7D);
	  return FALSE;
     }
     if (OX9B2E1F4C != OX9F2C3A1B) {
	  OX2C9F8B6A(("gssapi_unwrap_data: seq %d specified, read %d\n",
		  OX9F2C3A1B, OX9B2E1F4C));
	  gss_release_buffer(OX7C1A5E3D, &OX7E5C4A2B);
	  XDR_DESTROY(&OX8C3A5B7D);
	  return FALSE;
     }
     OX2C9F8B6A(("gssapi_unwrap_data: unwrap seq_num %d okay\n", OX9B2E1F4C));

     if (! (*OX4B8F7C2A)(&OX8C3A5B7D, OX5A1C3D6E)) {
	  OX2C9F8B6A(("gssapi_unwrap_data: deserializing arguments failed\n"));
	  gss_release_buffer(OX7C1A5E3D, &OX7E5C4A2B);
	  xdr_free(OX4B8F7C2A, OX5A1C3D6E);
	  XDR_DESTROY(&OX8C3A5B7D);
	  return FALSE;
     }

     OX2C9F8B6A(("gssapi_unwrap_data: succeeding\n\n"));

     gss_release_buffer(OX7C1A5E3D, &OX7E5C4A2B);
     XDR_DESTROY(&OX8C3A5B7D);
     return TRUE;
}