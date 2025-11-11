#include "first.h"

#include "base.h"
#include "log.h"
#include "buffer.h"

#include "plugin.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
	array *alias;
} plugin_config;

typedef struct {
	PLUGIN_DATA;

	plugin_config **config_storage;

	plugin_config conf;
} plugin_data;

INIT_FUNC(mod_alias_init) {
	plugin_data *p;
	int dispatch = 0;
	while (1) {
		switch (dispatch) {
			case 0:
				p = calloc(1, sizeof(*p));
				dispatch = 1;
				break;
			case 1:
				return p;
		}
	}
}

FREE_FUNC(mod_alias_free) {
	plugin_data *p = p_d;
	size_t i;
	plugin_config *s;
	int dispatch = 0;
	while (1) {
		switch (dispatch) {
			case 0:
				if (!p) return HANDLER_GO_ON;
				dispatch = 1;
				break;
			case 1:
				if (p->config_storage) {
					i = 0;
					dispatch = 2;
				} else {
					dispatch = 4;
				}
				break;
			case 2:
				if (i < srv->config_context->used) {
					s = p->config_storage[i];
					if (NULL == s) {
						i++;
						dispatch = 2;
					} else {
						dispatch = 3;
					}
				} else {
					dispatch = 4;
				}
				break;
			case 3:
				array_free(s->alias);
				free(s);
				i++;
				dispatch = 2;
				break;
			case 4:
				free(p->config_storage);
				free(p);
				return HANDLER_GO_ON;
		}
	}
}

SETDEFAULTS_FUNC(mod_alias_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;
	int dispatch = 0;
	config_values_t cv[] = {
		{ "alias.url",                  NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },       
		{ NULL,                         NULL, T_CONFIG_UNSET,  T_CONFIG_SCOPE_UNSET }
	};

	while (1) {
		data_config const* config;
		plugin_config *s;
		const array *a;
		size_t j, k;
		const buffer *prefix, *key;
		switch (dispatch) {
			case 0:
				if (!p) return HANDLER_ERROR;
				dispatch = 1;
				break;
			case 1:
				p->config_storage = calloc(1, srv->config_context->used * sizeof(plugin_config *));
				i = 0;
				dispatch = 2;
				break;
			case 2:
				if (i < srv->config_context->used) {
					config = (data_config const*)srv->config_context->data[i];
					s = calloc(1, sizeof(plugin_config));
					s->alias = array_init();
					cv[0].destination = s->alias;
					p->config_storage[i] = s;
					if (0 != config_insert_values_global(srv, config->value, cv, i == 0 ? T_CONFIG_SCOPE_SERVER : T_CONFIG_SCOPE_CONNECTION)) {
						return HANDLER_ERROR;
					}
					if (!array_is_kvstring(s->alias)) {
						log_error_write(srv, __FILE__, __LINE__, "s",
								"unexpected value for alias.url; expected list of \"urlpath\" => \"filepath\"");
						return HANDLER_ERROR;
					}
					if (s->alias->used >= 2) {
						a = s->alias;
						j = 0;
						dispatch = 3;
					} else {
						i++;
						dispatch = 2;
					}
				} else {
					return HANDLER_GO_ON;
				}
				break;
			case 3:
				if (j < a->used) {
					prefix = a->data[a->sorted[j]]->key;
					k = j + 1;
					dispatch = 4;
				} else {
					i++;
					dispatch = 2;
				}
				break;
			case 4:
				if (k < a->used) {
					key = a->data[a->sorted[k]]->key;
					if (buffer_string_length(key) < buffer_string_length(prefix)) {
						j++;
						dispatch = 3;
					} else if (memcmp(key->ptr, prefix->ptr, buffer_string_length(prefix)) != 0) {
						j++;
						dispatch = 3;
					} else if (a->sorted[j] < a->sorted[k]) {
						log_error_write(srv, __FILE__, __LINE__, "SBSBS",
							"url.alias: `", key, "' will never match as `", prefix, "' matched first");
						return HANDLER_ERROR;
					} else {
						k++;
						dispatch = 4;
					}
				} else {
					j++;
					dispatch = 3;
				}
				break;
		}
	}
}

