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

#include "list_view.h"
#include "list_view_private.h"
#include "media/album_item.h"
#include "controller/media_controller.h"
#include "ui/utils.h"

struct list_sys
{
    LIST_VIEW_COMMON
    char* psz_artist_name;
};

struct list_view_item
{
    const list_sys*                 p_list_sys;
    const Elm_Genlist_Item_Class*   itc;
    album_item*                     p_album_item;
    Elm_Object_Item*                p_object_item;
};

/* List songs when an album is clicked */
static void
audio_list_album_item_selected(void *data, Evas_Object *obj /*EINA_UNUSED*/, void *event_info)
{
    list_view_item *p_view_item = (list_view_item*)data;

    list_view* p_songs_view = audio_list_song_view_album_create(p_view_item->p_list_sys->p_intf, p_view_item->p_list_sys->p_parent,
            p_view_item->p_album_item->psz_name, LIST_CREATE_ALL);

    Evas_Object* p_new_list = p_songs_view->pf_get_widget(p_songs_view->p_sys);
    Elm_Object_Item *it = elm_naviframe_item_push(p_view_item->p_list_sys->p_parent, "", NULL, NULL, p_new_list, NULL);
    elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
}

static void
free_list_item_data(void *data, Evas_Object *obj, void *event_info)
{
    list_view_item *p_view_item = data;
    album_item_destroy(p_view_item->p_album_item);
    free(p_view_item);
}

static Evas_Object*
genlist_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    list_view_item *ali = data;
    const Elm_Genlist_Item_Class *itc = ali->itc;
    Evas_Object *layout = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.icon.1")) {
            layout = elm_layout_add(obj);
            elm_layout_theme_set(layout, "layout", "list/B/type.1", "default");
            Evas_Object *icon;
            if (ali->p_album_item->psz_artwork != NULL)
                icon = create_image(layout, ali->p_album_item->psz_artwork);
            else
                icon = create_icon(layout, "background_cone.png");
            elm_layout_content_set(layout, "elm.swallow.content", icon);
        }
    }

    return layout;
}

static char *
genlist_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    list_view_item *p_view_item = data;
    const Elm_Genlist_Item_Class *itc = p_view_item->itc;
    char *buf = NULL;

    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.text.main.left.top")) {
            asprintf(&buf, "<b>%s</b>", p_view_item->p_album_item->psz_name);
            return buf;
        }
        else if (!strcmp(part, "elm.text.sub.right.bottom")) {
            asprintf(&buf, "%d track%s", p_view_item->p_album_item->i_nb_tracks,
                    p_view_item->p_album_item->i_nb_tracks > 1 ? "s" : "" );
            return buf;
        }
    }
    return NULL;
}

static const void*
audio_list_album_item_get_media_item(list_view_item* p_view_item)
{
    return p_view_item->p_album_item;
}

static void
audio_list_album_item_set_media_item(list_view_item* p_view_item, void* p_data)
{
    album_item* p_media_item = (album_item*)p_data;
    p_view_item->p_album_item = p_media_item;
    ecore_main_loop_thread_safe_call_async((Ecore_Cb)elm_genlist_item_update, p_view_item->p_object_item);
}

static list_view_item*
audio_list_album_view_append_item(list_sys *p_list_sys, void* p_data)
{
    album_item* p_album_item = (album_item*)p_data;
    list_view_item *p_view_item = calloc(1, sizeof(*p_view_item));
    p_view_item->p_list_sys = p_list_sys;
    p_view_item->itc = p_list_sys->p_default_item_class;

    p_view_item->p_album_item = p_album_item;

    /* Set and append new item in the genlist */
    Elm_Object_Item *it = elm_genlist_item_append(p_list_sys->p_list,
            p_list_sys->p_default_item_class,           /* genlist item class               */
            p_view_item,                                /* genlist item class user data     */
            NULL,                                       /* genlist parent item              */
            ELM_GENLIST_ITEM_NONE,                      /* genlist item type                */
            audio_list_album_item_selected,             /* genlist select smart callback    */
            p_view_item);                               /* genlist smart callback user data */

    /* */
    elm_object_item_del_cb_set(it, free_list_item_data);
    list_view_toggle_empty(p_list_sys, false);
    return p_view_item;
}

static void
audio_list_album_view_delete(list_sys* p_list_sys)
{
    media_library_controller_destroy(p_list_sys->p_ctrl);
    elm_genlist_item_class_free(p_list_sys->p_default_item_class);
    free(p_list_sys->psz_artist_name);
    free(p_list_sys);
}

static void
audio_list_album_get_albums_cb(media_library* p_ml, media_library_list_cb cb, void* p_user_data )
{
    list_sys* p_list_sys = (list_sys*)p_user_data;
    if (p_list_sys->psz_artist_name != NULL)
        media_library_get_artist_albums(p_ml, p_list_sys->psz_artist_name, cb, p_list_sys->p_ctrl);
    else
        media_library_get_albums(p_ml, cb, p_list_sys->p_ctrl);
}

list_view*
audio_list_album_view_create(interface* p_intf, Evas_Object* p_parent, const char* psz_artist_name, list_view_create_option opts)
{
    list_view* p_list_view = calloc(1, sizeof(*p_list_view));
    if (p_list_view == NULL)
        return NULL;
    list_sys* p_list_sys = p_list_view->p_sys = calloc(1, sizeof(*p_list_sys));
    if (p_list_sys == NULL)
        return NULL;
    if (psz_artist_name != NULL)
        p_list_sys->psz_artist_name = strdup(psz_artist_name);

    list_view_common_setup(p_list_view, p_list_sys, p_intf, p_parent, opts);

    evas_object_size_hint_weight_set(p_list_sys->p_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p_list_sys->p_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Item Class */
    p_list_sys->p_default_item_class->func.text_get = genlist_text_get_cb;
    p_list_sys->p_default_item_class->func.content_get = genlist_content_get_cb;

    p_list_view->pf_append_item = &audio_list_album_view_append_item;
    p_list_view->pf_get_item = &audio_list_album_item_get_media_item;
    p_list_view->pf_set_item = &audio_list_album_item_set_media_item;
    p_list_view->pf_del = &audio_list_album_view_delete;

    application* p_app = intf_get_application( p_intf );
    p_list_sys->p_ctrl = album_controller_create(p_app, p_list_view);
    media_library_controller_set_content_callback(p_list_sys->p_ctrl, audio_list_album_get_albums_cb, p_list_sys);
    media_library_controller_refresh(p_list_sys->p_ctrl);
    return p_list_view;
}
