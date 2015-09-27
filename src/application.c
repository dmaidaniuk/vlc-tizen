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

#include <common.h>

#include <app.h>
#include <storage.h>
#include <system_settings.h>

#include <Emotion.h>
#include <Elementary.h>                 /* Elm_language_set */

#include "ui/interface.h"

#include "media_storage.h"

static bool
app_create(void *data)
{
    application_sys *app = data;

    /* Fetching the media path and keep it in memory as soon as the app is create */
    app->media_path = fetching_media_path();
    dlog_print(DLOG_ERROR, LOG_TAG, "Media Path %s", app->media_path);

    /* */
    create_base_gui(app);

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
    application_sys *app = data;
    free(app);
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
    application_sys *app = malloc(sizeof(*app));

    /* Emotion start */
    emotion_init();

    ui_app_lifecycle_callback_s event_callback = {0,};
    app_event_handler_h handlers[5] = {NULL, };

    event_callback.create = app_create;
    event_callback.terminate = app_terminate;
    event_callback.pause = app_pause;
    event_callback.resume = app_resume;
    event_callback.app_control = app_control;

    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, app);
    ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, app);
    ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, app);
    ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, app);
    ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, app);
    ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

    int ret = ui_app_main(argc, argv, &event_callback, app);
    if (ret != APP_ERROR_NONE) {
        dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
    }

    return ret;
}
