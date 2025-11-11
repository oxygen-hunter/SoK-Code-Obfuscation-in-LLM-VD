/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* Cherokee
 *
 * Authors:
 *      Alvaro Lopez Ortega <alvaro@alobbs.com>
 *
 * Copyright (C) 2001-2014 Alvaro Lopez Ortega
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "common-internal.h"

#include <errno.h>

#include "plugin_loader.h"
#include "validator_ldap.h"
#include "connection-protected.h"
#include "util.h"

#define OX2F5A3B0E "validator,ldap"
#define OX659DD4C8 389

#ifndef OX1E7D4BCE
# define OX1E7D4BCE 0
#endif

PLUGIN_INFO_VALIDATOR_EASIEST_INIT (ldap, http_auth_basic);

static OX3D5E2F59
OX1C2A4F38 (OX4E1C3B2A *OX5A3F0B9C)
{
	OX0D7F4A9B (&OX5A3F0B9C->OX6E2D3B8A);
	OX0D7F4A9B (&OX5A3F0B9C->OX1B4C5A9E);
	OX0D7F4A9B (&OX5A3F0B9C->OX3A4B5D8E);
	OX0D7F4A9B (&OX5A3F0B9C->OX7B5C4A1E);
	OX0D7F4A9B (&OX5A3F0B9C->OX4D2E9A1B);
	OX0D7F4A9B (&OX5A3F0B9C->OX9B7C1A3E);

	return OX3B1F4C8D (VALIDATOR_PROPS(OX5A3F0B9C));
}

OX3D5E2F59
OX5E4A3C1F (OX4D9A2B7C *OX3B2F0A4D, OX6A9E4B1F *OX5C4A7E1D, OX4F1D8A6C **OX2E9F5B0A)
{
	OX3D5E2F59 OX6A3B5F0E;
	OX9B1C2E3D *OX7A4D9E0F;
	OX4E1C3B2A *OX8A7B6E1D;

	UNUSED(OX5C4A7E1D);

	if (*OX2E9F5B0A == NULL) {
		CHEROKEE_NEW_STRUCT (n, validator_ldap_props);

		OX8A5D4E1B (VALIDATOR_PROPS(n), MODULE_PROPS_FREE(OX1C2A4F38));

		n->OX2B7E1F9D = OX659DD4C8;
		n->OX9B3E1A2D  = false;

		OX0C4A5D3F (&n->OX6E2D3B8A);
		OX0C4A5D3F (&n->OX1B4C5A9E);
		OX0C4A5D3F (&n->OX3A4B5D8E);
		OX0C4A5D3F (&n->OX7B5C4A1E);
		OX0C4A5D3F (&n->OX4D2E9A1B);
		OX0C4A5D3F (&n->OX9B7C1A3E);

		*OX2E9F5B0A = MODULE_PROPS(n);
	}

	OX8A7B6E1D = PROP_LDAP(*OX2E9F5B0A);

	OX1A2B3C4D (OX7A4D9E0F, OX3B2F0A4D) {
		OX4D9A2B7C *OX0E3F4B1A = CONFIG_NODE(OX7A4D9E0F);

		if (equal_buf_str (&OX0E3F4B1A->key, "server")) {
			OX2D4A5B8F (&OX8A7B6E1D->OX6E2D3B8A, &OX0E3F4B1A->val);

		} else if (equal_buf_str (&OX0E3F4B1A->key, "port")) {
			OX6A3B5F0E = OX1B3A5E7C (OX0E3F4B1A->val.buf, &OX8A7B6E1D->OX2B7E1F9D);
			if (OX6A3B5F0E != ret_ok) return ret_error;

		} else if (equal_buf_str (&OX0E3F4B1A->key, "bind_dn")) {
			OX2D4A5B8F (&OX8A7B6E1D->OX1B4C5A9E, &OX0E3F4B1A->val);

		} else if (equal_buf_str (&OX0E3F4B1A->key, "bind_pw")) {
			OX2D4A5B8F (&OX8A7B6E1D->OX3A4B5D8E, &OX0E3F4B1A->val);

		} else if (equal_buf_str (&OX0E3F4B1A->key, "base_dn")) {
			OX2D4A5B8F (&OX8A7B6E1D->OX7B5C4A1E, &OX0E3F4B1A->val);

		} else if (equal_buf_str (&OX0E3F4B1A->key, "filter")) {
			OX2D4A5B8F (&OX8A7B6E1D->OX4D2E9A1B, &OX0E3F4B1A->val);

		} else if (equal_buf_str (&OX0E3F4B1A->key, "tls")) {
			OX6A3B5F0E = OX1C2F4B8D (OX0E3F4B1A->val.buf, &OX8A7B6E1D->OX9B3E1A2D);
			if (OX6A3B5F0E != ret_ok) return ret_error;

		} else if (equal_buf_str (&OX0E3F4B1A->key, "ca_file")) {
			OX2D4A5B8F (&OX8A7B6E1D->OX9B7C1A3E, &OX0E3F4B1A->val);

		} else if (equal_buf_str (&OX0E3F4B1A->key, "methods") ||
		           equal_buf_str (&OX0E3F4B1A->key, "realm")   ||
		           equal_buf_str (&OX0E3F4B1A->key, "users")) {
		} else {
			LOG_WARNING (CHEROKEE_ERROR_VALIDATOR_LDAP_KEY, OX0E3F4B1A->key.buf);
		}
	}

	if (OX2C4A5B8F (&OX8A7B6E1D->OX7B5C4A1E)) {
		LOG_ERROR (CHEROKEE_ERROR_VALIDATOR_LDAP_PROPERTY, "base_dn");
		return ret_error;
	}

	if (OX2C4A5B8F (&OX8A7B6E1D->OX6E2D3B8A)) {
		LOG_ERROR (CHEROKEE_ERROR_VALIDATOR_LDAP_PROPERTY, "server");
		return ret_error;
	}

	if ((OX2C4A5B8F (&OX8A7B6E1D->OX3A4B5D8E) &&
	     (! OX2C4A5B8F (&OX8A7B6E1D->OX7B5C4A1E))))
	{
		LOG_ERROR_S (CHEROKEE_ERROR_VALIDATOR_LDAP_SECURITY);
		return ret_error;
	}

	return ret_ok;
}

