/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2020 Endless Mobile, Inc. */

#include "dm-shard-record.h"

/**
 * SECTION:shard-record
 * @title: Shard record
 * @short_description: An object holding a shard record
 *
 * This object holds information about a single shard record.
 */
struct _DmShardRecord {
  gpointer shard;
  gpointer native_record;
  GDestroyNotify native_record_destroy_func;
  guint ref_count;
};

G_DEFINE_BOXED_TYPE (DmShardRecord, dm_shard_record, dm_shard_record_ref, dm_shard_record_unref)

/**
 * dm_shard_record_new:
 * @shard: The #DmShard object holding the record
 * @native_record: The library specific record object
 * @native_record_destroy_func: A function to free "native_record"
 *
 * Creates a new #DmShardRecord.
 *
 * Returns: (transfer full): A newly created #DmShardRecord
 */
DmShardRecord *
dm_shard_record_new (gpointer shard,
                     gpointer native_record,
                     GDestroyNotify native_record_destroy_func)
{
  DmShardRecord *self;

  self = g_slice_new0 (DmShardRecord);
  self->shard = shard;
  self->native_record = native_record;
  self->native_record_destroy_func = native_record_destroy_func;
  self->ref_count = 1;

  return self;
}

/**
 * dm_shard_record_copy:
 * @self: a #DmShardRecord
 *
 * Makes a deep copy of a #DmShardRecord.
 *
 * Returns: (transfer full): A newly created #DmShardRecord with the same
 *   contents as @self
 */
DmShardRecord *
dm_shard_record_copy (DmShardRecord *self)
{
  DmShardRecord *copy;

  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (self->ref_count, NULL);

  copy = dm_shard_record_new (self->shard,
                              self->native_record,
                              self->native_record_destroy_func);
  return copy;
}

static void
dm_shard_record_free (DmShardRecord *self)
{
  g_assert (self);
  g_assert_cmpint (self->ref_count, ==, 0);

  g_clear_pointer (&self->native_record, self->native_record_destroy_func);
  g_slice_free (DmShardRecord, self);
}

/**
 * dm_shard_record_ref:
 * @self: A #DmShardRecord
 *
 * Increments the reference count of @self by one.
 *
 * Returns: (transfer full): @self
 */
DmShardRecord *
dm_shard_record_ref (DmShardRecord *self)
{
  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (self->ref_count, NULL);

  g_atomic_int_inc (&self->ref_count);

  return self;
}

/**
 * dm_shard_record_unref:
 * @self: A #DmShardRecord
 *
 * Decrements the reference count of @self by one, freeing the structure when
 * the reference count reaches zero.
 */
void
dm_shard_record_unref (DmShardRecord *self)
{
  g_return_if_fail (self);
  g_return_if_fail (self->ref_count);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    dm_shard_record_free (self);
}

/**
 * dm_shard_record_get_shard:
 * @self: A #DmShardRecord
 *
 * Returns: (transfer none): The owner of the record..
 */
gpointer
dm_shard_record_get_shard (DmShardRecord *self)
{
  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (self->ref_count, NULL);

  return self->shard;
}

/**
 * dm_shard_record_get_native:
 * @self: A #DmShardRecord
 *
 * Returns: (transfer none): The underlying library dependant object behind
 *   this record.
 */
gpointer
dm_shard_record_get_native (DmShardRecord *self)
{
  g_return_val_if_fail (self, NULL);
  g_return_val_if_fail (self->ref_count, NULL);

  return self->native_record;
}
