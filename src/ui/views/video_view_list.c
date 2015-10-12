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

#include "video_view_list.h"

#include "controller/media_controller.h"
#include "ui/interface.h"
#include "media/media_item.h"
#include "ui/utils.h"
#include "video_player.h"


struct list_view_item
{
    list_sys*                       p_list;
    media_item*                     p_media_item;
    const Elm_Genlist_Item_Class*   itc;

    //For refresh purposes.
    Elm_Object_Item*                p_object_item;
};

struct list_sys
{
    media_library_controller*       p_controller;
    Evas_Object*                    p_video_list;
    Elm_Genlist_Item_Class*         p_default_item_class;
    interface*                      p_intf;
    Evas_Object*                    p_box;
    Evas_Object*                    p_empty_label;
};

static void
genlist_update_empty_view(list_sys *p_sys)
{
    //TODO improve me
    unsigned int count = elm_genlist_items_count(p_sys->p_video_list);
    if (count == 0) {
        elm_box_unpack_all(p_sys->p_box);
        elm_box_pack_end(p_sys->p_box, p_sys->p_empty_label);
        evas_object_show(p_sys->p_empty_label);
        evas_object_hide(p_sys->p_video_list);
    } else {
        elm_box_unpack_all(p_sys->p_box);
        elm_box_pack_end(p_sys->p_box, p_sys->p_video_list);
        evas_object_hide(p_sys->p_empty_label);
        evas_object_show(p_sys->p_video_list);
    }
}

void
genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    list_view_item *vli = (list_view_item*)data;

    intf_video_player_play(vli->p_list->p_intf, vli->p_media_item->psz_path);
}

static void
free_list_item(void *data, Evas_Object *obj, void *event_info)
{
    list_view_item *vli = data;
    media_item_destroy(vli->p_media_item);
    free(vli);
}

static char *
genlist_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    list_view_item *vli = data;
    const Elm_Genlist_Item_Class *itc = vli->itc;

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.text.main.left.top")) {
            char *buff;
            asprintf(&buff, "<b>%s</b>", media_item_title(vli->p_media_item));
            return buff;
        }
        else if (!strcmp(part, "elm.text.sub.left.bottom")) {
            if(vli->p_media_item->i_duration < 0)
                return NULL;
            else
                return media_timetostr(vli->p_media_item->i_duration/1000);
        }
        else if (!strcmp(part, "elm.text.sub.right.bottom")) {
            if (vli->p_media_item->i_w <= 0 || vli->p_media_item->i_h <= 0)
                return NULL;
            char *str_resolution;
            asprintf( &str_resolution, "%dx%d", vli->p_media_item->i_w, vli->p_media_item->i_h);
            return str_resolution;
        }
    }
    return NULL;
}

static const void*
video_list_item_get_media_item(list_view_item* p_list_item)
{
    return p_list_item->p_media_item;
}

static void
video_list_item_set_media_item(list_view_item* p_item, void* p_data)
{
    media_item* p_media_item = (media_item*)p_data;
    p_item->p_media_item = p_media_item;
    ecore_main_loop_thread_safe_call_async((Ecore_Cb)elm_genlist_item_update, p_item->p_object_item);
}

static Evas_Object*
genlist_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    list_view_item *vli = data;
    const Elm_Genlist_Item_Class *itc = vli->itc;

    Evas_Object *layout = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.icon.1")) {
            layout = elm_layout_add(obj);
            elm_layout_theme_set(layout, "layout", "list/B/type.1", "default");
            Evas_Object *icon;
            if (vli->p_media_item->psz_snapshot != NULL)
                icon = create_image(layout, vli->p_media_item->psz_snapshot);
            else
                icon = create_icon(layout, "background_cone.png");
            elm_layout_content_set(layout, "elm.swallow.content", icon);
        }
    }

    return layout;
}

static void
genlist_loaded_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
    /* Set the callbacks when one of the genlist item is loaded */
}

static void
genlist_realized_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    /* Set the callbacks when one of the genlist item is realized */
}

static void
genlist_longpressed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    /* Set the callbacks when one of the genlist item is longpress */
}

