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

#ifndef AUDIO_LIST_SONG_ITEM_H_
# define AUDIO_LIST_SONG_ITEM_H_

#include "media/media_item.h"
#include "ui/interface.h"

typedef struct audio_list_item audio_list_item;

const media_item*
audio_list_song_item_get_media_item(audio_list_item* p_item);

void
audio_list_song_item_set_media_item(audio_list_item* p_item, media_item* p_media_item);

audio_list_item *
audio_list_song_view_append_item(list_sys *p_sys, media_item* p_item);

void
audio_list_song_view_clear(list_sys* p_list);

list_sys*
audio_list_song_view_create(application* p_app, interface* p_intf, Evas_Object* p_genlist);

void
audio_list_song_view_show(list_sys* p_sys, Evas_Object* p_parent);

void
audio_list_song_view_destroy(list_sys* p_list);

#endif // AUDIO_LIST_SONG_ITEM_H_
