/* Copyright 2016 Endless Mobile, Inc. */

#pragma once

#include "dm-domain.h"

G_BEGIN_DECLS

DmDomain *
dm_domain_get_for_app_id (const char *app_id,
                          const char *path,
                          const char *language,
                          GCancellable *cancellable,
                          GError **error);

G_END_DECLS
