/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include <glib-object.h>
#include <xapian-glib.h>

G_BEGIN_DECLS

#define DM_TYPE_QUERY dm_query_get_type ()
G_DECLARE_FINAL_TYPE (DmQuery, dm_query, DM, QUERY, GObject)

/**
 * DmQueryMode:
 * @DM_QUERY_MODE_INCREMENTAL: Queries will match partially typed words.
 *   For example a query for 'dragonba' will match the Dragonball article.
 * @DM_QUERY_MODE_DELIMITED: Queries will be assumed to be entire words.
 *
 * Enumeration for different query modes.
 */
typedef enum {
  DM_QUERY_MODE_INCREMENTAL,
  DM_QUERY_MODE_DELIMITED,
} DmQueryMode;

/**
 * DmQueryMatch:
 * @DM_QUERY_MATCH_ONLY_TITLE: Only article titles will match against
 *   the query string.
 * @DM_QUERY_MATCH_TITLE_SYNOPSIS: Article titles and synopsis will
 *   match the query string.
 *
 * Enumeration of what to match in the source document.
 */
typedef enum {
  DM_QUERY_MATCH_ONLY_TITLE,
  DM_QUERY_MATCH_TITLE_SYNOPSIS,
} DmQueryMatch;

/**
 * DmQuerySort:
 * @DM_QUERY_SORT_RELEVANCE: Use xapian relevance ranking to sort
 *   articles. Exact title matches will be weighted most heavily.
 * @DM_QUERY_SORT_SEQUENCE_NUMBER: Uses the article page rank to sort
 *   results.
 * @DM_QUERY_SORT_DATE: Uses the article date to sort the results.
 * @DM_QUERY_SORT_ALPHABETICAL: Uses the article title to sort the results.
 *
 * Enumeration for different ways to sort query results.
 */
typedef enum {
  DM_QUERY_SORT_RELEVANCE,
  DM_QUERY_SORT_SEQUENCE_NUMBER,
  DM_QUERY_SORT_DATE,
  DM_QUERY_SORT_ALPHABETICAL,
} DmQuerySort;

/**
 * DmQueryOrder:
 * @DM_QUERY_ORDER_ASCENDING: Return articles in ascending order.
 * @DM_QUERY_ORDER_DESCENDING: Return articles in descending order.
 *
 * Enumeration for different ways to order sorted query (e.g. ascending).
 */
typedef enum {
  DM_QUERY_ORDER_ASCENDING,
  DM_QUERY_ORDER_DESCENDING,
} DmQueryOrder;

char * const *
dm_query_get_tags_match_all (DmQuery *self);

char * const *
dm_query_get_tags_match_any (DmQuery *self);

char * const *
dm_query_get_ids (DmQuery *self);

char * const *
dm_query_get_excluded_ids (DmQuery *self);

char * const *
dm_query_get_excluded_tags (DmQuery *self);

const char *
dm_query_get_search_terms (DmQuery *self);

guint
dm_query_get_cutoff (DmQuery *self);

gint
dm_query_get_sort_value (DmQuery *self);

gboolean
dm_query_is_match_all (DmQuery *self);

guint
dm_query_get_offset (DmQuery *self);

guint
dm_query_get_limit (DmQuery *self);

const char *
dm_query_get_content_type (DmQuery *self);

const char *
dm_query_get_excluded_content_type (DmQuery *self);

XapianQuery *
dm_query_get_query (DmQuery *self,
                    XapianQueryParser *qp,
                    GError **error_out);

char *
dm_query_to_string (DmQuery *self);

DmQuery *
dm_query_new_from_object (DmQuery *source,
                          const char *first_property_name,
                          ...);

G_END_DECLS
