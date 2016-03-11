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

#include "common.h"

#include "list_view.h"
#include "audio_view.h"
#include "controller/media_controller.h"
#include "list_view_private.h"
#include "ui/interface.h"
#include "ui/utils.h"
#include "media/playlist_item.h"
#include "ui/menu/popup_menu.h"

typedef void (*playlists_selected_cb)(void *data, Evas_Object *obj, void *event_info);

struct list_sys
{
    LIST_VIEW_COMMON
    playlists_selected_cb pf_selected;

    Evas_Object *current_popup;
    Elm_Object_Item *current_item;
};

struct list_view_item
{
    const list_sys*                 p_list_sys;
    const Elm_Genlist_Item_Class*   itc;
    playlist_item*                  p_playlist_item;
    Elm_Object_Item*                p_object_item;
};

static void
free_list_item_data(void *data, Evas_Object *obj, void *event_info)
{
    list_view_item *ali = data;
    playlist_item_destroy(ali->p_playlist_item);
    free(ali);
}

static char *
genlist_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    list_view_item *ali = data;
    const Elm_Genlist_Item_Class *itc = ali->itc;
    char *buf;

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.text.main.left.top")) {
            // Don't display track number out of the album songs view (ie. when i_album_id != 0)
            asprintf(&buf, "<b>%s</b>", ali->p_playlist_item->psz_name);
            return buf;
        }
    }
    return NULL;
}

const void*
audio_list_playlists_item_get_playlist_item(list_view_item* p_item)
{
    return p_item->p_playlist_item;
}

static void
audio_list_playlists_item_set_playlist_item(list_view_item* p_item, void* p_data)
{
    playlist_item *p_playlist_item = (playlist_item*)p_data;
    p_item->p_playlist_item = p_playlist_item;
    ecore_main_loop_thread_safe_call_async((Ecore_Cb)elm_genlist_item_update, p_item->p_object_item);
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
            icon = create_icon(layout, "background_cone.png");
            elm_layout_content_set(layout, "elm.swallow.content", icon);
        }
    }

    return layout;
}

static void
audio_list_playlists_item_selected(void *data, Evas_Object *obj, void *event_info)
{
    list_view_item* p_view_item = (list_view_item*)data;
    list_view* p_view;

    if (p_view_item->p_list_sys->current_popup)
    {
        // A popup is already open, discard the click event.
        // Because a longpress trigger a longpress event + a click
        // we have to handle that manually.
        return;
    }

    p_view = audio_list_song_view_playlist_create(p_view_item->p_list_sys->p_intf,
            p_view_item->p_list_sys->p_parent,
            p_view_item->p_playlist_item->i_id, LIST_CREATE_ALL);
    Evas_Object* p_new_list = p_view->pf_get_widget(p_view->p_sys);
    Elm_Object_Item *it = elm_naviframe_item_push(p_view_item->p_list_sys->p_parent, "", NULL, NULL, p_new_list, NULL);
    elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
    elm_object_item_data_set(it, p_view);
}

static list_view_item*
audio_list_playlists_view_append_item(list_sys *p_sys, void* p_data)
{
    playlist_item* p_playlist_item = (playlist_item*)p_data;
    list_view_item *ali = calloc(1, sizeof(*ali));
    ali->p_list_sys = p_sys;
    ali->itc = p_sys->p_default_item_class;

    ali->p_playlist_item = p_playlist_item;

    /* Set and append new item in the genlist */
    Elm_Object_Item *it = elm_genlist_item_append(p_sys->p_list,
            p_sys->p_default_item_class,                /* genlist item class               */
            ali,                                        /* genlist item class user data     */
            NULL,                                       /* genlist parent item              */
            ELM_GENLIST_ITEM_NONE,                      /* genlist item type                */
            p_sys->pf_selected,                         /* genlist select smart callback    */
            ali);                                       /* genlist smart callback user data */

    /* */
    elm_object_item_del_cb_set(it, free_list_item_data);
    list_view_toggle_empty(p_sys, false);
    return ali;
}

static void
audio_list_playlists_view_delete(list_sys* p_list_sys)
{
    media_library_controller_destroy(p_list_sys->p_ctrl);
    elm_genlist_item_class_free(p_list_sys->p_default_item_class);
    free(p_list_sys);
}

