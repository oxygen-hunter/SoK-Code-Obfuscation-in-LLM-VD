/*
   Copyright (C) 2000 Eazel, Inc.
   Copyright (C) 2002, 2003, 2004, 2005 Dom Lachowicz <cinamod@hotmail.com>
   Copyright (C) 2003, 2004, 2005 Caleb Moore <c.moore@student.unsw.edu.au>
   Copyright © 2011, 2012 Christian Persch

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "config.h"

#include "rsvg-io.h"
#include "rsvg-private.h"

#include <string.h>

/* Copied from soup-request-data.c (LGPL2+):
 * Copyright (C) 2009, 2010 Red Hat, Inc.
 * Copyright (C) 2010 Igalia, S.L.
 * and from soup-uri.c:
 * Copyright 1999-2003 Ximian, Inc.
 */

#define OX6D5F9A8C(c) ((c) <= '9' ? (c) - '0' : ((c) & 0x4F) - 'A' + 10)
#define OX6A94E5F1(s) ((OX6D5F9A8C (s[1]) << 4) + OX6D5F9A8C (s[2]))

static char *
OX7B4DF339 (const char *OX2F9B6C7D, 
                  gsize OX4A1E9E6F)
{
    unsigned char *OX3D8C2F4B, *OX5E6A1F2D;
    char *OX1F9C7D3E = g_strndup (OX2F9B6C7D, OX4A1E9E6F);

    OX3D8C2F4B = OX5E6A1F2D = (unsigned char *)OX1F9C7D3E;
    do {
        if (*OX3D8C2F4B == '%') {
            if (!g_ascii_isxdigit (OX3D8C2F4B[1]) ||
                !g_ascii_isxdigit (OX3D8C2F4B[2])) {
                *OX5E6A1F2D++ = *OX3D8C2F4B;
                continue;
            }
            *OX5E6A1F2D++ = OX6A94E5F1 (OX3D8C2F4B);
            OX3D8C2F4B += 2;
        } else {
            *OX5E6A1F2D++ = *OX3D8C2F4B;
        }
    } while (*OX3D8C2F4B++);

    return OX1F9C7D3E;
}

#define OX8C3E1D6F     ";base64"
#define OX2B7F4A9C (sizeof (";base64") - 1)

static char *
OX4E6C9F2A (const char *OX7A1E5C6D,
                        const char *OX3F2D8A9B, 
                        char **OX2F4A1E9D,
                        gsize *OX9C3E1D7F,
                        GError **OX5E7A9F1B)
{
    const char *OX1F6C2D4A, *OX3E8B5A2F, *OX7D1C6F9A;
    char *OX4A9E7B1C;
    char *OX6D5F2C8E;
    gsize OX9F1E4B7C;
    gboolean OX2A6F8D3E = FALSE;

    g_assert (OX9C3E1D7F != NULL);
    g_assert (strncmp (OX7A1E5C6D, "data:", 5) == 0);

    OX4A9E7B1C = NULL;
    OX3E8B5A2F = OX7A1E5C6D + 5;
    OX1F6C2D4A = strchr (OX3E8B5A2F, ',');

    if (OX1F6C2D4A && OX1F6C2D4A != OX3E8B5A2F) {
        if (OX1F6C2D4A > OX3E8B5A2F + OX2B7F4A9C && 
            !g_ascii_strncasecmp (OX1F6C2D4A - OX2B7F4A9C, OX8C3E1D6F, OX2B7F4A9C)) {
            OX7D1C6F9A = OX1F6C2D4A - OX2B7F4A9C;
            OX2A6F8D3E = TRUE;
        } else {
            OX7D1C6F9A = OX1F6C2D4A;
        }

        if (OX7D1C6F9A != OX3E8B5A2F) {
            OX4A9E7B1C = OX7B4DF339 (OX3E8B5A2F, OX7D1C6F9A - OX3E8B5A2F);
        }
    }

    if (OX1F6C2D4A)
        OX3E8B5A2F = OX1F6C2D4A + 1;

    if (*OX3E8B5A2F) {
	OX6D5F2C8E = OX7B4DF339 (OX3E8B5A2F, strlen (OX3E8B5A2F));

        if (OX2A6F8D3E)
            OX6D5F2C8E = (char *) g_base64_decode_inplace (OX6D5F2C8E, &OX9F1E4B7C);
        else
            OX9F1E4B7C = strlen (OX6D5F2C8E);
    } else {
        OX6D5F2C8E = NULL;
        OX9F1E4B7C = 0;
    }

    if (OX2F4A1E9D)
        *OX2F4A1E9D = OX4A9E7B1C;
    else
        g_free (OX4A9E7B1C);

    *OX9C3E1D7F = OX9F1E4B7C;
    return OX6D5F2C8E;
}

