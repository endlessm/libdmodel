/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2020 Endless Mobile, Inc. */

#pragma once

#include <glib-object.h>
#include <gio/gio.h>

#include "dm-content.h"
#include "dm-shard-record.h"

G_BEGIN_DECLS

#define DM_TYPE_SHARD (dm_shard_get_type ())

G_DECLARE_DERIVABLE_TYPE (DmShard, dm_shard, DM, SHARD, GObject)

struct _DmShardClass
{
  GObjectClass parent_class;

  DmShardRecord * (*find_by_id) (DmShard *self,
                                 const char *object_id);

  DmContent * (*get_model) (DmShard *self,
                            DmShardRecord *record,
                            GCancellable *cancellable,
                            GError **error);

  GInputStream * (*stream_data) (DmShard *self,
                                 DmShardRecord *record,
                                 GCancellable *cancellable,
                                 GError **error);

  gsize (*get_data_size) (DmShard *self,
                          DmShardRecord *record);

  gchar *(*test_link) (DmShard *self,
                       const gchar *link,
                       GError **error);

  gint64 (*calculate_db_offset) (DmShard *self);

  gpointer padding[12];
};

DmShardRecord *dm_shard_find_by_id (DmShard *self,
                                    const char *object_id);

DmContent *dm_shard_get_model (DmShard *self, DmShardRecord *record,
                               GCancellable *cancellable, GError **error);

GInputStream *dm_shard_stream_data (DmShard *self, DmShardRecord *record,
                                    GCancellable *cancellable, GError **error);

gsize dm_shard_get_data_size (DmShard *self,
                              DmShardRecord *record);

gchar *dm_shard_test_link (DmShard *self,
                           const gchar *link,
                           GError **error);

gchar *dm_shard_get_path (DmShard *self);

gint64 dm_shard_calculate_db_offset (DmShard *self);

void dm_shard_override_db_offset (DmShard *self,
                                  gint64 db_offset);

gint64 dm_shard_get_db_offset (DmShard *self);

G_END_DECLS
