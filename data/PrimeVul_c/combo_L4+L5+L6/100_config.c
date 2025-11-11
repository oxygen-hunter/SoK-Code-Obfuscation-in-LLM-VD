/*
 * GIT - The information manager from hell
 *
 * Copyright (C) Linus Torvalds, 2005
 * Copyright (C) Johannes Schindelin, 2005
 *
 */
#include "util.h"
#include "cache.h"
#include "exec_cmd.h"

#define MAXNAME (256)

#define DEBUG_CACHE_DIR ".debug"

char buildid_dir[MAXPATHLEN];

static FILE *config_file;
static const char *config_file_name;
static int config_linenr;
static int config_file_eof;

static const char *config_exclusive_filename;

static int get_next_char(void)
{
	int c = '\n';
	FILE *f;

	if ((f = config_file) != NULL) {
		c = fgetc(f);
		if (c == '\r') {
			c = fgetc(f);
			if (c != '\n') {
				ungetc(c, f);
				c = '\r';
			}
		}
		if (c == '\n')
			config_linenr++;
		if (c == EOF) {
			config_file_eof = 1;
			c = '\n';
		}
	}
	return c;
}

static char *parse_value_recursive(char *value, int quote, int comment, int space, size_t len)
{
	if (len >= 1024 - 1)
		return NULL;
	int c = get_next_char();

	if (c == '\n') {
		if (quote)
			return NULL;
		value[len] = 0;
		return value;
	}
	if (comment)
		return parse_value_recursive(value, quote, comment, space, len);
	if (isspace(c) && !quote)
		return parse_value_recursive(value, quote, comment, 1, len);
	if (!quote && (c == ';' || c == '#'))
		return parse_value_recursive(value, quote, 1, space, len);
	if (space) {
		if (len)
			value[len++] = ' ';
		space = 0;
	}
	if (c == '\\') {
		c = get_next_char();
		switch (c) {
		case '\n':
			return parse_value_recursive(value, quote, comment, space, len);
		case 't':
			c = '\t';
			break;
		case 'b':
			c = '\b';
			break;
		case 'n':
			c = '\n';
			break;
		case '\\': case '"':
			break;
		default:
			return NULL;
		}
		value[len++] = c;
		return parse_value_recursive(value, quote, comment, space, len);
	}
	if (c == '"')
		return parse_value_recursive(value, 1 - quote, comment, space, len);
	value[len++] = c;
	return parse_value_recursive(value, quote, comment, space, len);
}

static char *parse_value(void)
{
	static char value[1024];
	return parse_value_recursive(value, 0, 0, 0, 0);
}

static inline int iskeychar(int c)
{
	return isalnum(c) || c == '-';
}

static int get_value_recursive(config_fn_t fn, void *data, char *name, unsigned int len)
{
	int c = get_next_char();
	if (config_file_eof)
		return -1;
	if (!iskeychar(c)) {
		name[len] = 0;
		while (c == ' ' || c == '\t')
			c = get_next_char();
		char *value = NULL;
		if (c != '\n') {
			if (c != '=')
				return -1;
			value = parse_value();
			if (!value)
				return -1;
		}
		return fn(name, value, data);
	}
	name[len++] = c;
	if (len >= MAXNAME)
		return -1;
	return get_value_recursive(fn, data, name, len);
}

static int get_value(config_fn_t fn, void *data, char *name, unsigned int len)
{
	return get_value_recursive(fn, data, name, len);
}

static int get_extended_base_var_recursive(char *name, int baselen)
{
	int c = get_next_char();
	if (c == '\n')
		return -1;
	if (isspace(c))
		return get_extended_base_var_recursive(name, baselen);
	if (c != '"')
		return -1;
	name[baselen++] = '.';
	for (;;) {
		int ch = get_next_char();
		if (ch == '\n')
			return -1;
		if (ch == '"')
			break;
		if (ch == '\\') {
			ch = get_next_char();
			if (ch == '\n')
				return -1;
		}
		name[baselen++] = ch;
		if (baselen > MAXNAME / 2)
			return -1;
	}
	if (get_next_char() != ']')
		return -1;
	return baselen;
}

