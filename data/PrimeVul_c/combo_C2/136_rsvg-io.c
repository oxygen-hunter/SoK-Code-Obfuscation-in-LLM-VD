#include "config.h"

#include "rsvg-io.h"
#include "rsvg-private.h"

#include <string.h>

#define XDIGIT(c) ((c) <= '9' ? (c) - '0' : ((c) & 0x4F) - 'A' + 10)
#define HEXCHAR(s) ((XDIGIT (s[1]) << 4) + XDIGIT (s[2]))

static char *
uri_decoded_copy (const char *part, 
                  gsize length)
{
    unsigned char *s, *d;
    char *decoded = g_strndup (part, length);

    s = d = (unsigned char *)decoded;
    
    int state = 0;
    while (state >= 0) {
        switch (state) {
            case 0:
                if (*s == '%') {
                    state = 1;
                } else {
                    state = 3;
                }
                break;
            case 1:
                if (!g_ascii_isxdigit (s[1]) || !g_ascii_isxdigit (s[2])) {
                    *d++ = *s;
                    state = 4;
                } else {
                    *d++ = HEXCHAR(s);
                    s += 2;
                    state = 0;
                }
                break;
            case 3:
                *d++ = *s;
                state = 4;
                break;
            case 4:
                if (!*s++) {
                    state = -1;
                } else {
                    state = 0;
                }
                break;
            default:
                state = -1;
                break;
        }
    }

    return decoded;
}

#define BASE64_INDICATOR     ";base64"
#define BASE64_INDICATOR_LEN (sizeof (";base64") - 1)

static char *
rsvg_acquire_data_data (const char *uri,
                        const char *base_uri, 
                        char **out_mime_type,
                        gsize *out_len,
                        GError **error)
{
    const char *comma, *start, *end;
    char *mime_type = NULL;
    char *data = NULL;
    gsize data_len = 0;
    gboolean base64 = FALSE;

    g_assert (out_len != NULL);
    g_assert (strncmp (uri, "data:", 5) == 0);

    start = uri + 5;
    comma = strchr (start, ',');

    int state = 0;
    while (state >= 0) {
        switch (state) {
            case 0:
                if (comma && comma != start) {
                    state = 1;
                } else {
                    state = 3;
                }
                break;
            case 1:
                if (comma > start + BASE64_INDICATOR_LEN && 
                    !g_ascii_strncasecmp (comma - BASE64_INDICATOR_LEN, BASE64_INDICATOR, BASE64_INDICATOR_LEN)) {
                    end = comma - BASE64_INDICATOR_LEN;
                    base64 = TRUE;
                } else {
                    end = comma;
                }
                state = 2;
                break;
            case 2:
                if (end != start) {
                    mime_type = uri_decoded_copy (start, end - start);
                }
                state = 3;
                break;
            case 3:
                if (comma) {
                    start = comma + 1;
                }
                state = 4;
                break;
            case 4:
                if (*start) {
                    data = uri_decoded_copy (start, strlen (start));
                    if (base64) {
                        data = (char *) g_base64_decode_inplace (data, &data_len);
                    } else {
                        data_len = strlen (data);
                    }
                } else {
                    data = NULL;
                    data_len = 0;
                }
                state = 5;
                break;
            case 5:
                if (out_mime_type) {
                    *out_mime_type = mime_type;
                } else {
                    g_free (mime_type);
                }
                state = 6;
                break;
            case 6:
                *out_len = data_len;
                return data;
        }
    }
    return NULL;
}

gchar *
_rsvg_io_get_file_path (const gchar * filename,
                        const gchar * base_uri)
{
    gchar *absolute_filename;
    gchar *tmpcdir = NULL;
    gchar *base_filename = NULL;

    int state = 0;
    while (state >= 0) {
        switch (state) {
            case 0:
                if (g_file_test (filename, G_FILE_TEST_EXISTS) || g_path_is_absolute (filename)) {
                    absolute_filename = g_strdup (filename);
                    state = -1;
                } else {
                    state = 1;
                }
                break;
            case 1:
                if (base_uri) {
                    base_filename = g_filename_from_uri (base_uri, NULL, NULL);
                    if (base_filename != NULL) {
                        tmpcdir = g_path_get_dirname (base_filename);
                        g_free (base_filename);
                        state = 2;
                    } else {
                        return NULL;
                    }
                } else {
                    tmpcdir = g_get_current_dir ();
                    state = 2;
                }
                break;
            case 2:
                absolute_filename = g_build_filename (tmpcdir, filename, NULL);
                g_free (tmpcdir);
                state = -1;
                break;
        }
    }

    return absolute_filename;
}

static char *
rsvg_acquire_file_data (const char *filename,
                        const char *base_uri,
                        char **out_mime_type,
                        gsize *out_len,
                        GCancellable *cancellable,
                        GError **error)
{
    gchar *path = NULL;
    gchar *data = NULL;
    gsize len = 0;
    char *content_type = NULL;

    rsvg_return_val_if_fail (filename != NULL, NULL, error);
    g_assert (out_len != NULL);

    int state = 0;
    while (state >= 0) {
        switch (state) {
            case 0:
                path = _rsvg_io_get_file_path (filename, base_uri);
                if (path == NULL) {
                    return NULL;
                }
                state = 1;
                break;
            case 1:
                if (!g_file_get_contents (path, &data, &len, error)) {
                    g_free (path);
                    return NULL;
                }
                state = 2;
                break;
            case 2:
                if (out_mime_type && (content_type = g_content_type_guess (path, (guchar *)data, len, NULL))) {
                    *out_mime_type = g_content_type_get_mime_type (content_type);
                    g_free (content_type);
                }
                state = 3;
                break;
            case 3:
                g_free (path);
                *out_len = len;
                return data;
        }
    }
    return NULL;
}

