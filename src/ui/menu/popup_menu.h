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

#ifndef POPUP_MENU_H_
#define POPUP_MENU_H_

#include "common.h"
#include "ui/interface.h"

typedef struct popup_menu
{
    char* title;
    char* icon;

    Evas_Smart_Cb cb;
} popup_menu;

Evas_Object *
popup_menu_add(popup_menu *menu, void *data, Evas_Object *parent);

Evas_Object *
popup_menu_orient_add(popup_menu *menu, Elm_Popup_Orient orient, void *data, Evas_Object *parent);

#endif
