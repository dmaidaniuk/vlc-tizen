/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
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
#include "system_storage.h"

#include "ui/utils.h"
#include "ui/views/audio_view.h"
#include "ui/interface.h"
#include "ui/views/list_view.h"
#include "ui/menu/popup_menu.h"

#include <Elementary.h>

struct view_sys
{
    interface*              p_intf;
    Evas_Object*            nf_toolbar;
    Evas_Object*            p_tabbar;
    Evas_Object*            p_overflow_menu;
    audio_view_type         i_current_tab;
    bool                    b_tab_noreload;
};

typedef struct toolbar_tab {
    audio_view_type type;
    view_sys *p_view_sys;
} toolbar_tab;

static list_view*
create_audio_list_type(view_sys *p_view_sys, audio_view_type type )
{
    list_view* p_view;

    switch (type)
    {
    case AUDIO_VIEW_SONG:
    default:
        type = AUDIO_VIEW_SONG;
        p_view = audio_list_song_view_all_create(p_view_sys->p_intf, p_view_sys->nf_toolbar, LIST_CREATE_ALL);
        break;
    case AUDIO_VIEW_ARTIST:
        p_view = audio_list_artist_view_create(p_view_sys->p_intf, p_view_sys->nf_toolbar, LIST_CREATE_ALL);
        break;
    case AUDIO_VIEW_ALBUM:
        p_view = audio_list_album_view_create(p_view_sys->p_intf, p_view_sys->nf_toolbar, 0, LIST_CREATE_ALL);
        break;
    case AUDIO_VIEW_GENRE:
        p_view = audio_list_genres_view_create(p_view_sys->p_intf, p_view_sys->nf_toolbar, LIST_CREATE_ALL);
        break;
    case AUDIO_VIEW_PLAYLIST:
        p_view = audio_list_playlists_view_create(p_view_sys->p_intf, p_view_sys->nf_toolbar, LIST_CREATE_ALL);
        break;
    }

    /* Set the view's tab type so we can restore it on back press (since it's not part of the naviframe) */
    p_view->type = type;

    // Purge the audio view naviframe when switching tabs
    naviframe_clear(p_view_sys->nf_toolbar);

    Evas_Object* p_list = p_view->pf_get_widget(p_view->p_sys);
    Elm_Object_Item *it = elm_naviframe_item_push(p_view_sys->nf_toolbar, "", NULL, NULL, p_list, NULL);
    elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);
    elm_object_item_data_set(it, p_view);
    evas_object_show(p_list);

    return p_view;
}

static void
tabbar_item_cb(void *data, Evas_Object *obj, void *event_info)
{
    toolbar_tab *item = data;
    if (item->p_view_sys->i_current_tab == item->type || item->p_view_sys->b_tab_noreload)
        return;

    item->p_view_sys->i_current_tab = item->type;

    create_audio_list_type(item->p_view_sys, item->type);
}

static void
tabbar_item_del(void *data, Evas_Object *obj, void *event_info)
{
    toolbar_tab *item = data;
    free(item);
}

static Elm_Object_Item*
toolbar_item_append(Evas_Object *obj, audio_view_type type, const char *label, Evas_Smart_Cb func, void *data)
{
    Elm_Object_Item* it;
    toolbar_tab *it_data = malloc(sizeof(*it_data));
    it_data->type = type;
    it_data->p_view_sys = (view_sys *)data;

    it = elm_toolbar_item_append(obj, NULL, label, func, it_data);
    elm_object_item_del_cb_set(it, tabbar_item_del);

    return it;
}

