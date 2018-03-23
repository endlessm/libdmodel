/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"

G_BEGIN_DECLS

DmContent *
dm_model_from_json_node (JsonNode *node,
                         GError **error);

G_END_DECLS
