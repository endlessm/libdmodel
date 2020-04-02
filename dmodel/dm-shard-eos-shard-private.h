/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2020 Endless Mobile, Inc. */

#pragma once

#include <glib-object.h>

#include "dm-shard.h"

G_BEGIN_DECLS

#define DM_TYPE_SHARD_EOS_SHARD (dm_shard_eos_shard_get_type())

G_DECLARE_FINAL_TYPE (DmShardEosShard, dm_shard_eos_shard, DM, SHARD_EOS_SHARD, DmShard)

DmShardEosShard *dm_shard_eos_shard_new (const gchar *path);

G_END_DECLS
