/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
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
#include "media_item.h"

media_item *
media_item_create(const char *psz_path, enum MEDIA_ITEM_TYPE i_type)
{
    media_item *p_mi = calloc(1, sizeof(media_item));
    if (!p_mi)
        return NULL;

    p_mi->psz_path = strdup(psz_path);
    if (!p_mi->psz_path)
        goto error;

    p_mi->i_type = i_type;

error:
    media_item_destroy(p_mi);
    return NULL;
}

void
media_item_destroy(media_item *p_mi)
{
    free(p_mi->psz_path);
    free(p_mi);
}
