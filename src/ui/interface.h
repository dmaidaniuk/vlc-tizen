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

#include "media/media_library.hpp"

typedef struct interface interface;


/* Creation / Destruction */
interface *
intf_create(application *);

application *
intf_get_application(interface *);

void
intf_destroy(interface *);

/* Views */
typedef struct view_sys view_sys;

enum {
    INTERFACE_VIEW_EVENT_NONE,
    INTERFACE_VIEW_EVENT_BACK,
    INTERFACE_VIEW_EVENT_MENU,
};
typedef struct interface_view {
    Evas_Object *view;                /* The Evas View prepared to be stacked */
    view_sys *p_view_sys;                   /* The view private data */

    void (*pf_start)(view_sys *p_view_sys); /* CB when the view is started/resumed */
    void (*pf_stop) (view_sys *p_view_sys); /* CB when the view is stoped/paused */
    bool (*pf_event)(view_sys *p_view_sys, int event); /* */
} interface_view;

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
intf_show_view(interface *, view_e);

// FIXME REMOVE
void
intf_show_previous_view(interface *);

/* Video and Audio players */
void
intf_create_video_player(interface *, const char *psz_path);

void
intf_create_audio_player(interface *, const char *psz_path);

/* Mini Player */
bool
intf_mini_player_visible_get(interface *);

bool
intf_mini_player_visible_set(interface *intf, bool visible);

//FIXME REMOVE
void
intf_update_mini_player(interface *);

/* Other */
Evas_Object *
intf_get_main_naviframe(interface *intf);

/* Media Library */
void
intf_register_file_changed(interface *intf, view_e type,
        media_library_file_list_changed_cb cb, void* p_user_data);

void
intf_ml_file_changed( void* p_user_data );

#endif /* INTERFACE_H_ */
