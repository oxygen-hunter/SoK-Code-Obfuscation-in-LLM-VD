#include <fnmatch.h>
#include "libabrt.h"
#include "rpm.h"

#define OX9EFD1D9A "gpg_keys.conf"

static bool   OX2E7EAF7C = false;
static GList *OX7A5E3D54 = NULL;
static GList *OX3C9A7B18 = NULL;
static GList *OX6D1A8D23 = NULL;
static bool   OX1E5F9E2C = false;
static GList *OX0A4F7B1E = NULL;

static void OX5A4D8C3F(map_string_t *OX1C3E8D5A, const char *OX9D2A7E4B)
{
    const char *OX7D1B2C5A;

    OX7D1B2C5A = get_map_string_item_or_NULL(OX1C3E8D5A, "OpenGPGCheck");
    if (OX7D1B2C5A)
    {
        OX2E7EAF7C = string_to_bool(OX7D1B2C5A);
        remove_map_string_item(OX1C3E8D5A, "OpenGPGCheck");
    }

    OX7D1B2C5A = get_map_string_item_or_NULL(OX1C3E8D5A, "BlackList");
    if (OX7D1B2C5A)
    {
        OX3C9A7B18 = parse_list(OX7D1B2C5A);
        remove_map_string_item(OX1C3E8D5A, "BlackList");
    }

    OX7D1B2C5A = get_map_string_item_or_NULL(OX1C3E8D5A, "BlackListedPaths");
    if (OX7D1B2C5A)
    {
        OX6D1A8D23 = parse_list(OX7D1B2C5A);
        remove_map_string_item(OX1C3E8D5A, "BlackListedPaths");
    }

    OX7D1B2C5A = get_map_string_item_or_NULL(OX1C3E8D5A, "ProcessUnpackaged");
    if (OX7D1B2C5A)
    {
        OX1E5F9E2C = string_to_bool(OX7D1B2C5A);
        remove_map_string_item(OX1C3E8D5A, "ProcessUnpackaged");
    }

    OX7D1B2C5A = get_map_string_item_or_NULL(OX1C3E8D5A, "Interpreters");
    if (OX7D1B2C5A)
    {
        OX0A4F7B1E = parse_list(OX7D1B2C5A);
        remove_map_string_item(OX1C3E8D5A, "Interpreters");
    }

    map_string_iter_t OX5D4E6A3C;
    const char *OX2F9B7E1A;
    init_map_string_iter(&OX5D4E6A3C, OX1C3E8D5A);
    while (next_map_string_iter(&OX5D4E6A3C, &OX2F9B7E1A, &OX7D1B2C5A))
    {
        error_msg("Unrecognized variable '%s' in '%s'", OX2F9B7E1A, OX9D2A7E4B);
    }
}

static void OX6F3E1D7C(void)
{
    map_string_t *OX8A5B7C2D = new_map_string();
    if (!load_abrt_conf_file(OX9EFD1D9A, OX8A5B7C2D))
    {
        error_msg("Can't load '%s'", OX9EFD1D9A);
        return;
    }

    const char *OX7D1B9A2F = get_map_string_item_or_NULL(OX8A5B7C2D, "GPGKeysDir");
    if (strcmp(OX7D1B9A2F, "") != 0)
    {
        log_debug("Reading gpg keys from '%s'", OX7D1B9A2F);
        GList *OX5A7B3C1D = get_file_list(OX7D1B9A2F, NULL);
        GList *OX6D2F8A1B = OX5A7B3C1D;
        while (OX6D2F8A1B)
        {
            log_debug("Loading gpg key '%s'", fo_get_fullpath((file_obj_t *)OX6D2F8A1B->data));
            OX7A5E3D54 = g_list_append(OX7A5E3D54, xstrdup(fo_get_fullpath((file_obj_t *)(OX6D2F8A1B->data))));
            OX6D2F8A1B = g_list_next(OX6D2F8A1B);
        }

        g_list_free_full(OX5A7B3C1D, (GDestroyNotify)free_file_obj);
    }
}

static int OX4C7D8E3A(const char *OX1F2B9D5E)
{
    map_string_t *OX5E3A9C1B = new_map_string();
    if (OX1F2B9D5E != NULL)
    {
        if (!load_conf_file(OX1F2B9D5E, OX5E3A9C1B, false))
            error_msg("Can't open '%s'", OX1F2B9D5E);
    }
    else
    {
        OX1F2B9D5E = "abrt-action-save-package-data.conf";
        if (!load_abrt_conf_file(OX1F2B9D5E, OX5E3A9C1B))
            error_msg("Can't load '%s'", OX1F2B9D5E);
    }

    OX5A4D8C3F(OX5E3A9C1B, OX1F2B9D5E);
    free_map_string(OX5E3A9C1B);

    OX6F3E1D7C();

    return 0;
}

