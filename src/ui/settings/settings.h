/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Nicolas Rechatin <nicolas@videolabs.io>
 *          Ludovic Fauvet <etix@videolan.org>
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

#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "ui/interface.h"
#include "ui/settings/menu_id.h"

/* Defines */
#define SETTINGS_TYPE_CATEGORY 0
#define SETTINGS_TYPE_ITEM 1
#define SETTINGS_TYPE_TOGGLE 2

/* Forward declartions */
struct settings_item;
typedef struct settings_item settings_item;
struct settings_menu_selected;
typedef struct settings_menu_selected settings_menu_selected;

/* Functions pointers */
typedef void (*Settings_menu_callback)(settings_menu_selected *menu_info, view_sys* p_view_sys, void *data, Evas_Object *parent);

// Structs
typedef struct settings_menu_selected
{
    menu_id id;
    int index;
    int menu_len;
    settings_item *menu;

} settings_menu_selected;

typedef struct settings_item
{
    menu_id id;
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
    void *data;
} settings_internal_data;

/* Declarations */

Evas_Object *
settings_list_add(settings_item *menu, int len, Settings_menu_callback global_menu_cb, void *data, view_sys* p_view_sys, Evas_Object *parent);

Evas_Object *
settings_popup_add(settings_item *menu, int menu_len, Settings_menu_callback global_menu_cb, void *data, view_sys* p_view_sys, Evas_Object *parent);

void
settings_toggle_set_all(settings_item *menu, int menu_len, bool value);

void
settings_toggle_set_one_by_index(settings_item *menu, int menu_len, int index, bool value, bool toggle_others);

void
settings_toggle_set_one_by_id(settings_item *menu, int menu_len, int id, bool value, bool toggle_others);

int
settings_get_int(char *key, int default_value);

#endif
