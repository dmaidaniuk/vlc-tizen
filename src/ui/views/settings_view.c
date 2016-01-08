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

#include "settings_view.h"
#include "preferences/preferences.h"
#include "ui/settings/settings.h"
#include "ui/interface.h"
#include "ui/utils.h"
#include "playback_service.h"

#include <app_preference.h>

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

void
menu_directories_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_hwacceleration_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_subsenc_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_vorientation_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_performance_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_deblocking_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_developer_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);

struct view_sys {
    interface* p_intf;
    Evas_Object *nav;
    Evas_Object *popup;
};

settings_item settings_menu[] =
{
        {0,                             "General",                      NULL,                               SETTINGS_TYPE_CATEGORY},
        {SETTINGS_ID_DIRECTORIES,       "Directories",                  "ic_menu_folder.png",               SETTINGS_TYPE_ITEM,         menu_directories_selected_cb},
        {SETTINGS_ID_HWACCELERATION,    "Hardware acceleration",        "ic_menu_preferences.png",          SETTINGS_TYPE_ITEM,         menu_hwacceleration_selected_cb},
        {SETTINGS_ID_SUBSENC,           "Subtitles text encoding",      "ic_browser_subtitle_normal.png",   SETTINGS_TYPE_ITEM,         menu_subsenc_selected_cb},
        {SETTINGS_ID_VORIENTATION,      "Video orientation",            "ic_menu_video.png",                SETTINGS_TYPE_ITEM,         menu_vorientation_selected_cb},
        {0,                             "Extra settings",               NULL,                               SETTINGS_TYPE_CATEGORY},
        {SETTINGS_ID_PERFORMANCES,      "Performances",                 "ic_menu_preferences.png",          SETTINGS_TYPE_ITEM,         menu_performance_selected_cb},
        {SETTINGS_ID_DEBLOCKING,        "Deblocking filter settings",   "ic_menu_preferences.png",          SETTINGS_TYPE_ITEM,         menu_deblocking_selected_cb},
        {SETTINGS_ID_DEVELOPER,         "Developer",                    "ic_menu_preferences.png",          SETTINGS_TYPE_ITEM,         menu_developer_selected_cb}
};

/* Create the setting submenu items */

settings_item directory_menu[] =
{
        {DIRECTORIES_INTERNAL,  "Internal memory",              NULL,   SETTINGS_TYPE_TOGGLE},
        {DIRECTORIES_EXTERNAL,  "External memory (SD card)",    NULL,   SETTINGS_TYPE_TOGGLE},
        //{DIRECTORIES_ADDLOCATION,   "Add location",    "call_button_add_call_press.png",   SETTINGS_TYPE_ITEM}
};

settings_item hardware_acceleration_menu[] =
{
        {HWACCELERATION_AUTOMATIC,  "Automatic",                NULL, SETTINGS_TYPE_TOGGLE},
        {HWACCELERATION_DISABLED,   "Disabled",                 NULL, SETTINGS_TYPE_TOGGLE},
        {HWACCELERATION_DECODING,   "Decoding acceleration",    NULL, SETTINGS_TYPE_TOGGLE},
        {HWACCELERATION_FULL,       "Full acceleration",        NULL, SETTINGS_TYPE_TOGGLE}
};

settings_item video_orientation_menu[] =
{
        {ORIENTATION_AUTOMATIC,     "Automatic (sensor)",   NULL, SETTINGS_TYPE_TOGGLE},
        {ORIENTATION_LOCKED,        "Locked at start",      NULL, SETTINGS_TYPE_TOGGLE},
        {ORIENTATION_LANDSCAPE,     "Landscape",            NULL, SETTINGS_TYPE_TOGGLE},
        {ORIENTATION_PORTRAIT,      "Portrait",             NULL, SETTINGS_TYPE_TOGGLE},
        {ORIENTATION_R_LANDSCAPE,   "Reverse landscape",    NULL, SETTINGS_TYPE_TOGGLE},
        {ORIENTATION_R_PORTRAIT,    "Reverse portrait",     NULL, SETTINGS_TYPE_TOGGLE}
};