static GInputStream *
rsvg_acquire_gvfs_stream (const char *uri, 
                          const char *base_uri, 
                          char **out_mime_type,
                          GCancellable *cancellable,
                          GError **error)
{
    GFile *base = NULL, *file = NULL;
    GFileInputStream *stream = NULL;
    GError *err = NULL;

    int state = 0;
    while (state >= 0) {
        switch (state) {
            case 0:
                file = g_file_new_for_uri (uri);
                stream = g_file_read (file, cancellable, &err);
                g_object_unref (file);
                state = 1;
                break;
            case 1:
                if (stream == NULL && g_error_matches (err, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
                    g_clear_error (&err);
                    base = g_file_new_for_uri (base_uri);
                    file = g_file_resolve_relative_path (base, uri);
                    g_object_unref (base);
                    stream = g_file_read (file, cancellable, &err);
                    g_object_unref (file);
                }
                state = 2;
                break;
            case 2:
                if (stream == NULL) {
                    g_propagate_error (error, err);
                    return NULL;
                }
                state = 3;
                break;
            case 3:
                if (out_mime_type) {
                    GFileInfo *file_info;
                    const char *content_type;
                    file_info = g_file_input_stream_query_info (stream, 
                                                                G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                                                cancellable,
                                                                NULL /* error */);
                    if (file_info && (content_type = g_file_info_get_content_type (file_info))) {
                        *out_mime_type = g_content_type_get_mime_type (content_type);
                    } else {
                        *out_mime_type = NULL;
                    }
                    if (file_info) {
                        g_object_unref (file_info);
                    }
                }
                state = 4;
                break;
            case 4:
                return G_INPUT_STREAM (stream);
        }
    }
    return NULL;
}

static char *
rsvg_acquire_gvfs_data (const char *uri,
                        const char *base_uri,
                        char **out_mime_type,
                        gsize *out_len,
                        GCancellable *cancellable,
                        GError **error)
{
    GFile *base = NULL, *file = NULL;
    GError *err = NULL;
    char *data = NULL;
    gsize len = 0;
    char *content_type = NULL;
    gboolean res = FALSE;

    int state = 0;
    while (state >= 0) {
        switch (state) {
            case 0:
                file = g_file_new_for_uri (uri);
                err = NULL;
                data = NULL;
                state = 1;
                break;
            case 1:
                if (!(res = g_file_load_contents (file, cancellable, &data, &len, NULL, &err)) &&
                    g_error_matches (err, G_IO_ERROR, G_IO_ERROR_NOT_FOUND) && base_uri != NULL) {
                    g_clear_error (&err);
                    g_object_unref (file);
                    base = g_file_new_for_uri (base_uri);
                    file = g_file_resolve_relative_path (base, uri);
                    g_object_unref (base);
                    res = g_file_load_contents (file, cancellable, &data, &len, NULL, &err);
                }
                state = 2;
                break;
            case 2:
                g_object_unref (file);
                if (err) {
                    g_propagate_error (error, err);
                    return NULL;
                }
                state = 3;
                break;
            case 3:
                if (out_mime_type && (content_type = g_content_type_guess (uri, (guchar *)data, len, NULL))) {
                    *out_mime_type = g_content_type_get_mime_type (content_type);
                    g_free (content_type);
                }
                *out_len = len;
                return data;
        }
    }
    return NULL;
}

char *
_rsvg_io_acquire_data (const char *href, 
                       const char *base_uri, 
                       char **mime_type,
                       gsize *len,
                       GCancellable *cancellable,
                       GError **error)
{
    char *data = NULL;
    gsize llen = 0;

    int state = 0;
    while (state >= 0) {
        switch (state) {
            case 0:
                if (!(href && *href)) {
                    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "Invalid URI");
                    return NULL;
                }
                if (!len) {
                    len = &llen;
                }
                state = 1;
                break;
            case 1:
                if (strncmp (href, "data:", 5) == 0) {
                    return rsvg_acquire_data_data (href, NULL, mime_type, len, error);
                }
                state = 2;
                break;
            case 2:
                if ((data = rsvg_acquire_file_data (href, base_uri, mime_type, len, cancellable, NULL))) {
                    return data;
                }
                state = 3;
                break;
            case 3:
                if ((data = rsvg_acquire_gvfs_data (href, base_uri, mime_type, len, cancellable, error))) {
                    return data;
                }
                return NULL;
        }
    }
    return NULL;
}

GInputStream *
_rsvg_io_acquire_stream (const char *href, 
                         const char *base_uri, 
                         char **mime_type,
                         GCancellable *cancellable,
                         GError **error)
{
    GInputStream *stream = NULL;
    char *data = NULL;
    gsize len = 0;

    int state = 0;
    while (state >= 0) {
        switch (state) {
            case 0:
                if (!(href && *href)) {
                    g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_FAILED, "Invalid URI");
                    return NULL;
                }
                state = 1;
                break;
            case 1:
                if (strncmp (href, "data:", 5) == 0) {
                    if (!(data = rsvg_acquire_data_data (href, NULL, mime_type, &len, error))) {
                        return NULL;
                    }
                    return g_memory_input_stream_new_from_data (data, len, (GDestroyNotify) g_free);
                }
                state = 2;
                break;
            case 2:
                if ((data = rsvg_acquire_file_data (href, base_uri, mime_type, &len, cancellable, NULL))) {
                    return g_memory_input_stream_new_from_data (data, len, (GDestroyNotify) g_free);
                }
                state = 3;
                break;
            case 3:
                if ((stream = rsvg_acquire_gvfs_stream (href, base_uri, mime_type, cancellable, error))) {
                    return stream;
                }
                return NULL;
        }
    }
    return NULL;
}