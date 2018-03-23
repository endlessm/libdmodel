/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-macros.h"
#include "dm-media.h"

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define DM_TYPE_IMAGE dm_image_get_type ()

DM_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DmImage, dm_image, DM, IMAGE, DmMedia)

struct _DmImageClass
{
  DmMediaClass parent_class;

  gpointer padding[8];
};

DM_AVAILABLE_IN_ALL
DmContent *
dm_image_new_from_json_node (JsonNode *node);

G_END_DECLS