settings_item subtitles_text_encoding_menu[] =
{
        {42, "Default (Windows-1252)",                  NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Universal (UTF-8)",                       NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Universal (UTF-16)",                      NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Universal (big endian UTF-16)",           NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Universal (little endian UTF-16)",        NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Universal, Chinese (GB18030)",            NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Western European (Latin-9)",              NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Western European (Windows-1252)",         NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Western European (IBM 00850)",            NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Eastern European (Latin-2)",              NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Eastern European (Windows-1250)",         NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Esperanto (Latin-3)",                     NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Nordic (Latin-6)",                        NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Cyrillic (Windows-1251)",                 NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Russian (KOI8-R)",                        NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Ukrainian (KOI8-U)",                      NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Arabic (ISO 8859-6)",                     NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Arabic (Windows-1256)",                   NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Greek (ISO 8859-7)",                      NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Greek (Windows-1253)",                    NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Hebrew (ISO 8859-8)",                     NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Hebrew (Windows-1255)",                   NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Turkish (ISO 8859-9)",                    NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Turkish (Windows-1254)",                  NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Thai (TIS 620-2533/ISO 8859-11)",         NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Thai (Windows-874)",                      NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Baltic (Latin-7)",                        NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Baltic (Windows-1257)",                   NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Celtic (Latin-8)",                        NULL, SETTINGS_TYPE_TOGGLE},
        {42, "South-Estearn (Latin-10)",                NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Simplified Chinese (ISO-2022-CN-EXT)",    NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Simplified Chinese Unix (EUC-CN)",        NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Japanese (7-bits JIS/ISO-2022-JP-2)",     NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Japanese (Shift JIS)",                    NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Korean (EUC-KR/CP949)",                   NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Korean (ISO-2022-KR)",                    NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Traditional Chinese (Big5)",              NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Traditional Chinese Unix (EUC-TW)",       NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Hong-Kong Supplementary (HKSCS)",         NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Vietnamese (VISCII)",                     NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Vietnamese (Windows-1258)",               NULL, SETTINGS_TYPE_TOGGLE}

};

settings_item performance_menu[] =
{
        {PERFORMANCE_FRAME_SKIP, "Enable frame skip",   NULL, SETTINGS_TYPE_TOGGLE},
        {PERFORMANCE_STRETCH, "Time-stretching audio",  NULL, SETTINGS_TYPE_TOGGLE}

};

settings_item deblocking_filter_settings_menu[] =
{
        {DEBLOCKING_AUTOMATIC, "Automatic",             NULL, SETTINGS_TYPE_TOGGLE},
        {DEBLOCKING_FULL, "Full deblocking (slowest)",  NULL, SETTINGS_TYPE_TOGGLE},
        {DEBLOCKING_MEDIUM, "Medium deblocking",        NULL, SETTINGS_TYPE_TOGGLE},
        {DEBLOCKING_LOW, "Low deblocking",              NULL, SETTINGS_TYPE_TOGGLE},
        {DEBLOCKING_NO, "No deblocking (fastest)",      NULL, SETTINGS_TYPE_TOGGLE}

};

settings_item developer_menu[] =
{
        {DEVELOPER_VERBOSE, "Verbose", NULL, SETTINGS_TYPE_TOGGLE}
};

void
settings_view_directories_save(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    switch (selected->menu[selected->index].id)
    {
    case DIRECTORIES_INTERNAL:
    {
        bool newvalue = !selected->menu[selected->index].toggled;
        settings_toggle_set_one_by_id(directory_menu, 2, DIRECTORIES_INTERNAL, newvalue, false);
        elm_genlist_item_update(selected->item);
        preferences_set_bool(PREF_DIRECTORIES_INTERNAL, newvalue ? true : false);
        break;
    }
    case DIRECTORIES_EXTERNAL:
    {
        bool newvalue = !selected->menu[selected->index].toggled;
        settings_toggle_set_one_by_id(directory_menu, 2, DIRECTORIES_EXTERNAL, newvalue, false);
        elm_genlist_item_update(selected->item);
        preferences_set_bool(PREF_DIRECTORIES_EXTERNAL, newvalue ? true : false);
        break;
    }
    case DIRECTORIES_ADDLOCATION:
        break;
    default:
        break;
    }
}

void
settings_view_simple_save_toggle(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = data;

    if (ctx == NULL)
        return;

    switch (ctx->menu_id)
    {
    case SETTINGS_ID_HWACCELERATION:
        // Select the item (and unselect others)
        settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, true, true);
        // Save the value
        preferences_set_enum(PREF_HWACCELERATION, selected->menu[selected->index].id);
        // Close the popup
        evas_object_del(p_view_sys->popup);
        break;
    case SETTINGS_ID_SUBSENC:
        settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, true, true);
        preferences_set_index(PREF_SUBSENC, selected->index);
        evas_object_del(p_view_sys->popup);
        break;
    case SETTINGS_ID_VORIENTATION:
        settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, true, true);
        preferences_set_enum(PREF_ORIENTATION, selected->menu[selected->index].id);
        evas_object_del(p_view_sys->popup);
        break;
    case SETTINGS_ID_PERFORMANCES:
    {
        bool newvalue = !selected->menu[selected->index].toggled;
        settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, newvalue, false);
        if (selected->menu[selected->index].id == PERFORMANCE_FRAME_SKIP)
            preferences_set_bool(PREF_FRAME_SKIP, newvalue);
        else if (selected->menu[selected->index].id == PERFORMANCE_STRETCH)
            preferences_set_bool(PREF_AUDIO_STRETCH, newvalue);
        evas_object_del(p_view_sys->popup);
        break;
    }
    case SETTINGS_ID_DEBLOCKING:
        settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, true, true);
        preferences_set_enum(PREF_DEBLOCKING, selected->menu[selected->index].id);
        evas_object_del(p_view_sys->popup);
        break;
    case SETTINGS_ID_DEVELOPER:
    {
        bool newvalue = !selected->menu[selected->index].toggled;
        settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, newvalue, false);
        elm_genlist_item_update(selected->item);
        if (selected->menu[selected->index].id == DEVELOPER_VERBOSE)
            preferences_set_bool(PREF_DEVELOPER_VERBOSE, newvalue);
        break;
    }
    default:
        break;
    }
}

