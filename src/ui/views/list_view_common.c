/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
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

#include "common.h"

#include "list_view_private.h"
#include "controller/media_library_controller.h"

struct list_sys
{
    LIST_VIEW_COMMON
};

static void
list_view_clear(list_sys* p_list)
{
    elm_genlist_clear(p_list->p_list);
}

static void
list_view_show(list_sys* p_list, Evas_Object* p_parent)
{
    //FIXME: This is wrong and makes assumptions about the parent widget
    Elm_Object_Item *it = elm_naviframe_item_push(p_parent, "", NULL, NULL, p_list->p_list, NULL);
    elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
    evas_object_show(p_list->p_list);
}

static void
list_view_destroy(list_sys* p_list)
{
    media_library_controller_destroy(p_list->p_ctrl);
    elm_genlist_item_class_free(p_list->p_default_item_class);
    free(p_list);
}

void
list_view_common_setup(list_view* p_view, list_sys* p_list, interface* p_intf, view_sys_cb* p_view_cb, Evas_Object* p_parent)
{
    p_list->p_intf = p_intf;
    p_list->p_view_cb = p_view_cb;

    /* Create genlist */
    p_list->p_list = elm_genlist_add(p_parent);
    elm_scroller_single_direction_set(p_list->p_list, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    elm_genlist_homogeneous_set(p_list->p_list, EINA_TRUE);
    elm_genlist_mode_set(p_list->p_list, ELM_LIST_COMPRESS);

    /* Item Class */
    p_list->p_default_item_class = elm_genlist_item_class_new();
    p_list->p_default_item_class->item_style = "2line.top.3";

    /* Setup common callbacks */
    p_view->pf_show = &list_view_show;
    p_view->pf_del = &list_view_destroy;
    p_view->pf_clear = &list_view_clear;
}
