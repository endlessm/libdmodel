/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define DM_TYPE_QUERY_RESULTS dm_query_results_get_type ()
G_DECLARE_FINAL_TYPE (DmQueryResults, dm_query_results, DM, QUERY_RESULTS,
                      GObject)

GSList *
dm_query_results_get_models (DmQueryResults *self);

gint
dm_query_results_get_upper_bound (DmQueryResults *self);

DmQueryResults *
dm_query_results_new_for_testing (GSList *models);

G_END_DECLS
