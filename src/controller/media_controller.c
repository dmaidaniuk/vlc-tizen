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

#include "media_controller.h"
#include "media_library_controller.h"
#include "media_library_controller_private.h"
#include "media/artist_item.h"
#include "media/album_item.h"
#include "ui/views/video_view.h"
#include "ui/views/audio_view.h"

static bool
video_controller_accept_item( const library_item* p_item )
{
    const media_item* p_media_item = (const media_item*)p_item;
    return p_item->i_library_item_type == LIBRARY_ITEM_MEDIA &&
            p_media_item->i_type == MEDIA_ITEM_TYPE_VIDEO;
}

static bool
audio_controller_accept_item( const library_item* p_item )
{
    const media_item* p_media_item = (const media_item*)p_item;
    return p_item->i_library_item_type == LIBRARY_ITEM_MEDIA &&
            p_media_item->i_type == MEDIA_ITEM_TYPE_AUDIO;
}

static bool
artist_controller_accept_item( const library_item* p_item )
{
    return p_item->i_library_item_type == LIBRARY_ITEM_ARTIST;
}

static bool
album_controller_accept_item( const library_item* p_item )
{
    return p_item->i_library_item_type == LIBRARY_ITEM_ALBUM;
}

media_library_controller*
video_controller_create(application* p_app, list_view* p_list_view)
{
    media_library_controller* p_ctrl = media_library_controller_create( p_app, p_list_view );
    if ( p_ctrl == NULL )
        return NULL;
    p_ctrl->pf_media_library_get_content = (pf_media_library_get_content_cb)&media_library_get_video_files;
    p_ctrl->pf_item_duplicate = (pf_item_duplicate_cb)&media_item_copy;
    p_ctrl->pf_item_compare = (pf_item_compare_cb)&media_item_identical;
    p_ctrl->pf_accept_item = &video_controller_accept_item;
    return p_ctrl;
}

media_library_controller*
audio_controller_create(application* p_app, list_view* p_list_view)
{
    media_library_controller* p_ctrl = media_library_controller_create( p_app, p_list_view );
    if ( p_ctrl == NULL )
        return NULL;
    p_ctrl->pf_media_library_get_content = (pf_media_library_get_content_cb)&media_library_get_audio_files;
    p_ctrl->pf_item_duplicate = (pf_item_duplicate_cb)&media_item_copy;
    p_ctrl->pf_item_compare = (pf_item_compare_cb)&media_item_identical;
    p_ctrl->pf_accept_item = &audio_controller_accept_item;
    return p_ctrl;
}

media_library_controller*
artist_controller_create(application* p_app, list_view* p_list_view)
{
    media_library_controller* p_ctrl = media_library_controller_create( p_app, p_list_view );
    if ( p_ctrl == NULL )
        return NULL;
    p_ctrl->pf_media_library_get_content = (pf_media_library_get_content_cb)&media_library_get_artists;
    p_ctrl->pf_item_duplicate = (pf_item_duplicate_cb)&artist_item_copy;
    p_ctrl->pf_item_compare = (pf_item_compare_cb)&artist_item_identical;
    p_ctrl->pf_accept_item = &artist_controller_accept_item;
    return p_ctrl;
}

media_library_controller*
album_controller_create(application* p_app, list_view* p_list_view)
{
    media_library_controller* p_ctrl = media_library_controller_create( p_app, p_list_view );
    if ( p_ctrl == NULL )
        return NULL;
    p_ctrl->pf_media_library_get_content = (pf_media_library_get_content_cb)&media_library_get_albums;
    p_ctrl->pf_item_duplicate = (pf_item_duplicate_cb)&album_item_copy;
    p_ctrl->pf_item_compare = (pf_item_compare_cb)&album_item_identical;
    p_ctrl->pf_accept_item = &album_controller_accept_item;
    return p_ctrl;
}
