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

#ifndef __vlc_common_H__
#define __vlc_common_H__

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>

#include <tizen.h>
#include <app_common.h>
#include <dlog.h>

#ifdef  LOG_TAG
# undef  LOG_TAG
#endif
#define LOG_TAG "vlc"

#undef LOGD
#undef LOGI
#undef LOGW
#undef LOGE
#undef LOGF
#ifdef _DEBUG
#define LOGD(...)  dlog_print(DLOG_DEBUG, LOG_TAG, __VA_ARGS__)
#else
#define LOGD(...)
#endif
#define LOGI(...)  dlog_print(DLOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...)  dlog_print(DLOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...)  dlog_print(DLOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGF(...)  dlog_print(DLOG_FATAL, LOG_TAG, __VA_ARGS__)


#if !defined(PACKAGE)
# define PACKAGE "org.videolan.vlc"
#endif

#define EDJPATH "edje"
#define ICONPATH "icon"
#define IMAGESPATH "images"
#define VIDEOPLAYEREDJ get_resource_path( EDJPATH "/video_player.edj")
#define AUDIOPLAYERMINIEDJ get_resource_path( EDJPATH "/audio_player_mini.edj")
#define AUDIOPLAYEREDJ get_resource_path( EDJPATH "/audio_player.edj")
#define ABOUTEDJ get_resource_path( EDJPATH "/about.edj")
#define THEMEEDJ get_resource_path( EDJPATH "/theme.edj")

#define RES_DIR "/opt/usr/apps/" PACKAGE "/res/"
#define ICON_DIR RES_DIR "/images/"

#include <Evas.h>

#define MAX_LENGTH_PATH 1024
static inline const char *get_resource_path(const char *file_path)
{
    static char absolute_path[MAX_LENGTH_PATH] = {'\0'};

    static char *res_path_buff = NULL;
    if(res_path_buff == NULL)
    {
        res_path_buff = app_get_resource_path();
    }

    snprintf(absolute_path, MAX_LENGTH_PATH, "%s%s", res_path_buff, file_path);

    return absolute_path;
}
#endif /* __vlc_common_H__ */
