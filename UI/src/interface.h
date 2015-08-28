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

#ifndef INTERFACE_H_
#define INTERFACE_H_

struct mini_player_data;
struct video_player_data;
typedef struct gui_data {
	Evas_Object *win;
	Evas_Object *conform, *nf_toolbar;
	Evas_Object *content;
	Evas_Object *panel;
	int panel_choice;
	Evas_Object *panel_toggle_btn,*popup_toggle_btn;
	Evas_Object *popup;
	Evas_Object *current_view;
	Evas_Object *content_box;
	struct mini_player_data *mini_player;
	struct video_player_data *video_player;
	char *media_path, *rmp;

} gui_data_s;

enum {
	VIEW_AUTO = -1,
	VIEW_VIDEO,
	VIEW_AUDIO,
	VIEW_FILES,
	VIEW_SETTINGS,
	VIEW_ABOUT,
	VIEW_MAX,
};

void
create_view(gui_data_s *gd, int panel);


#endif /* INTERFACE_H_ */
