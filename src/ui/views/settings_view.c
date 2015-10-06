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

#include "common.h"

#include "settings_view.h"
#include "ui/settings/settings.h"
#include "ui/interface.h"
#include "ui/utils.h"

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

struct view_sys {
    Evas_Object *nav;
};

settings_item settings_menu[] =
{
        {0,                             "General",                      "",                                 SETTINGS_TYPE_CATEGORY},
        {SETTINGS_ID_DIRECTORIES,       "Directories",                  "ic_menu_folder.png",               SETTINGS_TYPE_ITEM,         menu_directories_selected_cb},
        {SETTINGS_ID_HWACCELERATION,    "Hardware acceleration",        "ic_menu_preferences.png",          SETTINGS_TYPE_ITEM,         menu_hwacceleration_selected_cb},
        {SETTINGS_ID_SUBSENC,           "Subtitles text encoding",      "ic_browser_subtitle_normal.png",   SETTINGS_TYPE_ITEM,         menu_subsenc_selected_cb},
        {SETTINGS_ID_VORIENTATION,      "Video orientation",            "ic_menu_video.png",                SETTINGS_TYPE_ITEM,         menu_vorientation_selected_cb},
        {0,                             "Extra settings",               "",                                 SETTINGS_TYPE_CATEGORY},
        {SETTINGS_ID_PERFORMANCES,      "Performances",                 "ic_menu_preferences.png",          SETTINGS_TYPE_ITEM,         menu_performance_selected_cb},
        {SETTINGS_ID_DEBLOCKING,        "Deblocking filter settings",   "ic_menu_preferences.png",          SETTINGS_TYPE_ITEM,         menu_deblocking_selected_cb}
};

/* Create the setting submenu items */

settings_item directory_menu[] =
{
        {42, "Directories", "", SETTINGS_TYPE_CATEGORY},
        {42, "Internal memory", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Add repository", "call_button_add_call_press.png", SETTINGS_TYPE_ITEM}
};

settings_item hardware_acceleration_menu[] =
{
        {HWACCELERATION_AUTOMATIC, "Automatic", NULL, SETTINGS_TYPE_TOGGLE},
        {HWACCELERATION_DISABLED, "Disabled", NULL, SETTINGS_TYPE_TOGGLE},
        {HWACCELERATION_DECODING, "Decoding acceleration", NULL, SETTINGS_TYPE_TOGGLE},
        {HWACCELERATION_FULL, "Full acceleration", NULL, SETTINGS_TYPE_TOGGLE}
};

settings_item video_orientation_menu[] =
{
        {ORIENTATION_AUTOMATIC, "Automatic (sensor)", NULL, SETTINGS_TYPE_TOGGLE},
        {ORIENTATION_LOCKED, "Locked at start", NULL, SETTINGS_TYPE_TOGGLE},
        {ORIENTATION_LANDSCAPE, "Landscape", NULL, SETTINGS_TYPE_TOGGLE},
        {ORIENTATION_PORTRAIT, "Portrait", NULL, SETTINGS_TYPE_TOGGLE},
        {ORIENTATION_R_LANDSCAPE, "Reverse landscape", NULL, SETTINGS_TYPE_TOGGLE},
        {ORIENTATION_R_PORTRAIT, "Reverse portrait", NULL, SETTINGS_TYPE_TOGGLE}
};

settings_item subtitles_text_encoding_menu[] =
{
        {42, "Default (Windows-1252)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Universal (UTF-8)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Universal (UTF-16)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Universal (big endian UTF-16)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Universal (little endian UTF-16)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Universal, Chinese (GB18030)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Western European (Latin-9)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Western European (Windows-1252)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Western European (IBM 00850)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Eastern European (Latin-2)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Eastern European (Windows-1250)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Esperanto (Latin-3)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Nordic (Latin-6)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Cyrillic (Windows-1251)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Russian (KOI8-R)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Ukrainian (KOI8-U)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Arabic (ISO 8859-6)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Arabic (Windows-1256)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Greek (ISO 8859-7)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Greek (Windows-1253)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Hebrew (ISO 8859-8)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Hebrew (Windows-1255)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Turkish (ISO 8859-9)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Turkish (Windows-1254)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Thai (TIS 620-2533/ISO 8859-11)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Thai (Windows-874)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Baltic (Latin-7)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Baltic (Windows-1257)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Celtic (Latin-8)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "South-Estearn (Latin-10)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Simplified Chinese (ISO-2022-CN-EXT)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Simplified Chinese Unix (EUC-CN)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Japanese (7-bits JIS/ISO-2022-JP-2)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Japanese (Shift JIS)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Korean (EUC-KR/CP949)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Korean (ISO-2022-KR)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Traditional Chinese (Big5)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Traditional Chinese Unix (EUC-TW)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Hong-Kong Supplementary (HKSCS)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Vietnamese (VISCII)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Vietnamese (Windows-1258)", NULL, SETTINGS_TYPE_TOGGLE}

};

settings_item performance_menu[] =
{
        {PERFORMANCE_FRAME_SKIP, "Enable frame skip", NULL, SETTINGS_TYPE_TOGGLE},
        {PERFORMANCE_STRETCH, "Time-stretching audio", NULL, SETTINGS_TYPE_TOGGLE}

};

settings_item deblocking_filter_settings_menu[] =
{
        {DEBLOCKING_AUTOMATIC, "Automatic", NULL, SETTINGS_TYPE_TOGGLE},
        {DEBLOCKING_FULL, "Full deblocking (slowest)", NULL, SETTINGS_TYPE_TOGGLE},
        {DEBLOCKING_MEDIUM, "Medium deblocking", NULL, SETTINGS_TYPE_TOGGLE},
        {DEBLOCKING_LOW, "Low deblocking", NULL, SETTINGS_TYPE_TOGGLE},
        {DEBLOCKING_NO, "No deblocking (fastest)", NULL, SETTINGS_TYPE_TOGGLE}

};

void
settings_simple_save_toggle(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
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
        preference_set_int("HWACCELERATION", selected->menu[selected->index].id);
        // Close the popup
        evas_object_del(parent);
        break;
    case SETTINGS_ID_SUBSENC:
        settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, true, true);
        preference_set_int("SUBENC", selected->index);
        evas_object_del(parent);
        break;
    case SETTINGS_ID_VORIENTATION:
        settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, true, true);
        preference_set_int("ORIENTATION", selected->menu[selected->index].id);
        evas_object_del(parent);
        break;
    case SETTINGS_ID_PERFORMANCES:
    {
        bool newvalue = !selected->menu[selected->index].toggled;
        settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, newvalue, false);
        if (selected->menu[selected->index].id == PERFORMANCE_FRAME_SKIP)
            preference_set_int("FRAME_SKIP", newvalue);
        else if (selected->menu[selected->index].id == PERFORMANCE_STRETCH)
            preference_set_int("AUDIO_STRETCH", newvalue);
        evas_object_del(parent);
        break;
    }
    case SETTINGS_ID_DEBLOCKING:
        settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, true, true);
        preference_set_int("DEBLOCKING", selected->menu[selected->index].id);
        evas_object_del(parent);
        break;
    }

    free(ctx);
}

