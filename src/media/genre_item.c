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

#include "common.h"
#include "genre_item.h"

genre_item*
genre_item_create(const char* psz_name)
{
    genre_item* p_item = calloc(1, sizeof(*p_item));
    if (p_item == NULL)
        return NULL;
    p_item->i_library_item_type = LIBRARY_ITEM_GENRE;
    p_item->psz_name = strdup(psz_name);
    if (p_item->psz_name == NULL)
    {
        free(p_item);
        return NULL;
    }
    return p_item;
}

genre_item*
genre_item_copy(const genre_item* p_item)
{
    genre_item* p_new_item = genre_item_create(p_item->psz_name);
    if (p_new_item == NULL)
        return NULL;
    p_new_item->i_id = p_item->i_id;
    return p_new_item;
}

bool
genre_item_identical(const genre_item* p_left, const genre_item* p_right)
{
    return p_left->i_id == p_right->i_id;
}


void
genre_item_destroy(genre_item* p_item)
{
    if (p_item == NULL)
        return;
    free(p_item->psz_name);
    free(p_item);
}
