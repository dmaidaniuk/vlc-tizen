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
#include "interface.h"
#include <player.h>
#ifndef MINI_PLAYER_H_
#define MINI_PLAYER_H_


typedef struct mini_player_data {
    bool visible_state;
    bool play_state, save_state, shuffle_state, playlist_state, more_state, fs_state;
    int repeat_state;
    char *file_path;
    player_h player;
    Evas_Object *parent, *table, *fs_table, *popup;
    Evas_Object *mini_player_box, *box, *fullscreen_box;
    Evas_Object *cover, *fs_cover, *fs_view, *fs_time, *fs_total_time;
    Evas_Object *title, *sub_title, *fs_title, *fs_sub_title;
    Evas_Object *play_pause_img;
    Evas_Object *fs_play_pause_img;
    Evas_Object *fs_save_btn, *fs_playlist_btn, *fs_more_btn;
    Evas_Object *fs_repeat_btn, *fs_shuffle_btn;
    gui_data_s *gd;

} mini_player_data_s;

mini_player_data_s*
mini_player_create(gui_data_s *gd, Evas_Object *parent);

bool
mini_player_visibility_state(mini_player_data_s *mpd);

void
mini_player_show(mini_player_data_s *mpd);

void
mini_player_hide(mini_player_data_s *mpd);

void
create_base_player(mini_player_data_s *mpd, char *file_path);

#endif /* MINI_PLAYER_H_ */
