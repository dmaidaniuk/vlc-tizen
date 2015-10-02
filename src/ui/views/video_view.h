/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Nicolas Rechatin [nicolas@videolabs.io]
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

#ifndef VIDEO_VIEW_H_
#define VIDEO_VIEW_H_

#include "ui/interface.h"
#include <Elementary.h>

typedef struct video_list_item video_list_item;

interface_view*
create_video_view(interface *intf, Evas_Object *parent);

void
video_view_update(view_sys* vv, Eina_List* p_content);

video_list_item *
video_view_append_item(view_sys *videoview, media_item* p_item);

void
video_view_clear(view_sys* videoview);

const media_item*
video_list_item_get_media_item(video_list_item* p_item);

void
video_list_item_set_media_item(video_list_item* p_item, const media_item* p_media_item);

#endif /* VIDEO_VIEW_H_ */