void settings_view_delete_context_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    settings_menu_context *ctx = data;
    free(ctx);
}

void settings_view_popup_clear_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;
    p_sys->popup = NULL;
}

void
menu_directories_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    bool internal = preferences_get_bool(PREF_DIRECTORIES_INTERNAL, true);
    bool sdcard = preferences_get_bool(PREF_DIRECTORIES_EXTERNAL, true);
    int len = COUNT_OF(directory_menu);
    Evas_Object *genlist = settings_list_add_styled(directory_menu, len, settings_view_directories_save, NULL, p_view_sys, parent);
    settings_toggle_set_one_by_id(directory_menu, len, DIRECTORIES_INTERNAL, internal, false);
    settings_toggle_set_one_by_id(directory_menu, len, DIRECTORIES_EXTERNAL, sdcard, false);
    elm_naviframe_item_push(p_view_sys->nav, "Media library", NULL, NULL, genlist, NULL);
    evas_object_show(genlist);
}

void
menu_hwacceleration_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_HWACCELERATION;

    menu_id value = preferences_get_enum(PREF_HWACCELERATION, HWACCELERATION_AUTOMATIC);
    int len = COUNT_OF(hardware_acceleration_menu);
    p_view_sys->popup = settings_popup_add(hardware_acceleration_menu, len, settings_view_simple_save_toggle, ctx, p_view_sys, parent);
    settings_toggle_set_one_by_id(hardware_acceleration_menu, len, value, true, true);
    evas_object_show(p_view_sys->popup);
    evas_object_event_callback_add(p_view_sys->popup, EVAS_CALLBACK_FREE, settings_view_delete_context_cb, ctx);
    evas_object_event_callback_add(p_view_sys->popup, EVAS_CALLBACK_FREE, settings_view_popup_clear_cb, p_view_sys);
}

void
menu_subsenc_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_SUBSENC;

    int value = preferences_get_index(PREF_SUBSENC, 0);
    int len = COUNT_OF(subtitles_text_encoding_menu);
    p_view_sys->popup = settings_popup_add(subtitles_text_encoding_menu, len, settings_view_simple_save_toggle, ctx, p_view_sys, parent);
    settings_toggle_set_one_by_index(subtitles_text_encoding_menu, len, value, true, true);
    evas_object_show(p_view_sys->popup);
    evas_object_event_callback_add(p_view_sys->popup, EVAS_CALLBACK_FREE, settings_view_delete_context_cb, ctx);
    evas_object_event_callback_add(p_view_sys->popup, EVAS_CALLBACK_FREE, settings_view_popup_clear_cb, p_view_sys);
}

void
menu_vorientation_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_VORIENTATION;

    menu_id value = preferences_get_enum(PREF_ORIENTATION, ORIENTATION_AUTOMATIC);
    int len = COUNT_OF(video_orientation_menu);
    p_view_sys->popup = settings_popup_add(video_orientation_menu, len, settings_view_simple_save_toggle, ctx, p_view_sys, parent);
    settings_toggle_set_one_by_id(video_orientation_menu, len, value, true, true);
    evas_object_show(p_view_sys->popup);
    evas_object_event_callback_add(p_view_sys->popup, EVAS_CALLBACK_FREE, settings_view_delete_context_cb, ctx);
    evas_object_event_callback_add(p_view_sys->popup, EVAS_CALLBACK_FREE, settings_view_popup_clear_cb, p_view_sys);
}

