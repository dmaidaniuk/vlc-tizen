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

#include "media/library/media_library.hpp"

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

typedef enum interface_view_event {
    INTERFACE_VIEW_EVENT_NONE,
    INTERFACE_VIEW_EVENT_BACK,
    INTERFACE_VIEW_EVENT_MENU,
}interface_view_event;

typedef enum view_e {
    VIEW_AUTO = -1,
    VIEW_VIDEO,
    VIEW_AUDIO,
    VIEW_FILES,
    VIEW_SETTINGS,
    VIEW_ABOUT,
    VIEW_MAX,
} view_e;

typedef struct interface_view {
    Evas_Object *view;                      /* The Evas View prepared to be stacked */
    view_sys *p_view_sys;                   /* The view private data */
    view_e type;

    void (*pf_start)(view_sys *p_view_sys);    /* CB when the view is started/resumed */
    void (*pf_stop) (view_sys *p_view_sys);    /* CB when the view is stoped/paused */

    bool (*pf_has_menu)(view_sys *p_view_sys); /* Does the view needs an overflow menu? */
    bool (*pf_event)(view_sys *p_view_sys, interface_view_event event); /* */

} interface_view;


void
intf_show_view(interface *, view_e);

// FIXME REMOVE
void
intf_show_previous_view(interface *);

/* Video and Audio players */
void
intf_video_player_play(interface *intf, const char *psz_path);

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

/* List view items */
typedef struct list_sys list_sys;
typedef struct list_view list_view;
typedef struct list_view_item list_view_item;
typedef struct list_view
{
    list_sys* p_sys;
    void            (*pf_del)(list_sys* p_sys);
    void            (*pf_show)(list_sys* p_sys, Evas_Object* p_parent);
    list_view_item* (*pf_append_item)(list_sys* p_sys, void* p_item);
    void            (*pf_clear)(list_sys* p_sys);
    const void*     (*pf_get_item)(list_view_item* p_list_item);
    void            (*pf_set_item)(list_view_item* p_list_item, void* p_item);
} list_view;

#endif /* INTERFACE_H_ */
