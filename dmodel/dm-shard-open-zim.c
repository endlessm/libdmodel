/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */

/* Copyright 2020 Endless Mobile, Inc. */

#include <zim-glib-3.0/file.h>

#include "dm-base.h"

#include "dm-shard.h"
#include "dm-shard-open-zim-private.h"
#include "stdio.h"

#define XAPIAN_FULLTEXT_INDEX_URL "X/fulltext/xapian"

/**
 * SECTION:shard-open-zim
 * @title: libzim shard implementation
 * @short_description: Implementation for libzim based shards
 *
 * This class implements the libzim based shards with an underlying
 * #OpenZimShardFile object.
 */
struct _DmShardOpenZim {
  GObject parent_instance;
  gchar *path;
  ZimFile *zim_file;
};

static void g_async_initable_interface_init (GAsyncInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (DmShardOpenZim, dm_shard_open_zim, DM_TYPE_SHARD,
                         G_IMPLEMENT_INTERFACE (G_TYPE_ASYNC_INITABLE,
                                                g_async_initable_interface_init))

/**
 * dm_shard_open_zim_new:
 * @path: The ZIM file path
 *
 * Returns: The newly created #DmShardOpenZim.
 */
DmShardOpenZim *
dm_shard_open_zim_new (const gchar *path)
{
  return DM_SHARD_OPEN_ZIM (g_object_new (DM_TYPE_SHARD_OPEN_ZIM, "path", path, NULL));
}

static void
dm_shard_open_zim_finalize (GObject *object)
{
  DmShardOpenZim *self = (DmShardOpenZim *)object;

  g_clear_pointer (&self->zim_file, g_object_unref);

  G_OBJECT_CLASS (dm_shard_open_zim_parent_class)->finalize (object);
}

static DmShardRecord *
dm_shard_open_zim_find_by_id (DmShard *self,
                              const char *object_id)
{
  DmShardOpenZim *_self = DM_SHARD_OPEN_ZIM (self);

  g_auto(GStrv) tokens = g_strsplit (object_id, "/", 2);

  ZimArticle *returned_article;
  ZimArticle *article = zim_file_get_article_by_namespace (_self->zim_file,
                                                           tokens[0][0],
                                                           tokens[1]);
  if (zim_article_good (article) && zim_article_is_redirect (article))
    {
      returned_article = zim_article_get_redirect_article (article);
      g_clear_pointer (&article, g_object_unref);
    }
  else if (zim_article_good (article))
    {
      returned_article = article;
    }
  else
    return NULL;

  return dm_shard_record_new (_self, returned_article, g_object_unref);
}

static DmContent *
dm_shard_open_zim_get_model (G_GNUC_UNUSED DmShard *self,
                             DmShardRecord *record,
                             G_GNUC_UNUSED GCancellable *cancellable,
                             GError **error)
{
  ZimArticle *zim_article = (ZimArticle *) dm_shard_record_get_native (record);
  JsonBuilder *builder = json_builder_new ();
  GSList *tags = NULL;

  json_builder_begin_object (builder);

  gchar namespace = zim_article_get_namespace (zim_article);
  gchar *type;
  switch (namespace)
  {
  case 'A':
    type = "ekn://_vocab/ArticleObject";
    tags = g_slist_append (tags, g_strdup ("EknArticleObject"));
    break;

  case 'I':
    type = "ekn://_vocab/ImageObject";
    tags = g_slist_append (tags, g_strdup ("EknMediaObject"));
    break;

  default:
    type = "ekn://_vocab/ContentObject";
    break;
  }
  json_builder_set_member_name (builder, "@type");
  json_builder_add_string_value (builder, type);

  g_autofree gchar *id = g_strdup_printf ("ekn+zim:///%c/%s",
                                          zim_article_get_namespace (zim_article),
                                          zim_article_get_url (zim_article));
  json_builder_set_member_name (builder, "@id");
  json_builder_add_string_value (builder, id);

  json_builder_set_member_name (builder, "title");
  json_builder_add_string_value (builder, zim_article_get_title (zim_article));

  json_builder_set_member_name (builder, "contentType");
  json_builder_add_string_value (builder, zim_article_get_mime_type (zim_article));

  json_builder_set_member_name (builder, "isServerTemplated");
  json_builder_add_boolean_value (builder, TRUE);

  json_builder_set_member_name (builder, "tags");
  json_builder_begin_array (builder);
  for (GSList *l = tags; l; l = g_slist_next (l))
    json_builder_add_string_value (builder, l->data);
  json_builder_end_array (builder);
  g_slist_free_full (tags, g_free);

  json_builder_end_object (builder);

  return dm_model_from_json_node (json_builder_get_root (builder), error);
}

static GInputStream *
dm_shard_open_zim_stream_data (G_GNUC_UNUSED DmShard *self,
                               DmShardRecord *record,
                               G_GNUC_UNUSED GCancellable *cancellable,
                               G_GNUC_UNUSED GError **error)
{
  ZimArticle *zim_article = (ZimArticle *) dm_shard_record_get_native (record);

  gsize size;
  const char *data = zim_article_get_data (zim_article, &size);

  return g_memory_input_stream_new_from_data (data, size, g_free);
}

static gsize
dm_shard_open_zim_get_data_size (G_GNUC_UNUSED DmShard *self,
                                 DmShardRecord *record)
{
  ZimArticle *zim_article = (ZimArticle *) dm_shard_record_get_native (record);
  return zim_article_get_data_size (zim_article);
}

static gint64
dm_shard_open_zim_calculate_db_offset (DmShard *self)
{
  DmShardRecord *record = dm_shard_open_zim_find_by_id (self, XAPIAN_FULLTEXT_INDEX_URL);

  if (!record)
  {
    g_warning ("The Xapian fulltext index for '%s' was not found", dm_shard_get_path (self));
    return -1;
  }

  ZimArticle *zim_article = (ZimArticle *) dm_shard_record_get_native (record);
  return zim_article_get_offset (zim_article);
}

static void
dm_shard_open_zim_init_async (GAsyncInitable      *initable,
                              G_GNUC_UNUSED int    io_priority,
                              GCancellable        *cancellable,
                              GAsyncReadyCallback  callback,
                              gpointer             user_data)
{
  DmShardOpenZim *self = DM_SHARD_OPEN_ZIM (initable);
  GTask *task = g_task_new (initable, cancellable, callback, user_data);
  g_autoptr(GError) error = NULL;

  self->zim_file = zim_file_new (dm_shard_get_path (DM_SHARD (initable)), &error);

  if (error)
    g_task_return_error (task, g_error_copy (error));
  else
    g_task_return_boolean (task, TRUE);
}

static gboolean
dm_shard_open_zim_init_finish (GAsyncInitable *initable,
                               GAsyncResult   *result,
                               GError        **error)
{
  g_return_val_if_fail (g_task_is_valid (result, initable), FALSE);

  return g_task_propagate_boolean (G_TASK (result), error);
}

static void
dm_shard_open_zim_class_init (DmShardOpenZimClass *klass)
{
  DmShardClass *dm_shard_class = DM_SHARD_CLASS (klass);

  dm_shard_class->find_by_id = dm_shard_open_zim_find_by_id;
  dm_shard_class->get_model = dm_shard_open_zim_get_model;
  dm_shard_class->stream_data = dm_shard_open_zim_stream_data;
  dm_shard_class->get_data_size = dm_shard_open_zim_get_data_size;
  dm_shard_class->calculate_db_offset = dm_shard_open_zim_calculate_db_offset;

  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = dm_shard_open_zim_finalize;
}

static void
g_async_initable_interface_init (GAsyncInitableIface *iface)
{
  iface->init_async = dm_shard_open_zim_init_async;
  iface->init_finish = dm_shard_open_zim_init_finish;
}

static void
dm_shard_open_zim_init (G_GNUC_UNUSED DmShardOpenZim *self)
{
}
