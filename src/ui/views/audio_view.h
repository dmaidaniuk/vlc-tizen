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

#ifndef AUDIO_VIEW_H_
#define AUDIO_VIEW_H_

#include "ui/audio_player.h"

#include <Elementary.h>

typedef struct audio_list_data {

    char *file_path;
    const char *str;
    Evas_Object *parent;
    Elm_Object_Item *item;
    interface_sys *gd;

} audio_list_data_s;

Evas_Object *
create_audio_view(interface_sys *gd, Evas_Object *parent);

Evas_Object*
create_audio_list(char* path, interface_sys *gd);

#endif /* AUDIO_VIEW_H_ */