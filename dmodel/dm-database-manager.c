/* Copyright 2014  Endless Mobile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "dm-database-manager-private.h"
#include "dm-query-private.h"
#include "dm-shard.h"

#include <endless/endless.h>

#define PREFIX_METADATA_KEY "XbPrefixes"
#define STOPWORDS_METADATA_KEY "XbStopwords"

G_DEFINE_QUARK (dm-database-manager-error-quark, dm_database_manager_error)

typedef struct {
  /* string lang_name => object XapianStem */
  GHashTable *stemmers;

  GSList *shards;

  XapianQueryParser *query_parser;
  XapianDatabase *database;
} DmDatabaseManagerPrivate;

struct _DmDatabaseManager {
  GObject parent_class;
};

enum {
  PROP_0,
  PROP_SHARDS,
  N_PROPS
};

static GParamSpec *dm_database_manager_properties[N_PROPS];

G_DEFINE_TYPE_WITH_PRIVATE (DmDatabaseManager, dm_database_manager, G_TYPE_OBJECT)

/* Registers the prefixes and booleanPrefixes contained in the JSON object
 * to the query parser.
 */
static void
dm_database_manager_add_queryparser_prefixes (DmDatabaseManager *self,
                                              JsonObject *object)
{
  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);
  JsonNode *element_node;
  JsonObject *element_object;
  JsonArray *array;
  GList *elements, *l;

  array = json_object_get_array_member (object, "prefixes");
  elements = json_array_get_elements (array);

  for (l = elements; l != NULL; l = l->next)
    {
      element_node = l->data;
      element_object = json_node_get_object (element_node);

      xapian_query_parser_add_prefix (priv->query_parser,
                                      json_object_get_string_member (element_object, "field"),
                                      json_object_get_string_member (element_object, "prefix"));
    }

  g_list_free (elements);

  array = json_object_get_array_member (object, "booleanPrefixes");
  elements = json_array_get_elements (array);

  for (l = elements; l != NULL; l = l->next)
    {
      element_node = l->data;
      element_object = json_node_get_object (element_node);

      xapian_query_parser_add_boolean_prefix (priv->query_parser,
                                              json_object_get_string_member (element_object, "field"),
                                              json_object_get_string_member (element_object, "prefix"),
                                              FALSE);
    }

  g_list_free (elements);
}

static void
dm_database_manager_add_queryparser_standard_prefixes (DmDatabaseManager *self)
{
  static const struct {
    const gchar *field;
    const gchar *prefix;
  } standard_prefixes[] = {
    { "title", "S" },
    { "exact_title", "XEXACTS" },
  }, standard_boolean_prefixes[] = {
    { "tag", "K" },
    { "id", "Q" },
  };

  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);

  guint idx;

  for (idx = 0; idx < G_N_ELEMENTS (standard_prefixes); idx++)
    xapian_query_parser_add_prefix (priv->query_parser,
                                    standard_prefixes[idx].field,
                                    standard_prefixes[idx].prefix);

  for (idx = 0; idx < G_N_ELEMENTS (standard_boolean_prefixes); idx++)
    xapian_query_parser_add_boolean_prefix (priv->query_parser,
                                            standard_boolean_prefixes[idx].field,
                                            standard_boolean_prefixes[idx].prefix,
                                            FALSE);
}

static void
dm_database_manager_set_property (GObject *gobject,
                                  guint prop_id,
                                  const GValue *value,
                                  GParamSpec *pspec)
{
  DmDatabaseManager *self = DM_DATABASE_MANAGER (gobject);
  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_SHARDS:
      priv->shards = g_value_get_pointer (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
dm_database_manager_get_property (GObject *gobject,
                                  guint prop_id,
                                  GValue *value,
                                  GParamSpec *pspec)
{
  DmDatabaseManager *self = DM_DATABASE_MANAGER (gobject);
  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);

  switch (prop_id)
    {
    case PROP_SHARDS:
      g_value_set_pointer (value, priv->shards);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (gobject, prop_id, pspec);
    }
}

static void
dm_database_manager_finalize (GObject *object)
{
  DmDatabaseManager *self = DM_DATABASE_MANAGER (object);
  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);

  g_clear_object (&priv->database);
  g_clear_object (&priv->query_parser);

  g_clear_pointer (&priv->stemmers, g_hash_table_unref);

  G_OBJECT_CLASS (dm_database_manager_parent_class)->finalize (object);
}