void
menu_performance_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_PERFORMANCES;

    int len = COUNT_OF(performance_menu);
    p_view_sys->popup = settings_popup_add(performance_menu, len, settings_view_simple_save_toggle, ctx, p_view_sys, parent);

    bool frameskip = preferences_get_bool(PREF_FRAME_SKIP, false);
    bool stretch = preferences_get_bool(PREF_AUDIO_STRETCH, false);

    settings_toggle_set_one_by_id(performance_menu, len, PERFORMANCE_FRAME_SKIP, frameskip, false);
    settings_toggle_set_one_by_id(performance_menu, len, PERFORMANCE_STRETCH, stretch, false);

    evas_object_show(p_view_sys->popup);
    evas_object_event_callback_add(p_view_sys->popup, EVAS_CALLBACK_FREE, settings_view_delete_context_cb, ctx);
    evas_object_event_callback_add(p_view_sys->popup, EVAS_CALLBACK_FREE, settings_view_popup_clear_cb, p_view_sys);
}

void
menu_deblocking_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_DEBLOCKING;

    int value = preferences_get_enum(PREF_DEBLOCKING, DEBLOCKING_AUTOMATIC);
    int len = COUNT_OF(deblocking_filter_settings_menu);
    p_view_sys->popup = settings_popup_add(deblocking_filter_settings_menu, len, settings_view_simple_save_toggle, ctx, p_view_sys, parent);
    settings_toggle_set_one_by_id(deblocking_filter_settings_menu, len, value, true, true);
    evas_object_show(p_view_sys->popup);
    evas_object_event_callback_add(p_view_sys->popup, EVAS_CALLBACK_FREE, settings_view_delete_context_cb, ctx);
    evas_object_event_callback_add(p_view_sys->popup, EVAS_CALLBACK_FREE, settings_view_popup_clear_cb, p_view_sys);
}

void
menu_developer_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_DEVELOPER;

    bool verbose = preferences_get_bool(PREF_DEVELOPER_VERBOSE, false);
    int len = COUNT_OF(developer_menu);
    Evas_Object *genlist = settings_list_add_styled(developer_menu, len, settings_view_simple_save_toggle, ctx, p_view_sys, parent);
    settings_toggle_set_one_by_id(developer_menu, len, DEVELOPER_VERBOSE, verbose, false);
    elm_naviframe_item_push(p_view_sys->nav, "Developer options", NULL, NULL, genlist, NULL);
    evas_object_show(genlist);
    evas_object_event_callback_add(genlist, EVAS_CALLBACK_FREE, settings_view_delete_context_cb, ctx);
}

static bool
settings_event(view_sys *p_view_sys, interface_view_event event)
{
    if(event == INTERFACE_VIEW_EVENT_BACK && p_view_sys->popup)
    {
        // Close any visible popup
        evas_object_del(p_view_sys->popup);
        p_view_sys->popup = NULL;
        return true;
    }
    return false;
}

Eina_Bool
settings_view_last_view_pop(void *data, Elm_Object_Item *it)
{
    view_sys *p_view_sys = data;
    application *p_app = intf_get_application(p_view_sys->p_intf);
    playback_service *p_ps = application_get_playback_service(p_app);

    playback_service_restart_emotion(p_ps, false);
    return EINA_TRUE;
}

void
settings_view_start(view_sys *p_view_sys)
{
    // The settings has been pushed to top
    Elm_Object_Item *it = elm_naviframe_top_item_get(p_view_sys->nav);
    if (it == NULL)
        return;

    // Attach a callback for reloading the settings
    elm_naviframe_item_pop_cb_set(it, settings_view_last_view_pop, p_view_sys);
}

interface_view*
create_settings_view(interface *intf, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));

    view->p_view_sys = calloc(1, sizeof(*view->p_view_sys));
    view->p_view_sys->nav = intf_get_main_naviframe(intf);
    view->pf_start = settings_view_start;
    view->pf_event = settings_event;
    view->p_view_sys->p_intf = intf;

    int len = COUNT_OF(settings_menu);
    view->view = settings_list_add_styled(settings_menu, len, NULL, NULL, view->p_view_sys, view->p_view_sys->nav);
    evas_object_show(view->view);

    return view;
}

void
destroy_settings_view(interface_view *view)
{
    free(view->p_view_sys);
    free(view);
}

#undef COUNT_OF
