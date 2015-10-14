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

#ifndef MEDIA_CONTROLLER_H_
# define MEDIA_CONTROLLER_H_

#include "application.h"
#include "ui/interface.h"

// Allow one file to include "media_controller.h" and still get public media_library_controller API
#include "media_library_controller.h"

media_library_controller*
video_controller_create(application* p_app, list_view* p_list_view);

media_library_controller*
audio_controller_create(application* p_app, list_view* p_list_view);

media_library_controller*
artist_controller_create(application* p_app, list_view* p_list_view);

media_library_controller*
album_controller_create(application* p_app, list_view* p_list_view);

#endif // MEDIA_CONTROLLER_H_
