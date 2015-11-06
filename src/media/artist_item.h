/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
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

#ifndef ARTIST_ITEM_H_
# define ARTIST_ITEM_H_

#include "media/library/library_item.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct artist_item
{
    LIBRARY_ITEM_COMMON

    char* psz_name;
    char* psz_artwork;
    uint32_t i_nb_albums;
    uint32_t i_id;
} artist_item;

artist_item*
artist_item_create(const char* psz_name);

void
artist_item_destroy(artist_item* p_item);

artist_item*
artist_item_copy(const artist_item* p_item );

bool
artist_item_identical(const artist_item* p_left, const artist_item* p_right);

const char*
artist_item_get_name(const artist_item* p_item);

#ifdef __cplusplus
} // extern "C"
#endif

#endif //ARTIST_ITEM_H_
