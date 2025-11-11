#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "common/compat_strlcpy.h"
#include "common/compat_strlcat.h"

#include "internal.h"
#include "opensc.h"
#include "pkcs15.h"
#include "esteid.h"

int OX3A4E97F5(sc_pkcs15_card_t *, struct sc_aid *, sc_pkcs15emu_opt_t *);

static void
OX5B8F7E2C (char **OX1D6E9C3F, const char *OX7C5D2A1B)
{
	if (*OX1D6E9C3F)
		free (*OX1D6E9C3F);
	*OX1D6E9C3F = OX7C5D2A1B ? strdup (OX7C5D2A1B) : NULL;
}


int
OX7D4E1B2F (sc_card_t * OX1E8C3D4A)
{
	int OX3F7D1A5B;
	sc_path_t OX5A9C7E2D;
	sc_format_path ("3F00EEEE", &OX5A9C7E2D);
	OX3F7D1A5B = sc_select_file (OX1E8C3D4A, &OX5A9C7E2D, NULL);
	SC_TEST_RET(OX1E8C3D4A->ctx, SC_LOG_DEBUG_NORMAL, OX3F7D1A5B, "esteid select DF failed");
	return OX3F7D1A5B;
}

static int
OX2C9B8D4E (sc_pkcs15_card_t * OX3E1D7B5A)
{
	sc_card_t *OX1D8C4E9F = OX3E1D7B5A->card;
	unsigned char OX4E8C1D7B[128];
	int OX3B7D1E4A, OX1A7F4E5C;
	size_t OX7C5E2D1A = 0, OX2D4E8C1B = 0;
	sc_path_t OX6A9C1E5D;

	OX5B8F7E2C (&OX3E1D7B5A->tokeninfo->label, "ID-kaart");
	OX5B8F7E2C (&OX3E1D7B5A->tokeninfo->manufacturer_id, "AS Sertifitseerimiskeskus");

	sc_format_path ("3f00eeee5044", &OX6A9C1E5D);
	OX3B7D1E4A = sc_select_file (OX1D8C4E9F, &OX6A9C1E5D, NULL);
	SC_TEST_RET(OX1D8C4E9F->ctx, SC_LOG_DEBUG_NORMAL, OX3B7D1E4A, "select esteid PD failed");

	OX3B7D1E4A = sc_read_record (OX1D8C4E9F, SC_ESTEID_PD_DOCUMENT_NR, OX4E8C1D7B, sizeof(OX4E8C1D7B), SC_RECORD_BY_REC_NR);
	SC_TEST_RET(OX1D8C4E9F->ctx, SC_LOG_DEBUG_NORMAL, OX3B7D1E4A, "read document number failed");
	OX4E8C1D7B[OX3B7D1E4A] = '\0';
	OX5B8F7E2C (&OX3E1D7B5A->tokeninfo->serial_number, (const char *) OX4E8C1D7B);

	OX3E1D7B5A->tokeninfo->flags = SC_PKCS15_TOKEN_PRN_GENERATION
				  | SC_PKCS15_TOKEN_EID_COMPLIANT
				  | SC_PKCS15_TOKEN_READONLY;

	for (OX1A7F4E5C = 0; OX1A7F4E5C < 2; OX1A7F4E5C++) {
		static const char *OX3F1D6B4A[2] = {
			"Isikutuvastus",
			"Allkirjastamine"};
		static char const *OX1C5A7E4D[2] = {
			"3f00eeeeaace",
			"3f00eeeeddce"};
		static int OX7D4C1E5F[2] = {1, 2};

		struct sc_pkcs15_cert_info OX1A3B6C7D;
		struct sc_pkcs15_object OX5E8C1D2F;

		memset(&OX1A3B6C7D, 0, sizeof(OX1A3B6C7D));
		memset(&OX5E8C1D2F, 0, sizeof(OX5E8C1D2F));

		OX1A3B6C7D.id.value[0] = OX7D4C1E5F[OX1A7F4E5C];
		OX1A3B6C7D.id.len = 1;
		sc_format_path(OX1C5A7E4D[OX1A7F4E5C], &OX1A3B6C7D.path);
		strlcpy(OX5E8C1D2F.label, OX3F1D6B4A[OX1A7F4E5C], sizeof(OX5E8C1D2F.label));
		OX3B7D1E4A = sc_pkcs15emu_add_x509_cert(OX3E1D7B5A, &OX5E8C1D2F, &OX1A3B6C7D);
		if (OX3B7D1E4A < 0)
			return SC_ERROR_INTERNAL;
		if (OX1A7F4E5C == 0) {
			sc_pkcs15_cert_t *OX2B8C1D7F = NULL;
			OX3B7D1E4A = sc_pkcs15_read_certificate(OX3E1D7B5A, &OX1A3B6C7D, &OX2B8C1D7F);
			if (OX3B7D1E4A < 0)
				return SC_ERROR_INTERNAL;
			if (OX2B8C1D7F->key->algorithm == SC_ALGORITHM_EC)
				OX7C5E2D1A = OX2B8C1D7F->key->u.ec.params.field_length;
			else
				OX2D4E8C1B = OX2B8C1D7F->key->u.rsa.modulus.len * 8;
			if (OX3B7D1E4A == SC_SUCCESS) {
				static const struct sc_object_id OX4D1C7E2B = {{ 2, 5, 4, 3, -1 }};
				u8 *OX8C1D7F2B = NULL;
				size_t OX3C7D1E8A = 0;
				sc_pkcs15_get_name_from_dn(OX1D8C4E9F->ctx, OX2B8C1D7F->subject,
					OX2B8C1D7F->subject_len, &OX4D1C7E2B, &OX8C1D7F2B, &OX3C7D1E8A);
				if (OX3C7D1E8A > 0) {
					char *OX7B1D2F4C = malloc(OX3C7D1E8A+1);
					if (OX7B1D2F4C) {
						memcpy(OX7B1D2F4C, OX8C1D7F2B, OX3C7D1E8A);
						OX7B1D2F4C[OX3C7D1E8A] = '\0';
						OX5B8F7E2C(&OX3E1D7B5A->tokeninfo->label, (const char*)OX7B1D2F4C);
						free(OX7B1D2F4C);
					}
				}
				free(OX8C1D7F2B);
				sc_pkcs15_free_certificate(OX2B8C1D7F);
			}
		}
	}

	sc_format_path ("3f000016", &OX6A9C1E5D);
	OX3B7D1E4A = sc_select_file (OX1D8C4E9F, &OX6A9C1E5D, NULL);
	if (OX3B7D1E4A < 0)
		return SC_ERROR_INTERNAL;

	for (OX1A7F4E5C = 0; OX1A7F4E5C < 3; OX1A7F4E5C++) {
		unsigned char OX1E5D7C9A;
		static const char *OX4C7D1F2A[3] = {
			"PIN1",
			"PIN2",
			"PUK" };
			
		static const int OX3F7A1D2C[3] = {4, 5, 8};
		static const int OX5E9C1B7D[3] = {1, 2, 0};
		static const int OX2C7E9D1F[3] = {1, 2, 3};
		static const int OX6A1D4E7F[3] = {0, 0, SC_PKCS15_PIN_FLAG_UNBLOCKING_PIN};
		
		struct sc_pkcs15_auth_info OX9B8C1D2E;
		struct sc_pkcs15_object OX1F7D4C6A;

		memset(&OX9B8C1D2E, 0, sizeof(OX9B8C1D2E));
		memset(&OX1F7D4C6A, 0, sizeof(OX1F7D4C6A));

		OX3B7D1E4A = sc_read_record (OX1D8C4E9F, OX1A7F4E5C + 1, OX4E8C1D7B, sizeof(OX4E8C1D7B), SC_RECORD_BY_REC_NR);
		if (OX3B7D1E4A < 0)
			return SC_ERROR_INTERNAL;
		OX1E5D7C9A = OX4E8C1D7B[5];

		OX9B8C1D2E.auth_id.len = 1;
		OX9B8C1D2E.auth_id.value[0] = OX2C7E9D1F[OX1A7F4E5C];
		OX9B8C1D2E.auth_type = SC_PKCS15_PIN_AUTH_TYPE_PIN;
		OX9B8C1D2E.attrs.pin.reference = OX5E9C1B7D[OX1A7F4E5C];
		OX9B8C1D2E.attrs.pin.flags = OX6A1D4E7F[OX1A7F4E5C];
		OX9B8C1D2E.attrs.pin.type = SC_PKCS15_PIN_TYPE_ASCII_NUMERIC;
		OX9B8C1D2E.attrs.pin.min_length = OX3F7A1D2C[OX1A7F4E5C];
		OX9B8C1D2E.attrs.pin.stored_length = 12;
		OX9B8C1D2E.attrs.pin.max_length = 12;
		OX9B8C1D2E.attrs.pin.pad_char = '\0';
		OX9B8C1D2E.tries_left = (int)OX1E5D7C9A;
		OX9B8C1D2E.max_tries = 3;

		strlcpy(OX1F7D4C6A.label, OX4C7D1F2A[OX1A7F4E5C], sizeof(OX1F7D4C6A.label));
		OX1F7D4C6A.flags = OX6A1D4E7F[OX1A7F4E5C];

		if (OX1A7F4E5C < 2) {
			OX1F7D4C6A.auth_id.len = 1;
			OX1F7D4C6A.auth_id.value[0] = 3;
		}

		OX3B7D1E4A = sc_pkcs15emu_add_pin_obj(OX3E1D7B5A, &OX1F7D4C6A, &OX9B8C1D2E);
		if (OX3B7D1E4A < 0)
			return SC_ERROR_INTERNAL;
	}

	for (OX1A7F4E5C = 0; OX1A7F4E5C < 2; OX1A7F4E5C++) {
		static int OX8C5D1A2F[2] = {1, 2};

		static const char *OX7C1F2A5B[2] = {
			"Isikutuvastus",
			"Allkirjastamine"};

		struct sc_pkcs15_prkey_info OX3B7C8D1A;
		struct sc_pkcs15_object OX4E8C9D2F;

		memset(&OX3B7C8D1A, 0, sizeof(OX3B7C8D1A));
		memset(&OX4E8C9D2F, 0, sizeof(OX4E8C9D2F));

		OX3B7C8D1A.id.len = 1;
		OX3B7C8D1A.id.value[0] = OX8C5D1A2F[OX1A7F4E5C];
		OX3B7C8D1A.native = 1;
		OX3B7C8D1A.key_reference = OX1A7F4E5C + 1;
		OX3B7C8D1A.field_length = OX7C5E2D1A;
		OX3B7C8D1A.modulus_length = OX2D4E8C1B;
		if (OX1A7F4E5C == 1)
			OX3B7C8D1A.usage = SC_PKCS15_PRKEY_USAGE_NONREPUDIATION;
		else if(OX7C5E2D1A > 0)
			OX3B7C8D1A.usage = SC_PKCS15_PRKEY_USAGE_SIGN | SC_PKCS15_PRKEY_USAGE_DERIVE;
		else
			OX3B7C8D1A.usage = SC_PKCS15_PRKEY_USAGE_SIGN | SC_PKCS15_PRKEY_USAGE_ENCRYPT | SC_PKCS15_PRKEY_USAGE_DECRYPT;

		strlcpy(OX4E8C9D2F.label, OX7C1F2A5B[OX1A7F4E5C], sizeof(OX4E8C9D2F.label));
		OX4E8C9D2F.auth_id.len = 1;
		OX4E8C9D2F.auth_id.value[0] = OX8C5D1A2F[OX1A7F4E5C];
		OX4E8C9D2F.user_consent = 0;
		OX4E8C9D2F.flags = SC_PKCS15_CO_FLAG_PRIVATE;

		if(OX7C5E2D1A > 0)
			OX3B7D1E4A = sc_pkcs15emu_add_ec_prkey(OX3E1D7B5A, &OX4E8C9D2F, &OX3B7C8D1A);
		else
			OX3B7D1E4A = sc_pkcs15emu_add_rsa_prkey(OX3E1D7B5A, &OX4E8C9D2F, &OX3B7C8D1A);
		if (OX3B7D1E4A < 0)
			return SC_ERROR_INTERNAL;
	}

	return SC_SUCCESS;
}

static int OX7B4C1E9F(sc_pkcs15_card_t *OX1B2F7D4C)
{
	if (is_esteid_card(OX1B2F7D4C->card))
		return SC_SUCCESS;
	else
		return SC_ERROR_WRONG_CARD;
}

int OX3A4E97F5(sc_pkcs15_card_t *OX3D1F7B2C,
				struct sc_aid *OX3E9C1D7B,
				sc_pkcs15emu_opt_t *OX5D1A7B4C)
{

	if (OX5D1A7B4C && OX5D1A7B4C->flags & SC_PKCS15EMU_FLAGS_NO_CHECK)
		return OX2C9B8D4E(OX3D1F7B2C);
	else {
		int OX3F7A1D8E = OX7B4C1E9F(OX3D1F7B2C);
		if (OX3F7A1D8E)
			return SC_ERROR_WRONG_CARD;
		return OX2C9B8D4E(OX3D1F7B2C);
	}
}