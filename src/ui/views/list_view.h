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

#ifndef LIST_VIEW_H_
# define LIST_VIEW_H_

#include "ui/interface.h"

typedef enum list_view_create_option
{
    LIST_CREATE_MEDIA_CONTROLLER = 1,
    LIST_CREATE_LIST             = 1 << 1,
    LIST_CREATE_ALL              = ~0
} list_view_create_option;

list_view*
audio_list_artist_view_create(interface* p_intf, Evas_Object* p_parent, list_view_create_option opts );

list_view*
audio_list_song_view_create(interface* p_intf, Evas_Object* p_parent, list_view_create_option opts );

list_view*
video_view_list_create(interface *intf, Evas_Object *p_parent, list_view_create_option opts );

list_view*
audio_list_album_view_create(interface* p_intf, Evas_Object* p_parent, list_view_create_option opts);

#endif // LIST_VIEW_H_
