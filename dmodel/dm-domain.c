/* Copyright 2016 Endless Mobile, Inc. */

#include "dm-domain-private.h"

#include "dm-shard.h"
#include "dm-shard-eos-shard-private.h"
#include "dm-shard-open-zim-private.h"
#include "dm-database-manager-private.h"
#include "dm-base.h"
#include "dm-utils.h"
#include "dm-utils-private.h"

#include <string.h>

#define dm_domain_return_malformed_manifest(error,element) \
  G_STMT_START{                                            \
    g_set_error (error, DM_DOMAIN_ERROR,                   \
                 DM_DOMAIN_ERROR_BAD_MANIFEST,             \
                 "Manifest element '%s' is not valid",     \
                 element);                                 \
    return FALSE;                                          \
  }G_STMT_END;

GQuark
dm_domain_error_quark (void)
{
  return g_quark_from_static_string ("dm-domain-error-quark");
}

/**
 * SECTION:domain
 * @title: Domain
 * @short_description: Grab content for a specific application
 *
 * The domain object handles querying for content for a specific application.
 * Usually you will want to use #DmEngine rather than use this directly.
 */
struct _DmDomain
{
  GObject parent_instance;

  gchar *app_id;
  gchar *path;
  GList *subscriptions;

  char *language;

  DmDatabaseManager *db_manager;
  GMutex db_lock;
  gboolean using_3rd_party_search_index;

  // List of DmShard items
  GSList *shards;
};

static void initable_iface_init (GInitableIface *initable_iface);

G_DEFINE_TYPE_WITH_CODE (DmDomain, dm_domain, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, initable_iface_init))

enum {
  PROP_APP_ID = 1,
  PROP_PATH,
  PROP_LANGUAGE,

  NPROPS
};

static GParamSpec *dm_domain_props[NPROPS] = { NULL, };