static void
dm_database_manager_class_init (DmDatabaseManagerClass *klass)
{
  GObjectClass *gobject_class = (GObjectClass *)klass;

  gobject_class->set_property = dm_database_manager_set_property;
  gobject_class->get_property = dm_database_manager_get_property;
  gobject_class->finalize = dm_database_manager_finalize;

  dm_database_manager_properties[PROP_SHARDS] =
    g_param_spec_pointer ("shards", "Shards", "The list of shards",
                          G_PARAM_READWRITE |
                          G_PARAM_CONSTRUCT_ONLY |
                          G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (gobject_class, N_PROPS, dm_database_manager_properties);
}

static void
dm_database_manager_init (DmDatabaseManager *self)
{
  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);

  priv->stemmers = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_object_unref);
  g_hash_table_insert (priv->stemmers, g_strdup ("none"), xapian_stem_new ());
}

static gboolean
dm_database_manager_register_prefixes (DmDatabaseManager *self,
                                       GError **error_out)
{
  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);
  GError *error = NULL;
  JsonNode *root;

  /* Attempt to read the database's custom prefix association metadata */
  g_autofree char *metadata_json =
    xapian_database_get_metadata (priv->database, PREFIX_METADATA_KEY, &error);

  if (error != NULL)
    {
      dm_database_manager_add_queryparser_standard_prefixes (self);
      g_propagate_error (error_out, error);
      return FALSE;
    }

  g_autoptr(JsonParser) parser = json_parser_new_immutable ();
  json_parser_load_from_data (parser, metadata_json, -1, &error);
  if (error != NULL)
    {
      dm_database_manager_add_queryparser_standard_prefixes (self);
      g_propagate_error (error_out, error);
      return FALSE;
    }

  root = json_parser_get_root (parser);
  if (root != NULL)
    dm_database_manager_add_queryparser_prefixes (self, json_node_get_object (root));
  else
    dm_database_manager_add_queryparser_standard_prefixes (self);

  return TRUE;
}

static gboolean
dm_database_manager_register_stopwords (DmDatabaseManager *self,
                                        GError **error_out)
{
  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);
  GError *error = NULL;

  g_autofree char *stopwords_json =
    xapian_database_get_metadata (priv->database, STOPWORDS_METADATA_KEY, &error);

  if (error != NULL)
    {
      g_propagate_error (error_out, error);
      return FALSE;
    }

  g_autoptr(JsonParser) parser = json_parser_new_immutable ();
  json_parser_load_from_data (parser, stopwords_json, -1, &error);

  if (error != NULL)
    {
      g_propagate_error (error_out, error);
      return FALSE;
    }

  /* empty JSON is not an error */
  JsonNode *node = json_parser_get_root (parser);
  if (node == NULL)
    return TRUE;

  JsonArray *array = json_node_get_array (node);
  GList *elements = json_array_get_elements (array);

  g_autoptr(XapianSimpleStopper) stopper = xapian_simple_stopper_new ();

  for (GList *l = elements; l != NULL; l = l->next)
    {
      const char *stopword = json_node_get_string (l->data);

      /* In older databases, each stopword had a newline appended.
       * This has now been fixed, but to avoid having to rebuild
       * everything, we check for and remove such newlines.
       */
      if (g_str_has_suffix (stopword, "\n"))
        {
          g_autofree char *stopword_chomped = g_strdup (stopword);

          g_strchomp (stopword_chomped);

          xapian_simple_stopper_add (stopper, stopword_chomped);
        }
      else
        {
          xapian_simple_stopper_add (stopper, stopword);
        }
    }

  g_list_free (elements);

  xapian_query_parser_set_stopper (priv->query_parser, XAPIAN_STOPPER (stopper));

  return TRUE;
}

static XapianDatabase *
create_database_from_shards (GSList *shards,
                             GError **error_out)
{
  GError *error = NULL;

  g_autoptr(EosProfileProbe) probe = EOS_PROFILE_PROBE ("/dmodel/database/create");

  XapianDatabase *db = xapian_database_new (&error);
  if (error != NULL)
    {
      g_propagate_error (error_out, error);
      return NULL;
    }

  for (GSList *l = shards; l; l = g_slist_next (l))
    {
      g_autoptr(EosProfileProbe) db_probe =
        EOS_PROFILE_PROBE ("/dmodel/database/create/internal_db");

      gint64 offset = dm_shard_get_db_offset (l->data);
      if (offset == -1)
        continue;

      g_autoptr(XapianDatabase) internal_db =
        g_initable_new (XAPIAN_TYPE_DATABASE,
                        NULL, &error,
                        "path", dm_shard_get_path (l->data),
                        "offset", offset,
                        NULL);

      if (error != NULL)
        {
          g_propagate_error (error_out, error);
          g_clear_object (&db);
          break;
        }

      xapian_database_add_database (db, internal_db);
    }

  return db;
}

