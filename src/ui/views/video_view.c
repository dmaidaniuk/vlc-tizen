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

#include "controller/video_controller.h"
#include "ui/interface.h"
#include "video_view.h"
#include "video_player.h"

#include "ui/utils.h"

#include "media/media_item.h"

#include <Elementary.h>

struct view_sys
{
    interface *p_intf;

    Evas_Object *p_parent;
    Evas_Object *p_genlist;
    video_controller* p_controller;
    Elm_Genlist_Item_Class* p_default_itc;

};

typedef struct video_list_item
{
    view_sys *videoview;

    const Elm_Genlist_Item_Class *itc;

    media_item *p_media_item;

    //For refresh purposes.
    Elm_Object_Item* p_object_item;

} video_list_item;

void
genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    video_list_item *vli = (video_list_item*)data;

    intf_video_player_play(vli->videoview->p_intf, vli->p_media_item->psz_path);
}

static void
free_list_item(void *data, Evas_Object *obj, void *event_info)
{
    video_list_item *vli = data;
    media_item_destroy(vli->p_media_item);
    free(vli);
}

static char *
duration_string(int64_t duration)
{
    lldiv_t d;
    long long sec;
    char *str;

    duration /= 1000;
    d = lldiv(duration, 60);
    sec = d.rem;
    d = lldiv(d.quot, 60);
    asprintf(&str, "%02lld:%02lld:%02lld", d.quot, d.rem, sec);
    return str;
}

static char *
genlist_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    video_list_item *vli = data;
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
                return duration_string(vli->p_media_item->i_duration);
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

const media_item*
video_list_item_get_media_item(video_list_item* p_item)
{
    return p_item->p_media_item;
}

void
video_list_item_set_media_item(video_list_item* p_item, const media_item* p_media_item)
{
    bool b_changed = false;
    if (p_item->p_media_item->i_duration != p_media_item->i_duration)
    {
        p_item->p_media_item->i_duration = p_media_item->i_duration;
        b_changed = true;
    }
    if (p_media_item->psz_snapshot != NULL && (
            p_item->p_media_item->psz_snapshot == NULL ||
            strcmp( p_item->p_media_item->psz_snapshot, p_media_item->psz_snapshot)))
    {
        free(p_item->p_media_item->psz_snapshot);
        p_item->p_media_item->psz_snapshot = strdup(p_media_item->psz_snapshot);
        b_changed = true;
    }
    if ( p_item->p_media_item->i_w != p_media_item->i_w ||
            p_item->p_media_item->i_h != p_media_item->i_h )
    {
        p_item->p_media_item->i_w = p_media_item->i_w;
        p_item->p_media_item->i_h = p_media_item->i_h;
        b_changed = true;
    }
    if (b_changed == true)
    {
        ecore_main_loop_thread_safe_call_async((Ecore_Cb)elm_genlist_item_update, p_item->p_object_item);
    }
}

static Evas_Object*
genlist_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    video_list_item *vli = data;
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
                icon = create_image(layout, ICON_DIR "background_cone.png");
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

video_list_item *
video_view_append_item(view_sys *videoview, media_item* p_item)
{
    /* */
    video_list_item *vli = calloc(1, sizeof(*vli));
    if (vli == NULL)
        return NULL;
    vli->videoview = videoview;
    vli->itc = videoview->p_default_itc;

    /* Item instantiation */
    vli->p_media_item = p_item;
    /* Set and append new item in the genlist */
    vli->p_object_item = elm_genlist_item_append(videoview->p_genlist,
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
    return vli;
}

void
video_view_clear(view_sys* videoview)
{
    elm_genlist_clear(videoview->p_genlist);
}

interface_view*
create_video_view(interface *intf, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));
    view_sys *vv = malloc(sizeof(*vv));
    vv->p_intf = intf;
    vv->p_parent = parent;
    vv->p_controller = video_controller_create(intf_get_application(intf), vv);

    /* Genlist class */
    vv->p_default_itc = elm_genlist_item_class_new();
    //FIXME: this is leaking, please call elm_genlist_item_class_free() from somewhere
    vv->p_default_itc->item_style = "2line.top.3";
    vv->p_default_itc->func.text_get = genlist_text_get_cb;
    vv->p_default_itc->func.content_get = genlist_content_get_cb;
    view->p_view_sys = vv;

    /* Set then create the Genlist object */
    Evas_Object *genlist = vv->p_genlist = elm_genlist_add(parent);

    /* Set the genlist modes */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    /* Set smart Callbacks */
    evas_object_smart_callback_add(genlist, "realized", genlist_realized_cb, NULL);
    evas_object_smart_callback_add(genlist, "loaded", genlist_loaded_cb, NULL);
    evas_object_smart_callback_add(genlist, "longpressed", genlist_longpressed_cb, NULL);
    evas_object_smart_callback_add(genlist, "contracted", genlist_contracted_cb, NULL);

    view->view = genlist;

    /* */
    return view;
}

void
destroy_video_view(interface_view *view)
{
}
