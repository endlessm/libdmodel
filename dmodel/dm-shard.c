/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2020 Endless Mobile, Inc. */

#include "dm-shard.h"

/**
 * SECTION:shard
 * @title: Shard
 * @short_description: The shard base class
 *
 * This abstract class must be inherited by the different shard classes
 * each of those with a different supported shard backend.
 */
typedef struct
{
  gchar *path;
  gint64 db_offset_override;
  gint64 calculated_db_offset;
} DmShardPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (DmShard, dm_shard, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_PATH,
  PROP_DB_OFFSET_OVERRIDE,
  N_PROPS
};

static GParamSpec *properties [N_PROPS];

static void
dm_shard_finalize (GObject *object)
{
  DmShardPrivate *priv = dm_shard_get_instance_private (DM_SHARD (object));

  g_clear_pointer (&priv->path, g_free);

  G_OBJECT_CLASS (dm_shard_parent_class)->finalize (object);
}

static void
dm_shard_get_property (GObject    *object,
                       guint       prop_id,
                       GValue     *value,
                       GParamSpec *pspec)
{
  DmShardPrivate *priv = dm_shard_get_instance_private (DM_SHARD (object));

  switch (prop_id)
    {
    case PROP_PATH:
      g_value_set_string (value, priv->path);
      break;

    case PROP_DB_OFFSET_OVERRIDE:
      g_value_set_int64 (value, priv->db_offset_override);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_shard_set_property (GObject      *object,
                       guint         prop_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  DmShardPrivate *priv = dm_shard_get_instance_private (DM_SHARD (object));

  switch (prop_id)
    {
    case PROP_PATH:
      g_clear_pointer (&priv->path, g_free);
      priv->path = g_value_dup_string (value);
      break;

    case PROP_DB_OFFSET_OVERRIDE:
      priv->db_offset_override = g_value_get_int64 (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_shard_class_init (DmShardClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dm_shard_finalize;
  object_class->get_property = dm_shard_get_property;
  object_class->set_property = dm_shard_set_property;

  properties [PROP_PATH] =
    g_param_spec_string ("path",
                         "Path",
                         "Path",
                         NULL,
                         (G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT |
                          G_PARAM_STATIC_STRINGS));

  properties [PROP_DB_OFFSET_OVERRIDE] =
    g_param_spec_int64 ("db-offset-override",
                        "DB Offset Override",
                        "DB Offset Override",
                        G_MININT64,
                        G_MAXINT64,
                        -1,
                        (G_PARAM_READWRITE |
                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_properties (object_class, N_PROPS, properties);
}

static void
dm_shard_init (DmShard *self)
{
  DmShardPrivate *priv = dm_shard_get_instance_private (self);
  priv->db_offset_override = -1;
  priv->calculated_db_offset = -1;
}

/**
 * dm_shard_find_by_id:
 * @self: the #DmShard object
 * @object_id: The object_id to find
 *
 * Look for a record in a shard.
 *
 * Returns: (transfer full): A matching record for the specified
 *  object_id or %NULL if no record was found.
 */
DmShardRecord *
dm_shard_find_by_id (DmShard *self,
                     const char *object_id)
{
  DmShardClass *klass;

  g_return_val_if_fail (DM_IS_SHARD (self), NULL);

  klass = DM_SHARD_GET_CLASS (self);
  g_return_val_if_fail (klass->find_by_id != NULL, NULL);
  return klass->find_by_id (self, object_id);
}

/**
 * dm_shard_get_model:
 * @self: the #DmShard object
 * @record: the #DmShardRecord belonged by the shard
 * @cancellable: (nullable): a #GCancellable
 * @error: (nullable): return location for an error, or %NULL
 *
 * Get the #DmContent for an found shard record.
 *
 * Returns: (transfer full): The #DmContent representing the content
 *   of %record.
 */
DmContent *
dm_shard_get_model (DmShard *self, DmShardRecord *record,
                    GCancellable *cancellable, GError **error)
{
  DmShardClass *klass;

  g_return_val_if_fail (DM_IS_SHARD (self), NULL);

  klass = DM_SHARD_GET_CLASS (self);
  g_return_val_if_fail (klass->get_model != NULL, NULL);
  return klass->get_model (self, record, cancellable, error);
}

/**
 * dm_shard_stream_data:
 * @self: the #DmShard object
 * @record: the #DmShardRecord belonged by the shard
 * @cancellable: (nullable): a #GCancellable
 * @error: (nullable): return location for an error, or %NULL
 *
 * Get a stream to read the record data.
 *
 * Returns: (transfer full): A stream to read the record data.
 */
GInputStream *
dm_shard_stream_data (DmShard *self, DmShardRecord *record,
                      GCancellable *cancellable, GError **error)
{
  DmShardClass *klass;

  g_return_val_if_fail (DM_IS_SHARD (self), NULL);

  klass = DM_SHARD_GET_CLASS (self);
  g_return_val_if_fail (klass->stream_data != NULL, NULL);
  return klass->stream_data (self, record, cancellable, error);
}

/**
 * dm_shard_get_data_size:
 * @self: the #DmShard object
 * @record: the #DmShardRecord belonged by the shard
 *
 * Get the record uncompressed data size.
 *
 * Returns: The record uncompressed data size.
 */
gsize
dm_shard_get_data_size (DmShard *self,
                        DmShardRecord *record)
{
  DmShardClass *klass;

  g_return_val_if_fail (DM_IS_SHARD (self), 0);

  klass = DM_SHARD_GET_CLASS (self);
  g_return_val_if_fail (klass->get_data_size != NULL, 0);
  return klass->get_data_size (self, record);
}

/**
 * dm_shard_test_link:
 * @self: the #DmShard object
 * @link: the link to test
 * @error: (nullable): return location for an error, or %NULL
 *
 * When supported, get the object URI for the specified link
 * using the shard's internal matching links dictionary.
 *
 * Returns: (transfer full): The object URI matching the link
 *   or NULL on error or if this feature is not supported.
 */
gchar *
dm_shard_test_link (DmShard *self,
                    const gchar *link,
                    GError **error)
{
  DmShardClass *klass;

  g_return_val_if_fail (DM_IS_SHARD (self), NULL);

  klass = DM_SHARD_GET_CLASS (self);
  g_return_val_if_fail (klass->test_link != NULL, NULL);
  return klass->test_link (self, link, error);
}

gchar *
dm_shard_get_path (DmShard *self)
{
  DmShardPrivate *priv = dm_shard_get_instance_private (self);
  return priv->path;
}

gint64
dm_shard_calculate_db_offset (DmShard *self)
{
  DmShardClass *klass;

  g_return_val_if_fail (DM_IS_SHARD (self), -1);

  klass = DM_SHARD_GET_CLASS (self);
  g_return_val_if_fail (klass->calculate_db_offset != NULL, -1);
  return klass->calculate_db_offset (self);
}

void
dm_shard_override_db_offset (DmShard *self,
                             gint64 db_offset)
{
  DmShardPrivate *priv = dm_shard_get_instance_private (self);
  priv->db_offset_override = db_offset;
}

gint64
dm_shard_get_db_offset (DmShard *self)
{
  DmShardPrivate *priv = dm_shard_get_instance_private (self);

  if (priv->db_offset_override >= 0)
    return priv->db_offset_override;

  if (priv->calculated_db_offset == -1)
    priv->calculated_db_offset = dm_shard_calculate_db_offset (self);

  return priv->calculated_db_offset;
}
