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
#include "media/artist_item.h"
#include "controller/media_controller.h"

struct list_sys
{
    LIST_VIEW_COMMON
};

struct list_view_item
{
    const list_sys*                 p_list_sys;
    const Elm_Genlist_Item_Class*   itc;
    artist_item*                    p_artist_item;
    Elm_Object_Item*                p_object_item;
    // members used when spawning a new view out of an item
    media_library_controller*       p_controller;
};

/*
 * List albums when an artist is clicked
 */

/* This adds a layer to plumb the proper parameters in the media_controller call */
static void
audio_list_artist_item_get_album_cb(media_library* p_ml, media_library_list_cb cb, void* p_user_data )
{
    list_view_item* p_view_item = (list_view_item*)p_user_data;
    media_library_get_artist_albums(p_ml, p_view_item->p_artist_item->psz_name, cb, p_view_item->p_controller);
}

static void
audio_list_artist_item_get_songs_cb(media_library* p_ml, media_library_list_cb cb, void* p_user_data )
{
    list_view_item* p_view_item = (list_view_item*)p_user_data;
    media_library_get_artist_songs(p_ml, p_view_item->p_artist_item->psz_name, cb, p_view_item->p_controller);
}

static void
audio_list_artist_item_selected(void *data, Evas_Object *obj /*EINA_UNUSED*/, void *event_info)
{
    list_view_item *p_view_item = (list_view_item*)data;
    application* p_app = intf_get_application(p_view_item->p_list_sys->p_intf);
    list_view* p_view;

    if (p_view_item->p_artist_item->psz_name != NULL)
    {
        p_view = audio_list_album_view_create(p_view_item->p_list_sys->p_intf, p_view_item->p_list_sys->p_parent,
                LIST_CREATE_ALL & ~LIST_CREATE_MEDIA_CONTROLLER);
        /* Create a controller that will feed the new view */
        p_view->p_sys->p_ctrl = p_view_item->p_controller = album_controller_create(p_app, p_view);
        /* Tweak the video controller to list the songs of a specific artist only */
        media_library_controller_set_content_callback(p_view_item->p_controller,
                    audio_list_artist_item_get_album_cb, p_view_item);
    }
    else
    {
        p_view = audio_list_song_view_create(p_view_item->p_list_sys->p_intf, p_view_item->p_list_sys->p_parent,
                LIST_CREATE_ALL & ~LIST_CREATE_MEDIA_CONTROLLER);
        p_view->p_sys->p_ctrl = p_view_item->p_controller = audio_controller_create(p_app, p_view);
        media_library_controller_set_content_callback(p_view_item->p_controller,
                    audio_list_artist_item_get_songs_cb, p_view_item);
    }

    media_library_controller_refresh(p_view_item->p_controller);

    Evas_Object* p_new_list = p_view->pf_get_widget(p_view->p_sys);
    Elm_Object_Item *it = elm_naviframe_item_push(p_view_item->p_list_sys->p_parent, "", NULL, NULL, p_new_list, NULL);
    elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
}

static void
free_list_item_data(void *data, Evas_Object *obj, void *event_info)
{
    list_view_item *p_view_item = data;
    artist_item_destroy(p_view_item->p_artist_item);
    free(p_view_item);
}

static char *
genlist_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    list_view_item *p_view_item = data;
    const Elm_Genlist_Item_Class *itc = p_view_item->itc;
    char *buf;

    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.text.main.left.top")) {
            asprintf(&buf, "<b>%s</b>", p_view_item->p_artist_item->psz_name ?
                    p_view_item->p_artist_item->psz_name : "Unknown Artist");
            return buf;
        }
    }
    return NULL;
}

static const void*
audio_list_artist_item_get_media_item(list_view_item* p_item)
{
    return p_item->p_artist_item;
}

static void
audio_list_artist_item_set_media_item(list_view_item* p_view_item, void* p_data)
{
    artist_item* p_media_item = (artist_item*)p_data;
    p_view_item->p_artist_item = p_media_item;
    ecore_main_loop_thread_safe_call_async((Ecore_Cb)elm_genlist_item_update, p_view_item->p_object_item);
}

static list_view_item*
audio_list_artist_view_append_item(list_sys *p_sys, void* p_data)
{
    artist_item* p_artist_item = (artist_item*)p_data;
    list_view_item *p_view_item = calloc(1, sizeof(*p_view_item));
    p_view_item->p_list_sys = p_sys;
    p_view_item->itc = p_sys->p_default_item_class;

    p_view_item->p_artist_item = p_artist_item;

    /* Set and append new item in the genlist */
    Elm_Object_Item *it = elm_genlist_item_append(p_sys->p_list,
            p_sys->p_default_item_class,                /* genlist item class               */
            p_view_item,                                /* genlist item class user data     */
            NULL,                                       /* genlist parent item              */
            ELM_GENLIST_ITEM_NONE,                      /* genlist item type                */
            audio_list_artist_item_selected,            /* genlist select smart callback    */
            p_view_item);                               /* genlist smart callback user data */

    /* */
    elm_object_item_del_cb_set(it, free_list_item_data);
    list_view_toggle_empty(p_sys, false);
    return p_view_item;
}

list_view*
audio_list_artist_view_create(interface* p_intf, Evas_Object* p_parent, list_view_create_option opts)
{
    list_view* p_list_view = calloc(1, sizeof(*p_list_view));
    if (p_list_view == NULL)
        return NULL;
    list_sys* p_list_sys = p_list_view->p_sys = calloc(1, sizeof(*p_list_sys));
    if (p_list_sys == NULL)
        return NULL;

    list_view_common_setup(p_list_view, p_list_sys, p_intf, p_parent, opts);

    evas_object_size_hint_weight_set(p_list_sys->p_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p_list_sys->p_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Item Class */
    p_list_sys->p_default_item_class->func.text_get = genlist_text_get_cb;

    p_list_view->pf_append_item = &audio_list_artist_view_append_item;
    p_list_view->pf_get_item = &audio_list_artist_item_get_media_item;
    p_list_view->pf_set_item = &audio_list_artist_item_set_media_item;

    if (opts & LIST_CREATE_MEDIA_CONTROLLER)
    {
        application* p_app = intf_get_application( p_intf );
        p_list_sys->p_ctrl = artist_controller_create(p_app, p_list_view);
        media_library_controller_refresh( p_list_sys->p_ctrl );
    }

    return p_list_view;
}
