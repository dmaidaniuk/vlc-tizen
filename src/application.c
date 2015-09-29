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

#include "application.h"
#include "ui/interface.h"

#include "media_storage.h"
#include "playback_service.h"
#include "media_library.hpp"

struct application {
    interface        *p_intf;         /* Main interface structure */
    media_storage    *p_ms;           /* Access to the device storage */
    /* settings */
    media_library    *p_mediaLibrary; /* Media Library */
    playback_service *p_ps;           /* Playback, using Emotion and libVLC */
};

static void app_terminate(void *data);

static bool
app_create(void *data)
{
    application *app = data;

    /* */
    app->p_ms = media_storage_create(app);
    if (!app->p_ms)
        goto error;

    /* */
    app->p_mediaLibrary = CreateMediaLibrary(app);
    if (!app->p_mediaLibrary)
        goto error;

    /* */
    app->p_intf = intf_create(app);
    if (!app->p_intf)
        goto error;

    /* */
    app->p_ps = playback_service_create(app);
    if (!app->p_ps)
        goto error;

    return true;
error:
    app_terminate(app);
    return false;
}

static void
app_control(app_control_h app_control, void *data)
{
    /* Handle the launch request. */
    //    application *app = data;
}

static void
app_pause(void *data)
{
    /* Take necessary actions when application becomes invisible. */
    //    application *app = data;
}

static void
app_resume(void *data)
{
    /* Take necessary actions when application becomes visible. */
    //    application *app = data;
}

static void
app_terminate(void *data)
{
    application *app = data;
    if (app->p_ms)
        media_storage_destroy(app->p_ms);
    if (app->p_mediaLibrary)
        DeleteMediaLibrary(app->p_mediaLibrary);
    if (app->p_intf)
        intf_destroy(app->p_intf);
    if (app->p_ps)
        playback_service_destroy(app->p_ps);
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

const char *
application_get_media_path(application *app, media_directory_e type)
{
    return media_storage_get_path(app->p_ms, type);
}

const media_storage *
application_get_media_storage(application *app)
{
    return app->p_ms;
}

const media_library *
application_get_media_library(application *app)
{
    return app->p_mediaLibrary;
}

const playback_service *
application_get_playback_service(application *app)
{
    return app->p_ps;
}

int
main(int argc, char *argv[])
{
    application *app = calloc(1, sizeof(*app));

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
        LOGE("app_main() is failed. err = %d", ret);
    }

    return ret;
}
