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

/* Types */
#define SETTINGS_TYPE_CATEGORY 0
#define SETTINGS_TYPE_ITEM 1
#define SETTINGS_TYPE_TOGGLE 2

/* Forward declartions */
struct settings_item;
typedef struct settings_item settings_item;

/* Functions pointers */
typedef void (*Settings_menu_callback)(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
typedef void (*Menu_item_callback)(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent, void *data);

/* Structs */
typedef struct settings_item
{
    int id;
    const char* title;
    const char* icon;
    int type;
    Settings_menu_callback cb;

    bool toggled;
} settings_item;

typedef struct setting_data
{
    int id;
    int index;
    Elm_Object_Item *item;
    Evas_Object *parent;
    Evas_Object *genlist_test;

    settings_item *menu;
    int menu_len;
    Settings_menu_callback global_cb;

} setting_data;

typedef struct popup_genlist_data
{
    Evas_Object *parent;
    Elm_Object_Item *item;

    settings_item *menu;
    int index;
    int menu_len;

    Menu_item_callback cb;
    void *data;
} popup_genlist_data_s;

typedef struct popup_menu_item
{
    const int id;
    const char* title;
    const char* icon;

    int type;

    Settings_menu_callback cb;
} popup_menu_item_s;

/* Declarations */
interface_view*
create_setting_view(interface *intf, Evas_Object *parent);

void
destroy_setting_view(interface_view *);

static void
menu_directories_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
static void
menu_hwacceleration_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
static void
menu_subsenc_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
static void
menu_vorientation_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
static void
menu_performance_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
static void
menu_deblocking_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);

static Evas_Object *
settings_list_add(settings_item *menu, int len, Settings_menu_callback global_menu_cb, Evas_Object *parent);

static Evas_Object *
settings_popup_add(settings_item *menu, int menu_len, Settings_menu_callback global_menu_cb, Evas_Object *parent);

static void
settings_toggle_switch(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);

#endif /* SETTINGS_VIEW_H_ */