void
menu_directories_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    Evas_Object *genlist = settings_list_add(directory_menu, COUNT_OF(directory_menu), NULL, NULL, p_view_sys, parent);
    elm_naviframe_item_push(p_view_sys->nav, "Media library", NULL, NULL, genlist, NULL);
    evas_object_show(genlist);
}

void
menu_hwacceleration_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_HWACCELERATION;

    int value = settings_get_int("HWACCELERATION", HWACCELERATION_AUTOMATIC);
    int len = COUNT_OF(hardware_acceleration_menu);
    Evas_Object *popup = settings_popup_add(hardware_acceleration_menu, len, settings_simple_save_toggle, ctx, p_view_sys, parent);
    settings_toggle_set_one_by_id(hardware_acceleration_menu, len, value, true, true);
    evas_object_show(popup);
}

void
menu_subsenc_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_SUBSENC;

    int value = settings_get_int("SUBENC", 0); // WARNING: we store the index, instead of the ID
    int len = COUNT_OF(subtitles_text_encoding_menu);
    Evas_Object *popup = settings_popup_add(subtitles_text_encoding_menu, len, settings_simple_save_toggle, ctx, p_view_sys, parent);
    settings_toggle_set_one_by_index(subtitles_text_encoding_menu, len, value, true, true);
    evas_object_show(popup);
}

void
menu_vorientation_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_VORIENTATION;

    int value = settings_get_int("ORIENTATION", ORIENTATION_AUTOMATIC);
    int len = COUNT_OF(video_orientation_menu);
    Evas_Object *popup = settings_popup_add(video_orientation_menu, len, settings_simple_save_toggle, ctx, p_view_sys, parent);
    settings_toggle_set_one_by_id(video_orientation_menu, len, value, true, true);
    evas_object_show(popup);
}

void
menu_performance_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_PERFORMANCES;

    int len = COUNT_OF(performance_menu);
    Evas_Object *popup = settings_popup_add(performance_menu, len, settings_simple_save_toggle, ctx, p_view_sys, parent);

    bool frameskip = (bool)settings_get_int("FRAME_SKIP", 0);
    bool stretch = (bool)settings_get_int("AUDIO_STRETCH", 0);

    settings_toggle_set_one_by_id(performance_menu, len, PERFORMANCE_FRAME_SKIP, frameskip, false);
    settings_toggle_set_one_by_id(performance_menu, len, PERFORMANCE_STRETCH, stretch, false);

    evas_object_show(popup);
}

void
menu_deblocking_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent)
{
    settings_menu_context *ctx = malloc(sizeof(*ctx));
    ctx->menu_id = SETTINGS_ID_DEBLOCKING;

    int value = settings_get_int("DEBLOCKING", DEBLOCKING_AUTOMATIC);
    int len = COUNT_OF(deblocking_filter_settings_menu);
    Evas_Object *popup = settings_popup_add(deblocking_filter_settings_menu, len, settings_simple_save_toggle, ctx, p_view_sys, parent);
    settings_toggle_set_one_by_id(deblocking_filter_settings_menu, len, value, true, true);
    evas_object_show(popup);
}

bool
view_callback(view_sys *p_view_sys, interface_view_event event)
{
    elm_naviframe_item_pop(p_view_sys->nav);
    return true;
}

interface_view*
create_setting_view(interface *intf, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));

    view->p_view_sys = calloc(1, sizeof(*view->p_view_sys));
    view->p_view_sys->nav = intf_get_main_naviframe(intf);
    view->pf_event = view_callback;

    int len = COUNT_OF(settings_menu);
    view->view = settings_list_add(settings_menu, len, NULL, NULL, view->p_view_sys, view->p_view_sys->nav);
    evas_object_show(view->view);

    return view;
}

void
destroy_setting_view(interface_view *view)
{
    free(view->p_view_sys);
    free(view);
}

#undef COUNT_OF
