/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
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

#ifndef SYSTEM_STORAGE_H_
#define SYSTEM_STORAGE_H_

typedef enum
{
    MEDIA_DIRECTORY,
    MEDIA_DIRECTORY_VIDEOS,           /* Videos directory */
    MEDIA_DIRECTORY_CAMERA,           /* Camera directory */
    MEDIA_DIRECTORY_MUSIC,            /* Music directory */
    MEDIA_DIRECTORY_MAX
} media_directory_e;

#include "application.h"

media_storage *
media_storage_create(application *app);

void
media_storage_destroy(media_storage *p_ms);

const char*
media_storage_get_path(media_storage *p_ms, media_directory_e type);

char*
system_storage_appdata_get();

void
media_storage_start_discovery(media_storage *p_ms);

Eina_List *
media_storage_external_list_get(const media_storage *p_ms);

#endif /* SYSTEM_STORAGE_H_ */
