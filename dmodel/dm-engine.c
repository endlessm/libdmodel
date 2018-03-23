/* Copyright 2016 Endless Mobile, Inc. */

#include "dm-engine.h"

#include "dm-domain-private.h"
#include "dm-utils.h"

/**
 * SECTION:engine
 * @title: Engine
 * @short_description: Grab content for a specific application
 *
 * The knowledge content engine is the main portal for query and fetching
 * content for knowledge applications. Usually you will be using this to fetch
 * content for a single application, in which case you should set the
 * #DmEngine:default-app-id property.
 */
struct _DmEngine
{
  GObject parent_instance;

  gchar *default_app_id;
  gchar *language;

  // Hash table with app id string keys, DmDomain values
  GHashTable *domains;
};

G_DEFINE_TYPE (DmEngine, dm_engine, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_DEFAULT_APP_ID,
  PROP_LANGUAGE,
  NPROPS
};

static GParamSpec *dm_engine_props[NPROPS] = { NULL, };

static void
dm_engine_get_property (GObject *object,
                        guint prop_id,
                        GValue *value,
                        GParamSpec *pspec)
{
  DmEngine *self = DM_ENGINE (object);

  switch (prop_id)
    {
    case PROP_DEFAULT_APP_ID:
      g_value_set_string (value, self->default_app_id);
      break;

    case PROP_LANGUAGE:
      g_value_set_string (value, self->language);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_engine_set_property (GObject *object,
                        guint prop_id,
                        const GValue *value,
                        GParamSpec *pspec)
{
  DmEngine *self = DM_ENGINE (object);

  switch (prop_id)
    {
    case PROP_DEFAULT_APP_ID:
      g_clear_pointer (&self->default_app_id, g_free);
      self->default_app_id = g_value_dup_string (value);
      break;

    case PROP_LANGUAGE:
      g_clear_pointer (&self->language, g_free);
      self->language = g_value_dup_string (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_engine_finalize (GObject *object)
{
  DmEngine *self = DM_ENGINE (object);

  g_clear_pointer (&self->default_app_id, g_free);
  g_clear_pointer (&self->language, g_free);
  g_clear_pointer (&self->domains, g_hash_table_unref);

  G_OBJECT_CLASS (dm_engine_parent_class)->finalize (object);
}

static void
dm_engine_class_init (DmEngineClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dm_engine_get_property;
  object_class->set_property = dm_engine_set_property;
  object_class->finalize = dm_engine_finalize;

  /**
   * DmEngine:default-app-id:
   *
   * The default application the engine should serve up content for.
   */
  dm_engine_props[PROP_DEFAULT_APP_ID] =
    g_param_spec_string ("default-app-id", "Default Application ID",
      "Default Application ID",
      "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  /**
   * DmEngine:language:
   *
   * The ISO639 language code which will be used for various search features,
   * such as term stemming and spelling correction.
   */
  dm_engine_props[PROP_LANGUAGE] =
    g_param_spec_string ("language", "Language",
      "The language to use",
      "", G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class,
                                     NPROPS,
                                     dm_engine_props);
}

static void
dm_engine_init (DmEngine *self)
{
  self->domains = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
}

/**
 * dm_engine_test_link:
 * @self: the engine
 * @link: the ekn id of link to check for
 * @error: #GError for error reporting.
 *
 * Attempts to determine if the given link corresponds to content within
 * the default domain.
 *
 * Returns: (transfer full) (nullable): Returns an EKN URI to that content if
 * so, and %NULL otherwise.
 */
gchar *
dm_engine_test_link (DmEngine *self,
                     const char *link,
                     GError **error)
{
  g_return_val_if_fail (DM_IS_ENGINE (self), NULL);

  return dm_engine_test_link_for_app (self, link, self->default_app_id, error);
}

/**
 * dm_engine_test_link_for_app:
 * @self: the engine
 * @link: the ekn id of link to check for
 * @app_id: the id of the application to load the object from
 * @error: #GError for error reporting.
 *
 * Attempts to determine if the given link corresponds to content within
 * the domain for the given application id.
 *
 * Returns: (transfer full) (nullable): Returns an EKN URI to that content if
 * so, and %NULL otherwise.
 */
gchar *
dm_engine_test_link_for_app (DmEngine *self,
                             const char *link,
                             const char *app_id,
                             GError **error)
{
  g_return_val_if_fail (DM_IS_ENGINE (self), NULL);
  g_return_val_if_fail (link && *link, NULL);
  g_return_val_if_fail (app_id && *app_id, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  DmDomain *domain = dm_engine_get_domain_for_app (self, app_id, error);
  if (domain == NULL)
    return NULL;
  return dm_domain_test_link (domain, link, error);
}

/**
 * dm_engine_get_object:
 * @self: the engine
 * @id: the ekn id of the object to load
 * @cancellable: (allow-none): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): callback to call when the request is satisfied.
 * @user_data: (closure): the data to pass to callback function.
 *
 * Asynchronously fetches an object with ID for the default application.
 * Requires the default-app-id property to be set.
 */
void
dm_engine_get_object (DmEngine *self,
                      const char *id,
                      GCancellable *cancellable,
                      GAsyncReadyCallback callback,
                      gpointer user_data)
{
  g_return_if_fail (DM_IS_ENGINE (self));

  dm_engine_get_object_for_app (self, id, self->default_app_id, cancellable,
                                callback, user_data);
}

/**
 * dm_engine_get_object_finish:
 * @self: the engine
 * @result: the #GAsyncResult that was provided to the callback.
 * @error: #GError for error reporting.
 *
 * Finish a dm_engine_get_object() call.
 *
 * Returns: (transfer full): the content object model that was fetched
 */
DmContent *
dm_engine_get_object_finish (DmEngine *self,
                             GAsyncResult *result,
                             GError **error)
{
  g_return_val_if_fail (DM_IS_ENGINE (self), NULL);
  g_return_val_if_fail (G_IS_TASK (result), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return dm_engine_get_object_for_app_finish (self, result, error);
}

static void
on_domain_object_finished (GObject *source,
                           GAsyncResult *result,
                           gpointer user_data)
{
  DmDomain *domain = DM_DOMAIN (source);
  g_autoptr(GTask) task = user_data;
  GError *error = NULL;

  DmContent *model;
  if (!(model = dm_domain_get_object_finish (domain, result, &error)))
    {
      g_task_return_error (task, error);
      return;
    }

  g_task_return_pointer (task, model, g_object_unref);
}

/**
 * dm_engine_get_object_for_app:
 * @self: the engine
 * @id: the ekn id of the object to load
 * @app_id: the id of the application to load the object from
 * @cancellable: (allow-none): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): callback to call when the request is satisfied.
 * @user_data: (closure): the data to pass to callback function.
 *
 * Asynchronously load an object model for the given ekn_id
 */
void
dm_engine_get_object_for_app (DmEngine *self,
                              const char *id,
                              const char *app_id,
                              GCancellable *cancellable,
                              GAsyncReadyCallback callback,
                              gpointer user_data)
{
  g_return_if_fail (DM_IS_ENGINE (self));
  g_return_if_fail (id != NULL);
  g_return_if_fail (G_IS_CANCELLABLE (cancellable) || cancellable == NULL);
  g_return_if_fail (app_id && *app_id);

  g_autoptr(GTask) task = g_task_new (self, cancellable, callback, user_data);
  GError *error = NULL;
  DmDomain *domain;
  if (!(domain = dm_engine_get_domain_for_app (self, app_id, &error)))
    {
      g_task_return_error (task, error);
      return;
    }

  dm_domain_get_object (domain, id, cancellable, on_domain_object_finished,
                        g_steal_pointer (&task));
}

/**
 * dm_engine_get_object_for_app_finish:
 * @self: the engine
 * @result: the #GAsyncResult that was provided to the callback.
 * @error: #GError for error reporting.
 *
 * Finish a dm_engine_get_object_for_app() call.
 *
 * Returns: (transfer full): the content object model that was fetched
 */
DmContent *
dm_engine_get_object_for_app_finish (DmEngine *self,
                                     GAsyncResult *result,
                                     GError **error)
{
  g_return_val_if_fail (DM_IS_ENGINE (self), NULL);
  g_return_val_if_fail (G_IS_TASK (result), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return g_task_propagate_pointer (G_TASK (result), error);
}

static void
on_domain_query_finished (GObject *source,
                          GAsyncResult *result,
                          gpointer user_data)
{
  DmDomain *domain = DM_DOMAIN (source);
  g_autoptr(GTask) task = user_data;
  GError *error = NULL;

  DmQueryResults *results;
  if (!(results = dm_domain_query_finish (domain, result, &error)))
    {
      g_task_return_error (task, error);
      return;
    }

  g_task_return_pointer (task, results, g_object_unref);
}

static void
on_query_fixed_finished (GObject *source,
                         GAsyncResult *result,
                         gpointer user_data)
{
  DmDomain *domain = DM_DOMAIN (source);
  g_autoptr(GTask) task = user_data;
  GCancellable *cancellable = g_task_get_cancellable (task);
  GError *error = NULL;

  g_autoptr(DmQuery) query = NULL;
  if (!(query = dm_domain_get_fixed_query_finish (domain, result, &error)))
    {
      g_task_return_error (task, error);
      return;
    }

  dm_domain_query (domain, query, cancellable, on_domain_query_finished,
                   g_steal_pointer (&task));
}

/**
 * dm_engine_query:
 * @self: the engine
 * @query: the query object to fix
 * @cancellable: (allow-none): optional #GCancellable object, %NULL to ignore.
 * @callback: (scope async): callback to call when the request is satisfied.
 * @user_data: (closure): the data to pass to callback function.
 *
 * Asynchronously queries the Xapian database for a given #DmQuery,
 * and return a list of matching #DmContent objects.
 */
void
dm_engine_query (DmEngine *self,
                 DmQuery *query,
                 GCancellable *cancellable,
                 GAsyncReadyCallback callback,
                 gpointer user_data)
{
  g_return_if_fail (DM_IS_ENGINE (self));
  g_return_if_fail (DM_IS_QUERY (query));
  g_return_if_fail (G_IS_CANCELLABLE (cancellable) || cancellable == NULL);

  g_autofree gchar *query_app_id = NULL;
  g_object_get (G_OBJECT (query),
                "app_id", &query_app_id,
                NULL);
  gchar *app_id = self->default_app_id;
  if (query_app_id && *query_app_id)
    app_id = query_app_id;

  g_return_if_fail (app_id && *app_id);

  g_autoptr(GTask) task = g_task_new (self, cancellable, callback, user_data);
  GError *error = NULL;
  DmDomain *domain;
  if (!(domain = dm_engine_get_domain_for_app (self, app_id, &error)))
    {
      g_task_return_error (task, error);
      return;
    }

  const char *search_terms = dm_query_get_search_terms (query);
  if (search_terms  != NULL && *search_terms != '\0')
    {
      dm_domain_get_fixed_query (domain, query, cancellable,
                                 on_query_fixed_finished, g_steal_pointer (&task));
      return;
    }

  /* We're searching for tags without a query string. */
  dm_domain_query (domain, query, cancellable, on_domain_query_finished,
                   g_steal_pointer (&task));
}

/**
 * dm_engine_query_finish:
 * @self: the engine
 * @result: the #GAsyncResult that was provided to the callback.
 * @error: #GError for error reporting.
 *
 * Finishes a call to dm_engine_query(). Returns a #DmQueryResults object.
 * Throws an error if one occurred.
 *
 * Returns: (transfer full): the results object
 */
DmQueryResults *
dm_engine_query_finish (DmEngine *self,
                        GAsyncResult *result,
                        GError **error)
{
  g_return_val_if_fail (DM_IS_ENGINE (self), NULL);
  g_return_val_if_fail (G_IS_TASK (result), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  return g_task_propagate_pointer (G_TASK (result), error);
}

/**
 * dm_engine_get_domain:
 * @self: the engine
 * @error: #GError for error reporting.
 *
 * Get a #DmDomain object for the default-app-id. Requires the default-app-id
 * property to be set.
 *
 * Returns the domain if it has already been created, and synchronously creates
 * one if none exists. Returns an error if one occurred while initializing the
 * domain.
 *
 * Returns: (transfer none): the default domain
 */
DmDomain *
dm_engine_get_domain (DmEngine *self,
                      GError **error)
{
  g_return_val_if_fail (DM_IS_ENGINE (self), NULL);

  return dm_engine_get_domain_for_app (self, self->default_app_id, error);
}

/**
 * dm_engine_get_domain_for_app:
 * @self: the engine
 * @app_id: the id of the application to load the object from
 * @error: #GError for error reporting.
 *
 * Get a #DmDomain object for a given app id.
 *
 * Returns the domain if it has already been created, and synchronously creates
 * one if none exists. Returns an error if one occurred while initializing the
 * domain.
 *
 * Returns: (transfer none): the domain
 */
DmDomain *
dm_engine_get_domain_for_app (DmEngine *self,
                              const char *app_id,
                              GError **error)
{
  g_return_val_if_fail (DM_IS_ENGINE (self), NULL);
  g_return_val_if_fail (app_id && *app_id, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  DmDomain *domain = g_hash_table_lookup (self->domains, app_id);

  if (domain != NULL)
    return domain;

  domain = dm_domain_get_for_app_id (app_id, NULL, self->language, NULL, error);
  if (domain == NULL)
    return NULL;

  // Hash table takes ownership of domain
  g_hash_table_insert (self->domains, g_strdup (app_id), domain);

  return domain;
}

/**
 * dm_engine_add_domain_for_path:
 * @self: the engine
 * @app_id: the id of the application to load the object from
 * @path: the path to the content directory
 * @error: #GError for error reporting.
 *
 * Adds a domain for an specific content path
 */
void
dm_engine_add_domain_for_path (DmEngine *self,
                               const char *app_id,
                               const char *path,
                               GError **error)
{
  g_return_if_fail (DM_IS_ENGINE (self));
  g_return_if_fail (app_id && *app_id);
  g_return_if_fail (path && *path);

  if (g_hash_table_contains (self->domains, app_id))
    return;

  DmDomain *domain = dm_domain_get_for_app_id (app_id, path, self->language, NULL, error);
  if (domain == NULL)
    return;

  // Hash table takes ownership of domain
  g_hash_table_insert (self->domains, g_strdup (app_id), domain);
}

/**
 * dm_engine_get_default:
 *
 * Get the default engine object. Generally you should use this instead of
 * creating an engine object manually.
 *
 * Returns: (transfer none): the default engine
 */
DmEngine *
dm_engine_get_default (void)
{
  static DmEngine *engine;

  if (g_once_init_enter (&engine))
    {
      const char *language = dm_get_current_language ();
      g_once_init_leave (&engine, g_object_new (DM_TYPE_ENGINE,
                                                "language", language,
                                                NULL));
    }

  return engine;
}
