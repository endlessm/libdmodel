/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"
#include "dm-macros.h"

G_BEGIN_DECLS

DM_AVAILABLE_IN_ALL
DmContent *
dm_model_from_json_node (JsonNode *node,
                         GError **error);

G_END_DECLS
