/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define DM_TYPE_ARTICLE dm_article_get_type ()

DM_AVAILABLE_IN_ALL
G_DECLARE_DERIVABLE_TYPE (DmArticle, dm_article, DM, ARTICLE, DmContent)

struct _DmArticleClass
{
  DmContentClass parent_class;

  gpointer padding[8];
};

DM_AVAILABLE_IN_ALL
char * const *
dm_article_get_authors (DmArticle *self);

DM_AVAILABLE_IN_ALL
char * const *
dm_article_get_temporal_coverage (DmArticle *self);

DM_AVAILABLE_IN_ALL
char * const *
dm_article_get_outgoing_links (DmArticle *self);

DM_AVAILABLE_IN_ALL
GVariant *
dm_article_get_table_of_contents (DmArticle *self);

DM_AVAILABLE_IN_ALL
GInputStream *
dm_article_get_archive_member_content_stream (DmArticle *self,
                                              const char *member_name,
                                              GError **error);

DM_AVAILABLE_IN_ALL
DmContent *
dm_article_new_from_json_node (JsonNode *node);

G_END_DECLS