static gboolean
dm_database_manager_create_db_internal (DmDatabaseManager *self,
                                        GError **error_out)
{
  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);

  GError *error = NULL;

  priv->database = create_database_from_shards (priv->shards, &error);
  if (error != NULL)
    {
      g_set_error (error_out, DM_DATABASE_MANAGER_ERROR,
                   DM_DATABASE_MANAGER_ERROR_INVALID_PATH,
                   "Cannot create XapianDatabase: %s",
                   error->message);
      g_error_free (error);
      return FALSE;
    }

  /* Create a XapianQueryParser for this particular database, stemming
   * by its registered language
   */
  priv->query_parser = xapian_query_parser_new ();
  xapian_query_parser_set_database (priv->query_parser, priv->database);

  if (!dm_database_manager_register_prefixes (self, &error))
    {
      /* Non-fatal */
      g_warning ("Could not register database prefixes: %s", error->message);
      g_clear_error (&error);
    }

  if (!dm_database_manager_register_stopwords (self, &error))
    {
      /* Non-fatal */
      g_warning ("Could not add database stop words: %s.", error->message);
      g_clear_error (&error);
    }

  return TRUE;
}

static gboolean
ensure_db (DmDatabaseManager *self,
           GError **error_out)
{
  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);

  if (priv->database != NULL)
    return TRUE;

  return dm_database_manager_create_db_internal (self, error_out);
}

static XapianMSet *
fetch_results (XapianEnquire *enquire,
               XapianQuery *query,
               guint offset,
               guint limit,
               GError **error_out)
{
  GError *error = NULL;

  xapian_enquire_set_query (enquire, query, xapian_query_get_length (query));

  XapianMSet *matches = xapian_enquire_get_mset (enquire, offset, limit, &error);
  if (error != NULL)
    {
      g_propagate_error (error_out, error);
      return NULL;
    }

  return matches;
}

/* Checks if the given database is empty (has no documents). Empty databases
 * cause problems with XapianEnquire, so we need to assert that a db isn't empty
 * before making a XapianEnquire for it.
 */
static gboolean
database_is_empty (XapianDatabase *db)
{
  return xapian_database_get_doc_count (db) == 0;
}

static gboolean
dm_database_manager_fix_query_internal (DmDatabaseManager *self,
                                        const char *search_terms,
                                        char **stop_fixed_terms_out,
                                        char **spell_fixed_terms_out,
                                        GError **error_out)
{
  g_return_val_if_fail (stop_fixed_terms_out != NULL, FALSE);
  g_return_val_if_fail (spell_fixed_terms_out != NULL, FALSE);

  g_autoptr(EosProfileProbe) probe = EOS_PROFILE_PROBE ("/dmodel/fix_query");

  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);
  GError *error = NULL;
  XapianStopper *stopper = xapian_query_parser_get_stopper (priv->query_parser);

  if (stopper != NULL && stop_fixed_terms_out != NULL)
    {
      g_auto(GStrv) words = g_strsplit (search_terms, " ", -1);

      /* This is not a GStrv, as we don't copy the words, just their pointer */
      g_autofree char **filtered_words = g_new0 (char *, g_strv_length (words) + 1);

      char **filtered_iter = filtered_words;
      char **words_iter;

      for (words_iter = words; *words_iter != NULL; words_iter++)
        {
          if (!xapian_stopper_is_stop_term (stopper, *words_iter))
            *filtered_iter++ = *words_iter;
        }

      *stop_fixed_terms_out = g_strjoinv (" ", filtered_words);
    }

  xapian_query_parser_set_default_op (priv->query_parser, XAPIAN_QUERY_OP_AND);

  /* Parse the user's query so we can request a spelling correction. */
  xapian_query_parser_parse_query_full (priv->query_parser,
                                        search_terms != NULL ? search_terms : "",
                                        XAPIAN_QUERY_PARSER_FEATURE_DEFAULT |
                                        XAPIAN_QUERY_PARSER_FEATURE_WILDCARD |
                                        XAPIAN_QUERY_PARSER_FEATURE_PURE_NOT |
                                        XAPIAN_QUERY_PARSER_FEATURE_SPELLING_CORRECTION,
                                        "",
                                        &error);
  if (error != NULL)
    {
      g_propagate_error (error_out, error);
      return FALSE;
    }

  if (spell_fixed_terms_out != NULL)
    {
      /* In older databases, each entry in the spelling dictionary had a
       * newline appended. This has now been fixed, but in order to avoid
       * having to rebuild everything, we remove all excess whitespace from
       * each corrected term. */
      g_autofree char *corrected = xapian_query_parser_get_corrected_query_string (priv->query_parser);
      g_auto(GStrv) temp_array = g_strsplit (corrected, " ", -1);

      for (char **iter = temp_array; *iter != NULL; iter++)
        *iter = g_strstrip (*iter);

      *spell_fixed_terms_out = g_strjoinv (" ", temp_array);
    }

  g_debug (G_STRLOC ":\n"
           " - search terms: '%s'\n"
           " - stop_fixed_terms: '%s'\n"
           " - spell_fixed_terms: '%s'",
           search_terms ? search_terms : "<none>",
           *stop_fixed_terms_out != NULL ? *stop_fixed_terms_out : "<ignored>",
           *spell_fixed_terms_out != NULL ? *spell_fixed_terms_out : "<ignored>");

  return TRUE;
}

