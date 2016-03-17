/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Ludovic Fauvet <etix@videolan.org>
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
#include "stream_view.h"

#include "ui/interface.h"
#include "ui/utils.h"
#include "ui/menu/popup_menu.h"

#include <sqlite3.h>

struct view_sys
{
    interface           *p_intf;
    Elm_Object_Item     *current_item;

    /* Widgets */
    Evas_Object *layout;
    Evas_Object *uri;
    Evas_Object *history;
    Evas_Object *current_popup;

    /* Database */
    sqlite3 *db;
};

typedef struct {
    char *uri;
    view_sys *p_sys;
} item_data;

void
stream_view_load_history(view_sys *p_sys);

static void
stream_view_play(view_sys *p_sys, const char *psz_path)
{
    int rc;
    sqlite3_stmt *stmt;

    rc = sqlite3_prepare_v2(p_sys->db, "REPLACE INTO stream_history (URI) VALUES (?);", -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        LOGE("Unable to prepare statement");
        goto play;
    }

    rc = sqlite3_bind_text(stmt, 1, psz_path, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        LOGE("Unable to bind text");
        goto play;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ERROR)
    {
        LOGE("Unable to execute statement: %s", sqlite3_errmsg(p_sys->db));
        goto play;
    }

play:
    intf_video_player_play(p_sys->p_intf, psz_path, 0);

    if (rc == SQLITE_DONE || rc == SQLITE_OK)
    {
        // Reload the list for reordering
        stream_view_load_history(p_sys);
    }
}

static void
stream_view_uri_activated(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;

    const char *psz_path = elm_entry_entry_get(p_sys->uri);

    if (strlen(psz_path) == 0)
        return;

    stream_view_play(p_sys, psz_path);
    elm_entry_entry_set(p_sys->uri, "");
}

static bool
stream_view_has_menu(view_sys *p_sys)
{
    return false;
}

void
clicked_reuse(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;
    item_data *item_data = elm_object_item_data_get(p_sys->current_item);

    elm_entry_entry_set(p_sys->uri, item_data->uri);
    evas_object_del(p_sys->current_popup);
    p_sys->current_popup = NULL;
}

void
clicked_remove(void *data, Evas_Object *obj, void *event_info)
{
    int rc;
    sqlite3_stmt *stmt;
    view_sys *p_sys = data;
    item_data *item_data = elm_object_item_data_get(p_sys->current_item);

    rc = sqlite3_prepare_v2(p_sys->db, "DELETE FROM stream_history WHERE URI=?;", -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        LOGE("Unable to prepare statement");
        return;
    }

    rc = sqlite3_bind_text(stmt, 1, item_data->uri, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK)
    {
        LOGE("Unable to bind text");
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ERROR)
    {
        LOGE("Unable to execute statement: %s", sqlite3_errmsg(p_sys->db));
        return;
    }

    elm_object_item_del(p_sys->current_item);
    evas_object_del(p_sys->current_popup);
    p_sys->current_popup = NULL;
}

static popup_menu longpress_menu[] =
{
        {"Reuse",   NULL,   clicked_reuse},
        {"Remove",  NULL,   clicked_remove},
        {0}
};

void stream_view_clear_popup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;
    p_sys->current_item = NULL;
    p_sys->current_popup = NULL;
}

static void
stream_view_history_longpress_callback(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;
    Elm_Object_Item *it = event_info;

    p_sys->current_item = it;

    Evas_Object *popup = p_sys->current_popup = popup_menu_orient_add(longpress_menu, ELM_POPUP_ORIENT_CENTER, p_sys, p_sys->layout);
    evas_object_show(popup);
    evas_object_event_callback_add(popup, EVAS_CALLBACK_FREE, stream_view_clear_popup_cb, p_sys);
}

static void
stream_view_item_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    item_data *item_data = data;
    view_sys *p_sys = item_data->p_sys;

    if (p_sys->current_popup != NULL)
    {
        // A popup is open, discard event.
        return;
    }

    stream_view_play(p_sys, item_data->uri);
}

void
stream_view_item_del_cb(void *data, Evas_Object *obj, void *event_info)
{
    item_data *item_data = data;
    free(item_data->uri);
    free(item_data);
}

int
stream_view_history_result_cb(
        void* data,    /* Data provided in the 4th argument of sqlite3_exec() */
        int argc,      /* The number of columns in row */
        char** argv,   /* An array of strings representing fields in the row */
        char** colname    /* An array of strings representing column names */
        )
{
    view_sys *p_sys = data;

    if (argc <= 0)
        return 1;

    item_data *item_data = malloc(sizeof(*item_data));
    item_data->p_sys = p_sys;
    item_data->uri = strdup(argv[0]);

    Elm_Object_Item *it = elm_list_item_append(p_sys->history, item_data->uri, NULL, NULL, stream_view_item_clicked_cb, item_data);

    // Add a callback to free the item when deleted
    elm_object_item_del_cb_set(it, stream_view_item_del_cb);

    return 0;
}

