/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define DM_TYPE_SET dm_set_get_type ()
G_DECLARE_DERIVABLE_TYPE (DmSet, dm_set, DM, SET, DmContent)

struct _DmSetClass
{
  DmContentClass parent_class;

  gpointer padding[8];
};

char * const *
dm_set_get_child_tags (DmSet *self);

DmContent *
dm_set_new_from_json_node (JsonNode *node);

G_END_DECLS