static void
dm_domain_get_property (GObject *object,
                        guint prop_id,
                        GValue *value,
                        GParamSpec *pspec)
{
  DmDomain *self = DM_DOMAIN (object);

  switch (prop_id)
    {
    case PROP_APP_ID:
      g_value_set_string (value, self->app_id);
      break;

    case PROP_PATH:
      g_value_set_string (value, self->path);
      break;

    case PROP_LANGUAGE:
      g_value_set_string (value, self->language);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_domain_set_property (GObject *object,
                        guint prop_id,
                        const GValue *value,
                        GParamSpec *pspec)
{
  DmDomain *self = DM_DOMAIN (object);

  switch (prop_id)
    {
    case PROP_APP_ID:
      g_free (self->app_id);
      self->app_id = g_value_dup_string (value);
      break;

    case PROP_PATH:
      g_free (self->path);
      self->path = g_value_dup_string (value);
      break;

    case PROP_LANGUAGE:
      g_free (self->language);
      self->language = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_domain_finalize (GObject *object)
{
  DmDomain *self = DM_DOMAIN (object);

  g_free (self->app_id);
  g_free (self->path);
  g_free (self->language);

  g_clear_object (&self->db_manager);
  g_mutex_clear (&self->db_lock);

  g_list_free_full (self->subscriptions, g_free);

  g_slist_free_full (self->shards, g_object_unref);

  G_OBJECT_CLASS (dm_domain_parent_class)->finalize (object);
}

static void
dm_domain_class_init (DmDomainClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dm_domain_get_property;
  object_class->set_property = dm_domain_set_property;
  object_class->finalize = dm_domain_finalize;

  /**
   * DmDomain:app-id:
   *
   * The application id of the content this domain should be loading.
   */
  dm_domain_props[PROP_APP_ID] =
    g_param_spec_string ("app-id", "Application ID",
      "Application ID of the domain content",
      NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * DmDomain:path:
   *
   * Path to the domain content.
   */
  dm_domain_props[PROP_PATH] =
    g_param_spec_string ("path", "Path",
      "Path to the domain content",
      NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * DmDomain:language:
   *
   * The language used for the domain, as an ISO 639 locale code.
   */
  dm_domain_props[PROP_LANGUAGE] =
    g_param_spec_string ("language", "Language",
      "The language used by the domain, as an ISO 639 locale code",
      NULL,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class,
                                     NPROPS,
                                     dm_domain_props);
}

static void
dm_domain_init (G_GNUC_UNUSED DmDomain *self)
{
}

static gboolean
dm_domain_process_subscription (DmDomain *self,
                                GFile *subscription_dir,
                                GError **error)
{
  g_autofree gchar *subscription_path = g_file_get_path (subscription_dir);
  JsonParser *json_parser = json_parser_new ();
  g_autofree gchar *manifest_filename = g_build_filename (subscription_path, "manifest.json", NULL);
  gboolean parsing_success = json_parser_load_from_file (json_parser, manifest_filename, error);
  if (!parsing_success)
    return FALSE;

  g_autoptr(JsonNode) manifest_node = json_parser_get_root (json_parser);
  JsonObject *manifest = json_node_get_object (manifest_node);
  g_autoptr(GHashTable) db_offset_by_path = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                                   NULL, g_free);

  JsonNode *xapian_databases_node = json_object_get_member (manifest, "xapian_databases");
  if (xapian_databases_node && JSON_NODE_HOLDS_ARRAY (xapian_databases_node))
    {
      JsonArray *xapian_databases = json_node_get_array (xapian_databases_node);
      for (guint i = 0; i < json_array_get_length (xapian_databases); i++)
        {
          JsonNode *xapian_database_node = json_array_get_element (xapian_databases, i);
          if (!JSON_NODE_HOLDS_OBJECT (xapian_database_node))
            dm_domain_return_malformed_manifest (error, "xapian_databases.[]");

          JsonNode *path_node = json_object_get_member (json_node_get_object (xapian_database_node), "path");
          if (path_node == NULL || json_node_get_value_type (path_node) != G_TYPE_STRING)
            dm_domain_return_malformed_manifest (error, "xapian_databases.[].path");

          JsonNode *offset_node = json_object_get_member (json_node_get_object (xapian_database_node), "offset");
          if (offset_node == NULL || json_node_get_value_type (offset_node) != G_TYPE_INT64)
            dm_domain_return_malformed_manifest (error, "xapian_databases.[].offset");

          gint64 *offset = g_new (gint64, 1);
          *offset = json_node_get_int (offset_node);

          g_hash_table_insert (db_offset_by_path,
                               (gchar *) json_node_get_string (path_node),
                               offset);
        }
    }

  JsonNode *shards_node = json_object_get_member (manifest, "shards");
  if (shards_node == NULL || !JSON_NODE_HOLDS_ARRAY (shards_node))
    dm_domain_return_malformed_manifest (error, "shards");

  gchar *current_shard_type = NULL;
  JsonArray *shards_array = json_node_get_array (shards_node);
  for (guint i = 0; i < json_array_get_length (shards_array); i++)
    {
      DmShard *shard;

      JsonNode *shard_node = json_array_get_element (shards_array, i);
      if (!JSON_NODE_HOLDS_OBJECT (shard_node))
        dm_domain_return_malformed_manifest (error, "shards.[]");

      JsonNode *path_node = json_object_get_member (json_node_get_object (shard_node), "path");
      if (path_node == NULL || json_node_get_value_type (path_node) != G_TYPE_STRING)
        dm_domain_return_malformed_manifest (error, "shards.[].path");

      const gchar *relative_path = json_node_get_string (path_node);
      g_autofree gchar *path = g_build_filename (subscription_path, relative_path, NULL);

      JsonNode *type_node = json_object_get_member (json_node_get_object (shard_node), "type");
      gchar *type;
      if (type_node == NULL || json_node_get_value_type (type_node) != G_TYPE_STRING)
        type = "eosshard";
      else
        type = (gchar *) json_node_get_string (type_node);

      if (!current_shard_type)
        current_shard_type = type;
      else if (g_strcmp0 (type, current_shard_type) != 0)
        {
          g_set_error (error, DM_DOMAIN_ERROR, DM_DOMAIN_ERROR_BAD_MANIFEST,
                       "Mixing shard types is not yet supported");
          return FALSE;
        }

      if (g_strcmp0 (type, "eosshard") == 0)
        shard = DM_SHARD (dm_shard_eos_shard_new (path));
      else if (g_strcmp0 (type, "openzim") == 0)
        {
          shard = DM_SHARD (dm_shard_open_zim_new (path));
          self->using_3rd_party_search_index = TRUE;
        }
      else
        {
          g_set_error (error, DM_DOMAIN_ERROR, DM_DOMAIN_ERROR_BAD_MANIFEST,
                       "Invalid shard type \"%s\"", type);
          return FALSE;
        }

      gpointer db_offset = g_hash_table_lookup (db_offset_by_path, relative_path);
      if (db_offset)
        dm_shard_override_db_offset (shard, *((gint64 *) db_offset));

      self->shards = g_slist_append (self->shards, shard);
    }

  return TRUE;
}

static gboolean
dm_domain_import_subscriptions (DmDomain *self,
                                GFile *subscriptions_dir,
                                GCancellable *cancellable,
                                GError **error)
{
  g_autoptr(GFileEnumerator) subscriptions_iter = g_file_enumerate_children (subscriptions_dir, "",
                                                                             G_FILE_QUERY_INFO_NONE,
                                                                             cancellable, error);
  while (TRUE)
    {
      GFile *dir = NULL;
      GFileInfo *info = NULL;
      if (!g_file_enumerator_iterate (subscriptions_iter, &info, &dir, cancellable, error))
        return FALSE;
      if (dir == NULL)
        break;
      if (g_file_info_get_file_type (info) != G_FILE_TYPE_DIRECTORY)
        continue;
      if (!dm_domain_process_subscription (self, dir, error))
          return FALSE;

      self->subscriptions = g_list_prepend (self->subscriptions, g_strdup (g_file_info_get_name (info)));
    }

  return TRUE;
}

static gboolean
is_directory (GFile *file,
              GCancellable *cancellable)
{
  return g_file_query_file_type (file, G_FILE_QUERY_INFO_NONE, cancellable) == G_FILE_TYPE_DIRECTORY;
}

static gboolean
dm_domain_initable_init (GInitable *initable,
                         GCancellable *cancellable,
                         GError **error)
{
  DmDomain *self = DM_DOMAIN (initable);

  gboolean has_app_id = (self->app_id != NULL && *self->app_id != '\0');
  gboolean has_path = (self->path != NULL && *self->path != '\0');

  self->using_3rd_party_search_index = FALSE;

  if (has_path)
    {
      g_autoptr(GFile) path_file = g_file_new_for_path (self->path);
      if (!is_directory (path_file, cancellable))
        {
          g_set_error (error, DM_DOMAIN_ERROR, DM_DOMAIN_ERROR_PATH_NOT_FOUND,
                       "%s is not a valid directory", self->path);
          return FALSE;
        }

      if (!dm_domain_process_subscription (self, path_file, error))
        return FALSE;
    }
  else if (has_app_id)
    {
      /* Import subscriptions from data directory */
      g_autoptr(GFile) content_dir = dm_get_data_dir (self->app_id);
      g_autoptr(GFile) subscriptions_dir = g_file_get_child (content_dir,
                                                             "com.endlessm.subscriptions");
      if (is_directory (subscriptions_dir, cancellable))
        {
          if (!dm_domain_import_subscriptions (self, subscriptions_dir,
                                               cancellable, error))
              return FALSE;
        }

      /* Import subscriptions from extensions directories */
      g_autoptr(GList) extensions_dirs = dm_get_extensions_dirs (self->app_id);
      for (GList *l = extensions_dirs; l != NULL; l = l->next)
        {
          GFile *extension_dir = l->data;
          if (is_directory (extension_dir, cancellable))
            {
              if (!dm_domain_import_subscriptions (self, extension_dir,
                                                   cancellable, error))
                  return FALSE;
            }
        }
    }
  else
    {
      g_set_error (error, DM_DOMAIN_ERROR, DM_DOMAIN_ERROR_APP_ID_NOT_SET,
                   "You must set an app id or path to initialize a domain object");
      return FALSE;
    }

  if (self->shards == NULL)
    {
      g_set_error (error, DM_DOMAIN_ERROR, DM_DOMAIN_ERROR_EMPTY,
                   "Content is empty");
      return FALSE;
    }

  self->db_manager = dm_database_manager_new (self->shards);
  g_mutex_init (&self->db_lock);

  if (!dm_utils_parallel_init (self->shards, 0, cancellable, error))
    return FALSE;

  return TRUE;
}

static void
initable_iface_init (GInitableIface *initable_iface)
{
  initable_iface->init = dm_domain_initable_init;
}

static DmShardRecord *
dm_domain_load_record (DmDomain *self,
                       const char *uri,
                       GError **error)
{
  g_autofree gchar *object_id = (gchar *) dm_utils_uri_get_object_id (uri);
  if (!object_id)
    {
      g_set_error (error, DM_DOMAIN_ERROR, DM_DOMAIN_ERROR_ID_NOT_VALID,
                   "The asset URI is not valid: %s", uri);
      return NULL;
    }

  DmShardRecord *record = NULL;
  for (GSList *l = self->shards; l && !record; l = g_slist_next (l))
    record = dm_shard_find_by_id (l->data, object_id);
  return record;
}

/**
 * dm_domain_get_subscription_id:
 * @self: the domain
 *
 * Gets subscription id of the domain
 *
 * Returns: (transfer none): the subscription id
 *
 * Deprecated: Use dm_domain_get_subscription_ids() instead.
 */
const gchar *
dm_domain_get_subscription_id (DmDomain *self)
{
  g_return_val_if_fail (DM_IS_DOMAIN (self), NULL);

  return (self->subscriptions) ? self->subscriptions->data : NULL;
}

/**
 * dm_domain_get_subscription_ids:
 * @self: the domain
 *
 * Gets subscription IDs belonging to the domain.
 *
 * Returns: (transfer container): a list of the subscription IDs.
 */
gchar * const *
dm_domain_get_subscription_ids (DmDomain *self)
{
  g_return_val_if_fail (DM_IS_DOMAIN (self), NULL);

  guint num_subscriptions = g_list_length (self->subscriptions);
  gchar **retval = g_new0 (gchar *, num_subscriptions + 1);
  for (guint ix = 0; ix < num_subscriptions; ix++)
    {
      retval[ix] = g_list_nth_data (self->subscriptions, ix);
    }
  return retval;
}

/**
 * dm_domain_get_shards:
 * @self: the domain
 *
 * Gets the list of shard files in the domain.
 *
 * Returns: (element-type DmShard) (transfer none): the shards
 */
GSList *
dm_domain_get_shards (DmDomain *self)
{
  g_return_val_if_fail (DM_IS_DOMAIN (self), NULL);

  return self->shards;
}

/**
 * dm_domain_test_link:
 * @self: the domain
 * @link: the URI to check for
 * @error: #GError for error reporting.
 *
 * Attempts to determine if the given link corresponds to content within
 * this domain.
 *
 * Returns: (transfer full) (nullable): Returns an ID to that content if
 * so, and %NULL otherwise.
 */
gchar *
dm_domain_test_link (DmDomain *self,
                     const gchar *link,
                     GError **error)
{
  gchar *object_uri = NULL;
  for (GSList *l = self->shards; l && !object_uri && !*error; l = g_slist_next (l))
    object_uri = dm_shard_test_link (l->data, link, error);
  return object_uri;
}

/**
 * dm_domain_get_object:
 * @self: the domain
 * @uri: the URI of the object to load
 * @cancellable: (allow-none): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): callback to call when the request is satisfied.
 * @user_data: (closure): the data to pass to callback function.
 *
 * Asynchronously load an object model for the given ID
 */
void
dm_domain_get_object (DmDomain *self,
                      const char *uri,
                      GCancellable *cancellable,
                      GAsyncReadyCallback callback,
                      gpointer user_data)
{
  g_return_if_fail (DM_IS_DOMAIN (self));
  g_return_if_fail (G_IS_CANCELLABLE (cancellable) || cancellable == NULL);

  GError *error = NULL;

  g_autoptr(GTask) task = g_task_new (self, cancellable, callback, user_data);

  g_autoptr(DmShardRecord) record = dm_domain_load_record (self, uri, NULL);
  if (record == NULL)
    {
      g_task_return_new_error (task, DM_DOMAIN_ERROR, DM_DOMAIN_ERROR_ID_NOT_FOUND,
                               "Could not find shard record for URI %s", uri);
      return;
    }

  DmContent *model = dm_shard_get_model (dm_shard_record_get_shard (record),
                                         record, cancellable, &error);

  if (error != NULL)
    {
      g_task_return_error (task, error);
      return;
    }

  g_task_return_pointer (task, model, g_object_unref);
}

/**
 * dm_domain_get_object_finish:
 * @self: domain
 * @result: the #GAsyncResult that was provided to the callback.
 * @error: #GError for error reporting.
 *
 * Finish a dm_domain_get_object call.
 *
 * Returns: (transfer full): the content object model that was fetched
 */
DmContent *
dm_domain_get_object_finish (DmDomain *self,
                             GAsyncResult *result,
                             GError **error)
{
  g_return_val_if_fail (DM_IS_DOMAIN (self), NULL);
  g_return_val_if_fail (g_task_is_valid (result, self), NULL);

  return g_task_propagate_pointer (G_TASK (result), error);
}

static DmContent *
dm_domain_get_object_sync (DmDomain *self,
                           const char *uri,
                           GCancellable *cancellable,
                           GError **error)
{
  g_autoptr(DmShardRecord) record = dm_domain_load_record (self, uri, NULL);
  if (record == NULL)
    {
      g_set_error (error, DM_DOMAIN_ERROR, DM_DOMAIN_ERROR_ID_NOT_FOUND,
                   "Could not find shard record for URI %s", uri);
      return NULL;
    }

  return dm_shard_get_model (dm_shard_record_get_shard (record),
                             record, cancellable, error);
}

typedef struct
{
  DmDomain *domain;

  DmQuery *query;

  DmDatabaseManager *db_manager;

  char *fixed_stop_terms;
  char *fixed_spell_terms;
} RequestState;

static void
request_state_free (gpointer data)
{
  RequestState *state = data;

  g_free (state->fixed_stop_terms);
  g_free (state->fixed_spell_terms);

  g_clear_object (&state->domain);
  g_clear_object (&state->query);
  g_clear_object (&state->db_manager);

  g_slice_free (RequestState, state);
}

static void
query_fix_task (GTask *task,
                gpointer source_obj,
                gpointer task_data,
                G_GNUC_UNUSED GCancellable *cancellable)
{
  DmDomain *self = source_obj;
  RequestState *request = task_data;
  GError *error = NULL;

  if (g_task_return_error_if_cancelled (task))
    return;

  g_autoptr(GMutexLocker) db_lock = g_mutex_locker_new (&self->db_lock);

  if (request->domain->using_3rd_party_search_index)
    g_object_set (request->query,
                  "match", DM_QUERY_MATCH_TITLE_SYNOPSIS,
                  "tags-match-all", NULL,
                  "tags-match-any", NULL,
                  "content-type", NULL,
                  "excluded-content-type", NULL,
                  NULL);

  dm_database_manager_fix_query (request->db_manager,
                                 dm_query_get_search_terms (request->query),
                                 &request->fixed_stop_terms,
                                 &request->fixed_spell_terms, &error);
  if (error != NULL)
    {
      g_task_return_error (task, error);
      return;
    }

  g_task_return_boolean (task, TRUE);
}

/**
 * dm_domain_get_fixed_query:
 * @self: the domain
 * @query: the query object to fix
 * @cancellable: (allow-none): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): callback to call when the request is satisfied.
 * @user_data: (closure): the data to pass to callback function.
 *
 * Asynchronously queries the associated Xapian database to retrieve a
 * corrected version of the given query object. The corrections can be zero or
 * more of the following:
 *      - the query with its stop words removed
 *      - the query which has had spelling correction applied to it.
 *
 * Note that the spelling correction will be performed on the original
 * search terms string, and not the string with stop words removed.
 */
void
dm_domain_get_fixed_query (DmDomain *self,
                           DmQuery *query,
                           GCancellable *cancellable,
                           GAsyncReadyCallback callback,
                           gpointer user_data)
{
  g_return_if_fail (DM_IS_DOMAIN (self));
  g_return_if_fail (DM_IS_QUERY (query));
  g_return_if_fail (G_IS_CANCELLABLE (cancellable) || cancellable == NULL);

  GTask *task = g_task_new (self, cancellable, callback, user_data);

  RequestState *state = g_slice_new0 (RequestState);
  state->domain = g_object_ref (self);
  state->db_manager = g_object_ref (self->db_manager);
  state->query = g_object_ref (query);

  g_task_set_task_data (task, state, request_state_free);

  g_task_run_in_thread (task, query_fix_task);
  g_object_unref (task);
}

/**
 * dm_domain_get_fixed_query_finish:
 * @self: domain
 * @result: the #GAsyncResult that was provided to the callback.
 * @error: #GError for error reporting.
 *
 * Finish a dm_domain_get_fixed_query call.
 *
 * Returns: (transfer full): a new query object with the fixed query.
 */
DmQuery *
dm_domain_get_fixed_query_finish (DmDomain *self,
                                  GAsyncResult *result,
                                  GError **error)
{
  g_return_val_if_fail (DM_IS_DOMAIN (self), NULL);
  g_return_val_if_fail (G_IS_TASK (result), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  gboolean success = g_task_propagate_boolean (G_TASK (result), error);

  if (success)
    {
      RequestState *request = g_task_get_task_data (G_TASK (result));

      /* If we didn't get a corrected query, we can just reuse the existing query object */
      if (request->fixed_stop_terms == NULL && request->fixed_spell_terms == NULL)
        return g_object_ref (request->query);

      if (request->fixed_stop_terms != NULL && request->fixed_spell_terms != NULL)
        return dm_query_new_from_object (request->query,
                                         "stopword-free-terms", request->fixed_stop_terms,
                                         "corrected-terms", request->fixed_spell_terms,
                                         NULL);
      else if (request->fixed_stop_terms != NULL)
        return dm_query_new_from_object (request->query,
                                         "stopword-free-terms", request->fixed_stop_terms,
                                         NULL);
      else
        return dm_query_new_from_object (request->query,
                                         "corrected-terms", request->fixed_spell_terms,
                                         NULL);
    }

  return NULL;
}

static void
query_task (GTask *task,
            gpointer source_object,
            gpointer task_data,
            G_GNUC_UNUSED GCancellable *cancellable)
{
  RequestState *state = task_data;
  DmDomain *self = source_object;
  GError *error = NULL;

  if (g_task_return_error_if_cancelled (task))
    return;

  g_autoptr(GMutexLocker) db_lock = g_mutex_locker_new (&self->db_lock);

  const char *lang = self->language;
  if (lang == NULL || *lang == '\0')
    lang = "none";

  g_autoptr(XapianMSet) results =
    dm_database_manager_query (state->db_manager, state->query, lang, &error);
  if (error != NULL)
    {
      g_task_return_error (task, error);
      return;
    }

  int n_results = xapian_mset_get_size (results);
  int upper_bound = xapian_mset_get_matches_upper_bound (results);

  g_debug (G_STRLOC ": Found %d results (upper bound: %d)\n", n_results, upper_bound);

  GList *models = NULL;

  g_autoptr(XapianMSetIterator) iter = xapian_mset_get_begin (results);
  while (xapian_mset_iterator_next (iter))
    {
      GError *internal_error = NULL;

      XapianDocument *document = xapian_mset_iterator_get_document (iter, &internal_error);
      if (internal_error != NULL)
        {
          g_debug ("INTERNAL: Unable to fetch document from iterator: %s",
                   internal_error->message);
          g_error_free (internal_error);
          continue;
        }

      g_autofree char *document_data = xapian_document_get_data (document);
      g_autofree char *uri = NULL;

      if (!g_str_has_prefix (document_data, "ekn://"))
        uri = g_strconcat ("ekn+zim:///", document_data, NULL);
      else
        uri = g_strdup (document_data);

      g_debug ("Retrieving document object '%s'\n", uri);

      DmContent *model =
        dm_domain_get_object_sync (self, uri, NULL, &internal_error);

      if (internal_error != NULL)
        {
          g_task_return_error (task, internal_error);
          return;
        }

      models = g_list_prepend (models, model);
    }

  g_debug ("Models found: %d of %d matches", g_list_length (models), n_results);

  DmQueryResults *query_results =
    g_object_new (DM_TYPE_QUERY_RESULTS,
                  "upper-bound", upper_bound,
                  "models", g_list_reverse (models),
                  NULL);

  g_task_return_pointer (task, query_results, g_object_unref);
}

/**
 * dm_domain_query:
 * @self: the domain
 * @query: the query object
 * @cancellable: (allow-none): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): callback to call when the request is satisfied.
 * @user_data: (closure): the data to pass to callback function.
 *
 * Asynchronously fetch a #DmQueryResults for a #DmQuery.
 */
void
dm_domain_query (DmDomain *self,
                 DmQuery *query,
                 GCancellable *cancellable,
                 GAsyncReadyCallback callback,
                 gpointer user_data)
{
  g_return_if_fail (DM_IS_DOMAIN (self));
  g_return_if_fail (DM_IS_QUERY (query));
  g_return_if_fail (G_IS_CANCELLABLE (cancellable) || cancellable == NULL);

  GTask *task = g_task_new (self, cancellable, callback, user_data);

  RequestState *state = g_slice_new0 (RequestState);
  state->domain = g_object_ref (self);
  state->db_manager = g_object_ref (self->db_manager);
  state->query = g_object_ref (query);

  g_task_set_task_data (task, state, request_state_free);

  g_task_run_in_thread (task, query_task);
  g_object_unref (task);
}

/**
 * dm_domain_query_finish:
 * @self: domain
 * @result: the #GAsyncResult that was provided to the callback.
 * @error: #GError for error reporting.
 *
 * Finish a dm_domain_query() call.
 *
 * Returns: (transfer full): the results object
 */
DmQueryResults *
dm_domain_query_finish (DmDomain *self,
                        GAsyncResult *result,
                        GError **error)
{
  g_return_val_if_fail (DM_IS_DOMAIN (self), NULL);
  g_return_val_if_fail (G_IS_TASK (result), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return g_task_propagate_pointer (G_TASK (result), error);
}

/**
 * dm_domain_read_uri:
 * @self: domain
 * @uri: the ekn uri to read
 * @bytes: (out) (transfer full) (allow-none): return location for the contents GBytes
 * @mime_type: (out) (transfer full) (allow-none): return location for the content mime type
 * @error: #GError for error reporting
 *
 * Reads the contents of a ekn uri and returns a GBytes of the contents and the
 * contents mime type, if the ekn uri contents was found.
 *
 * Returns: true if the uri was successfully searched for, false if an error occurred
 */
gboolean
dm_domain_read_uri (DmDomain *self,
                    const char *uri,
                    GBytes **bytes,
                    const char **mime_type,
                    GError **error)
{
  g_autoptr(GError) internal_error = NULL;
  g_autoptr(DmShardRecord) record = dm_domain_load_record (self, uri, &internal_error);

  if (!record)
    {
      if (internal_error)
        {
          g_propagate_error (error, internal_error);
          return FALSE;
        }

      return TRUE;
    }

  DmContent *model = dm_shard_get_model (dm_shard_record_get_shard (record),
                                         record, NULL, NULL);
  if (model)
    g_object_get (model, "content-type", *mime_type, NULL);

  if (bytes)
    {
      g_autoptr(GInputStream) stream = dm_shard_stream_data (dm_shard_record_get_shard (record),
                                                             record, NULL, &internal_error);
      if (!internal_error)
        {
          gsize size = dm_shard_get_data_size (dm_shard_record_get_shard (record), record);
          *bytes = g_input_stream_read_bytes (stream, size, NULL, &internal_error);
        }

      if (internal_error)
        {
          g_propagate_error (error, internal_error);
          return FALSE;
        }
    }

  return TRUE;
}

/*< private >
 * dm_domain_get_for_app_id:
 * @app_id: the domains app id
 * @path: path to the content
 * @language: the language used by the domain
 * @cancellable: optional #GCancellable object, %NULL to ignore.
 * @error: #GError for error reporting
 *
 * Gets a domain object for a given app id.
 *
 * Returns: (transfer full): the newly created domain object
 */
DmDomain *
dm_domain_get_for_app_id (const char *app_id,
                          const char *path,
                          const char *language,
                          GCancellable *cancellable,
                          GError **error)
{
  DmDomain *domain = g_object_new (DM_TYPE_DOMAIN,
                                   "app-id", app_id,
                                   "path", path,
                                   "language", language,
                                   NULL);

  if (!g_initable_init (G_INITABLE (domain), cancellable, error))
    g_clear_pointer (&domain, g_object_unref);

  return domain;
}