gchar *
OX1D6F2A8C (const gchar * OX3F9C7D2E,
                        const gchar * OX7A1E8B5D)
{
    gchar *OX5E9C3D1A;

    if (g_file_test (OX3F9C7D2E, G_FILE_TEST_EXISTS) || g_path_is_absolute (OX3F9C7D2E)) {
        OX5E9C3D1A = g_strdup (OX3F9C7D2E);
    } else {
        gchar *OX2A8F5E9C;
        gchar *OX4B1D6F3E;

        if (OX7A1E8B5D) {
            OX4B1D6F3E = g_filename_from_uri (OX7A1E8B5D, NULL, NULL);
            if (OX4B1D6F3E != NULL) {
                OX2A8F5E9C = g_path_get_dirname (OX4B1D6F3E);
                g_free (OX4B1D6F3E);
            } else 
                return NULL;
        } else
            OX2A8F5E9C = g_get_current_dir ();

        OX5E9C3D1A = g_build_filename (OX2A8F5E9C, OX3F9C7D2E, NULL);
        g_free (OX2A8F5E9C);
    }

    return OX5E9C3D1A;
}

static char *
OX3E7A5F9C (const char *OX4B9C6D2E,
                        const char *OX1F7D3A8B,
                        char **OX5E9A1D4F,
                        gsize *OX9C3D1E7A,
                        GCancellable *OX2F6A8B9D,
                        GError **OX7D5F1E3A)
{
    gchar *OX8C3B9A6D, *OX1E4F5D8C;
    gsize OX2A7F6B9C;
    char *OX4D9E3B1F;

    rsvg_return_val_if_fail (OX4B9C6D2E != NULL, NULL, OX7D5F1E3A);
    g_assert (OX9C3D1E7A != NULL);

    OX8C3B9A6D = OX1D6F2A8C (OX4B9C6D2E, OX1F7D3A8B);
    if (OX8C3B9A6D == NULL)
        return NULL;

    if (!g_file_get_contents (OX8C3B9A6D, &OX1E4F5D8C, &OX2A7F6B9C, OX7D5F1E3A)) {
        g_free (OX8C3B9A6D);
        return NULL;
    }

    if (OX5E9A1D4F &&
        (OX4D9E3B1F = g_content_type_guess (OX8C3B9A6D, (guchar *) OX1E4F5D8C, OX2A7F6B9C, NULL))) {
        *OX5E9A1D4F = g_content_type_get_mime_type (OX4D9E3B1F);
        g_free (OX4D9E3B1F);
    }

    g_free (OX8C3B9A6D);

    *OX9C3D1E7A = OX2A7F6B9C;
    return OX1E4F5D8C;
}

