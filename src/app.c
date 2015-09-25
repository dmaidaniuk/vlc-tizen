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

#include "interface.h"

#include <app.h>
#include <storage.h>
#include <system_settings.h>

#include <Emotion.h>
#include <Elementary.h>                 /* Elm_language_set */

static int internal_storage_id;
static bool storage_cb(int storage_id, storage_type_e type, storage_state_e state, const char *path, void *user_data)
{
    if (type == STORAGE_TYPE_INTERNAL)
    {
        internal_storage_id = storage_id;
        dlog_print(DLOG_DEBUG, LOG_TAG, "Storage refreshed");
        return false;
    }

    return true;
}

void
fetching_media_path(gui_data_s *gd)
{
    int error;
    char *buff;
    /* Connect to the device storage */
    error = storage_foreach_device_supported(storage_cb, NULL);
    error = storage_get_directory(internal_storage_id, STORAGE_DIRECTORY_VIDEOS, &gd->media_path);
    gd->rmp = gd->media_path ;
    /* Concatenate the media path with .. to acces the general media directory */
    asprintf(&gd->rmp,"%s/%s", gd->rmp, "..");
    /* Then do a realpath to get a "clean" path */
    buff = realpath(gd->rmp, NULL);
    gd->rmp = buff;

    if (error == STORAGE_ERROR_NONE)
    {
        dlog_print(DLOG_DEBUG, LOG_TAG, "Connected to storage");

    }
}

static bool
app_create(void *data)
{
    gui_data_s *gd = data;

    /* Fetching the media path and keep it in memory as soon as the app is create */
    fetching_media_path(gd);
    /* */
    create_base_gui(gd);

    return true;
}

static void
app_control(app_control_h app_control, void *data)
{
    /* Handle the launch request. */
}

static void
app_pause(void *data)
{
    /* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
    /* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
    gui_data_s *gd = data;
    free(gd->mini_player);
    emotion_shutdown();
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
    /*APP_EVENT_LANGUAGE_CHANGED*/
    char *locale = NULL;
    system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
    elm_language_set(locale);
    free(locale);
    return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
    /*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
    return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
    /*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
    /*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
    /*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
    gui_data_s gd = {0};
    int ret = 0;

    emotion_init();

    ui_app_lifecycle_callback_s event_callback = {0,};
    app_event_handler_h handlers[5] = {NULL, };

    event_callback.create = app_create;
    event_callback.terminate = app_terminate;
    event_callback.pause = app_pause;
    event_callback.resume = app_resume;
    event_callback.app_control = app_control;

    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &gd);
    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &gd);
    ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &gd);
    ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &gd);
    ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &gd);
    ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

    ret = ui_app_main(argc, argv, &event_callback, &gd);
    if (ret != APP_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
    }

    return ret;
}
