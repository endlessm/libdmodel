/* Copyright 2016 Endless Mobile, Inc. */

#include "dm-query-results.h"
#include "dm-content.h"

/**
 * SECTION:query-results
 * @title: Query Results
 * @short_description: Results and metadata from Xapian
 *
 * The #QueryResults class is returned from search operations.
 * It provides a list of search results, as well as metadata about the search,
 * such as the number of total search results.
 *
 * This class has no functionality, but is just a bag of properties.
 * Instances are immutable after creation and all properties must be passed in
 * on construction.
 *
 * You are not intended to create instances of this class; it is returned as
 * a result of search operations.
 */
struct _DmQueryResults
{
  GObject parent_instance;

  GSList *models;
  gint upper_bound;  /* One would think guint, but Xapian::doccount == int */
};

G_DEFINE_TYPE (DmQueryResults, dm_query_results, G_TYPE_OBJECT)

enum {
  PROP_0,
  PROP_MODELS,
  PROP_UPPER_BOUND,
  NPROPS
};

static GParamSpec *dm_query_results_props[NPROPS] = { NULL, };


static void
dm_query_results_get_property (GObject *object,
                               guint prop_id,
                               GValue *value,
                               GParamSpec *pspec)
{
  DmQueryResults *self = DM_QUERY_RESULTS (object);

  switch (prop_id)
    {
    case PROP_MODELS:
      g_value_set_pointer (value, self->models);
      break;

    case PROP_UPPER_BOUND:
      g_value_set_int (value, self->upper_bound);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}


static void
dm_query_results_set_property (GObject *object,
                               guint prop_id,
                               const GValue *value,
                               GParamSpec *pspec)
{
  DmQueryResults *self = DM_QUERY_RESULTS (object);

  switch (prop_id)
    {
    case PROP_MODELS:
      g_assert(self->models == NULL);
      self->models = g_slist_copy_deep (g_value_get_pointer (value),
                                        (GCopyFunc) g_object_ref, NULL);
      break;

    case PROP_UPPER_BOUND:
      self->upper_bound = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    }
}

static void
dm_query_results_finalize (GObject *object)
{
  DmQueryResults *self = DM_QUERY_RESULTS (object);

  g_slist_free_full (self->models, g_object_unref);

  G_OBJECT_CLASS (dm_query_results_parent_class)->finalize (object);
}


static void
dm_query_results_class_init (DmQueryResultsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->get_property = dm_query_results_get_property;
  object_class->set_property = dm_query_results_set_property;
  object_class->finalize = dm_query_results_finalize;

  /**
   * DmQueryResults:models: (type GSList(DmContent))
   *
   * A list of the content object models making up the batch of search results
   * returned from a search operation.
   */
  dm_query_results_props[PROP_MODELS] =
    g_param_spec_pointer ("models", "Models",
      "Batch of content object models in search results",
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  /**
   * DmQueryResults:upper-bound:
   *
   * An upper bound on the total number of results for the query.
   * (This is often different from the number of results in
   * #DmQueryResults:models, since the results can be paginated.)
   */
  dm_query_results_props[PROP_UPPER_BOUND] =
    g_param_spec_int ("upper-bound", "Upper bound",
      "Upper bound on total number of results",
      G_MININT, G_MAXINT, 0,
      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

  g_object_class_install_properties (object_class, NPROPS,
                                     dm_query_results_props);
}

static void
dm_query_results_init (G_GNUC_UNUSED DmQueryResults *self)
{
}

/**
 * dm_query_results_get_models:
 * @self: the #DmQueryResults
 *
 * See #DmQueryResults:models.
 *
 * Returns: (element-type DmContent) (transfer none): list of models
 */
GSList *
dm_query_results_get_models (DmQueryResults *self)
{
  g_return_val_if_fail (DM_IS_QUERY_RESULTS (self), NULL);
  return self->models;
}

/**
 * dm_query_results_get_upper_bound:
 * @self: the #DmQueryResults
 *
 * See #DmQueryResults:upper-bound.
 *
 * Returns: the upper bound on the number of results
 */
gint
dm_query_results_get_upper_bound (DmQueryResults *self)
{
  g_return_val_if_fail (DM_IS_QUERY_RESULTS (self), 0);
  return self->upper_bound;
}

/**
 * dm_query_results_new_for_testing:
 * @models: (element-type DmContent) (transfer none):
 *
 * For testing within DModel only.
 *
 * Returns: (transfer full):
 */
DmQueryResults *
dm_query_results_new_for_testing (GSList *models)
{
  return DM_QUERY_RESULTS (g_object_new (DM_TYPE_QUERY_RESULTS,
                                         "upper-bound", 42,
                                         "models", models,
                                         NULL));

}