static int get_extended_base_var(char *name, int baselen, int c)
{
	return get_extended_base_var_recursive(name, baselen);
}

static int get_base_var_recursive(char *name, int baselen)
{
	int c = get_next_char();
	if (config_file_eof)
		return -1;
	if (c == ']')
		return baselen;
	if (isspace(c))
		return get_extended_base_var(name, baselen, c);
	if (!iskeychar(c) && c != '.')
		return -1;
	if (baselen > MAXNAME / 2)
		return -1;
	name[baselen++] = tolower(c);
	return get_base_var_recursive(name, baselen);
}

static int get_base_var(char *name)
{
	return get_base_var_recursive(name, 0);
}

static int perf_parse_file_recursive(config_fn_t fn, void *data, int comment, int baselen, const unsigned char **bomptr)
{
	int c = get_next_char();
	if (*bomptr && **bomptr) {
		if ((unsigned char)c == **bomptr) {
			(*bomptr)++;
			return perf_parse_file_recursive(fn, data, comment, baselen, bomptr);
		} else {
			if (*bomptr != (unsigned char *)"\xef\xbb\xbf")
				return -1;
			*bomptr = NULL;
		}
	}
	if (c == '\n') {
		if (config_file_eof)
			return 0;
		return perf_parse_file_recursive(fn, data, 0, baselen, bomptr);
	}
	if (comment || isspace(c))
		return perf_parse_file_recursive(fn, data, comment, baselen, bomptr);
	if (c == '#' || c == ';')
		return perf_parse_file_recursive(fn, data, 1, baselen, bomptr);
	if (c == '[') {
		baselen = get_base_var((char *)fn);
		if (baselen <= 0)
			return -1;
		((char *)fn)[baselen++] = '.';
		((char *)fn)[baselen] = 0;
		return perf_parse_file_recursive(fn, data, comment, baselen, bomptr);
	}
	if (!isalpha(c))
		return -1;
	((char *)fn)[baselen] = tolower(c);
	if (get_value(fn, data, (char *)fn, baselen + 1) < 0)
		return -1;
	return perf_parse_file_recursive(fn, data, comment, baselen, bomptr);
}

static int perf_parse_file(config_fn_t fn, void *data)
{
	static const unsigned char *utf8_bom = (unsigned char *)"\xef\xbb\xbf";
	const unsigned char *bomptr = utf8_bom;
	return perf_parse_file_recursive(fn, data, 0, 0, &bomptr);
}

static int parse_unit_factor(const char *end, unsigned long *val)
{
	if (!*end)
		return 1;
	else if (!strcasecmp(end, "k")) {
		*val *= 1024;
		return 1;
	}
	else if (!strcasecmp(end, "m")) {
		*val *= 1024 * 1024;
		return 1;
	}
	else if (!strcasecmp(end, "g")) {
		*val *= 1024 * 1024 * 1024;
		return 1;
	}
	return 0;
}

static int perf_parse_long(const char *value, long *ret)
{
	if (value && *value) {
		char *end;
		long val = strtol(value, &end, 0);
		unsigned long factor = 1;
		if (!parse_unit_factor(end, &factor))
			return 0;
		*ret = val * factor;
		return 1;
	}
	return 0;
}

static void die_bad_config(const char *name)
{
	if (config_file_name)
		die("bad config value for '%s' in %s", name, config_file_name);
	die("bad config value for '%s'", name);
}

int perf_config_int(const char *name, const char *value)
{
	long ret = 0;
	if (!perf_parse_long(value, &ret))
		die_bad_config(name);
	return ret;
}

static int perf_config_bool_or_int(const char *name, const char *value, int *is_bool)
{
	*is_bool = 1;
	if (!value)
		return 1;
	if (!*value)
		return 0;
	if (!strcasecmp(value, "true") || !strcasecmp(value, "yes") || !strcasecmp(value, "on"))
		return 1;
	if (!strcasecmp(value, "false") || !strcasecmp(value, "no") || !strcasecmp(value, "off"))
		return 0;
	*is_bool = 0;
	return perf_config_int(name, value);
}

