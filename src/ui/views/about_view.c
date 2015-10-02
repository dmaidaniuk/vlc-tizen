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
 ******************************************************************************/

#include "common.h"

#include "ui/interface.h"
#include "ui/views/about_view.h"

#include <Elementary.h>

static Evas_Object*
create_about_section(Evas_Object *parent)
{
    /* */
    Evas_Object *box = elm_box_add(parent);
    elm_box_horizontal_set(box, EINA_FALSE);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Add and set the label in the box */
    Evas_Object *lbl_about_title = elm_label_add(box);
    evas_object_size_hint_align_set(lbl_about_title, EVAS_HINT_FILL, 0.5);
    elm_object_text_set(lbl_about_title, "<align=center><b>Building VLC for Tizen</b></align>");
    elm_box_pack_end(box, lbl_about_title);
    evas_object_show(lbl_about_title);

    /* */
    evas_object_show(box);
    return box;
}

static Evas_Object*
create_licence_section(Evas_Object *parent)
{
    /* Add set set the box in the toolbar naviframe */
    Evas_Object *box = elm_box_add(parent);
    elm_box_horizontal_set(box, EINA_FALSE);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Add and set the label in the box */
    Evas_Object *lbl_license_title = elm_label_add(box);
    evas_object_size_hint_align_set(lbl_license_title, EVAS_HINT_FILL, 0.5);
    elm_object_text_set(lbl_license_title, "<align=center><b>VLC License for Tizen</b></align>");
    elm_box_pack_end(box, lbl_license_title);  //Add the label in the box
    evas_object_show(lbl_license_title);

    /* */
    evas_object_show(box);
    return box;
}

static void
tabbar_item_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *nf_toolbar = data;
    const char *str = elm_object_item_text_get((Elm_Object_Item *)event_info);

    /* Create the view depending on the item selected in the toolbar */
    Evas_Object *content;
    if (str && !strcmp(str, "License")) {
        content = create_licence_section(nf_toolbar);
    }
    else {
        content = create_about_section(nf_toolbar);
    }

    /* Show it without title */
    Elm_Object_Item *item = elm_naviframe_item_push(nf_toolbar, str, NULL, NULL, content, NULL);
    elm_naviframe_item_title_enabled_set(item, EINA_FALSE, EINA_FALSE);
}

static Evas_Object*
create_toolbar(Evas_Object *nf_toolbar)
{
    /* Create and set the toolbar */
    Evas_Object *tabbar = elm_toolbar_add(nf_toolbar);

    /* Set the toolbar options */
    elm_toolbar_shrink_mode_set(tabbar, ELM_TOOLBAR_SHRINK_NONE);
    elm_toolbar_transverse_expanded_set(tabbar, EINA_TRUE);
    elm_toolbar_select_mode_set(tabbar, ELM_OBJECT_SELECT_MODE_DEFAULT);

    /* Append new entry in the toolbar with the Icon & Label wanted */
    elm_toolbar_item_append(tabbar, NULL, "About",   tabbar_item_cb, nf_toolbar);
    elm_toolbar_item_append(tabbar, NULL, "License", tabbar_item_cb, nf_toolbar);

    return tabbar;
}

interface_view *
create_about_view(interface *intf, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));

    /* Content box */
    Evas_Object *about_box = elm_box_add(parent);
    evas_object_size_hint_weight_set(about_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(about_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Toolbar Naviframe */
    Evas_Object *nf_toolbar = elm_naviframe_add(about_box);
    evas_object_size_hint_weight_set(nf_toolbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(nf_toolbar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Create the toolbar in the view */
    Evas_Object *tabbar = create_toolbar(nf_toolbar);

    /* Add them to the box */
    elm_box_pack_end(about_box, tabbar);
    evas_object_show(tabbar);
    elm_box_pack_end(about_box, nf_toolbar);
    evas_object_show(nf_toolbar);

    /* Set the first item in the toolbar */
    elm_toolbar_item_selected_set(elm_toolbar_first_item_get(tabbar), EINA_TRUE);

    /*  */
    evas_object_show(about_box);
    view->view = about_box;
    return view;
}

void
destroy_about_view(interface_view *view)
{
}
