/* Copyright 2018 Endless Mobile, Inc. */

#pragma once

#include <glib.h>
#include <xapian-glib.h>

#include "dm-query.h"

G_BEGIN_DECLS

void
dm_query_configure_enquire (DmQuery *self,
                            XapianEnquire *enquire);

G_END_DECLS
