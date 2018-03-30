/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include <gio/gio.h>
#include <json-glib/json-glib.h>

#include "dm-macros.h"

G_BEGIN_DECLS

#define DM_TYPE_CONTENT dm_content_get_type ()

DM_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DmContent, dm_content, DM, CONTENT, GObject)

struct _DmContentClass
{
  GObjectClass parent_class;

  gpointer padding[8];
};

DM_AVAILABLE_IN_ALL
char * const *
dm_content_get_tags (DmContent *self);

DM_AVAILABLE_IN_ALL
char * const *
dm_content_get_resources (DmContent *self);

DM_AVAILABLE_IN_ALL
JsonObject *
dm_content_get_discovery_feed_content (DmContent *self);

DM_AVAILABLE_IN_ALL
DmContent *
dm_content_new_from_json_node (JsonNode *node);

DM_AVAILABLE_IN_ALL
GFileInputStream *
dm_content_get_content_stream (DmContent *self,
                               GError **error);

G_END_DECLS
