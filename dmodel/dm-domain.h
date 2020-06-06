/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-content.h"
#include "dm-macros.h"
#include "dm-query.h"
#include "dm-query-results.h"

#include <gio/gio.h>

G_BEGIN_DECLS

#define DM_TYPE_DOMAIN dm_domain_get_type ()

DM_AVAILABLE_IN_ALL
G_DECLARE_FINAL_TYPE (DmDomain, dm_domain, DM, DOMAIN, GObject)

/**
 * DmDomainError:
 * @DM_DOMAIN_ERROR_APP_ID_NOT_SET: App id property not set on object
 * @DM_DOMAIN_ERROR_ID_NOT_FOUND: Requested ID not found
 * @DM_DOMAIN_ERROR_BAD_MANIFEST: Error found while parsing the manifest.json
 * @DM_DOMAIN_ERROR_BAD_RESULTS: Error found while parsing the results json from xapian
 * @DM_DOMAIN_ERROR_UNSUPPORTED_VERSION: Unsupported version of content found
 * @DM_DOMAIN_ERROR_PATH_NOT_FOUND: Provided path was not found
 * @DM_DOMAIN_ERROR_ID_NOT_VALID: Requested ID is not valid
 * @DM_DOMAIN_ERROR_EMPTY: Content is empty
 *
 * Error enumeration for domain related errors.
 */
typedef enum {
  DM_DOMAIN_ERROR_APP_ID_NOT_SET,
  DM_DOMAIN_ERROR_ID_NOT_FOUND,
  DM_DOMAIN_ERROR_BAD_MANIFEST,
  DM_DOMAIN_ERROR_BAD_RESULTS,
  DM_DOMAIN_ERROR_UNSUPPORTED_VERSION,
  DM_DOMAIN_ERROR_PATH_NOT_FOUND,
  DM_DOMAIN_ERROR_ID_NOT_VALID,
  DM_DOMAIN_ERROR_EMPTY,
} DmDomainError;

#define DM_DOMAIN_ERROR dm_domain_error_quark ()

DM_AVAILABLE_IN_ALL
GQuark dm_domain_error_quark (void);

DM_AVAILABLE_IN_ALL
const gchar *
dm_domain_get_subscription_id (DmDomain *self);

DM_AVAILABLE_IN_ALL
gchar * const *
dm_domain_get_subscription_ids (DmDomain *self);

DM_AVAILABLE_IN_ALL
GSList *
dm_domain_get_shards (DmDomain *self);

DM_AVAILABLE_IN_ALL
gchar *
dm_domain_test_link (DmDomain *self,
                     const char *link,
                     GError **error);

DM_AVAILABLE_IN_ALL
void
dm_domain_get_object (DmDomain *self,
                      const char *uri,
                      GCancellable *cancellable,
                      GAsyncReadyCallback callback,
                      gpointer user_data);

DM_AVAILABLE_IN_ALL
DmContent *
dm_domain_get_object_finish (DmDomain *self,
                             GAsyncResult *result,
                             GError **error);

DM_AVAILABLE_IN_ALL
void
dm_domain_get_fixed_query (DmDomain *self,
                           DmQuery *query,
                           GCancellable *cancellable,
                           GAsyncReadyCallback callback,
                           gpointer user_data);

DM_AVAILABLE_IN_ALL
DmQuery *
dm_domain_get_fixed_query_finish (DmDomain *self,
                                  GAsyncResult *result,
                                  GError **error);

DM_AVAILABLE_IN_ALL
void
dm_domain_query (DmDomain *self,
                 DmQuery *query,
                 GCancellable *cancellable,
                 GAsyncReadyCallback callback,
                 gpointer user_data);

DM_AVAILABLE_IN_ALL
gboolean
dm_domain_read_uri (DmDomain *self,
                    const char *uri,
                    GBytes **bytes,
                    const char **mime_type,
                    GError **error);

DM_AVAILABLE_IN_ALL
DmQueryResults *
dm_domain_query_finish (DmDomain *self,
                        GAsyncResult *result,
                        GError **error);

G_END_DECLS