static Evas_Object*
create_toolbar(view_sys *p_view_sys, Evas_Object *parent)
{
    /* Create and set the toolbar */
    Evas_Object *tabbar = p_view_sys->p_tabbar = elm_toolbar_add(parent);

    /* Set the toolbar shrink mode */
    elm_toolbar_shrink_mode_set(tabbar, ELM_TOOLBAR_SHRINK_EXPAND);

    elm_toolbar_homogeneous_set(tabbar, EINA_FALSE);
    elm_toolbar_transverse_expanded_set(tabbar, EINA_FALSE);

    evas_object_size_hint_weight_set(tabbar, EVAS_HINT_FILL, 0.0);
    evas_object_size_hint_align_set(tabbar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Append new entry in the toolbar with the Icon & Label wanted */
    Elm_Object_Item *it = toolbar_item_append(tabbar, AUDIO_VIEW_ARTIST,  "Artists",  tabbar_item_cb, p_view_sys);
    toolbar_item_append(tabbar, AUDIO_VIEW_ALBUM,   "Albums",   tabbar_item_cb, p_view_sys);
    toolbar_item_append(tabbar, AUDIO_VIEW_SONG,    "Songs",    tabbar_item_cb, p_view_sys);
    toolbar_item_append(tabbar, AUDIO_VIEW_GENRE,   "Genre",    tabbar_item_cb, p_view_sys);
    toolbar_item_append(tabbar, AUDIO_VIEW_PLAYLIST,"Playlist", tabbar_item_cb, p_view_sys);

    // Select the first tab.
    elm_toolbar_item_selected_set(it, EINA_TRUE);

    return tabbar;
}

static void
audio_view_refresh_cb(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;
    if (!p_sys)
        return;

    application* p_app = intf_get_application(p_sys->p_intf);
    media_library* p_ml = (media_library*)application_get_media_library(p_app);
    if (p_ml != NULL)
        media_library_reload(p_ml);

    /* */
    evas_object_del(obj);
    p_sys->p_overflow_menu = NULL;
}

static popup_menu audio_view_popup_menu[] =
{
        {"Refresh", NULL, audio_view_refresh_cb},
        {0}
};

static void
audio_view_popup_close_cb(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_view_sys = data;
    evas_object_del(p_view_sys->p_overflow_menu);
    p_view_sys->p_overflow_menu = NULL;
}

static bool
audio_view_callback(view_sys *p_view_sys, interface_view_event event)
{
    switch (event) {
    case INTERFACE_VIEW_EVENT_MENU:
    {
        if (p_view_sys->p_overflow_menu)
        {
            evas_object_del(p_view_sys->p_overflow_menu);
            p_view_sys->p_overflow_menu = NULL;
        }
        else
        {
            p_view_sys->p_overflow_menu = popup_menu_add(audio_view_popup_menu, p_view_sys, p_view_sys->nf_toolbar);
            evas_object_smart_callback_add(p_view_sys->p_overflow_menu, "block,clicked", audio_view_popup_close_cb, p_view_sys);
            evas_object_show(p_view_sys->p_overflow_menu);
        }
        return true;
    }
    case INTERFACE_VIEW_EVENT_BACK:
    {
        /* Hide overflow menu */
        if (p_view_sys->p_overflow_menu) {
            evas_object_del(p_view_sys->p_overflow_menu);
            p_view_sys->p_overflow_menu = NULL;
            return true;
        }

        /* Handle back button for sub views */
        Elm_Object_Item *it = elm_naviframe_top_item_get(p_view_sys->nf_toolbar);
        list_view* p_view = (list_view *)elm_object_item_data_get(it);
        if (p_view && p_view->pf_view_event_back)
            if (p_view->pf_view_event_back(p_view->p_sys) == true)
                return true;

        /* Pop view */
        if (naviframe_count(p_view_sys->nf_toolbar) > 1)
        {
            elm_naviframe_item_pop(p_view_sys->nf_toolbar);

            /* Restore the selected tab */
            Elm_Object_Item *it = elm_naviframe_top_item_get(p_view_sys->nf_toolbar);
            list_view* p_view = (list_view *)elm_object_item_data_get(it);

            Elm_Object_Item* tab = elm_toolbar_first_item_get(p_view_sys->p_tabbar);
            do
            {
                toolbar_tab *it_data = (toolbar_tab *)elm_object_item_data_get(tab);
                if (it_data->type == p_view->type)
                {
                    // Don't trigger a view reload while we manually switch tab
                    p_view_sys->b_tab_noreload = true;
                    elm_toolbar_item_selected_set(tab, EINA_TRUE);
                    p_view_sys->b_tab_noreload = false;
                    break;
                }
            }
            while ((tab = elm_toolbar_item_next_get(tab)) != NULL);

            return true;
        }
        return false;
    }
    default:
        break;
    }

    return false;
}

static bool
audio_view_has_menu(view_sys *p_view_sys)
{
    return true;
}

interface_view *
create_audio_view(interface *intf, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));

    /* Setup the audio_view */
    view_sys *audio_view_sys = calloc(1, sizeof(*audio_view_sys));
    audio_view_sys->p_intf = intf;
    audio_view_sys->i_current_tab = AUDIO_VIEW_NONE;

    view->pf_event = audio_view_callback;
    view->p_view_sys = audio_view_sys;
    view->pf_has_menu = audio_view_has_menu;

    /* Content box */
    Evas_Object *audio_box = elm_box_add(parent);
    evas_object_size_hint_weight_set(audio_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(audio_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Toolbar Naviframe */
    audio_view_sys->nf_toolbar = elm_naviframe_add(audio_box);
    evas_object_size_hint_weight_set(audio_view_sys->nf_toolbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(audio_view_sys->nf_toolbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(audio_view_sys->nf_toolbar);

    /* Create the toolbar in the view */
    Evas_Object *tabbar = create_toolbar(audio_view_sys, audio_box);
    elm_toolbar_select_mode_set(tabbar, ELM_OBJECT_SELECT_MODE_ALWAYS);
    evas_object_show(tabbar);

    /*
     * Items will only call their selection func and callback when first becoming selected
     * Doing it before creating the naviframe would cause the first tab selection to fail
     * since the selection callback requires a non NULL nf_toolbar.
     */
    elm_box_pack_end(audio_box, tabbar);
    elm_box_pack_end(audio_box, audio_view_sys->nf_toolbar );

    /*  */
    evas_object_show(audio_box);
    view->view = audio_box;

    return view;
}

void
destroy_audio_view(interface_view *view)
{
    view_sys* p_sys = view->p_view_sys;
    evas_object_del(view->view);
    free(p_sys);
    free(view);
}