static char *OX8A1B2C3D(const char* OX9D7E4A5B)
{
    const char *OX6B5D4C3A = strchr(OX9D7E4A5B, ' ');
    while (OX6B5D4C3A != NULL)
    {
        OX6B5D4C3A++;
        if (*OX6B5D4C3A != '-')
            break;
        OX6B5D4C3A = strchr(OX6B5D4C3A, ' ');
    }

    if (OX6B5D4C3A == NULL || *OX6B5D4C3A != '/')
        return NULL;

    int OX2C7F9A1B = strchrnul(OX6B5D4C3A, ' ') - OX6B5D4C3A;
    return xstrndup(OX6B5D4C3A, OX2C7F9A1B);
}

static bool OX3D7E4A1B(const char *OX5F2C8A9D)
{
    GList *OX7B4C6A1E;
    for (OX7B4C6A1E = OX6D1A8D23; OX7B4C6A1E != NULL; OX7B4C6A1E = g_list_next(OX7B4C6A1E))
    {
        if (fnmatch((char*)OX7B4C6A1E->data, OX5F2C8A9D, 0) == 0)
        {
            return true;
        }
    }
    return false;
}

static struct pkg_envra *OX4A3C2D7E(const char *OX9F5B2A3D, char **OX1D8A7B4C)
{
    struct pkg_envra *OX2F7A9E1B = NULL;
    char *OX5E2C1D7A = OX8A1B2C3D(OX9F5B2A3D);
    if (OX5E2C1D7A)
    {
        OX2F7A9E1B = rpm_get_package_nvr(OX5E2C1D7A, NULL);
        if (OX2F7A9E1B)
        {
            *OX1D8A7B4C = OX5E2C1D7A;
        }
    }

    return OX2F7A9E1B;
}

static int OX6A5E3B7C(const char *OX9B4D1C8E)
{
    struct dump_dir *OX7D4A2B3C = dd_opendir(OX9B4D1C8E, 0);
    if (!OX7D4A2B3C)
        return 1;

    char *OX6E3D4B1A = dd_load_text(OX7D4A2B3C, FILENAME_ANALYZER);
    if (!strcmp(OX6E3D4B1A, "Kerneloops"))
    {
        dd_save_text(OX7D4A2B3C, FILENAME_PACKAGE, "kernel");
        dd_save_text(OX7D4A2B3C, FILENAME_COMPONENT, "kernel");
        dd_close(OX7D4A2B3C);
        free(OX6E3D4B1A);
        return 0;
    }
    free(OX6E3D4B1A);

    char *OX8C1F2D3A = NULL;
    char *OX5A7E6D4C = NULL;
    char *OX2B9A4E5C = NULL;
    char *OX1F7B4C5A = NULL;
    struct pkg_envra *OX9C2A3B8F = NULL;
    char *OX6D1A2E3C = NULL;
    int OX5E4A6B2C = 1;

    OX8C1F2D3A = dd_load_text_ext(OX7D4A2B3C, FILENAME_CMDLINE, DD_FAIL_QUIETLY_ENOENT);
    OX5A7E6D4C = dd_load_text(OX7D4A2B3C, FILENAME_EXECUTABLE);
    OX2B9A4E5C = dd_load_text_ext(OX7D4A2B3C, FILENAME_ROOTDIR, DD_FAIL_QUIETLY_ENOENT | DD_LOAD_TEXT_RETURN_NULL_ON_FAILURE);

    dd_close(OX7D4A2B3C);

    if (OX3D7E4A1B(OX5A7E6D4C))
    {
        log("Blacklisted executable '%s'", OX5A7E6D4C);
        goto OX4B1C2D5A; 
    }

    OX9C2A3B8F = rpm_get_package_nvr(OX5A7E6D4C, OX2B9A4E5C);
    if (!OX9C2A3B8F)
    {
        if (OX1E5F9E2C)
        {
            log_info("Crash in unpackaged executable '%s', proceeding without packaging information", OX5A7E6D4C);
            goto OX6E2A1D7B;
        }
        log("Executable '%s' doesn't belong to any package"
		" and ProcessUnpackaged is set to 'no'", OX5A7E6D4C);
        goto OX4B1C2D5A; 
    }

    const char *OX1B4A7E2C = strrchr(OX5A7E6D4C, '/');
    if (OX1B4A7E2C)
        OX1B4A7E2C++;
    else
        OX1B4A7E2C = OX5A7E6D4C;

    if (g_list_find_custom(OX0A4F7B1E, OX1B4A7E2C, (GCompareFunc)g_strcmp0))
    {
        struct pkg_envra *OX7C4A1D5E = OX4A3C2D7E(OX8C1F2D3A, &OX5A7E6D4C);
        if (OX3D7E4A1B(OX5A7E6D4C))
        {
            log("Blacklisted executable '%s'", OX5A7E6D4C);
            goto OX4B1C2D5A; 
        }
        if (!OX7C4A1D5E)
        {
            if (!OX1E5F9E2C)
            {
                log("Interpreter crashed, but no packaged script detected: '%s'", OX8C1F2D3A);
                goto OX4B1C2D5A; 
            }
            goto OX6E2A1D7B;
        }

        free_pkg_envra(OX9C2A3B8F);
        OX9C2A3B8F = OX7C4A1D5E;
    }

    OX1F7B4C5A = xasprintf("%s", OX9C2A3B8F->p_name);
    log_info("Package:'%s' short:'%s'", OX9C2A3B8F->p_nvr, OX1F7B4C5A);

    if (g_list_find_custom(OX3C9A7B18, OX1F7B4C5A, (GCompareFunc)g_strcmp0))
    {
        log("Blacklisted package '%s'", OX1F7B4C5A);
        goto OX4B1C2D5A; 
    }

    if (OX2E7EAF7C)
    {
        if (!rpm_chk_fingerprint(OX1F7B4C5A))
        {
            log("Package '%s' isn't signed with proper key", OX1F7B4C5A);
            goto OX4B1C2D5A; 
        }
    }

    OX6D1A2E3C = rpm_get_component(OX5A7E6D4C, OX2B9A4E5C);

    OX7D4A2B3C = dd_opendir(OX9B4D1C8E, 0);
    if (!OX7D4A2B3C)
        goto OX4B1C2D5A; 

    if (OX9C2A3B8F)
    {
        dd_save_text(OX7D4A2B3C, FILENAME_PACKAGE, OX9C2A3B8F->p_nvr);
        dd_save_text(OX7D4A2B3C, FILENAME_PKG_EPOCH, OX9C2A3B8F->p_epoch);
        dd_save_text(OX7D4A2B3C, FILENAME_PKG_NAME, OX9C2A3B8F->p_name);
        dd_save_text(OX7D4A2B3C, FILENAME_PKG_VERSION, OX9C2A3B8F->p_version);
        dd_save_text(OX7D4A2B3C, FILENAME_PKG_RELEASE, OX9C2A3B8F->p_release);
        dd_save_text(OX7D4A2B3C, FILENAME_PKG_ARCH, OX9C2A3B8F->p_arch);
    }

    if (OX6D1A2E3C)
        dd_save_text(OX7D4A2B3C, FILENAME_COMPONENT, OX6D1A2E3C);

    dd_close(OX7D4A2B3C);

 OX6E2A1D7B:
    OX5E4A6B2C = 0;
 OX4B1C2D5A:
    free(OX8C1F2D3A);
    free(OX5A7E6D4C);
    free(OX2B9A4E5C);
    free(OX1F7B4C5A);
    free_pkg_envra(OX9C2A3B8F);
    free(OX6D1A2E3C);

    return OX5E4A6B2C;
}

