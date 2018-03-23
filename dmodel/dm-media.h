/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define DM_TYPE_MEDIA dm_media_get_type ()
G_DECLARE_DERIVABLE_TYPE (DmMedia, dm_media, DM, MEDIA, DmContent)

struct _DmMediaClass
{
  DmContentClass parent_class;

  gpointer padding[8];
};

DmContent *
dm_media_new_from_json_node (JsonNode *node);

G_END_DECLS
