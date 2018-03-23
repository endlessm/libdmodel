/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

/**
 * DmContentError:
 * @DM_CONTENT_ERROR_BAD_FORMAT: unexpected format in metadata json
 *
 * Error enumeration for object model creation.
 */
typedef enum {
  DM_CONTENT_ERROR_BAD_FORMAT,
} DmContentError;

#define DM_CONTENT_ERROR dm_content_error_quark ()
GQuark dm_content_error_quark (void);

gboolean
dm_utils_parallel_init (GSList *initables,
                        int io_priority,
                        GCancellable *cancellable,
                        GError **error);

gboolean
dm_default_vfs_set_shards (GSList *shards);

gboolean
dm_utils_is_valid_id (const char *ekn_id);

GFile *
dm_get_data_dir (const char *app_id);

GList *
dm_get_extensions_dirs (const char *app_id);

gchar *
dm_get_ekn_version (const char *app_id,
                    GCancellable *cancellable,
                    GError **error);

const gchar *
dm_get_current_language (void);

G_END_DECLS
