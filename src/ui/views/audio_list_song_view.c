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

#include "list_view.h"
#include "audio_view.h"
#include "controller/media_controller.h"
#include "list_view_private.h"
#include "ui/interface.h"
#include "ui/utils.h"

struct list_sys
{
    LIST_VIEW_COMMON
    unsigned int i_artist_id;
    unsigned int i_album_id;
};

struct list_view_item
{
    const list_sys*                 p_list;
    const Elm_Genlist_Item_Class*   itc;
    media_item*                     p_media_item;
    Elm_Object_Item*                p_object_item;
};

static void
free_list_item_data(void *data, Evas_Object *obj, void *event_info)
{
    list_view_item *ali = data;
    media_item_destroy(ali->p_media_item);
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
            if (ali->p_media_item->i_track_number > 0 && ali->p_list->i_album_id != 0)
                asprintf(&buf, "%d - <b>%s</b>", ali->p_media_item->i_track_number, media_item_title(ali->p_media_item));
            else
                asprintf(&buf, "<b>%s</b>", media_item_title(ali->p_media_item));
            return buf;
        }
        else if (!strcmp(part, "elm.text.sub.left.bottom")) {
            const char* psz_artist = media_item_artist(ali->p_media_item);
            if (psz_artist == NULL)
                psz_artist = "Unknown Artist";
            return strdup(psz_artist);
        }
        else if (!strcmp(part, "elm.text.sub.right.bottom")) {
            return media_timetostr( ali->p_media_item->i_duration / 1000 );
        }
    }
    return NULL;
}

static const void*
audio_list_song_item_get_media_item(list_view_item* p_item)
{
    return p_item->p_media_item;
}

static void
audio_list_song_item_set_media_item(list_view_item* p_item, void* p_data)
{
    media_item *p_media_item = (media_item*)p_data;
    p_item->p_media_item = p_media_item;
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
            if (ali->p_media_item->psz_snapshot != NULL)
                icon = create_image(layout, ali->p_media_item->psz_snapshot);
            else
                icon = create_icon(layout, "background_cone.png");
            elm_layout_content_set(layout, "elm.swallow.content", icon);
        }
    }

    return layout;
}

static void
genlist_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    list_view_item *ali = data;
    int items = elm_genlist_items_count(ali->p_list->p_list);
    Eina_Array *array = eina_array_new(items);
    Elm_Object_Item *it;
    int index = 0, pos = 0;

    it = elm_genlist_first_item_get(ali->p_list->p_list);

    do {
        list_view_item *lvi = elm_object_item_data_get(it);
        if (lvi == NULL)
            continue;

        if (media_item_identical(lvi->p_media_item, ali->p_media_item))
            pos = index;

        eina_array_push(array, media_item_copy(lvi->p_media_item));
        index++;
    } while ((it = elm_genlist_item_next_get(it)) != NULL);

    intf_start_audio_player(ali->p_list->p_intf, array, pos);
}


static list_view_item*
audio_list_song_view_append_item(list_sys *p_sys, void* p_data)
{
    media_item* p_media_item = (media_item*)p_data;
    list_view_item *ali = calloc(1, sizeof(*ali));
    ali->p_list = p_sys;
    ali->itc = p_sys->p_default_item_class;

    ali->p_media_item = p_media_item;

    /* Set and append new item in the genlist */
    Elm_Object_Item *it = elm_genlist_item_append(p_sys->p_list,
            p_sys->p_default_item_class,                /* genlist item class               */
            ali,                                        /* genlist item class user data     */
            NULL,                                       /* genlist parent item              */
            ELM_GENLIST_ITEM_NONE,                      /* genlist item type                */
            genlist_selected_cb,                        /* genlist select smart callback    */
            ali);                                       /* genlist smart callback user data */

    /* */
    elm_object_item_del_cb_set(it, free_list_item_data);
    list_view_toggle_empty(p_sys, false);
    return ali;
}

static void
audio_list_song_view_delete(list_sys* p_list_sys)
{
    media_library_controller_destroy(p_list_sys->p_ctrl);
    elm_genlist_item_class_free(p_list_sys->p_default_item_class);
    free(p_list_sys);
}

static void
audio_list_song_get_artist_songs_cb(media_library* p_ml, media_library_list_cb cb, void* p_user_data)
{
    list_sys* p_list_sys = (list_sys*)p_user_data;
    media_library_get_artist_songs(p_ml, p_list_sys->i_artist_id, cb, p_list_sys->p_ctrl);
}

static void
audio_list_song_get_album_songs_cb(media_library* p_ml, media_library_list_cb cb, void* p_user_data)
{
    list_sys* p_list_sys = (list_sys*)p_user_data;
    media_library_get_album_songs(p_ml, p_list_sys->i_album_id, cb, p_list_sys->p_ctrl);
}

static list_view*
audio_list_song_view_create(interface* p_intf, Evas_Object* p_parent, list_view_create_option opts)
{
    list_view* p_view = calloc(1, sizeof(*p_view));
    if (p_view == NULL)
        return NULL;
    list_sys* p_sys = p_view->p_sys = calloc(1, sizeof(*p_sys));
    if (p_sys == NULL)
        return NULL;

    /* Setup common parts */
    list_view_common_setup(p_view, p_sys, p_intf, p_parent, opts);

    /* Connect genlist callbacks */
    p_sys->p_default_item_class->func.text_get = genlist_text_get_cb;
    p_sys->p_default_item_class->func.content_get = genlist_content_get_cb;

    evas_object_size_hint_weight_set(p_sys->p_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p_sys->p_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    p_view->pf_append_item = &audio_list_song_view_append_item;
    p_view->pf_get_item = &audio_list_song_item_get_media_item;
    p_view->pf_set_item = &audio_list_song_item_set_media_item;
    p_view->pf_del = &audio_list_song_view_delete;

    application* p_app = intf_get_application( p_intf );
    p_sys->p_ctrl = audio_controller_create(p_app, p_view);

    return p_view;
}

list_view*
audio_list_song_view_all_create(interface* p_intf, Evas_Object* p_parent, list_view_create_option opts )
{
    list_view* p_view = audio_list_song_view_create(p_intf, p_parent, opts);
    // Default audio controller is listing all songs. No more config is required.
    media_library_controller_refresh(p_view->p_sys->p_ctrl);
    return p_view;
}

list_view*
audio_list_song_view_artist_create(interface* p_intf, Evas_Object* p_parent, unsigned int i_artist_id, list_view_create_option opts )
{
    list_view* p_view = audio_list_song_view_create(p_intf, p_parent, opts);
    p_view->p_sys->i_artist_id = i_artist_id;
    media_library_controller_set_content_callback(p_view->p_sys->p_ctrl, audio_list_song_get_artist_songs_cb, p_view->p_sys);
    media_library_controller_refresh(p_view->p_sys->p_ctrl);
    return p_view;
}

list_view*
audio_list_song_view_album_create(interface* p_intf, Evas_Object* p_parent, unsigned int i_album_id, list_view_create_option opts )
{
    list_view* p_view = audio_list_song_view_create(p_intf, p_parent, opts);
    p_view->p_sys->i_album_id = i_album_id;
    media_library_controller_set_content_callback(p_view->p_sys->p_ctrl, audio_list_song_get_album_songs_cb, p_view->p_sys);
    media_library_controller_refresh(p_view->p_sys->p_ctrl);
    return p_view;
}