bool
audio_list_playlists_back_callback(list_sys *p_sys)
{
    if (p_sys->current_popup != NULL)
    {
        evas_object_del(p_sys->current_popup);
        p_sys->current_popup = NULL;
        return true;
    }
    return false;
}

void
audio_list_playlists_longpress_remove_callback(void *data, Evas_Object *obj, void *event_info)
{
    list_sys *p_sys = data;
    application* p_app = intf_get_application(p_sys->p_intf);
    media_library* p_ml = (media_library*)application_get_media_library(p_app);
    list_view_item *ali = elm_object_item_data_get(p_sys->current_item);

    if (p_ml != NULL && ali != NULL)
        media_library_delete_playlist(p_ml, ali->p_playlist_item->i_id);

    media_library_controller_refresh(p_sys->p_ctrl);
    evas_object_del(p_sys->current_popup);
    p_sys->current_popup = NULL;
    p_sys->current_item = NULL;
}

static popup_menu audio_list_playlists_longpress_menu[] =
{
        {"Remove",  NULL,   audio_list_playlists_longpress_remove_callback},
        {0}
};

void audio_list_playlists_clear_popup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    list_sys *p_sys = data;
    p_sys->current_popup = NULL;
    p_sys->current_item = NULL;
}

static void
audio_list_playlists_longpress_callback(void *data, Evas_Object *obj, void *event_info)
{
    list_sys* p_sys = data;
    Elm_Object_Item *it = event_info;

    p_sys->current_item = it;

    Evas_Object *popup = p_sys->current_popup = popup_menu_orient_add(audio_list_playlists_longpress_menu, ELM_POPUP_ORIENT_CENTER, p_sys, p_sys->p_parent);
    evas_object_show(popup);
    evas_object_event_callback_add(popup, EVAS_CALLBACK_FREE, audio_list_playlists_clear_popup_cb, p_sys);
}

static list_view*
audio_list_playlists_view_create_private(interface* p_intf, Evas_Object* p_parent, list_view_create_option opts, bool list_items_on_selected )
{
    list_view* p_view = calloc(1, sizeof(*p_view));
    if (p_view == NULL)
        return NULL;
    list_sys* p_sys = p_view->p_sys = calloc(1, sizeof(*p_sys));
    if (p_sys == NULL)
        return NULL;

    if ( list_items_on_selected == true )
        p_sys->pf_selected = &audio_list_playlists_item_selected;
    else
        p_sys->pf_selected = NULL;
    /* Setup common parts */
    list_view_common_setup(p_view, p_sys, p_intf, p_parent, opts);

    /* Connect genlist callbacks */
    p_sys->p_default_item_class->func.text_get = genlist_text_get_cb;
    p_sys->p_default_item_class->func.content_get = genlist_content_get_cb;

    evas_object_size_hint_weight_set(p_sys->p_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p_sys->p_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    p_view->pf_append_item = &audio_list_playlists_view_append_item;
    p_view->pf_get_item = &audio_list_playlists_item_get_playlist_item;
    p_view->pf_set_item = &audio_list_playlists_item_set_playlist_item;
    p_view->pf_del = &audio_list_playlists_view_delete;
    p_view->pf_view_event_back = &audio_list_playlists_back_callback;

    application* p_app = intf_get_application( p_intf );
    p_sys->p_ctrl = playlist_controller_create(p_app, p_view);
    media_library_controller_refresh(p_view->p_sys->p_ctrl);

    evas_object_smart_callback_add(p_sys->p_list, "longpressed", audio_list_playlists_longpress_callback, p_sys);

    return p_view;
}

list_view*
audio_list_playlists_view_create(interface* p_intf, Evas_Object* p_parent, list_view_create_option opts)
{
    return audio_list_playlists_view_create_private( p_intf, p_parent, opts, true );
}

list_view*
audio_list_add_to_playlists_view_create(interface* p_intf, Evas_Object* p_parent, list_view_create_option opts)
{
    return audio_list_playlists_view_create_private( p_intf, p_parent, opts, false );
}
