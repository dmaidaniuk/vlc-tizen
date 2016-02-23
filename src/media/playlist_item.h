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

#ifndef PLAYLIST_ITEM_H_
# define PLAYLIST_ITEM_H_

#include "media/library/library_item.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct playlist_item {
    LIBRARY_ITEM_COMMON

    unsigned int i_id;
    char* psz_name;
} playlist_item;

playlist_item*
playlist_item_create( const char* psz_name );

playlist_item*
playlist_item_copy( const playlist_item* p_item );

bool
playlist_item_identical( const playlist_item* p_left, const playlist_item* p_right);

void
playlist_item_destroy( playlist_item* p_item );

#ifdef __cplusplus
} // extern "C"
#endif

#endif //PLAYLIST_ITEM_H_