void
stream_view_load_history(view_sys *p_sys)
{
    int rc;
    char *error;

    elm_list_clear(p_sys->history);
    p_sys->current_item = NULL;

    const char *req = "SELECT URI FROM stream_history ORDER BY datetime(Timestamp) DESC";

    rc = sqlite3_exec(p_sys->db, req, stream_view_history_result_cb, p_sys, &error);
    if( rc != SQLITE_OK )
    {
        LOGE("SQL Error: %s", error);
        sqlite3_free(error);
    }

    elm_list_go(p_sys->history);
}

void
stream_view_prepare_database(view_sys *p_sys)
{
    int rc;
    char *error;

    char *appdata = system_storage_appdata_get();
    char *dbpath;
    asprintf(&dbpath, "%s/streams.db", appdata);
    rc = sqlite3_open(dbpath, &p_sys->db);
    free(appdata);
    free(dbpath);

    if (rc != SQLITE_OK)
    {
        LOGE("Unable to open the database");
        return;
    }

    const char* req = "CREATE TABLE IF NOT EXISTS stream_history (" \
            "URI PRIMARY KEY NOT NULL," \
            "Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP" \
            ");";

    rc = sqlite3_exec(p_sys->db, req, NULL, NULL, &error);
    if (rc != SQLITE_OK)
    {
        LOGE("SQL Error: %s", error);
        sqlite3_free(error);
    }
}

static bool
stream_view_callback(view_sys *p_view_sys, interface_view_event event)
{
    switch (event) {
    case INTERFACE_VIEW_EVENT_BACK:
            if (p_view_sys->current_popup) {
                evas_object_del(p_view_sys->current_popup);
                p_view_sys->current_popup = NULL;
                return true;
            }
        return false;
    default:
        break;
    }
    return true;
}

interface_view *
create_stream_view(interface *intf, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));

    /* Setup the audio_view */
    view_sys *stream_view_sys = calloc(1, sizeof(*stream_view_sys));
    stream_view_sys->p_intf = intf;

    //view->pf_event = stream_view_callback;
    view->p_view_sys = stream_view_sys;
    view->pf_has_menu = stream_view_has_menu;
    view->pf_event = stream_view_callback;

    /* Open/prepare database */
    stream_view_prepare_database(stream_view_sys);

    /* Create layout and set the theme */
    Evas_Object *layout = stream_view_sys->layout = elm_layout_add(parent);
    elm_layout_theme_set(layout, "layout", "application", "default");

    /* Create the background */
    Evas_Object *bg = elm_bg_add(layout);
    elm_bg_color_set(bg, 255, 255, 255);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(bg);

    /* Set the background to the theme */
    elm_object_part_content_set(layout, "elm.swallow.bg", bg);

    /* Content table */
    Evas_Object *table = elm_table_add(layout);
    evas_object_size_hint_weight_set(table, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(table, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(table);

    /* Set the content to the theme */
    elm_object_part_content_set(layout, "elm.swallow.content", table);

    /* */
    Evas_Object *entry = stream_view_sys->uri = elm_entry_add(table);
    elm_entry_single_line_set(entry, EINA_TRUE);
    elm_entry_scrollable_set(entry, EINA_TRUE);
    elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_URL);
    elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_GO);
    elm_entry_prediction_allow_set(entry, EINA_FALSE);
    evas_object_size_hint_weight_set(entry, 0.9, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_object_part_text_set(entry, "guide", "e.g. http://, mms:// or rtsp://");
    evas_object_show(entry);

    Evas_Textblock_Style *style = evas_textblock_style_new();
    evas_textblock_style_set(style, "color=#ff0000");

    evas_object_textblock_style_set(entry, style);

    elm_table_pack(table, entry, 0, 0, 1, 1);

    /* */
    Evas_Object *button = elm_button_add(table);
    elm_object_text_set(button, "Go");
    evas_object_size_hint_weight_set(button, 0.1, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(button);

    evas_object_smart_callback_add(button, "clicked", stream_view_uri_activated, stream_view_sys);

    elm_table_pack(table, button, 1, 0, 1, 1);

    /* */
    Evas_Object *history = stream_view_sys->history = elm_list_add(table);
    elm_scroller_policy_set(history, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_ON);
    elm_scroller_bounce_set(history, EINA_TRUE, EINA_TRUE);

    stream_view_load_history(stream_view_sys);

    evas_object_smart_callback_add(history, "longpressed", stream_view_history_longpress_callback, stream_view_sys);
    evas_object_size_hint_weight_set(history, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(history, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(history);

    elm_table_pack(table, history, 0, 1, 2, 1);

    /*  */
    evas_object_show(layout);
    view->view = layout;

    return view;
}

void
destroy_stream_view(interface_view *view)
{
    view_sys* p_sys = view->p_view_sys;
    sqlite3_close(p_sys->db);
    evas_object_del(view->view);
    free(p_sys);
    free(view);
}

