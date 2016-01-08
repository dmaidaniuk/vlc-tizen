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

#ifndef MINI_PLAYER_H_
#define MINI_PLAYER_H_

#include "ui/interface.h"

typedef struct audio_player audio_player;

audio_player*
audio_player_create(interface *intf, playback_service *p_ps, Evas_Object *layout);

void
audio_player_start(audio_player *mpd, Eina_Array *array, int pos);

void
audio_player_stop(audio_player *);

bool
audio_player_play_state(audio_player *);

bool
audio_player_fs_state(audio_player *);

void
audio_player_collapse_fullscreen_player(audio_player *);

bool
audio_player_handle_back_key(audio_player *);

void
destroy_audio_player(audio_player *);

#endif /* MINI_PLAYER_H_ */
