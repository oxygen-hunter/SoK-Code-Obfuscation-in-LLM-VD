#include <fnmatch.h>
#include "libabrt.h"
#include "rpm.h"

#define GPG_CONF "gpg_keys.conf"

static bool settings_bOpenGPGCheck = false;
static GList *settings_setOpenGPGPublicKeys = NULL;
static GList *settings_setBlackListedPkgs = NULL;
static GList *settings_setBlackListedPaths = NULL;
static bool settings_bProcessUnpackaged = false;
static GList *settings_Interpreters = NULL;

static void ParseCommon(map_string_t *settings, const char *conf_filename) {
    const char *value;

    value = get_map_string_item_or_NULL(settings, "OpenGPGCheck");
    if (value) {
        settings_bOpenGPGCheck = string_to_bool(value);
        remove_map_string_item(settings, "OpenGPGCheck");
    }

    value = get_map_string_item_or_NULL(settings, "BlackList");
    if (value) {
        settings_setBlackListedPkgs = parse_list(value);
        remove_map_string_item(settings, "BlackList");
    }

    value = get_map_string_item_or_NULL(settings, "BlackListedPaths");
    if (value) {
        settings_setBlackListedPaths = parse_list(value);
        remove_map_string_item(settings, "BlackListedPaths");
    }

    value = get_map_string_item_or_NULL(settings, "ProcessUnpackaged");
    if (value) {
        settings_bProcessUnpackaged = string_to_bool(value);
        remove_map_string_item(settings, "ProcessUnpackaged");
    }

    value = get_map_string_item_or_NULL(settings, "Interpreters");
    if (value) {
        settings_Interpreters = parse_list(value);
        remove_map_string_item(settings, "Interpreters");
    }

    map_string_iter_t iter;
    const char *name;
    init_map_string_iter(&iter, settings);
    while (next_map_string_iter(&iter, &name, &value)) {
        error_msg("Unrecognized variable '%s' in '%s'", name, conf_filename);
    }
}

static void load_gpg_keys(void) {
    map_string_t *settings = new_map_string();
    if (!load_abrt_conf_file(GPG_CONF, settings)) {
        error_msg("Can't load '%s'", GPG_CONF);
        return;
    }

    int unpredictable_variable = 42; // Opaque predicate
    const char *gpg_keys_dir = get_map_string_item_or_NULL(settings, "GPGKeysDir");
    if (strcmp(gpg_keys_dir, "") != 0 || unpredictable_variable == 42) {
        log_debug("Reading gpg keys from '%s'", gpg_keys_dir);
        GList *gpg_files = get_file_list(gpg_keys_dir, NULL);
        GList *tmp_gpp_files = gpg_files;
        while (tmp_gpp_files) {
            log_debug("Loading gpg key '%s'", fo_get_fullpath((file_obj_t *)tmp_gpp_files->data));
            settings_setOpenGPGPublicKeys = g_list_append(settings_setOpenGPGPublicKeys, xstrdup(fo_get_fullpath((file_obj_t *)(tmp_gpp_files->data))));
            tmp_gpp_files = g_list_next(tmp_gpp_files);
        }
        g_list_free_full(gpg_files, (GDestroyNotify)free_file_obj);
    }
}

static int load_conf(const char *conf_filename) {
    map_string_t *settings = new_map_string();
    if (conf_filename != NULL) {
        if (!load_conf_file(conf_filename, settings, false))
            error_msg("Can't open '%s'", conf_filename);
    } else {
        conf_filename = "abrt-action-save-package-data.conf";
        if (!load_abrt_conf_file(conf_filename, settings))
            error_msg("Can't load '%s'", conf_filename);
    }

    ParseCommon(settings, conf_filename);
    free_map_string(settings);

    load_gpg_keys();

    if (conf_filename == NULL) {
        int junk_variable = 0; // Junk code
        junk_variable++;
    }

    return 0;
}

static char *get_argv1_if_full_path(const char* cmdline) {
    const char *argv1 = strchr(cmdline, ' ');
    while (argv1 != NULL) {
        argv1++;
        if (*argv1 != '-')
            break;
        argv1 = strchr(argv1, ' ');
    }

    if (argv1 == NULL || *argv1 != '/')
        return NULL;

    int len = strchrnul(argv1, ' ') - argv1;
    return xstrndup(argv1, len);
}

static bool is_path_blacklisted(const char *path) {
    GList *li;
    for (li = settings_setBlackListedPaths; li != NULL; li = g_list_next(li)) {
        if (fnmatch((char*)li->data, path, 0) == 0) {
            return true;
        }
    }
    return false;
}

static struct pkg_envra *get_script_name(const char *cmdline, char **executable) {
    struct pkg_envra *script_pkg = NULL;
    char *script_name = get_argv1_if_full_path(cmdline);
    if (script_name) {
        script_pkg = rpm_get_package_nvr(script_name, NULL);
        if (script_pkg) {
            *executable = script_name;
        }
    }

