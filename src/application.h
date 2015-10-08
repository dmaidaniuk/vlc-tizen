/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Jean-Baptiste Kempf <jb@videolan.org>
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

#ifndef APPLICATION_H_
# define APPLICATION_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct application application;
typedef struct interface interface;
typedef struct media_library media_library;
typedef struct playback_service playback_service;
typedef struct media_storage media_storage;
typedef struct media_item media_item;
typedef struct media_list media_list;
typedef struct media_controller media_controller;

#include "system_storage.h"

const char *
application_get_media_path(application *app, media_directory_e type);

const media_storage *
application_get_media_storage(application *app);

const media_library *
application_get_media_library(application *app);

playback_service *
application_get_playback_service(application *app);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
