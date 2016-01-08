/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

#ifndef LIST_VIEW_PRIVATE_H_
# define LIST_VIEW_PRIVATE_H_

#include "ui/interface.h"
#include "list_view.h"

#define LIST_VIEW_COMMON                                \
    interface*                  p_intf;                 \
    Evas_Object*                p_parent;               \
    Evas_Object*                p_list;                 \
    media_library_controller*   p_ctrl;                 \
    Elm_Genlist_Item_Class*     p_default_item_class;   \
    Evas_Object*                p_container;            \
    Evas_Object*                p_box;                  \
    Evas_Object*                p_empty_label;          \
    bool                        b_empty;

void
list_view_common_setup(list_view* p_view, list_sys* p_list, interface* p_intf, Evas_Object* p_parent, list_view_create_option opts);

void
list_view_toggle_empty(list_sys* p_view, bool b_empty);

#endif // LIST_VIEW_PRIVATE_H_
