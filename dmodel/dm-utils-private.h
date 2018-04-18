/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

void
dm_utils_append_gparam_from_json_node (JsonNode *node,
                                       GParamSpec *pspec,
                                       GArray *params);

void
dm_utils_free_gparam_array (GArray *params);

const gchar *
dm_utils_id_get_hash (const char *id);

G_END_DECLS
