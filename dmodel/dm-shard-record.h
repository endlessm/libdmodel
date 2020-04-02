/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2020 Endless Mobile, Inc. */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define DM_TYPE_SHARD_RECORD (dm_shard_record_get_type ())

typedef struct _DmShardRecord DmShardRecord;

GType              dm_shard_record_get_type (void) G_GNUC_CONST;
DmShardRecord     *dm_shard_record_new      (gpointer shard,
                                             gpointer native_record,
                                             GDestroyNotify native_record_destroy_func);
DmShardRecord     *dm_shard_record_copy     (DmShardRecord *self);
DmShardRecord     *dm_shard_record_ref      (DmShardRecord *self);
void               dm_shard_record_unref    (DmShardRecord *self);
gpointer dm_shard_record_get_shard (DmShardRecord *self);
gpointer dm_shard_record_get_native (DmShardRecord *self);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (DmShardRecord, dm_shard_record_unref)

G_END_DECLS
