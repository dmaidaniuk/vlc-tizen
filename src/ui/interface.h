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

#include "common.h"
#include "application.h"

struct mini_player_data;
struct interface_priv_sys;

typedef struct interface_sys {
    application_sys *app;

    struct interface_priv_sys *intf_p;
    struct mini_player_instance *mini_player;
} interface_sys;

Evas*
get_window(interface_sys *intf);

Evas_Object*
get_sidebar(interface_sys *intf);

Evas_Object *
get_miniplayer_content_box(interface_sys *intf);

Evas_Object *
get_content(interface_sys *intf);

Evas_Object *
get_toolbar(interface_sys *intf);

typedef enum view_e {
    VIEW_AUTO = -1,
    VIEW_VIDEO,
    VIEW_AUDIO,
    VIEW_FILES,
    VIEW_SETTINGS,
    VIEW_ABOUT,
    VIEW_MAX,
} view_e;

void
create_view(interface_sys *gd, view_e panel);

void
show_previous_view(interface_sys *);

void
create_base_gui(application_sys *);

void
update_mini_player(interface_sys *);

#endif /* INTERFACE_H_ */
