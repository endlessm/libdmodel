/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"
#include "dm-domain.h"

#include <gio/gio.h>

G_BEGIN_DECLS

#define DM_TYPE_ENGINE dm_engine_get_type ()
G_DECLARE_FINAL_TYPE (DmEngine, dm_engine, DM, ENGINE, GObject)

gchar *
dm_engine_test_link (DmEngine *self,
                     const char *link,
                     GError **error);

gchar *
dm_engine_test_link_for_app (DmEngine *self,
                             const char *link,
                             const char *app_id,
                             GError **error);
void
dm_engine_get_object (DmEngine *self,
                      const char *id,
                      GCancellable *cancellable,
                      GAsyncReadyCallback callback,
                      gpointer user_data);

DmContent *
dm_engine_get_object_finish (DmEngine *self,
                             GAsyncResult *result,
                             GError **error);

void
dm_engine_get_object_for_app (DmEngine *self,
                              const char *id,
                              const char *app_id,
                              GCancellable *cancellable,
                              GAsyncReadyCallback callback,
                              gpointer user_data);

DmContent *
dm_engine_get_object_for_app_finish (DmEngine *self,
                                     GAsyncResult *result,
                                     GError **error);

void
dm_engine_query (DmEngine *self,
                 DmQuery *query,
                 GCancellable *cancellable,
                 GAsyncReadyCallback callback,
                 gpointer user_data);

DmQueryResults *
dm_engine_query_finish (DmEngine *self,
                        GAsyncResult *result,
                        GError **error);

DmDomain *
dm_engine_get_domain (DmEngine *self,
                      GError **error);

DmDomain *
dm_engine_get_domain_for_app (DmEngine *self,
                              const char *app_id,
                              GError **error);

void
dm_engine_add_domain_for_path (DmEngine *self,
                               const char *app_id,
                               const char *path,
                               GError **error);

DmEngine *
dm_engine_get_default (void);

G_END_DECLS
