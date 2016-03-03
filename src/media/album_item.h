/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#ifndef ALBUM_ITEM_H_
# define ALBUM_ITEM_H_

#include "media/library/library_item.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct album_item {
    LIBRARY_ITEM_COMMON

    int64_t i_id;
    char* psz_name;
    char* psz_summary;
    time_t i_release_date;
    char* psz_artwork;
    uint32_t i_nb_tracks;
} album_item;

album_item*
album_item_create(const char* psz_name);

album_item*
album_item_copy(const album_item* p_item);

bool
album_item_identical(const album_item* p_left, const album_item* p_right);

void
album_item_destroy(album_item* p_item);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //ALBUM_ITEM_H_