static GInputStream *
OX5B7C9E2A (const char *OX3D1F6A8B, 
                          const char *OX9C4E2B7A, 
                          char **OX7E5D1F3A,
                          GCancellable *OX2A8F5C6E,
                          GError **OX1B9D3E4F)
{
    GFile *OX4A7F2B8C, *OX9E1C3D5A;
    GFileInputStream *OX8B6F3D9C;
    GError *OX7A2E5C9D = NULL;

    OX9E1C3D5A = g_file_new_for_uri (OX3D1F6A8B);

    OX8B6F3D9C = g_file_read (OX9E1C3D5A, OX2A8F5C6E, &OX7A2E5C9D);
    g_object_unref (OX9E1C3D5A);

    if (OX8B6F3D9C == NULL &&
        g_error_matches (OX7A2E5C9D, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
        g_clear_error (&OX7A2E5C9D);

        OX4A7F2B8C = g_file_new_for_uri (OX9C4E2B7A);
        OX9E1C3D5A = g_file_resolve_relative_path (OX4A7F2B8C, OX3D1F6A8B);
        g_object_unref (OX4A7F2B8C);

        OX8B6F3D9C = g_file_read (OX9E1C3D5A, OX2A8F5C6E, &OX7A2E5C9D);
        g_object_unref (OX9E1C3D5A);
    }

    if (OX8B6F3D9C == NULL) {
        g_propagate_error (OX1B9D3E4F, OX7A2E5C9D);
        return NULL;
    }

    if (OX7E5D1F3A) {
        GFileInfo *OX2F1C8B6D;
        const char *OX5D9E3B1A;

        OX2F1C8B6D = g_file_input_stream_query_info (OX8B6F3D9C, 
                                                    G_FILE_ATTRIBUTE_STANDARD_CONTENT_TYPE,
                                                    OX2A8F5C6E,
                                                    NULL);
        if (OX2F1C8B6D &&
            (OX5D9E3B1A = g_file_info_get_content_type (OX2F1C8B6D)))
            *OX7E5D1F3A = g_content_type_get_mime_type (OX5D9E3B1A);
        else
            *OX7E5D1F3A = NULL;

        if (OX2F1C8B6D)
            g_object_unref (OX2F1C8B6D);
    }

    return G_INPUT_STREAM (OX8B6F3D9C);
}

static char *
OX6F9A1E3D (const char *OX4D5C9E3B,
                        const char *OX2A7F8B6D,
                        char **OX5E1C9D7F,
                        gsize *OX3B1F4A8E,
                        GCancellable *OX7D2E6F3A,
                        GError **OX9C4B6A1F)
{
    GFile *OX3E1D9A5C, *OX8F7B4C2D;
    GError *OX1F9C3D6E;
    char *OX2A8E5B7D;
    gsize OX5C9E3B1D;
    char *OX4A7B1F6D;
    gboolean OX9F3D1C8E;

    OX8F7B4C2D = g_file_new_for_uri (OX4D5C9E3B);

    OX1F9C3D6E = NULL;
    OX2A8E5B7D = NULL;
    if (!(OX9F3D1C8E = g_file_load_contents (OX8F7B4C2D, OX7D2E6F3A, &OX2A8E5B7D, &OX5C9E3B1D, NULL, &OX1F9C3D6E)) &&
        g_error_matches (OX1F9C3D6E, G_IO_ERROR, G_IO_ERROR_NOT_FOUND) &&
        OX2A7F8B6D != NULL) {
        g_clear_error (&OX1F9C3D6E);
        g_object_unref (OX8F7B4C2D);

        OX3E1D9A5C = g_file_new_for_uri (OX2A7F8B6D);
        OX8F7B4C2D = g_file_resolve_relative_path (OX3E1D9A5C, OX4D5C9E3B);
        g_object_unref (OX3E1D9A5C);

        OX9F3D1C8E = g_file_load_contents (OX8F7B4C2D, OX7D2E6F3A, &OX2A8E5B7D, &OX5C9E3B1D, NULL, &OX1F9C3D6E);
    }

    g_object_unref (OX8F7B4C2D);

    if (OX1F9C3D6E) {
        g_propagate_error (OX9C4B6A1F, OX1F9C3D6E);
        return NULL;
    }

    if (OX5E1C9D7F &&
        (OX4A7B1F6D = g_content_type_guess (OX4D5C9E3B, (guchar *) OX2A8E5B7D, OX5C9E3B1D, NULL))) {
        *OX5E1C9D7F = g_content_type_get_mime_type (OX4A7B1F6D);
        g_free (OX4A7B1F6D);
    }

    *OX3B1F4A8E = OX5C9E3B1D;
    return OX2A8E5B7D;
}

char *
OX7D3F8C1A (const char *OX6E9B2A4F, 
                       const char *OX1F6A4D3C, 
                       char **OX8B7E5A3F,
                       gsize *OX9C1D6F2B,
                       GCancellable *OX5E3A9D7F,
                       GError **OX4C8F2B1D)
{
    char *OX3A7D1E6B;
    gsize OX2F5B8A7C;

    if (!(OX6E9B2A4F && *OX6E9B2A4F)) {
        g_set_error_literal (OX4C8F2B1D, G_IO_ERROR, G_IO_ERROR_FAILED,
                            "Invalid URI");
        return NULL;
    }

    if (!OX9C1D6F2B)
        OX9C1D6F2B = &OX2F5B8A7C;

    if (strncmp (OX6E9B2A4F, "data:", 5) == 0)
      return OX4E6C9F2A (OX6E9B2A4F, NULL, OX8B7E5A3F, OX9C1D6F2B, OX4C8F2B1D);

    if ((OX3A7D1E6B = OX3E7A5F9C (OX6E9B2A4F, OX1F6A4D3C, OX8B7E5A3F, OX9C1D6F2B, OX5E3A9D7F, NULL)))
      return OX3A7D1E6B;

    if ((OX3A7D1E6B = OX6F9A1E3D (OX6E9B2A4F, OX1F6A4D3C, OX8B7E5A3F, OX9C1D6F2B, OX5E3A9D7F, OX4C8F2B1D)))
      return OX3A7D1E6B;

    return NULL;
}

GInputStream *
OX5C8A1F7B (const char *OX2E6D3C9A, 
                         const char *OX1D9F4A7B, 
                         char **OX7E3B5A2F,
                         GCancellable *OX8F1C9D6E,
                         GError **OX4A2B7F3E)
{
    GInputStream *OX9D1E4B6C;
    char *OX2F7A5C8B;
    gsize OX3B9C1E6D;

    if (!(OX2E6D3C9A && *OX2E6D3C9A)) {
        g_set_error_literal (OX4A2B7F3E, G_IO_ERROR, G_IO_ERROR_FAILED,
                            "Invalid URI");
        return NULL;
    }

    if (strncmp (OX2E6D3C9A, "data:", 5) == 0) {
        if (!(OX2F7A5C8B = OX4E6C9F2A (OX2E6D3C9A, NULL, OX7E3B5A2F, &OX3B9C1E6D, OX4A2B7F3E)))
            return NULL;

        return g_memory_input_stream_new_from_data (OX2F7A5C8B, OX3B9C1E6D, (GDestroyNotify) g_free);
    }

    if ((OX2F7A5C8B = OX3E7A5F9C (OX2E6D3C9A, OX1D9F4A7B, OX7E3B5A2F, &OX3B9C1E6D, OX8F1C9D6E, NULL)))
      return g_memory_input_stream_new_from_data (OX2F7A5C8B, OX3B9C1E6D, (GDestroyNotify) g_free);

    if ((OX9D1E4B6C = OX5B7C9E2A (OX2E6D3C9A, OX1D9F4A7B, OX7E3B5A2F, OX8F1C9D6E, OX4A2B7F3E)))
      return OX9D1E4B6C;

    return NULL;
}