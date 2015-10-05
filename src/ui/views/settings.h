/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Ludovic Fauvet <etix@videolan.org>
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

#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct settings_item settings_item;
typedef void (*Settings_menu_callback)(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);

typedef struct popup_menu_item popup_menu_item_s;


typedef void (*Menu_item_callback)(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent, void *data);

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

#endif
