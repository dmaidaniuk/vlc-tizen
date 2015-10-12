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

#include "common.h"

#include "ui/interface.h"
#include "video_view.h"
#include "video_view_list.h"


#include <Elementary.h>

struct view_sys
{
    list_view* p_list;
};

interface_view*
create_video_view(interface *intf, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));
    view_sys *p_sys = view->p_view_sys = malloc(sizeof(*p_sys));

    /* Box container */
    Evas_Object *box = elm_box_add(parent);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(box);

    p_sys->p_list = video_view_list_create(intf, box);

    view->view = box;

    /* */
    return view;
}

void
destroy_video_view(interface_view *view)
{
    list_view* p_list_view = view->p_view_sys->p_list;
    p_list_view->pf_del(p_list_view->p_sys);
    free(p_list_view);
    free(view->p_view_sys);
    free(view);
}
