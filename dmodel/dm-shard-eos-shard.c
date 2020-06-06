/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2020 Endless Mobile, Inc. */

#include <eos-shard/eos-shard-blob.h>
#include <eos-shard/eos-shard-record.h>
#include <eos-shard/eos-shard-dictionary.h>
#include <eos-shard/eos-shard-shard-file.h>

#include "dm-base.h"

#include "dm-shard.h"
#include "dm-shard-eos-shard-private.h"
#include "stdio.h"

// This hash is derived from sha1('link-table'), and for now is the hardcoded
// location of link tables for all shards.
#define LINK_TABLE_ID "4dba9091495e8f277893e0d400e9e092f9f6f551"

/**
 * SECTION:shard-eos-shard
 * @title: eos-shard shard implementation
 * @short_description: Implementation for eos-shard based shards
 *
 * This class implements the eos-shard based shards with an underlying
 * #EosShardShardFile object.
 */
struct _DmShardEosShard {
  DmShard parent_instance;
  EosShardShardFile *shard_file;
  EosShardDictionary *link_table;
};

static void g_async_initable_interface_init (GAsyncInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (DmShardEosShard, dm_shard_eos_shard, DM_TYPE_SHARD,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE,
                                                g_async_initable_interface_init))

/**
 * dm_shard_eos_shard_new:
 * @path: The shard path
 *
 * Returns: The newly created #DmShardEosShard.
 */
DmShardEosShard *
dm_shard_eos_shard_new (const gchar *path)
{
  return DM_SHARD_EOS_SHARD (g_object_new (DM_TYPE_SHARD_EOS_SHARD, "path", path, NULL));
}

static void
dm_shard_eos_shard_finalize (GObject *object)
{
  DmShardEosShard *self = (DmShardEosShard *)object;

  g_clear_pointer (&self->shard_file, g_object_unref);
  g_clear_pointer (&self->link_table, eos_shard_dictionary_unref);

  G_OBJECT_CLASS (dm_shard_eos_shard_parent_class)->finalize (object);
}

static void
dm_shard_eos_shard_constructed (GObject *object)
{
  DmShardEosShard *self = DM_SHARD_EOS_SHARD (object);

  self->shard_file = g_object_new (EOS_SHARD_TYPE_SHARD_FILE,
                                   "path", dm_shard_get_path (DM_SHARD (object)),
                                   NULL);

  G_OBJECT_CLASS (dm_shard_eos_shard_parent_class)->constructed (object);
}

static DmShardRecord *
dm_shard_eos_shard_find_by_id (DmShard *self,
                               const char *object_id)
{
  DmShardEosShard *_self = DM_SHARD_EOS_SHARD (self);
  EosShardRecord *record = eos_shard_shard_file_find_record_by_hex_name (_self->shard_file,
                                                                          object_id);
  if (record)
    return dm_shard_record_new (_self, record, g_free);

  return NULL;
}

static DmContent *
dm_shard_eos_shard_get_model (G_GNUC_UNUSED DmShard *self,
                              DmShardRecord *record,
                              GCancellable *cancellable,
                              GError **error)
{
  EosShardRecord *eos_shard_record = (EosShardRecord *) dm_shard_record_get_native (record);
  g_autoptr(GInputStream) stream = eos_shard_blob_get_stream (eos_shard_record->metadata);

  g_autoptr(JsonParser) parser = json_parser_new_immutable ();
  gboolean parse_success =
    json_parser_load_from_stream (parser, stream, cancellable, error);
  if (!parse_success)
    return NULL;

  return dm_model_from_json_node (json_parser_get_root (parser), error);
}

static GInputStream *
dm_shard_eos_shard_stream_data (G_GNUC_UNUSED DmShard *self,
                                DmShardRecord *record,
                                G_GNUC_UNUSED GCancellable *cancellable,
                                G_GNUC_UNUSED GError **error)
{
  EosShardRecord *eos_shard_record = (EosShardRecord *) dm_shard_record_get_native (record);
  g_autoptr(EosShardBlob) blob = eos_shard_blob_ref (eos_shard_record->data);

  if (!blob)
    return NULL;

  return eos_shard_blob_get_stream (blob);
}