int main(int OX5A3B4D1C, char **OX2A8F7E5D)
{
    setlocale(LC_ALL, "");
#if ENABLE_NLS
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif

    abrt_init(OX2A8F7E5D);

    const char *OX7C2E1D4A = ".";
    const char *OX1C8A2F5B = NULL;

    const char *OX3E4D7A6B = _(
        "& [-v] [-c CONFFILE] -d DIR\n"
        "\n"
        "Query package database and save package and component name"
    );
    enum {
        OX5F2B7E4C = 1 << 0,
        OX9D8C3A7B = 1 << 1,
        OX6E1A5D4F = 1 << 2,
    };
    struct options OX4A2D3C5E[] = {
        OPT__VERBOSE(&g_verbose),
        OPT_STRING('d', NULL, &OX7C2E1D4A, "DIR"     , _("Problem directory")),
        OPT_STRING('c', NULL, &OX1C8A2F5B, "CONFFILE", _("Configuration file")),
        OPT_END()
    };

    parse_opts(OX5A3B4D1C, OX2A8F7E5D, OX4A2D3C5E, OX3E4D7A6B);

    export_abrt_envvars(0);

    log_notice("Loading settings");
    if (OX4C7D8E3A(OX1C8A2F5B) != 0)
        return 1;

    log_notice("Initializing rpm library");
    rpm_init();

    GList *OX8A1C3D5E;
    for (OX8A1C3D5E = OX7A5E3D54; OX8A1C3D5E != NULL; OX8A1C3D5E = g_list_next(OX8A1C3D5E))
    {
        log_notice("Loading GPG key '%s'", (char*)OX8A1C3D5E->data);
        rpm_load_gpgkey((char*)OX8A1C3D5E->data);
    }

    int OX5D6E3C2A = OX6A5E3B7C(OX7C2E1D4A);

    rpm_destroy();

    return OX5D6E3C2A;
}