/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2020 Endless Mobile, Inc. */

#pragma once

#include <glib-object.h>

#include "dm-shard.h"

G_BEGIN_DECLS

#define DM_TYPE_SHARD_OPEN_ZIM (dm_shard_open_zim_get_type())

G_DECLARE_FINAL_TYPE (DmShardOpenZim, dm_shard_open_zim, DM, SHARD_OPEN_ZIM, DmShard)

DmShardOpenZim *dm_shard_open_zim_new (const gchar *path);

G_END_DECLS
