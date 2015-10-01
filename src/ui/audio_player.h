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

#ifndef MINI_PLAYER_H_
#define MINI_PLAYER_H_

#include "ui/interface.h"

typedef struct mini_player mini_player;

mini_player*
mini_player_create(interface *intf, playback_service *p_ps, Evas_Object *parent, Evas_Object **);

void
create_base_player(mini_player *mpd, const char *file_path);

void
mini_player_stop(mini_player *);

bool
mini_player_play_state(mini_player *);

bool
mini_player_fs_state(mini_player *);

void
collapse_fullscreen_player(mini_player *);

#endif /* MINI_PLAYER_H_ */