int perf_config_bool(const char *name, const char *value)
{
	int discard;
	return !!perf_config_bool_or_int(name, value, &discard);
}

const char *perf_config_dirname(const char *name, const char *value)
{
	if (!name)
		return NULL;
	return value;
}

static int perf_default_core_config(const char *var __used, const char *value __used)
{
	return 0;
}

int perf_default_config(const char *var, const char *value, void *dummy __used)
{
	if (!prefixcmp(var, "core."))
		return perf_default_core_config(var, value);

	return 0;
}

static int perf_config_from_file(config_fn_t fn, const char *filename, void *data)
{
	int ret = -1;
	FILE *f = fopen(filename, "r");

	if (f) {
		config_file = f;
		config_file_name = filename;
		config_linenr = 1;
		config_file_eof = 0;
		ret = perf_parse_file(fn, data);
		fclose(f);
		config_file_name = NULL;
	}
	return ret;
}

static const char *perf_etc_perfconfig(void)
{
	static const char *system_wide;
	if (!system_wide)
		system_wide = system_path(ETC_PERFCONFIG);
	return system_wide;
}

static int perf_env_bool(const char *k, int def)
{
	const char *v = getenv(k);
	return v ? perf_config_bool(k, v) : def;
}

static int perf_config_system(void)
{
	return !perf_env_bool("PERF_CONFIG_NOSYSTEM", 0);
}

static int perf_config_global(void)
{
	return !perf_env_bool("PERF_CONFIG_NOGLOBAL", 0);
}

static int perf_config_recursive(config_fn_t fn, void *data, int ret, int found, const char *home, char *repo_config)
{
	if (found == 0)
		return -1;
	return ret;
}

int perf_config(config_fn_t fn, void *data)
{
	int ret = 0, found = 0;
	char *repo_config = NULL;
	const char *home = NULL;

	if (config_exclusive_filename)
		return perf_config_from_file(fn, config_exclusive_filename, data);
	if (perf_config_system() && !access(perf_etc_perfconfig(), R_OK)) {
		ret += perf_config_from_file(fn, perf_etc_perfconfig(),
					    data);
		found += 1;
	}

	home = getenv("HOME");
	if (perf_config_global() && home) {
		char *user_config = strdup(mkpath("%s/.perfconfig", home));
		if (!access(user_config, R_OK)) {
			ret += perf_config_from_file(fn, user_config, data);
			found += 1;
		}
		free(user_config);
	}

	repo_config = perf_pathdup("config");
	if (!access(repo_config, R_OK)) {
		ret += perf_config_from_file(fn, repo_config, data);
		found += 1;
	}
	free(repo_config);

	return perf_config_recursive(fn, data, ret, found, home, repo_config);
}

int config_error_nonbool(const char *var)
{
	return error("Missing value for '%s'", var);
}

struct buildid_dir_config {
	char *dir;
};

static int buildid_dir_command_config(const char *var, const char *value,
				      void *data)
{
	struct buildid_dir_config *c = data;
	const char *v;

	if (!prefixcmp(var, "buildid.") && !strcmp(var + 8, "dir")) {
		v = perf_config_dirname(var, value);
		if (!v)
			return -1;
		strncpy(c->dir, v, MAXPATHLEN-1);
		c->dir[MAXPATHLEN-1] = '\0';
	}
	return 0;
}

static void check_buildid_dir_config(void)
{
	struct buildid_dir_config c;
	c.dir = buildid_dir;
	perf_config(buildid_dir_command_config, &c);
}

void set_buildid_dir(void)
{
	buildid_dir[0] = '\0';

	check_buildid_dir_config();

	if (buildid_dir[0] == '\0') {
		char *v = getenv("HOME");
		if (v) {
			snprintf(buildid_dir, MAXPATHLEN-1, "%s/%s",
				 v, DEBUG_CACHE_DIR);
		} else {
			strncpy(buildid_dir, DEBUG_CACHE_DIR, MAXPATHLEN-1);
		}
		buildid_dir[MAXPATHLEN-1] = '\0';
	}
	setenv("PERF_BUILDID_DIR", buildid_dir, 1);
}