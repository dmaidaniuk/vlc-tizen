/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
 *          Ludovic Fauvet <etix@videolan.org>
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
#include "playlists.h"

#include "interface.h"
#include "playback_service.h"
#include "media/playlist_item.h"
#include "ui/views/list_view.h"

struct playlists {
    interface *intf;
    Evas_Object *popup, *new_playlist_popup;
    Evas_Object *p_playlist_input;

    int64_t media_id;
};

bool
playlists_is_popup_open(playlists *pl)
{
    if (!pl)
        return false;
    return pl->popup || pl->new_playlist_popup;
}

void
playlists_popup_close(playlists *pl)
{
    if (!pl)
        return;

    evas_object_del(pl->popup);
    evas_object_del(pl->new_playlist_popup);
    pl->popup = NULL;
    pl->new_playlist_popup = NULL;
}

void
playlists_popup_destroy(playlists *pl)
{
    playlists_popup_close(pl);
    free(pl);
}

static void
on_new_playlist_background_touched(void *data, Evas_Object *obj, void *event_info)
{
    playlists* pl = (playlists*)data;
    playlists_popup_close(pl);
}

static void
on_create_playlist(void* data, Evas_Object* obj, void* event_info)
{
    playlists* pl = (playlists*)data;
    application* p_app = intf_get_application(pl->intf);
    media_library* p_ml = (media_library*)application_get_media_library(p_app);
    media_library_create_add_to_playlist(p_ml, elm_entry_entry_get(pl->p_playlist_input), pl->media_id);

    evas_object_del(pl->new_playlist_popup);
    pl->new_playlist_popup = NULL;
    playlists_popup_close(pl);
}

static void
on_playlist_selected(void* data, Evas_Object* obj, void* event_info)
{
    Elm_Object_Item* p_list_item = (Elm_Object_Item*)event_info;
    list_view_item* p_list_view_item = (list_view_item*)elm_object_item_data_get(p_list_item);
    playlists* pl = (playlists*)data;
    application* p_app = intf_get_application(pl->intf);
    media_library* p_ml = (media_library*)application_get_media_library(p_app);
    const playlist_item* p_playlist_item = (const playlist_item*)audio_list_playlists_item_get_playlist_item(p_list_view_item);
    media_library_add_to_playlist(p_ml, p_playlist_item->i_id, pl->media_id);
    playlists_popup_close(pl);
}

static void
playlists_new_playlist_popup(playlists* pl)
{
    /* */
    pl->new_playlist_popup = elm_popup_add(intf_get_main_naviframe(pl->intf));
    elm_popup_orient_set(pl->new_playlist_popup, ELM_POPUP_ORIENT_CENTER);

    /* */
    Evas_Object* layout = elm_layout_add(pl->new_playlist_popup);
    elm_layout_theme_set(layout, "layout", "application", "default");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(layout);

    /* Create the background */
    Evas_Object *bg = elm_bg_add(layout);
    elm_bg_color_set(bg, 255, 255, 255);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(bg);

    /* Set the background to the theme */
    elm_object_part_content_set(layout, "elm.swallow.bg", bg);

    Evas_Object *box = elm_box_add(layout);
    evas_object_show(box);

    /* */
    Evas_Object *new_playlist_input = pl->p_playlist_input = elm_entry_add(box);
    elm_entry_single_line_set(new_playlist_input, EINA_TRUE);
    elm_entry_scrollable_set(new_playlist_input, EINA_TRUE);
    elm_entry_input_panel_layout_set(new_playlist_input, ELM_INPUT_PANEL_LAYOUT_NORMAL);
    elm_entry_input_panel_return_key_type_set(new_playlist_input, ELM_INPUT_PANEL_RETURN_KEY_TYPE_GO);
    elm_entry_prediction_allow_set(new_playlist_input, EINA_FALSE);
    evas_object_size_hint_weight_set(new_playlist_input, 0.9, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(new_playlist_input, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(new_playlist_input, "guide", "New playlist name");
    evas_object_smart_callback_add(new_playlist_input, "activated", on_create_playlist, pl);
    elm_box_pack_end(box, new_playlist_input);
    evas_object_show(new_playlist_input);

    Evas_Object *button = elm_button_add(box);
    elm_object_text_set(button, "Create");
    evas_object_size_hint_weight_set(button, 0.1, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(button);
    evas_object_smart_callback_add(button, "clicked", on_create_playlist, pl);
    elm_box_pack_end(box, button);

    elm_object_part_content_set(layout, "elm.swallow.content", box);

    /* */
    elm_object_content_set(pl->new_playlist_popup, layout);
    evas_object_show(pl->new_playlist_popup);
    evas_object_smart_callback_add(pl->new_playlist_popup, "block,clicked",
            on_new_playlist_background_touched, pl);

    // Give focus to the input
    elm_object_focus_set(new_playlist_input, EINA_TRUE);
}

static void
audio_player_on_new_playlist_clicked(void* data, Evas_Object* obj, void* event_info)
{
    playlists* pl = (playlists*)data;
    playlists_popup_close(pl);
    playlists_new_playlist_popup(pl);
}

playlists*
playlists_popup_show(interface *intf, int64_t media_id)
{
    Evas_Object *layout;
    playlists *pl = calloc(1, sizeof(*pl));
    if (!pl)
        return NULL;

    pl->intf = intf;
    pl->media_id = media_id;

    /* */
    pl->popup = elm_popup_add(intf_get_main_naviframe(intf));
    elm_popup_orient_set(pl->popup, ELM_POPUP_ORIENT_CENTER);

    /* */
    layout = elm_layout_add(pl->popup);
    elm_layout_theme_set(layout, "layout", "application", "default");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Create the background */
    Evas_Object *bg = elm_bg_add(layout);
    elm_bg_color_set(bg, 255, 255, 255);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(bg);

    /* Set the background to the theme */
    elm_object_part_content_set(layout, "elm.swallow.bg", bg);

    Evas_Object* table = elm_table_add(layout);
    evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(table);

    /* */
    Evas_Object* label = elm_label_add(table);
    evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_text_set(label, "Add to playlist...");
    evas_object_show(label);
    elm_table_pack(table, label, 0, 0, 1, 1);

    Evas_Object *button = elm_button_add(table);
    elm_object_text_set(button, "New playlist");
    evas_object_size_hint_weight_set(button, 0.1, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(button);
    evas_object_smart_callback_add(button, "clicked", audio_player_on_new_playlist_clicked, pl);
    elm_table_pack(table, button, 0, 1, 1, 1);

    list_view* playlists = audio_list_add_to_playlists_view_create(intf, table, LIST_CREATE_ALL ^ LIST_CREATE_PLACEHOLDER);
    Evas_Object* playlist_widget = playlists->pf_get_widget(playlists->p_sys);
    Evas_Object* playlist_genlist = playlists->pf_get_list(playlists->p_sys);
    evas_object_size_hint_weight_set(playlist_widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(playlist_widget, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_scroller_content_min_limit(playlist_genlist, EINA_TRUE, EINA_TRUE );
    evas_object_size_hint_min_set(playlist_genlist, 500, 500);
    evas_object_smart_callback_add(playlist_genlist, "selected", &on_playlist_selected, pl);
    evas_object_show(playlist_widget);
    elm_table_pack(table, playlist_widget, 0, 2, 1, 1);
    elm_object_part_content_set(layout, "elm.swallow.content", table);
    evas_object_show(layout);

    /* */
    elm_object_content_set(pl->popup, layout);
    evas_object_show(pl->popup);
    return pl;
}
