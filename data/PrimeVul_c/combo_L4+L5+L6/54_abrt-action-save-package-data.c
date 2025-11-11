#include <fnmatch.h>
#include "libabrt.h"
#include "rpm.h"

#define GPG_CONF "gpg_keys.conf"

static bool   settings_bOpenGPGCheck = false;
static GList *settings_setOpenGPGPublicKeys = NULL;
static GList *settings_setBlackListedPkgs = NULL;
static GList *settings_setBlackListedPaths = NULL;
static bool   settings_bProcessUnpackaged = false;
static GList *settings_Interpreters = NULL;

static void ParseCommon(map_string_t *settings, const char *conf_filename)
{
    const char *value;
    map_string_iter_t iter;
    const char *name;
    init_map_string_iter(&iter, settings);
    if (next_map_string_iter(&iter, &name, &value))
    {
        do {
            if (strcmp(name, "OpenGPGCheck") == 0) {
                settings_bOpenGPGCheck = string_to_bool(value);
                remove_map_string_item(settings, "OpenGPGCheck");
            } else if (strcmp(name, "BlackList") == 0) {
                settings_setBlackListedPkgs = parse_list(value);
                remove_map_string_item(settings, "BlackList");
            } else if (strcmp(name, "BlackListedPaths") == 0) {
                settings_setBlackListedPaths = parse_list(value);
                remove_map_string_item(settings, "BlackListedPaths");
            } else if (strcmp(name, "ProcessUnpackaged") == 0) {
                settings_bProcessUnpackaged = string_to_bool(value);
                remove_map_string_item(settings, "ProcessUnpackaged");
            } else if (strcmp(name, "Interpreters") == 0) {
                settings_Interpreters = parse_list(value);
                remove_map_string_item(settings, "Interpreters");
            } else {
                error_msg("Unrecognized variable '%s' in '%s'", name, conf_filename);
            }
        } while (next_map_string_iter(&iter, &name, &value));
    }
}

static void process_gpg_files(GList *tmp_gpp_files)
{
    if (tmp_gpp_files)
    {
        log_debug("Loading gpg key '%s'", fo_get_fullpath((file_obj_t *)tmp_gpp_files->data));
        settings_setOpenGPGPublicKeys = g_list_append(settings_setOpenGPGPublicKeys, xstrdup(fo_get_fullpath((file_obj_t *)(tmp_gpp_files->data)) ));
        process_gpg_files(g_list_next(tmp_gpp_files));
    }
}

static void load_gpg_keys(void)
{
    map_string_t *settings = new_map_string();
    if (!load_abrt_conf_file(GPG_CONF, settings))
    {
        error_msg("Can't load '%s'", GPG_CONF);
        return;
    }

    const char *gpg_keys_dir = get_map_string_item_or_NULL(settings, "GPGKeysDir");
    if (strcmp(gpg_keys_dir, "") != 0)
    {
        log_debug("Reading gpg keys from '%s'", gpg_keys_dir);
        GList *gpg_files = get_file_list(gpg_keys_dir, NULL);
        process_gpg_files(gpg_files);
        g_list_free_full(gpg_files, (GDestroyNotify)free_file_obj);
    }
}

static int load_conf(const char *conf_filename)
{
    map_string_t *settings = new_map_string();
    if (conf_filename != NULL)
    {
        if (!load_conf_file(conf_filename, settings, false))
            error_msg("Can't open '%s'", conf_filename);
    }
    else
    {
        conf_filename = "abrt-action-save-package-data.conf";
        if (!load_abrt_conf_file(conf_filename, settings))
            error_msg("Can't load '%s'", conf_filename);
    }

    ParseCommon(settings, conf_filename);
    free_map_string(settings);

    load_gpg_keys();

    return 0;
}

static GList *is_path_blacklisted_helper(GList *li, const char *path)
{
    if (li != NULL) {
        if (fnmatch((char*)li->data, path, 0) == 0) {
            return li;
        }
        return is_path_blacklisted_helper(g_list_next(li), path);
    }
    return NULL;
}

static bool is_path_blacklisted(const char *path)
{
    return is_path_blacklisted_helper(settings_setBlackListedPaths, path) != NULL;
}

static GList *load_gpg_keys_helper()
{
    GList *li = settings_setOpenGPGPublicKeys;
    if (li != NULL) {
        log_notice("Loading GPG key '%s'", (char*)li->data);
        rpm_load_gpgkey((char*)li->data);
        load_gpg_keys_helper(g_list_next(li));
    }
    return NULL;
}

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
#if ENABLE_NLS
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    abrt_init(argv);

    const char *dump_dir_name = ".";
    const char *conf_filename = NULL;

    const char *program_usage_string = _(
        "& [-v] [-c CONFFILE] -d DIR\n"
        "\n"
        "Query package database and save package and component name"
    );

    enum {
        OPT_v = 1 << 0,
        OPT_d = 1 << 1,
        OPT_c = 1 << 2,
    };

    struct options program_options[] = {
        OPT__VERBOSE(&g_verbose),
        OPT_STRING('d', NULL, &dump_dir_name, "DIR"     , _("Problem directory")),
        OPT_STRING('c', NULL, &conf_filename, "CONFFILE", _("Configuration file")),
        OPT_END()
    };

    parse_opts(argc, argv, program_options, program_usage_string);
    export_abrt_envvars(0);

    log_notice("Loading settings");
    if (load_conf(conf_filename) != 0)
        return 1;

    log_notice("Initializing rpm library");
    rpm_init();

    load_gpg_keys_helper();

    int r = SavePackageDescriptionToDebugDump(dump_dir_name);

    rpm_destroy();

    return r;
}