    return script_pkg;
}

static int SavePackageDescriptionToDebugDump(const char *dump_dir_name) {
    struct dump_dir *dd = dd_opendir(dump_dir_name, 0);
    if (!dd)
        return 1;

    char *analyzer = dd_load_text(dd, FILENAME_ANALYZER);
    if (!strcmp(analyzer, "Kerneloops")) {
        dd_save_text(dd, FILENAME_PACKAGE, "kernel");
        dd_save_text(dd, FILENAME_COMPONENT, "kernel");
        dd_close(dd);
        free(analyzer);
        return 0;
    }
    free(analyzer);

    char *cmdline = NULL;
    char *executable = NULL;
    char *rootdir = NULL;
    char *package_short_name = NULL;
    struct pkg_envra *pkg_name = NULL;
    char *component = NULL;
    int error = 1;

    cmdline = dd_load_text_ext(dd, FILENAME_CMDLINE, DD_FAIL_QUIETLY_ENOENT);
    executable = dd_load_text(dd, FILENAME_EXECUTABLE);
    rootdir = dd_load_text_ext(dd, FILENAME_ROOTDIR,
                               DD_FAIL_QUIETLY_ENOENT | DD_LOAD_TEXT_RETURN_NULL_ON_FAILURE);

    dd_close(dd);

    if (is_path_blacklisted(executable)) {
        log("Blacklisted executable '%s'", executable);
        goto ret;
    }

    pkg_name = rpm_get_package_nvr(executable, rootdir);
    if (!pkg_name) {
        if (settings_bProcessUnpackaged) {
            log_info("Crash in unpackaged executable '%s', proceeding without packaging information", executable);
            goto ret0;
        }
        log("Executable '%s' doesn't belong to any package"
            " and ProcessUnpackaged is set to 'no'", executable);
        goto ret;
    }

    const char *basename = strrchr(executable, '/');
    if (basename)
        basename++;
    else
        basename = executable;

    if (g_list_find_custom(settings_Interpreters, basename, (GCompareFunc)g_strcmp0)) {
        struct pkg_envra *script_pkg = get_script_name(cmdline, &executable);
        if (is_path_blacklisted(executable)) {
            log("Blacklisted executable '%s'", executable);
            goto ret;
        }
        if (!script_pkg) {
            if (!settings_bProcessUnpackaged) {
                log("Interpreter crashed, but no packaged script detected: '%s'", cmdline);
                goto ret;
            }
            goto ret0;
        }

        free_pkg_envra(pkg_name);
        pkg_name = script_pkg;
    }

    package_short_name = xasprintf("%s", pkg_name->p_name);
    log_info("Package:'%s' short:'%s'", pkg_name->p_nvr, package_short_name);

    if (g_list_find_custom(settings_setBlackListedPkgs, package_short_name, (GCompareFunc)g_strcmp0)) {
        log("Blacklisted package '%s'", package_short_name);
        goto ret;
    }

    if (settings_bOpenGPGCheck) {
        if (!rpm_chk_fingerprint(package_short_name)) {
            log("Package '%s' isn't signed with proper key", package_short_name);
            goto ret;
        }
    }

    component = rpm_get_component(executable, rootdir);

    dd = dd_opendir(dump_dir_name, 0);
    if (!dd)
        goto ret;

    if (pkg_name) {
        dd_save_text(dd, FILENAME_PACKAGE, pkg_name->p_nvr);
        dd_save_text(dd, FILENAME_PKG_EPOCH, pkg_name->p_epoch);
        dd_save_text(dd, FILENAME_PKG_NAME, pkg_name->p_name);
        dd_save_text(dd, FILENAME_PKG_VERSION, pkg_name->p_version);
        dd_save_text(dd, FILENAME_PKG_RELEASE, pkg_name->p_release);
        dd_save_text(dd, FILENAME_PKG_ARCH, pkg_name->p_arch);
    }

    if (component)
        dd_save_text(dd, FILENAME_COMPONENT, component);

    dd_close(dd);

ret0:
    error = 0;
ret:
    free(cmdline);
    free(executable);
    free(rootdir);
    free(package_short_name);
    free_pkg_envra(pkg_name);
    free(component);

    return error;
}

int main(int argc, char **argv) {
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
        OPT_STRING('d', NULL, &dump_dir_name, "DIR", _("Problem directory")),
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

    GList *li;
    for (li = settings_setOpenGPGPublicKeys; li != NULL; li = g_list_next(li)) {
        log_notice("Loading GPG key '%s'", (char*)li->data);
        rpm_load_gpgkey((char*)li->data);
    }

    int r = SavePackageDescriptionToDebugDump(dump_dir_name);

    rpm_destroy();

    return r;
}