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

#ifndef MAIN_POPUP_LIST_H_
#define MAIN_POPUP_LIST_H_

#include "ui/interface.h"
#include <Elementary.h>

typedef struct popup_menu_item
{
    const char* title;
    const char* icon;
} popup_menu_item_s;

typedef struct popup_genlist_data
{
    int index;
    int nb_item;
    popup_menu_item_s *menu_item;
    Evas_Object *box, *genlist;
    Evas_Object *img;
    Elm_Object_Item *item;
    Evas_Object *parent;

} popup_genlist_data_s;

Evas_Object *
create_settings_popup_genlist(Evas_Object *parent, popup_menu_item_s *directory_menu, int nb_item);

#endif /* MAIN_POPUP_LIST_H_ */