static gsize
dm_shard_eos_shard_get_data_size (G_GNUC_UNUSED DmShard *self,
                                  DmShardRecord *record)
{
  EosShardRecord *eos_shard_record = (EosShardRecord *) dm_shard_record_get_native (record);
  g_autoptr(EosShardBlob) blob = eos_shard_blob_ref (eos_shard_record->data);

  if (!blob)
    return 0;

  return eos_shard_blob_get_content_size (blob);
}

static gchar *
dm_shard_eos_shard_test_link (DmShard *self,
                              const gchar *link,
                              GError **error)
{
  DmShardEosShard *_self = DM_SHARD_EOS_SHARD (self);

  if (!_self->link_table)
    return NULL;

  return eos_shard_dictionary_lookup_key (_self->link_table, link, error);
}

struct shard_file_init_data {
  gpointer source_object;
  gpointer original_user_data;
  GAsyncReadyCallback original_callback;
};

static void
dm_shard_eos_shard_init_callback (GObject *source_object,
                                  GAsyncResult *result,
                                  gpointer user_data)
{
  struct shard_file_init_data *data = user_data;
  GTask *task = g_task_new (data->source_object, NULL,
                            data->original_callback,
                            data->original_user_data);

  if (g_task_is_valid (result, source_object))
    {
      DmShardEosShard *self = DM_SHARD_EOS_SHARD (data->source_object);
      g_autoptr(EosShardRecord) record = eos_shard_shard_file_find_record_by_hex_name (self->shard_file,
                                                                                       LINK_TABLE_ID);
      if (record)
        self->link_table = eos_shard_blob_load_as_dictionary (record->data, NULL);

      g_task_return_boolean (task, TRUE);
    }
  else
    {
      GError *error = g_error_new (g_quark_from_static_string ("dm-shard-eos-shard-error-quark"), 1, "foo");
      g_task_return_error (task, error);
    }

  g_free (data);
}

static void
dm_shard_eos_shard_init_async (GAsyncInitable      *initable,
                               int                  io_priority,
                               GCancellable        *cancellable,
                               GAsyncReadyCallback  callback,
                               gpointer             user_data)
{
  DmShardEosShard *self = DM_SHARD_EOS_SHARD (initable);
  GAsyncInitable *shard_file_initable = G_ASYNC_INITABLE (self->shard_file);

  struct shard_file_init_data *data = g_malloc (sizeof(struct shard_file_init_data));
  data->source_object = self;
  data->original_user_data = user_data;
  data->original_callback = callback;

  g_async_initable_init_async (shard_file_initable,
                               io_priority,
                               cancellable,
                               dm_shard_eos_shard_init_callback,
                               data);
}

static gboolean
dm_shard_eos_shard_init_finish (GAsyncInitable *initable,
                                GAsyncResult   *result,
                                GError        **error)
{
  g_return_val_if_fail (g_task_is_valid (result, initable), FALSE);

  return g_task_propagate_boolean (G_TASK (result), error);
}

static void
dm_shard_eos_shard_class_init (DmShardEosShardClass *klass)
{
  DmShardClass *dm_shard_class = DM_SHARD_CLASS (klass);

  dm_shard_class->find_by_id = dm_shard_eos_shard_find_by_id;
  dm_shard_class->get_model = dm_shard_eos_shard_get_model;
  dm_shard_class->stream_data = dm_shard_eos_shard_stream_data;
  dm_shard_class->get_data_size = dm_shard_eos_shard_get_data_size;
  dm_shard_class->test_link = dm_shard_eos_shard_test_link;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = dm_shard_eos_shard_constructed;
  object_class->finalize = dm_shard_eos_shard_finalize;
}

static void
g_async_initable_interface_init (GAsyncInitableIface *iface)
{
  iface->init_async = dm_shard_eos_shard_init_async;
  iface->init_finish = dm_shard_eos_shard_init_finish;
}

static void
dm_shard_eos_shard_init (G_GNUC_UNUSED DmShardEosShard *self)
{
}
