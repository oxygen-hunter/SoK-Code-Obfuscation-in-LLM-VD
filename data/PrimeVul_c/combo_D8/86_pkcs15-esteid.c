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

int sc_pkcs15emu_esteid_init_ex(sc_pkcs15_card_t *, struct sc_aid *, sc_pkcs15emu_opt_t *);

static void set_string (char **strp, const char *value) {
	if (*strp) free (*strp);
	*strp = value ? strdup (value) : NULL;
}

int select_esteid_df (sc_card_t * card) {
	int r;
	sc_path_t tmppath;
	sc_format_path ("3F00EEEE", &tmppath);
	r = sc_select_file (card, &tmppath, NULL);
	SC_TEST_RET(card->ctx, SC_LOG_DEBUG_NORMAL, r, "esteid select DF failed");
	return r;
}

static int sc_pkcs15emu_esteid_init (sc_pkcs15_card_t * p15card) {
	sc_card_t *card = p15card->card;
	unsigned char buff[128];
	int r, i;
	size_t field_length = 0, modulus_length = 0;
	sc_path_t tmppath;

	set_string (&p15card->tokeninfo->label, "ID-kaart");
	set_string (&p15card->tokeninfo->manufacturer_id, "AS Sertifitseerimiskeskus");

	sc_format_path ("3f00eeee5044", &tmppath);
	r = sc_select_file (card, &tmppath, NULL);
	SC_TEST_RET(card->ctx, SC_LOG_DEBUG_NORMAL, r, "select esteid PD failed");

	r = sc_read_record (card, SC_ESTEID_PD_DOCUMENT_NR, buff, sizeof(buff), SC_RECORD_BY_REC_NR);
	SC_TEST_RET(card->ctx, SC_LOG_DEBUG_NORMAL, r, "read document number failed");
	buff[r] = '\0';
	set_string (&p15card->tokeninfo->serial_number, (const char *) buff);

	p15card->tokeninfo->flags = SC_PKCS15_TOKEN_PRN_GENERATION
				  | SC_PKCS15_TOKEN_EID_COMPLIANT
				  | SC_PKCS15_TOKEN_READONLY;

	for (i = 0; i < 2; i++) {
		const char *getEsteidCertName(int idx) { return idx == 0 ? "Isikutuvastus" : "Allkirjastamine"; }
		const char *getEsteidCertPath(int idx) { return idx == 0 ? "3f00eeeeaace" : "3f00eeeeddce"; }
		int getEsteidCertId(int idx) { return idx + 1; }

		struct sc_pkcs15_cert_info cert_info;
		struct sc_pkcs15_object cert_obj;

		memset(&cert_info, 0, sizeof(cert_info));
		memset(&cert_obj, 0, sizeof(cert_obj));

		cert_info.id.value[0] = getEsteidCertId(i);
		cert_info.id.len = 1;
		sc_format_path(getEsteidCertPath(i), &cert_info.path);
		strlcpy(cert_obj.label, getEsteidCertName(i), sizeof(cert_obj.label));
		r = sc_pkcs15emu_add_x509_cert(p15card, &cert_obj, &cert_info);
		if (r < 0)
			return SC_ERROR_INTERNAL;
		if (i == 0) {
			sc_pkcs15_cert_t *cert = NULL;
			r = sc_pkcs15_read_certificate(p15card, &cert_info, &cert);
			if (r < 0)
				return SC_ERROR_INTERNAL;
			if (cert->key->algorithm == SC_ALGORITHM_EC)
				field_length = cert->key->u.ec.params.field_length;
			else
				modulus_length = cert->key->u.rsa.modulus.len * 8;
			if (r == SC_SUCCESS) {
				static const struct sc_object_id cn_oid = {{ 2, 5, 4, 3, -1 }};
				u8 *cn_name = NULL;
				size_t cn_len = 0;
				sc_pkcs15_get_name_from_dn(card->ctx, cert->subject,
					cert->subject_len, &cn_oid, &cn_name, &cn_len);
				if (cn_len > 0) {
					char *token_name = malloc(cn_len+1);
					if (token_name) {
						memcpy(token_name, cn_name, cn_len);
						token_name[cn_len] = '\0';
						set_string(&p15card->tokeninfo->label, (const char*)token_name);
						free(token_name);
					}
				}
				free(cn_name);
				sc_pkcs15_free_certificate(cert);
			}
		}
	}

	sc_format_path ("3f000016", &tmppath);
	r = sc_select_file (card, &tmppath, NULL);
	if (r < 0)
		return SC_ERROR_INTERNAL;

	for (i = 0; i < 3; i++) {
		unsigned char tries_left;
		const char *getEsteidPinName(int idx) { return idx == 0 ? "PIN1" : (idx == 1 ? "PIN2" : "PUK"); }
		int getEsteidPinMin(int idx) { return idx == 0 ? 4 : (idx == 1 ? 5 : 8); }
		int getEsteidPinRef(int idx) { return idx == 0 ? 1 : (idx == 1 ? 2 : 0); }
		int getEsteidPinAuthId(int idx) { return idx + 1; }
		int getEsteidPinFlags(int idx) { return idx == 2 ? SC_PKCS15_PIN_FLAG_UNBLOCKING_PIN : 0; }

		struct sc_pkcs15_auth_info pin_info;
		struct sc_pkcs15_object pin_obj;

		memset(&pin_info, 0, sizeof(pin_info));
		memset(&pin_obj, 0, sizeof(pin_obj));

		r = sc_read_record (card, i + 1, buff, sizeof(buff), SC_RECORD_BY_REC_NR);
		if (r < 0)
			return SC_ERROR_INTERNAL;
		tries_left = buff[5];

		pin_info.auth_id.len = 1;
		pin_info.auth_id.value[0] = getEsteidPinAuthId(i);
		pin_info.auth_type = SC_PKCS15_PIN_AUTH_TYPE_PIN;
		pin_info.attrs.pin.reference = getEsteidPinRef(i);
		pin_info.attrs.pin.flags = getEsteidPinFlags(i);
		pin_info.attrs.pin.type = SC_PKCS15_PIN_TYPE_ASCII_NUMERIC;
		pin_info.attrs.pin.min_length = getEsteidPinMin(i);
		pin_info.attrs.pin.stored_length = 12;
		pin_info.attrs.pin.max_length = 12;
		pin_info.attrs.pin.pad_char = '\0';
		pin_info.tries_left = (int)tries_left;
		pin_info.max_tries = 3;

		strlcpy(pin_obj.label, getEsteidPinName(i), sizeof(pin_obj.label));
		pin_obj.flags = getEsteidPinFlags(i);

		if (i < 2) {
			pin_obj.auth_id.len = 1;
			pin_obj.auth_id.value[0] = 3;
		}

		r = sc_pkcs15emu_add_pin_obj(p15card, &pin_obj, &pin_info);
		if (r < 0)
			return SC_ERROR_INTERNAL;
	}

	for (i = 0; i < 2; i++) {
		int getPrkeyPin(int idx) { return idx + 1; }
		const char *getPrkeyName(int idx) { return idx == 0 ? "Isikutuvastus" : "Allkirjastamine"; }

		struct sc_pkcs15_prkey_info prkey_info;
		struct sc_pkcs15_object prkey_obj;

		memset(&prkey_info, 0, sizeof(prkey_info));
		memset(&prkey_obj, 0, sizeof(prkey_obj));

		prkey_info.id.len = 1;
		prkey_info.id.value[0] = getPrkeyPin(i);
		prkey_info.native = 1;
		prkey_info.key_reference = getPrkeyPin(i);
		prkey_info.field_length = field_length;
		prkey_info.modulus_length = modulus_length;
		if (i == 1)
			prkey_info.usage = SC_PKCS15_PRKEY_USAGE_NONREPUDIATION;
		else if(field_length > 0)
			prkey_info.usage = SC_PKCS15_PRKEY_USAGE_SIGN | SC_PKCS15_PRKEY_USAGE_DERIVE;
		else
			prkey_info.usage = SC_PKCS15_PRKEY_USAGE_SIGN | SC_PKCS15_PRKEY_USAGE_ENCRYPT | SC_PKCS15_PRKEY_USAGE_DECRYPT;

		strlcpy(prkey_obj.label, getPrkeyName(i), sizeof(prkey_obj.label));
		prkey_obj.auth_id.len = 1;
		prkey_obj.auth_id.value[0] = getPrkeyPin(i);
		prkey_obj.user_consent = 0;
		prkey_obj.flags = SC_PKCS15_CO_FLAG_PRIVATE;

		if(field_length > 0)
			r = sc_pkcs15emu_add_ec_prkey(p15card, &prkey_obj, &prkey_info);
		else
			r = sc_pkcs15emu_add_rsa_prkey(p15card, &prkey_obj, &prkey_info);
		if (r < 0)
			return SC_ERROR_INTERNAL;
	}

	return SC_SUCCESS;
}

static int esteid_detect_card(sc_pkcs15_card_t *p15card) {
	if (is_esteid_card(p15card->card))
		return SC_SUCCESS;
	else
		return SC_ERROR_WRONG_CARD;
}

int sc_pkcs15emu_esteid_init_ex(sc_pkcs15_card_t *p15card, struct sc_aid *aid, sc_pkcs15emu_opt_t *opts) {
	if (opts && opts->flags & SC_PKCS15EMU_FLAGS_NO_CHECK)
		return sc_pkcs15emu_esteid_init(p15card);
	else {
		int r = esteid_detect_card(p15card);
		if (r)
			return SC_ERROR_WRONG_CARD;
		return sc_pkcs15emu_esteid_init(p15card);
	}
}