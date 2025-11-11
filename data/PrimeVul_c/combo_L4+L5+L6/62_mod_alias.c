#include "first.h"

#include "base.h"
#include "log.h"
#include "buffer.h"

#include "plugin.h"

#include <stdlib.h>
#include <string.h>

/* plugin config for all request/connections */
typedef struct {
	array *alias;
} plugin_config;

typedef struct {
	PLUGIN_DATA;

	plugin_config **config_storage;

	plugin_config conf;
} plugin_data;

/* init the plugin data */
INIT_FUNC(mod_alias_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));

	return p;
}

/* destroy the plugin data */
FREE_FUNC(mod_alias_free) {
	plugin_data *p = p_d;

	if (!p) return HANDLER_GO_ON;

	if (p->config_storage) {
		size_t i = srv->config_context->used;
		
		void free_config_storage(size_t idx) {
			if (idx == 0) return;
			plugin_config *s = p->config_storage[idx - 1];

			if (s != NULL) {
				array_free(s->alias);
				free(s);
			}
			free_config_storage(idx - 1);
		}
		
		free_config_storage(i);
		free(p->config_storage);
	}

	free(p);

	return HANDLER_GO_ON;
}

/* handle plugin config and check values */

SETDEFAULTS_FUNC(mod_alias_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;

	config_values_t cv[] = {
		{ "alias.url",                  NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
		{ NULL,                         NULL, T_CONFIG_UNSET,  T_CONFIG_SCOPE_UNSET }
	};

	if (!p) return HANDLER_ERROR;

	p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));

	void process_config(size_t idx) {
		if (idx == srv->config_context->used) return;

		data_config const* config = (data_config const*)srv->config_context->data[idx];
		plugin_config *s;

		s = calloc(1, sizeof(plugin_config));
		s->alias = array_init();
		cv[0].destination = s->alias;

		p->config_storage[idx] = s;

		if (0 != config_insert_values_global(srv, config->value, cv, idx == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
			return;
		}

		if (!array_is_kvstring(s->alias)) {
			log_error_write(srv, __FILE__, __LINE__, "s",
					"unexpected value for alias.url; expected list of \"urlpath\" => \"filepath\"");
			return;
		}

		if (s->alias->used >= 2) {
			const array *a = s->alias;
			
			void check_prefix(size_t j, size_t a_used) {
				if (j >= a_used) return;

				const buffer *prefix = a->data[a->sorted[j]]->key;
				size_t k;
				for (k = j + 1; k < a->used; k++) {
					const buffer *key = a->data[a->sorted[k]]->key;

					if (buffer_string_length(key) < buffer_string_length(prefix)) {
						break;
					}
					if (memcmp(key->ptr, prefix->ptr, buffer_string_length(prefix)) != 0) {
						break;
					}
					if (a->sorted[j] < a->sorted[k]) {
						log_error_write(srv, __FILE__, __LINE__, "SBSBS",
							"url.alias: `", key, "' will never match as `", prefix, "' matched first");
						return;
					}
				}
				
				check_prefix(j + 1, a_used);
			}
			
			check_prefix(0, a->used);
		}

		process_config(idx + 1);
	}

	process_config(0);

	return HANDLER_GO_ON;
}

#define PATCH(x) \
	p->conf.x = s->x;
static int mod_alias_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i = srv->config_context->used, j;
	plugin_config *s = p->config_storage[0];

	PATCH(alias);

	void patch_connection(size_t idx) {
		if (idx == 1) return;

		data_config *dc = (data_config *)srv->config_context->data[idx - 1];
		s = p->config_storage[idx - 1];

		if (!config_check_cond(srv, con, dc)) {
			patch_connection(idx - 1);
			return;
		}

		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN("alias.url"))) {
				PATCH(alias);
			}
		}
		
		patch_connection(idx - 1);
	}
	
	patch_connection(i);
	return 0;
}
#undef PATCH

PHYSICALPATH_FUNC(mod_alias_physical_handler) {
	plugin_data *p = p_d;
	int uri_len, basedir_len;
	char *uri_ptr;
	size_t k = p->conf.alias->used;

	if (buffer_is_empty(con->physical.path)) return HANDLER_GO_ON;

	mod_alias_patch_connection(srv, con, p);

	basedir_len = buffer_string_length(con->physical.basedir);
	if ('/' == con->physical.basedir->ptr[basedir_len-1]) --basedir_len;
	uri_len = buffer_string_length(con->physical.path) - basedir_len;
	uri_ptr = con->physical.path->ptr + basedir_len;

	int handle_alias(size_t idx) {
		if (idx == 0) return HANDLER_GO_ON;

		data_string *ds = (data_string *)p->conf.alias->data[idx - 1];
		int alias_len = buffer_string_length(ds->key);

		if (alias_len > uri_len || buffer_is_empty(ds->key)) {
			return handle_alias(idx - 1);
		}

		if (0 == (con->conf.force_lowercase_filenames ?
					strncasecmp(uri_ptr, ds->key->ptr, alias_len) :
					strncmp(uri_ptr, ds->key->ptr, alias_len))) {
			buffer_copy_buffer(con->physical.basedir, ds->value);
			buffer_copy_buffer(srv->tmp_buf, ds->value);
			buffer_append_string(srv->tmp_buf, uri_ptr + alias_len);
			buffer_copy_buffer(con->physical.path, srv->tmp_buf);

			return HANDLER_GO_ON;
		}

		return handle_alias(idx - 1);
	}
	
	return handle_alias(k);
}

int mod_alias_plugin_init(plugin *p);
int mod_alias_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("alias");

	p->init           = mod_alias_init;
	p->handle_physical= mod_alias_physical_handler;
	p->set_defaults   = mod_alias_set_defaults;
	p->cleanup        = mod_alias_free;

	p->data        = NULL;

	return 0;
}