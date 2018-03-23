/* Copyright 2017 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define DM_TYPE_DICTIONARY_ENTRY dm_dictionary_entry_get_type ()
G_DECLARE_DERIVABLE_TYPE (DmDictionaryEntry, dm_dictionary_entry, DM,
                          DICTIONARY_ENTRY, DmContent)

struct _DmDictionaryEntryClass
{
  DmContentClass parent_class;
};

DmContent *
dm_dictionary_entry_new_from_json_node (JsonNode *node);

G_END_DECLS
