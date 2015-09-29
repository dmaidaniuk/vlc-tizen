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

typedef struct interface interface;
typedef struct mini_player mini_player;


Evas*
intf_get_window(interface *intf);

Evas_Object*
intf_get_sidebar(interface *intf);

Evas_Object *
intf_get_miniplayer_content_box(interface *intf);

Evas_Object *
intf_get_main_naviframe(interface *intf);

Evas_Object *
intf_get_toolbar(interface *intf);

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
intf_create_view(interface *, view_e);

void
intf_show_previous_view(interface *);

interface *
intf_create_base_gui(application *);

void
intf_destroy(interface *);

void
intf_update_mini_player(interface *);

application *
intf_get_application(interface *);

mini_player *
intf_get_mini_player(interface *);

#endif /* INTERFACE_H_ */