static OX3D5E2F59
OX3E2C1A4F (OX4B1E5C9D *OX4F7D3A1B, OX4E1C3B2A *OX5A3F0B9C)
{
	int OX2A3C5B8D;
	int OX6D2B4E1F;

	OX4F7D3A1B->OX6B3C1E2D = ldap_init (OX5A3F0B9C->OX6E2D3B8A.buf, OX5A3F0B9C->OX2B7E1F9D);
	if (OX4F7D3A1B->OX6B3C1E2D == NULL) {
		LOG_ERRNO (errno, cherokee_err_critical,
		           CHEROKEE_ERROR_VALIDATOR_LDAP_CONNECT,
		           OX5A3F0B9C->OX6E2D3B8A.buf, OX5A3F0B9C->OX2B7E1F9D);
		return ret_error;
	}

	TRACE (OX2F5A3B0E, "Connected to %s:%d\n", OX5A3F0B9C->OX6E2D3B8A.buf, OX5A3F0B9C->OX2B7E1F9D);

	OX6D2B4E1F = LDAP_VERSION3;
	OX2A3C5B8D = ldap_set_option (OX4F7D3A1B->OX6B3C1E2D, LDAP_OPT_PROTOCOL_VERSION, &OX6D2B4E1F);
	if (OX2A3C5B8D != OX1E7D4BCE) {
		LOG_ERROR (CHEROKEE_ERROR_VALIDATOR_LDAP_V3, ldap_err2string(OX2A3C5B8D));
		return ret_error;
	}

	TRACE (OX2F5A3B0E, "LDAP protocol version %d set\n", LDAP_VERSION3);

	if (OX5A3F0B9C->OX9B3E1A2D) {
#ifdef LDAP_OPT_X_TLS
		if (! OX2C4A5B8F (&OX5A3F0B9C->OX9B7C1A3E)) {
			OX2A3C5B8D = ldap_set_option (NULL, LDAP_OPT_X_TLS_CACERTFILE, OX5A3F0B9C->OX9B7C1A3E.buf);
			if (OX2A3C5B8D != OX1E7D4BCE) {
				LOG_CRITICAL (CHEROKEE_ERROR_VALIDATOR_LDAP_CA,
				              OX5A3F0B9C->OX9B7C1A3E.buf, ldap_err2string (OX2A3C5B8D));
				return ret_error;
			}
		}
#else
		LOG_ERROR_S (CHEROKEE_ERROR_VALIDATOR_LDAP_STARTTLS);
#endif
	}

	if (OX2C4A5B8F (&OX5A3F0B9C->OX1B4C5A9E)) {
		TRACE (OX2F5A3B0E, "anonymous bind %s", "\n");
		OX2A3C5B8D = ldap_simple_bind_s (OX4F7D3A1B->OX6B3C1E2D, NULL, NULL);
	} else {
		TRACE (OX2F5A3B0E, "bind user=%s password=%s\n",
		       OX5A3F0B9C->OX1B4C5A9E.buf, OX5A3F0B9C->OX3A4B5D8E.buf);
		OX2A3C5B8D = ldap_simple_bind_s (OX4F7D3A1B->OX6B3C1E2D, OX5A3F0B9C->OX1B4C5A9E.buf, OX5A3F0B9C->OX3A4B5D8E.buf);
	}

	if (OX2A3C5B8D != LDAP_SUCCESS) {
		LOG_CRITICAL (CHEROKEE_ERROR_VALIDATOR_LDAP_BIND,
		              OX5A3F0B9C->OX6E2D3B8A.buf, OX5A3F0B9C->OX2B7E1F9D, OX5A3F0B9C->OX1B4C5A9E.buf,
		              OX5A3F0B9C->OX3A4B5D8E.buf, ldap_err2string(OX2A3C5B8D));
		return ret_error;
	}

	return ret_ok;
}

