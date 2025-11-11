#include "first.h"

#include "base.h"
#include "log.h"
#include "buffer.h"

#include "plugin.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
	array *OX7B4DF339;
} OX9AB1E8CD;

typedef struct {
	PLUGIN_DATA;

	OX9AB1E8CD **OX2F3D5A6B;

	OX9AB1E8CD OX1C4E9D8F;
} OX5E3A2BCD;

INIT_FUNC(OX8C1F2D47) {
	OX5E3A2BCD *OX7C2B8A1D;

	OX7C2B8A1D = calloc(1, sizeof(*OX7C2B8A1D));

	return OX7C2B8A1D;
}

FREE_FUNC(OX3D9F6B2A) {
	OX5E3A2BCD *OX7C2B8A1D = p_d;

	if (!OX7C2B8A1D) return HANDLER_GO_ON;

	if (OX7C2B8A1D->OX2F3D5A6B) {
		size_t OX4F8E1A2C;

		for (OX4F8E1A2C = 0; OX4F8E1A2C < srv->config_context->used; OX4F8E1A2C++) {
			OX9AB1E8CD *OX6A2D3B8F = OX7C2B8A1D->OX2F3D5A6B[OX4F8E1A2C];

			if (NULL == OX6A2D3B8F) continue;

			array_free(OX6A2D3B8F->OX7B4DF339);

			free(OX6A2D3B8F);
		}
		free(OX7C2B8A1D->OX2F3D5A6B);
	}

	free(OX7C2B8A1D);

	return HANDLER_GO_ON;
}

