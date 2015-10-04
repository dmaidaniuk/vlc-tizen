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

artist_item*
artist_item_create(const char* psz_name)
{
    artist_item* p_item = calloc(1, sizeof(*p_item));
    if (p_item == NULL)
        return NULL;
    p_item->psz_name = strdup(psz_name);
    if (p_item->psz_name == NULL)
    {
        free(p_item);
        return NULL;
    }
    return p_item;
}

void
artist_item_destroy(artist_item* p_item)
{
    if (p_item == NULL)
        return;
    free(p_item->psz_name);
    free(p_item);
}
