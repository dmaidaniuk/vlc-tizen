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

#ifndef MEDIALIBRARY_H_
#define MEDIALIBRARY_H_

#include "media_item.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "application.h"

typedef void (*media_library_file_list_changed_cb)( void* p_user_data );
typedef void (*media_library_list_video_cb)( Eina_List*, void *p_user_data );
typedef void (*media_library_list_audio_cb)( Eina_List*, void *p_user_data );

media_library* media_library_create(application* p_app);
bool media_library_start( media_library* p_media_library, media_library_file_list_changed_cb cb, void* p_user_data );
void media_library_delete(media_library* p_media_library);
void media_library_discover( media_library* p_ml, const char* psz_location );

void
media_library_get_video_files( media_library* p_ml, media_library_list_video_cb cb, void* p_user_data );

void
media_library_get_audio_files( media_library* p_ml, media_library_list_audio_cb cb, void* p_user_data );

#ifdef __cplusplus
}
#endif

#endif /* MEDIALIBRARY_H_ */
