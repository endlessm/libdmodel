/* Copyright 2018  Endless Mobile, Inc.
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

#pragma once

#include <glib.h>
#include "dm-version.h"

#ifndef _DM_EXTERN
#define _DM_EXTERN extern
#endif

#ifdef DM_DISABLE_DEPRECATION_WARNINGS
#define DM_DEPRECATED _DM_EXTERN
#define DM_DEPRECATED_FOR(f) _DM_EXTERN
#define DM_UNAVAILABLE(maj,min) _DM_EXTERN
#else
#define DM_DEPRECATED G_DEPRECATED _DM_EXTERN
#define DM_DEPRECATED_FOR(f) G_DEPRECATED_FOR(f) _DM_EXTERN
#define DM_UNAVAILABLE(maj,min) G_UNAVAILABLE(maj,min) _DM_EXTERN
#endif

/* Each new cycle should add a new version symbol here */

/**
 * DM_VERSION_0_0:
 *
 * A pre-processor macro that evaluates to the 0.0 version of DModel, in a
 * format that can be used by %DM_VERSION_MIN_REQUIRED and
 * %DM_VERSION_MAX_ALLOWED.
 */
#define DM_VERSION_0_0 (G_ENCODE_VERSION (0, 0))

/**
 * DM_VERSION_CUR_STABLE:
 *
 * Evaluates to the current stable version.
 *
 * For development cycles, this means the next stable cycle.
 */
#if (DM_MINOR_VERSION == 99)
#define DM_VERSION_CUR_STABLE (G_ENCODE_VERSION (DM_MAJOR_VERSION + 1, 0))
#elif (DM_MINOR_VERSION % 2)
#define DM_VERSION_CUR_STABLE (G_ENCODE_VERSION (DM_MAJOR_VERSION, DM_MINOR_VERSION + 1))
#else
#define DM_VERSION_CUR_STABLE (G_ENCODE_VERSION (DM_MAJOR_VERSION, DM_MINOR_VERSION))
#endif

/**
 * DM_VERSION_PREV_STABLE:
 *
 * Evaluates to the previous stable version.
 */
#if (DM_MINOR_VERSION == 99)
#define DM_VERSION_PREV_STABLE (G_ENCODE_VERSION (DM_MAJOR_VERSION + 1, 0))
#elif (DM_MINOR_VERSION % 2)
#define DM_VERSION_PREV_STABLE (G_ENCODE_VERSION (DM_MAJOR_VERSION, DM_MINOR_VERSION - 1))
#else
#define DM_VERSION_PREV_STABLE (G_ENCODE_VERSION (DM_MAJOR_VERSION, DM_MINOR_VERSION - 2))
#endif

/**
 * DM_VERSION_MIN_REQUIRED:
 *
 * A pre-processor symbol that should be defined by the user prior to including
 * the `dmodel.h` header.
 *
 * The value should be one of the predefined DModel version macros,
 * for instance: %DM_VERSION_0_0, %DM_VERSION_0_2, ...
 *
 * This symbol defines the earliest version of DModel that a project
 * is required to be able to compile against.
 *
 * If the compiler is configured to warn about the use of deprecated symbols,
 * then using symbols that were deprecated in version %DM_VERSION_MIN_REQUIRED,
 * or earlier versions, will emit a deprecation warning, but using functions
 * deprecated in later versions will not.
 *
 * See also: %DM_VERSION_MAX_ALLOWED
 */
#ifndef DM_VERSION_MIN_REQUIRED
# define DM_VERSION_MIN_REQUIRED (DM_VERSION_CUR_STABLE)
#endif

/**
 * DM_VERSION_MAX_ALLOWED:
 *
 * A pre-processor symbol that should be defined by the user prior to including
 * the `dmodel.h` header.
 *
 * The value should be one of the predefined DModel version macros,
 * for instance: %DM_VERSION_0_0, %DM_VERSION_0_2, ...
 *
 * This symbol defines the earliest version of DModel that a project
 * is required to be able to compile against.
 *
 * If the compiler is configured to warn about the use of deprecated symbols,
 * then using symbols that were deprecated in version %DM_VERSION_MAX_ALLOWED,
 * or later versions, will emit a deprecation warning, but using functions
 * deprecated in later versions will not.
 *
 * See also: %DM_VERSION_MIN_REQUIRED
 */
#ifndef DM_VERSION_MAX_ALLOWED
# if DM_VERSION_MIN_REQUIRED > DM_VERSION_PREV_STABLE
#  define DM_VERSION_MAX_ALLOWED (DM_VERSION_MIN_REQUIRED)
# else
#  define DM_VERSION_MAX_ALLOWED (DM_VERSION_CUR_STABLE)
# endif
#endif

/* sanity checks */
#if DM_VERSION_MAX_ALLOWED < DM_VERSION_MIN_REQUIRED
#error "DM_VERSION_MAX_ALLOWED must be >= DM_VERSION_MIN_REQUIRED"
#endif
#if DM_VERSION_MIN_REQUIRED < DM_VERSION_0_0
#error "DM_VERSION_MIN_REQUIRED must be >= DM_VERSION_0_0"
#endif

/* unconditional: we can't have an earlier API version */
#define DM_AVAILABLE_IN_ALL _DM_EXTERN
