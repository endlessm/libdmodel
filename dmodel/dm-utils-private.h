/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-utils.h"

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define dm_libarchive_set_error_and_return_if_fail(expr,arch,error,return_value) \
  G_STMT_START{                                                                  \
    if G_LIKELY(expr) {}                                                         \
    else                                                                         \
      {                                                                          \
        g_set_error_literal (error,                                              \
                             dm_content_error_quark (),                          \
                             archive_errno (arch),                               \
                             archive_error_string (arch));                       \
        return (return_value);                                                   \
      }                                                                          \
  }G_STMT_END;

void
dm_utils_append_gparam_from_json_node (JsonNode *node,
                                       GParamSpec *pspec,
                                       GArray *params);

void
dm_utils_free_gparam_array (GArray *params);

const gchar *
dm_utils_id_get_hash (const char *id);

G_END_DECLS
