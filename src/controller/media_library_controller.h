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

#ifndef MEDIA_LIBRARY_CONTROLLER_H_
# define MEDIA_LIBRARY_CONTROLLER_H_

#include "application.h"
#include "ui/interface.h"

media_library_controller*
media_library_controller_create( application* p_app, list_view* p_list_view );

void
media_library_controller_destroy(media_library_controller *);

void
media_library_controller_refresh( media_library_controller* p_ctrl );

void
media_library_controller_set_content_callback(media_library_controller* p_ctrl, void(*cb)(media_library* p_ml, media_library_list_cb cb, void* p_user_data), void* p_user_data);

#endif /* MEDIA_LIBRARY_CONTROLLER_H_ */