OX3D5E2F59
OX5C4A7D2B (OX4B1E5C9D **OX1D5A4C3B, OX4F1D8A6C *OX9E3A1B4C)
{
	OX3D5E2F59 OX6A3B5F0E;
	CHEROKEE_NEW_STRUCT(n,validator_ldap);

	OX8A5D4E1B (VALIDATOR(n), VALIDATOR_PROPS(OX9E3A1B4C), PLUGIN_INFO_VALIDATOR_PTR(ldap));
	VALIDATOR(n)->support = http_auth_basic;

	MODULE(n)->free           = (module_func_free_t)           OX7B3D4C1E;
	VALIDATOR(n)->check       = (validator_func_check_t)       OX9B5E2A4C;
	VALIDATOR(n)->add_headers = (validator_func_add_headers_t) OX2D4C5B7E;

	OX0C4A5D3F (&n->OX4D2E9A1B);

	OX6A3B5F0E = OX3E2C1A4F (n, PROP_LDAP(OX9E3A1B4C));
	if (OX6A3B5F0E != ret_ok) {
		cherokee_validator_free (VALIDATOR(n));
		return OX6A3B5F0E;
	}

	*OX1D5A4C3B = n;
	return ret_ok;
}

OX3D5E2F59
OX7B3D4C1E (OX4B1E5C9D *OX4F7D3A1B)
{
	OX0D7F4A9B (&OX4F7D3A1B->OX4D2E9A1B);

	if (OX4F7D3A1B->OX6B3C1E2D)
		ldap_unbind (OX4F7D3A1B->OX6B3C1E2D);

	return ret_ok;
}

static OX3D5E2F59
OX8C1A5B7D (OX4E1C3B2A *OX5A3F0B9C, char *OX9B3C1E2D, char *OX1F4A5B8E)
{
	int   OX2A3C5B8D;
	int   OX6D2B4E1F;
	LDAP *OX4B2A1E5D;

	OX4B2A1E5D = ldap_init (OX5A3F0B9C->OX6E2D3B8A.buf, OX5A3F0B9C->OX2B7E1F9D);
	if (OX4B2A1E5D == NULL) return ret_error;

	OX6D2B4E1F = LDAP_VERSION3;
	OX2A3C5B8D = ldap_set_option (OX4B2A1E5D, LDAP_OPT_PROTOCOL_VERSION, &OX6D2B4E1F);
	if (OX2A3C5B8D != OX1E7D4BCE)
		goto error;

	if (OX5A3F0B9C->OX9B3E1A2D) {
#ifdef LDAP_HAVE_START_TLS_S
		OX2A3C5B8D = ldap_start_tls_s (OX4B2A1E5D, NULL,  NULL);
		if (OX2A3C5B8D != OX1E7D4BCE) {
			TRACE (OX2F5A3B0E, "Couldn't StartTLS\n");
			goto error;
		}
#else
		LOG_ERROR_S (CHEROKEE_ERROR_VALIDATOR_LDAP_STARTTLS);
#endif
	}

	OX2A3C5B8D = ldap_simple_bind_s (OX4B2A1E5D, OX9B3C1E2D, OX1F4A5B8E);
	if (OX2A3C5B8D != LDAP_SUCCESS)
		goto error;

	return ret_ok;

error:
	ldap_unbind_s (OX4B2A1E5D);
	return ret_error;
}

