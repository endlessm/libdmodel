/* Copyright 2017 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"
#include "dm-macros.h"

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define DM_TYPE_AUDIO dm_audio_get_type ()

DM_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DmAudio, dm_audio, DM, AUDIO, DmContent)

struct _DmAudioClass
{
  DmContentClass parent_class;

  gpointer padding[8];
};

DM_AVAILABLE_IN_ALL
DmContent *
dm_audio_new_from_json_node (JsonNode *node);

G_END_DECLS
