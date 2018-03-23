/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"
#include "dm-macros.h"

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define DM_TYPE_SET dm_set_get_type ()

DM_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DmSet, dm_set, DM, SET, DmContent)

struct _DmSetClass
{
  DmContentClass parent_class;

  gpointer padding[8];
};

DM_AVAILABLE_IN_ALL
char * const *
dm_set_get_child_tags (DmSet *self);

DM_AVAILABLE_IN_ALL
DmContent *
dm_set_new_from_json_node (JsonNode *node);

G_END_DECLS
