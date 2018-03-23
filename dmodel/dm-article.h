/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"

#include <gio/gio.h>
#include <json-glib/json-glib.h>

G_BEGIN_DECLS

#define DM_TYPE_ARTICLE dm_article_get_type ()
G_DECLARE_DERIVABLE_TYPE (DmArticle, dm_article, DM, ARTICLE, DmContent)

struct _DmArticleClass
{
  DmContentClass parent_class;

  gpointer padding[8];
};

char * const *
dm_article_get_authors (DmArticle *self);

char * const *
dm_article_get_temporal_coverage (DmArticle *self);

char * const *
dm_article_get_outgoing_links (DmArticle *self);

GVariant *
dm_article_get_table_of_contents (DmArticle *self);

DmContent *
dm_article_new_from_json_node (JsonNode *node);

G_END_DECLS
