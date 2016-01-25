/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Ludovic Fauvet <etix@videolan.org>
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

#ifndef MINICONTROL_VIEW_H_
#define MINICONTROL_VIEW_H_

#include "ui/interface.h"

struct minicontrol;
typedef struct minicontrol minicontrol;

minicontrol*
mini_control_view_create();

void
mini_control_destroy(minicontrol *mc);

void
mini_control_visibility_set(minicontrol *mc, Eina_Bool visible);

void
mini_control_playing_set(minicontrol *mc, Eina_Bool playing);

void
mini_control_title_set(minicontrol *mc, const char* title);

void
mini_control_progress_set(minicontrol *mc, double progress);

void
mini_control_cover_set(minicontrol *mc, const char* path);

#endif /* MINICONTROL_VIEW_H_ */