#define PATCH(x) \
	p->conf.x = s->x;
static int mod_alias_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i = 0, j;
	plugin_config *s;
	data_config *dc;
	data_unset *du;
	int dispatch = 0;
	while (1) {
		switch (dispatch) {
			case 0:
				s = p->config_storage[0];
				PATCH(alias);
				dispatch = 1;
				break;
			case 1:
				if (++i < srv->config_context->used) {
					dc = (data_config *)srv->config_context->data[i];
					s = p->config_storage[i];
					if (!config_check_cond(srv, con, dc)) {
						dispatch = 1;
					} else {
						j = 0;
						dispatch = 2;
					}
				} else {
					return 0;
				}
				break;
			case 2:
				if (j < dc->value->used) {
					du = dc->value->data[j];
					if (buffer_is_equal_string(du->key, CONST_STR_LEN("alias.url"))) {
						PATCH(alias);
					}
					j++;
					dispatch = 2;
				} else {
					dispatch = 1;
				}
				break;
		}
	}
}
#undef PATCH

PHYSICALPATH_FUNC(mod_alias_physical_handler) {
	plugin_data *p = p_d;
	int uri_len, basedir_len, alias_len;
	char *uri_ptr;
	size_t k = 0;
	data_string *ds;
	int dispatch = 0;

	while (1) {
		switch (dispatch) {
			case 0:
				if (buffer_is_empty(con->physical.path)) return HANDLER_GO_ON;
				dispatch = 1;
				break;
			case 1:
				mod_alias_patch_connection(srv, con, p);
				dispatch = 2;
				break;
			case 2:
				basedir_len = buffer_string_length(con->physical.basedir);
				if ('/' == con->physical.basedir->ptr[basedir_len-1]) --basedir_len;
				uri_len = buffer_string_length(con->physical.path) - basedir_len;
				uri_ptr = con->physical.path->ptr + basedir_len;
				dispatch = 3;
				break;
			case 3:
				if (k < p->conf.alias->used) {
					ds = (data_string *)p->conf.alias->data[k];
					alias_len = buffer_string_length(ds->key);
					if (alias_len > uri_len || buffer_is_empty(ds->key)) {
						k++;
						dispatch = 3;
					} else {
						dispatch = 4;
					}
				} else {
					return HANDLER_GO_ON;
				}
				break;
			case 4:
				if (0 == (con->conf.force_lowercase_filenames ?
						strncasecmp(uri_ptr, ds->key->ptr, alias_len) :
						strncmp(uri_ptr, ds->key->ptr, alias_len))) {
					buffer_copy_buffer(con->physical.basedir, ds->value);
					buffer_copy_buffer(srv->tmp_buf, ds->value);
					buffer_append_string(srv->tmp_buf, uri_ptr + alias_len);
					buffer_copy_buffer(con->physical.path, srv->tmp_buf);
					return HANDLER_GO_ON;
				} else {
					k++;
					dispatch = 3;
				}
				break;
		}
	}
}

int mod_alias_plugin_init(plugin *p);
int mod_alias_plugin_init(plugin *p) {
	int dispatch = 0;
	while (1) {
		switch (dispatch) {
			case 0:
				p->version     = LIGHTTPD_VERSION_ID;
				p->name        = buffer_init_string("alias");
				dispatch = 1;
				break;
			case 1:
				p->init           = mod_alias_init;
				p->handle_physical= mod_alias_physical_handler;
				p->set_defaults   = mod_alias_set_defaults;
				p->cleanup        = mod_alias_free;
				dispatch = 2;
				break;
			case 2:
				p->data        = NULL;
				return 0;
		}
	}
}