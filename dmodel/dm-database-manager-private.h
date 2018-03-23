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

#ifndef __EKNC_DATABASE_MANAGER_H__
#define __EKNC_DATABASE_MANAGER_H__

#include <glib-object.h>
#include <json-glib/json-glib.h>
#include <xapian-glib.h>

#include "dm-query.h"

G_BEGIN_DECLS

#define DM_TYPE_DATABASE_MANAGER dm_database_manager_get_type()
G_DECLARE_FINAL_TYPE (DmDatabaseManager, dm_database_manager, DM,
                      DATABASE_MANAGER, GObject)

typedef enum {
  DM_DATABASE_MANAGER_ERROR_NOT_FOUND,
  DM_DATABASE_MANAGER_ERROR_INVALID_PATH,
  DM_DATABASE_MANAGER_ERROR_INVALID_PARAMS
} DmDatabaseManagerError;

#define DM_DATABASE_MANAGER_ERROR dm_database_manager_error_quark()
GQuark dm_database_manager_error_quark (void);

GType
dm_database_manager_get_type (void) G_GNUC_CONST;

DmDatabaseManager *
dm_database_manager_new (const char *path);

XapianMSet *
dm_database_manager_query (DmDatabaseManager *self,
                           DmQuery *query,
                           const char *lang,
                           GError **error_out);

gboolean
dm_database_manager_fix_query (DmDatabaseManager *self,
                               const char *search_terms,
                               char **stop_fixed_terms,
                               char **spell_fixed_terms,
                               GError **error_out);

G_END_DECLS

#endif /* __EKNC_DATABASE_MANAGER_H__ */