gboolean
dm_database_manager_fix_query (DmDatabaseManager *self,
                               const char *search_terms,
                               char **stop_fixed_terms,
                               char **spell_fixed_terms,
                               GError **error_out)
{
  g_return_val_if_fail (DM_IS_DATABASE_MANAGER (self), FALSE);

  if (!ensure_db (self, error_out))
    return FALSE;

  return dm_database_manager_fix_query_internal (self, search_terms,
                                                 stop_fixed_terms,
                                                 spell_fixed_terms, error_out);
}

/* If a database exists, queries it with the given #DmQuery. */
static XapianMSet *
dm_database_manager_query_internal (DmDatabaseManager *self,
                                    DmQuery *query,
                                    const char *lang,
                                    GError **error_out)
{
  g_autoptr(EosProfileProbe) probe = EOS_PROFILE_PROBE ("/dmodel/query");

  DmDatabaseManagerPrivate *priv = dm_database_manager_get_instance_private (self);
  GError *error = NULL;

  if (database_is_empty (priv->database))
    {
      g_set_error (error_out, DM_DATABASE_MANAGER_ERROR,
                   DM_DATABASE_MANAGER_ERROR_NOT_FOUND,
                   "Empty database found");
      return NULL;
    }

  XapianStem *stem = g_hash_table_lookup (priv->stemmers, lang);
  if (stem == NULL)
    {
      stem = xapian_stem_new_for_language (lang, &error);
      if (error != NULL)
        {
          g_warning ("Cannot create XapianStem for language %s: %s",
                     lang, error->message);
          g_clear_error (&error);

          stem = g_hash_table_lookup (priv->stemmers, "none");
        }
      else
        {
          g_hash_table_insert (priv->stemmers, g_strdup (lang), stem);
        }
    }

  g_assert (stem != NULL);

  xapian_query_parser_set_stemmer (priv->query_parser, stem);
  xapian_query_parser_set_stemming_strategy (priv->query_parser, XAPIAN_STEM_STRATEGY_STEM_SOME);

  g_autoptr(XapianEnquire) enquire = xapian_enquire_new (priv->database, &error);
  if (error != NULL)
    {
      g_propagate_error (error_out, error);
      return NULL;
    }

  dm_query_configure_enquire (query, enquire);

  g_autofree char *dump = dm_query_to_string (query);
  g_debug (G_STRLOC " %s", dump);

  g_autoptr(XapianQuery) parsed_query = dm_query_get_query (query,
                                                            priv->query_parser,
                                                            &error);
  if (error != NULL)
    {
      g_propagate_error (error_out, error);
      return NULL;
    }

  g_autofree char *query_dump = xapian_query_get_description (parsed_query);
  g_debug (G_STRLOC " %s", query_dump);

  guint offset = dm_query_get_offset (query);
  guint limit = dm_query_get_limit (query);

  return fetch_results (enquire, parsed_query, offset, limit, error_out);
}

XapianMSet *
dm_database_manager_query (DmDatabaseManager *self,
                           DmQuery *query,
                           const char *lang,
                           GError **error_out)
{
  g_return_val_if_fail (DM_IS_DATABASE_MANAGER (self), NULL);

  if (!ensure_db (self, error_out))
    return NULL;

  return dm_database_manager_query_internal (self, query, lang, error_out);
}

DmDatabaseManager *
dm_database_manager_new (GSList *shards)
{
  return g_object_new (DM_TYPE_DATABASE_MANAGER,
                       "shards", shards,
                       NULL);
}
