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

#ifndef SETTINGS_VIEW_H_
#define SETTINGS_VIEW_H_

#include "ui/interface.h"

/* ID */
#define SETTINGS_ID_DIRECTORIES 1
#define SETTINGS_ID_HWACCELERATION 2
#define SETTINGS_ID_SUBSENC 3
#define SETTINGS_ID_VORIENTATION 4
#define SETTINGS_ID_PERFORMANCES 5
#define SETTINGS_ID_DEBLOCKING 6

#define HWACCELERATION_AUTOMATIC 1
#define HWACCELERATION_DISABLED 2
#define HWACCELERATION_DECODING 3
#define HWACCELERATION_FULL 4

#define ORIENTATION_AUTOMATIC 1
#define ORIENTATION_LOCKED 2
#define ORIENTATION_LANDSCAPE 3
#define ORIENTATION_PORTRAIT 4
#define ORIENTATION_R_LANDSCAPE 5
#define ORIENTATION_R_PORTRAIT 6

#define PERFORMANCE_FRAME_SKIP 1
#define PERFORMANCE_STRETCH 2

/* Types */
#define SETTINGS_TYPE_CATEGORY 0
#define SETTINGS_TYPE_ITEM 1
#define SETTINGS_TYPE_TOGGLE 2

/* Forward declartions */
struct settings_item;
typedef struct settings_item settings_item;
struct settings_menu_selected;
typedef struct settings_menu_selected settings_menu_selected;

/* Functions pointers */
typedef void (*Settings_menu_callback)(settings_menu_selected *menu_info, view_sys* p_view_sys, Evas_Object *parent);

/* Structs */
typedef struct settings_menu_selected
{
    int id;
    int index;
    int menu_len;
    settings_item *menu;

} settings_menu_selected;

typedef struct settings_item
{
    int id;
    const char* title;
    const char* icon;
    int type;
    Settings_menu_callback cb;

    bool toggled;
} settings_item;

typedef struct settings_internal_data
{
    settings_menu_selected selected;
    Elm_Object_Item *item;
    Evas_Object *parent;

    Settings_menu_callback global_cb;

    view_sys* p_view_sys;
} settings_internal_data;

/* Declarations */
interface_view*
create_setting_view(interface *intf, Evas_Object *parent);

void
destroy_setting_view(interface_view *);

void
menu_directories_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent);
void
menu_hwacceleration_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent);
void
menu_subsenc_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent);
void
menu_vorientation_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent);
void
menu_performance_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent);
void
menu_deblocking_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent);

Evas_Object *
settings_list_add(settings_item *menu, int len, Settings_menu_callback global_menu_cb, view_sys* p_view_sys, Evas_Object *parent);

Evas_Object *
settings_popup_add(settings_item *menu, int menu_len, Settings_menu_callback global_menu_cb, view_sys* p_view_sys, Evas_Object *parent);

void
settings_toggle_switch(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent);

#endif /* SETTINGS_VIEW_H_ */
