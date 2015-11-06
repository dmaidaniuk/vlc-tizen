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

#include "common.h"
#include "artist_item.h"
#include "media/library/media_library.hpp"

artist_item*
artist_item_create(const char* psz_name)
{
    artist_item* p_item = calloc(1, sizeof(*p_item));
    if (p_item == NULL)
        return NULL;
    p_item->i_library_item_type = LIBRARY_ITEM_ARTIST;
    p_item->i_nb_albums = 0;
    if (psz_name != NULL && *psz_name != 0)
    {
        p_item->psz_name = strdup(psz_name);
        if (p_item->psz_name == NULL)
        {
            free(p_item);
            return NULL;
        }
    }
    return p_item;
}

void
artist_item_destroy(artist_item* p_item)
{
    if (p_item == NULL)
        return;
    free(p_item->psz_artwork);
    free(p_item->psz_name);
    free(p_item);
}

artist_item*
artist_item_copy(const artist_item* p_item )
{
    artist_item* p_new = artist_item_create( p_item->psz_name );
    if (p_new == NULL)
        return NULL;
    p_new->i_id = p_item->i_id;
    if (p_item->psz_artwork != NULL)
        p_new->psz_artwork = strdup(p_item->psz_artwork);
    p_new->i_nb_albums = p_item->i_nb_albums;
    return p_new;
}

bool
artist_item_identical(const artist_item* p_left, const artist_item* p_right)
{
    if (p_left->psz_name == NULL || p_right->psz_name == NULL)
        return p_left->psz_name == p_right->psz_name;
    return p_left->i_id == p_right->i_id;
}

const char*
artist_item_get_name(const artist_item* p_item)
{
    if (media_library_is_various_artist(p_item) == true)
        return "Various Artists";
    else if (*p_item->psz_name == 0)
        return "Unknown Artist";
    return p_item->psz_name;
}