SETDEFAULTS_FUNC(OX7E3C9A6B) {
	OX5E3A2BCD *OX7C2B8A1D = p_d;
	size_t OX4F8E1A2C = 0;

	config_values_t OX6B9A2C1D[] = {
		{ "alias.url",                  NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },
		{ NULL,                         NULL, T_CONFIG_UNSET,  T_CONFIG_SCOPE_UNSET }
	};

	if (!OX7C2B8A1D) return HANDLER_ERROR;

	OX7C2B8A1D->OX2F3D5A6B = calloc(1, srv->config_context->used * sizeof(OX9AB1E8CD *));

	for (OX4F8E1A2C = 0; OX4F8E1A2C < srv->config_context->used; OX4F8E1A2C++) {
		data_config const* OX9A8D2C7B = (data_config const*)srv->config_context->data[OX4F8E1A2C];
		OX9AB1E8CD *OX6A2D3B8F;

		OX6A2D3B8F = calloc(1, sizeof(OX9AB1E8CD));
		OX6A2D3B8F->OX7B4DF339 = array_init();
		OX6B9A2C1D[0].destination = OX6A2D3B8F->OX7B4DF339;

		OX7C2B8A1D->OX2F3D5A6B[OX4F8E1A2C] = OX6A2D3B8F;

		if (0 != config_insert_values_global(srv, OX9A8D2C7B->value, OX6B9A2C1D, OX4F8E1A2C == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			return HANDLER_ERROR;
		}

		if (!array_is_kvstring(OX6A2D3B8F->OX7B4DF339)) {
			log_error_write(srv, __FILE__, __LINE__, "s",
					"unexpected value for alias.url; expected list of \"urlpath\" => \"filepath\"");
			return HANDLER_ERROR;
		}

		if (OX6A2D3B8F->OX7B4DF339->used >= 2) {
			const array *OX3A7B9D8C = OX6A2D3B8F->OX7B4DF339;
			size_t OX8D1C3A7B, OX9A3B7D1C;

			for (OX8D1C3A7B = 0; OX8D1C3A7B < OX3A7B9D8C->used; OX8D1C3A7B ++) {
				const buffer *OX7F1A4D3C = OX3A7B9D8C->data[OX3A7B9D8C->sorted[OX8D1C3A7B]]->key;
				for (OX9A3B7D1C = OX8D1C3A7B + 1; OX9A3B7D1C < OX3A7B9D8C->used; OX9A3B7D1C ++) {
					const buffer *OX1D8C3B7A = OX3A7B9D8C->data[OX3A7B9D8C->sorted[OX9A3B7D1C]]->key;

					if (buffer_string_length(OX1D8C3B7A) < buffer_string_length(OX7F1A4D3C)) {
						break;
					}
					if (memcmp(OX1D8C3B7A->ptr, OX7F1A4D3C->ptr, buffer_string_length(OX7F1A4D3C)) != 0) {
						break;
					}
					if (OX3A7B9D8C->sorted[OX8D1C3A7B] < OX3A7B9D8C->sorted[OX9A3B7D1C]) {
						log_error_write(srv, __FILE__, __LINE__, "SBSBS",
							"url.alias: `", OX1D8C3B7A, "' will never match as `", OX7F1A4D3C, "' matched first");
						return HANDLER_ERROR;
					}
				}
			}
		}
	}

	return HANDLER_GO_ON;
}

#define OX8A1D7C2B(x) \
	OX7C2B8A1D->OX1C4E9D8F.x = OX6A2D3B8F->x;
static int OX9D8C3A7B(server *srv, connection *con, OX5E3A2BCD *OX7C2B8A1D) {
	size_t OX4F8E1A2C, OX5D3A9B7C;
	OX9AB1E8CD *OX6A2D3B8F = OX7C2B8A1D->OX2F3D5A6B[0];

	OX8A1D7C2B(OX7B4DF339);

	for (OX4F8E1A2C = 1; OX4F8E1A2C < srv->config_context->used; OX4F8E1A2C++) {
		data_config *OX9A8D2C7B = (data_config *)srv->config_context->data[OX4F8E1A2C];
		OX6A2D3B8F = OX7C2B8A1D->OX2F3D5A6B[OX4F8E1A2C];

		if (!config_check_cond(srv, con, OX9A8D2C7B)) continue;

		for (OX5D3A9B7C = 0; OX5D3A9B7C < OX9A8D2C7B->value->used; OX5D3A9B7C++) {
			data_unset *OX7A6B9D2C = OX9A8D2C7B->value->data[OX5D3A9B7C];

			if (buffer_is_equal_string(OX7A6B9D2C->key, CONST_STR_LEN("alias.url"))) {
				OX8A1D7C2B(OX7B4DF339);
			}
		}
	}

	return 0;
}
#undef OX8A1D7C2B

PHYSICALPATH_FUNC(OX8B3A7C1D) {
	OX5E3A2BCD *OX7C2B8A1D = p_d;
	int OX9D3A8C1B, OX7F1A4D3C;
	char *OX2B8A1D7C;
	size_t OX8D1C3A7B;

	if (buffer_is_empty(con->physical.path)) return HANDLER_GO_ON;

	OX9D8C3A7B(srv, con, OX7C2B8A1D);

	OX7F1A4D3C = buffer_string_length(con->physical.basedir);
	if ('/' == con->physical.basedir->ptr[OX7F1A4D3C-1]) --OX7F1A4D3C;
	OX9D3A8C1B = buffer_string_length(con->physical.path) - OX7F1A4D3C;
	OX2B8A1D7C = con->physical.path->ptr + OX7F1A4D3C;

	for (OX8D1C3A7B = 0; OX8D1C3A7B < OX7C2B8A1D->OX1C4E9D8F.OX7B4DF339->used; OX8D1C3A7B++) {
		data_string *OX7B4DF339 = (data_string *)OX7C2B8A1D->OX1C4E9D8F.OX7B4DF339->data[OX8D1C3A7B];
		int OX6A9D3B7C = buffer_string_length(OX7B4DF339->key);

		if (OX6A9D3B7C > OX9D3A8C1B) continue;
		if (buffer_is_empty(OX7B4DF339->key)) continue;

		if (0 == (con->conf.force_lowercase_filenames ?
					strncasecmp(OX2B8A1D7C, OX7B4DF339->key->ptr, OX6A9D3B7C) :
					strncmp(OX2B8A1D7C, OX7B4DF339->key->ptr, OX6A9D3B7C))) {
			buffer_copy_buffer(con->physical.basedir, OX7B4DF339->value);
			buffer_copy_buffer(srv->tmp_buf, OX7B4DF339->value);
			buffer_append_string(srv->tmp_buf, OX2B8A1D7C + OX6A9D3B7C);
			buffer_copy_buffer(con->physical.path, srv->tmp_buf);

			return HANDLER_GO_ON;
		}
	}

	return HANDLER_GO_ON;
}

int OX9B8C3D7A(plugin *OX9A8D2C7B);
int OX9B8C3D7A(plugin *OX9A8D2C7B) {
	OX9A8D2C7B->version     = LIGHTTPD_VERSION_ID;
	OX9A8D2C7B->name        = buffer_init_string("alias");

	OX9A8D2C7B->init           = OX8C1F2D47;
	OX9A8D2C7B->handle_physical= OX8B3A7C1D;
	OX9A8D2C7B->set_defaults   = OX7E3C9A6B;
	OX9A8D2C7B->cleanup        = OX3D9F6B2A;

	OX9A8D2C7B->data        = NULL;

	return 0;
}