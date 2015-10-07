/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Ludovic Fauvet <etix@videolan.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/
/*
 * By committing to this project, you allow VideoLAN and VideoLabs to relicense
 * the code to a different OSI approved license, in case it is required for
 * compatibility with the Store
 *****************************************************************************/

#ifndef PREFERENCES_H_
#define PREFERENCES_H_

#include "common.h"
#include "ui/settings/menu_id.h"

typedef struct preferences preferences;

typedef enum pref_enum {
    PREF_HWACCELERATION = 1000,
    PREF_ORIENTATION,
    PREF_DEBLOCKING,
} pref_enum;

typedef enum pref_index {
    PREF_SUBSENC = 2000,
} pref_index;

typedef enum pref_bool {
    PREF_FRAME_SKIP = 3000,
    PREF_AUDIO_STRETCH,
    PREF_DIRECTORIES_INTERNAL,
} pref_bool;

void
preferences_set_enum(pref_enum key, menu_id value);

void
preferences_set_index(pref_enum key, int value);

void
preferences_set_bool(pref_bool key, bool value);

menu_id
preferences_get_enum(pref_enum key, menu_id default_value);

int
preferences_get_index(pref_index key, int default_value);

bool
preferences_get_bool(pref_bool key, bool default_value);

char *
preferences_get_libvlc_options();

#endif
