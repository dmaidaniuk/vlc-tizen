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
#include "version.h"

#include "ui/interface.h"
#include "ui/views/about_view.h"

#include <Elementary.h>
#include <EWebKit.h>

typedef struct cone_animation {
    Evas_Object *obj;
    Evas_Object *container;
    int initial_y;
    int anim_begin_y;
    int anim_end_y;
    int container_height;
} cone_animation;

static Eina_Bool
cone_do_drop(void *data, double pos)
{
   cone_animation *anim = data;
   int x,y,w,h;
   double frame = pos;
   frame = ecore_animator_pos_map(pos, ECORE_POS_MAP_BOUNCE, 2, 4);

   evas_object_geometry_get(anim->obj, &x, &y, &w, &h);
   double posy = frame * (anim->anim_end_y - anim->anim_begin_y);
   evas_object_move(anim->obj, x, anim->anim_begin_y + posy);

   if (y > anim->container_height)
   {
       anim->anim_begin_y = -h;
       anim->anim_end_y = anim->initial_y;
       ecore_animator_timeline_add(1, cone_do_drop, anim);
       return EINA_FALSE;
   }

   return EINA_TRUE;
}

static void
cone_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    cone_animation *anim = data;
    int y, h;
    evas_object_geometry_get(anim->obj, NULL, &y, NULL, &h);

    if (anim->initial_y < 0)
        anim->initial_y = y;

    evas_object_geometry_get(anim->container, NULL, NULL, NULL, &anim->container_height);


    anim->anim_begin_y = y;
    anim->anim_end_y = anim->container_height + h;

    ecore_animator_timeline_add(2, cone_do_drop, anim);
}

static void
cone_delete_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    cone_animation *anim = data;
    free(anim);
}

static Evas_Object*
create_about_section(Evas_Object *parent)
{
    /* Create layout and set the theme */
    Evas_Object *layout = elm_layout_add(parent);

    /* */
    elm_layout_file_set(layout, ABOUTEDJ, "about");

    /* Cone */
    Evas_Object *cone = elm_image_add(layout);
    evas_object_size_hint_align_set(cone, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(cone, EVAS_HINT_FILL, EVAS_HINT_EXPAND);
    elm_image_file_set(cone, ICON_DIR"cone.png", NULL);
    elm_object_part_content_set(layout, "cone", cone);
    evas_object_show(cone);

    /* Description */
    Evas_Object *lbl_description = elm_label_add(layout);
    evas_object_size_hint_align_set(lbl_description, EVAS_HINT_FILL, 0);
    evas_object_size_hint_weight_set(lbl_description, EVAS_HINT_FILL, EVAS_HINT_EXPAND);
    elm_label_line_wrap_set(lbl_description, ELM_WRAP_MIXED);
    elm_object_text_set(lbl_description, "<font-size=22><align=center>VLC for Tizen™ is a port of VLC media player, the popular open-source media player.");
    elm_object_part_content_set(layout, "description", lbl_description);
    evas_object_show(lbl_description);

    /* Revision number */
    elm_object_part_text_set(layout, "revision", REVISION);

    /* Handle cone animation */
    cone_animation *anim = malloc(sizeof(*anim));
    anim->initial_y = -1; // Initial position unknown
    anim->obj = cone;
    anim->container = layout;
    evas_object_smart_callback_add(cone, "clicked", cone_clicked_cb, anim);
    evas_object_event_callback_add(layout, EVAS_CALLBACK_FREE, cone_delete_cb, anim);

    /* */
    evas_object_show(layout);
    return layout;
}

static Evas_Object*
create_licence_section(Evas_Object *parent)
{
    Evas *e_webview = evas_object_evas_get(parent);
    Evas_Object *browser = ewk_view_add(e_webview);
    evas_object_layer_set(browser, EVAS_LAYER_MIN);

    ewk_view_url_set(browser, "file://"RES_DIR"/license.html");

    evas_object_show(browser);
    return browser;
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
    elm_toolbar_shrink_mode_set(tabbar, ELM_TOOLBAR_SHRINK_EXPAND);
    elm_toolbar_homogeneous_set(tabbar, EINA_FALSE);
    elm_toolbar_transverse_expanded_set(tabbar, EINA_FALSE);
    elm_toolbar_select_mode_set(tabbar, ELM_OBJECT_SELECT_MODE_DEFAULT);
    evas_object_size_hint_weight_set(tabbar, EVAS_HINT_FILL, 0.0);
    evas_object_size_hint_align_set(tabbar, EVAS_HINT_FILL, EVAS_HINT_FILL);

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