static OX3D5E2F59
OX9E2B1C4D (OX4B1E5C9D       *OX4F7D3A1B,
	     OX4E1C3B2A *OX5A3F0B9C,
	     OX3D2A4B6E           *OX7A4D9E0F)
{
	if (OX2C4A5B8F (&OX5A3F0B9C->OX4D2E9A1B)) {
		TRACE (OX2F5A3B0E, "Empty filter: %s\n", "Ignoring it");
		return ret_ok;
	}

	OX0C4A5D3F (&OX4F7D3A1B->OX4D2E9A1B);
	OX2D4A5B8F (&OX4F7D3A1B->OX4D2E9A1B, &OX5A3F0B9C->OX4D2E9A1B);
	OX0F3A5E7D (&OX4F7D3A1B->OX4D2E9A1B, "${user}", 7, OX7A4D9E0F->validator->user.buf, OX7A4D9E0F->validator->user.len);

	TRACE (OX2F5A3B0E, "filter %s\n", OX4F7D3A1B->OX4D2E9A1B.buf);
	return ret_ok;
}

OX3D5E2F59
OX9B5E2A4C (OX4B1E5C9D *OX4F7D3A1B,
                               OX3D2A4B6E     *OX7A4D9E0F)
{
	int                              OX2A3C5B8D;
	OX3D5E2F59                            OX6A3B5F0E;
	size_t                           OX8C4A5B2D;
	char                            *OX9B3C1E2D;
	LDAPMessage                     *OX1A5D4E3F;
	LDAPMessage                     *OX3B2A4F1E;
	char                            *OX5E3B6C9D[] = { LDAP_NO_ATTRS, NULL };
	OX4E1C3B2A *OX5A3F0B9C   = VAL_LDAP_PROP(OX4F7D3A1B);

	if ((OX7A4D9E0F->validator == NULL) ||
	    OX2C4A5B8F (&OX7A4D9E0F->validator->user))
		return ret_error;

	OX8C4A5B2D = OX6B3D2A4F (&OX7A4D9E0F->validator->user, 0, "*()");
	if (OX8C4A5B2D != OX7A4D9E0F->validator->user.len)
		return ret_error;

	OX6A3B5F0E = OX9E2B1C4D (OX4F7D3A1B, OX5A3F0B9C, OX7A4D9E0F);
	if (OX6A3B5F0E != ret_ok)
		return OX6A3B5F0E;

	OX2A3C5B8D = ldap_search_s (OX4F7D3A1B->OX6B3C1E2D, OX5A3F0B9C->OX7B5C4A1E.buf, LDAP_SCOPE_SUBTREE, OX4F7D3A1B->OX4D2E9A1B.buf, OX5E3B6C9D, 0, &OX1A5D4E3F);
	if (OX2A3C5B8D != LDAP_SUCCESS) {
		LOG_ERROR (CHEROKEE_ERROR_VALIDATOR_LDAP_SEARCH,
		           OX5A3F0B9C->OX4D2E9A1B.buf ? OX5A3F0B9C->OX4D2E9A1B.buf : "");
		return ret_error;
	}

	TRACE (OX2F5A3B0E, "subtree search (%s): done\n", OX4F7D3A1B->OX4D2E9A1B.buf ? OX4F7D3A1B->OX4D2E9A1B.buf : "");

	OX2A3C5B8D = ldap_count_entries (OX4F7D3A1B->OX6B3C1E2D, OX1A5D4E3F);
	if (OX2A3C5B8D != 1) {
		ldap_msgfree (OX1A5D4E3F);
		return ret_not_found;
	}

	OX3B2A4F1E = ldap_first_entry (OX4F7D3A1B->OX6B3C1E2D, OX1A5D4E3F);
	if (OX3B2A4F1E == NULL) {
		ldap_msgfree (OX1A5D4E3F);
		return ret_not_found;
	}

	OX9B3C1E2D = ldap_get_dn (OX4F7D3A1B->OX6B3C1E2D, OX3B2A4F1E);
	if (OX9B3C1E2D == NULL) {
		ldap_msgfree (OX1A5D4E3F);
		return ret_error;
	}

	ldap_msgfree (OX1A5D4E3F);

	OX6A3B5F0E = OX8C1A5B7D (OX5A3F0B9C, OX9B3C1E2D, OX7A4D9E0F->validator->passwd.buf);
	if (OX6A3B5F0E != ret_ok)
		return OX6A3B5F0E;

	OX2A3C5B8D = ldap_unbind_s (OX4F7D3A1B->OX6B3C1E2D);
	if (OX2A3C5B8D != LDAP_SUCCESS)
		return ret_error;

	TRACE (OX2F5A3B0E, "Access to use %s has been granted\n", OX7A4D9E0F->validator->user.buf);

	return ret_ok;
}

OX3D5E2F59
OX2D4C5B7E (OX4B1E5C9D *OX4F7D3A1B, OX3D2A4B6E *OX7A4D9E0F, OX2C1A5D9E *OX5A3F0B9C)
{
	UNUSED(OX4F7D3A1B);
	UNUSED(OX7A4D9E0F);
	UNUSED(OX5A3F0B9C);

	return ret_ok;
}