static void
genlist_contracted_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    Elm_Object_Item *it = event_info;

    /* Free the genlist subitems when contracted */
    elm_genlist_item_subitems_clear(it);
}

static list_view_item*
video_view_append_item(list_sys *p_list, void* p_data)
{
    media_item* p_item = (media_item*)p_data;
    /* */
    LOGE("Adding media item %s", p_item->psz_path);
    list_view_item *vli = calloc(1, sizeof(*vli));
    if (vli == NULL)
        return NULL;
    vli->p_list = p_list;
    vli->itc = p_list->p_default_item_class;

    /* Item instantiation */
    vli->p_media_item = p_item;
    /* Set and append new item in the genlist */
    vli->p_object_item = elm_genlist_item_append(p_list->p_video_list,
            vli->itc,                       /* genlist item class               */
            vli,                            /* genlist item class user data     */
            NULL,                           /* genlist parent item for trees    */
            ELM_GENLIST_ITEM_NONE,          /* genlist item type                */
            genlist_item_selected_cb,       /* genlist select smart callback    */
            vli);                           /* genlist smart callback user data */
    if (vli->p_object_item == NULL)
    {
        free(vli);
        return NULL;
    }
    /* */
    elm_object_item_del_cb_set(vli->p_object_item, free_list_item);
    genlist_update_empty_view(p_list);
    return vli;
}

static void
video_view_clear(list_sys* videoview)
{
    elm_genlist_clear(videoview->p_video_list);
    genlist_update_empty_view(videoview);
}

static void
video_view_list_destroy(list_sys* p_list)
{
    elm_genlist_item_class_free(p_list->p_default_item_class);
    media_library_controller_destroy(p_list->p_controller);
    free(p_list);
}

list_view*
video_view_list_create(interface *p_intf, Evas_Object *p_parent)
{
    list_view* p_view = calloc(1, sizeof(*p_view));
    if (p_view == NULL)
        return NULL;

    list_sys* p_sys = p_view->p_sys = calloc(1, sizeof(*p_sys));
    if (p_sys == NULL)
        return NULL;

    p_sys->p_box = p_parent;

    /* Empty list label */
    p_sys->p_empty_label = elm_label_add(p_sys->p_box);
    elm_object_text_set(p_sys->p_empty_label, "No video content to show");

    /* Create the Video Genlist */
    Evas_Object *p_genlist = elm_genlist_add(p_sys->p_box);

    p_sys->p_video_list = p_genlist;
    /* Genlist class */
    p_sys->p_default_item_class = elm_genlist_item_class_new();
    p_sys->p_default_item_class->item_style = "2line.top.3";
    p_sys->p_default_item_class->func.text_get = genlist_text_get_cb;
    p_sys->p_default_item_class->func.content_get = genlist_content_get_cb;

    /* Set the genlist modes */
    elm_scroller_single_direction_set(p_genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    elm_genlist_homogeneous_set(p_genlist, EINA_TRUE);
    elm_genlist_mode_set(p_genlist, ELM_LIST_COMPRESS);

    evas_object_size_hint_weight_set(p_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Set smart Callbacks on the list */
    evas_object_smart_callback_add(p_genlist, "realized", genlist_realized_cb, NULL);
    evas_object_smart_callback_add(p_genlist, "loaded", genlist_loaded_cb, NULL);
    evas_object_smart_callback_add(p_genlist, "longpressed", genlist_longpressed_cb, NULL);
    evas_object_smart_callback_add(p_genlist, "contracted", genlist_contracted_cb, NULL);

    p_sys->p_intf = p_intf;

    p_view->pf_del = &video_view_list_destroy;

    p_view->pf_append_item = &video_view_append_item;
    p_view->pf_clear = &video_view_clear;
    p_view->pf_get_item = &video_list_item_get_media_item;
    p_view->pf_set_item = &video_list_item_set_media_item;

    p_sys->p_controller = video_controller_create(intf_get_application(p_intf), p_view);

    genlist_update_empty_view(p_sys);

    return p_view;
}

