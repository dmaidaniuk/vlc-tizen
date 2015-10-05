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
#include "ui/interface.h"
#include "ui/utils.h"

#include <app_preference.h>

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
        {42, "Enable frame skip", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Time-stretching audio", NULL, SETTINGS_TYPE_TOGGLE}

};

settings_item deblocking_filter_settings_menu[] =
{
        {42, "Automatic", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Full deblocking (slowest)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Medium deblocking", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Low deblocking", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "No deblocking (fastest)", NULL, SETTINGS_TYPE_TOGGLE}

};

void
settings_toggle_set_all(settings_item *menu, int menu_len, bool value)
{
    for (int i = 0; i < menu_len; i++)
    {
        menu[i].toggled = value;
    }
}

void
settings_toggle_set_one_by_index(settings_item *menu, int menu_len, int index, bool value, bool toggle_others)
{
    if (toggle_others) {
        for (int i = 0; i < menu_len; i++)
        {
            menu[i].toggled = !value;
        }
    }
    menu[index].toggled = value;
}

void
settings_toggle_set_one_by_id(settings_item *menu, int menu_len, int id, bool value, bool toggle_others)
{
    for (int i = 0; i < menu_len; i++)
    {
        if (menu[i].id == id)
        {
            menu[i].toggled = value;
            if (!toggle_others)
                break;
        }
        else if (toggle_others)
        {
            menu[i].toggled = !value;
        }
    }
}

int
settings_get_int(char *key, int default_value)
{
    int value;
    if (preference_get_int(key, &value) != PREFERENCE_ERROR_NONE)
            value = default_value;

    return value;
}

void
menu_directories_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent)
{
    int len = (int)sizeof(directory_menu) / (int)sizeof(*directory_menu);
    Evas_Object *genlist = settings_list_add(directory_menu, len, NULL, p_view_sys, parent);
    elm_naviframe_item_push(p_view_sys->nav, NULL, NULL, NULL, genlist, NULL);
    evas_object_show(genlist);
}

void
menu_hwacceleration_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent)
{
    int value = settings_get_int("HWACCELERATION", HWACCELERATION_AUTOMATIC);
    int len = (int)sizeof(hardware_acceleration_menu) / (int)sizeof(*hardware_acceleration_menu);
    Evas_Object *popup = settings_popup_add(hardware_acceleration_menu, len, settings_toggle_switch, p_view_sys, parent);
    settings_toggle_set_one_by_id(hardware_acceleration_menu, len, value, true, true);
    evas_object_show(popup);
}

void
menu_subsenc_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent)
{
    int value = settings_get_int("SUBENC", 0); // WARNING: we store the index, instead of the ID
    int len = (int)sizeof(subtitles_text_encoding_menu) / (int)sizeof(*subtitles_text_encoding_menu);
    Evas_Object *popup = settings_popup_add(subtitles_text_encoding_menu, len, settings_toggle_switch, p_view_sys, parent);
    settings_toggle_set_one_by_index(subtitles_text_encoding_menu, len, value, true, true);
    evas_object_show(popup);
}

void
menu_vorientation_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent)
{
    int value = settings_get_int("ORIENTATION", ORIENTATION_AUTOMATIC);
    int len = (int)sizeof(video_orientation_menu) / (int)sizeof(*video_orientation_menu);
    Evas_Object *popup = settings_popup_add(video_orientation_menu, len, settings_toggle_switch, p_view_sys, parent);
    settings_toggle_set_one_by_id(video_orientation_menu, len, value, true, true);
    evas_object_show(popup);
}

void
menu_performance_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent)
{
    int len = (int)sizeof(performance_menu) / (int)sizeof(*performance_menu);
    Evas_Object *popup = settings_popup_add(performance_menu, len, settings_toggle_switch, p_view_sys, parent);
    evas_object_show(popup);
}

void
menu_deblocking_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent)
{
    int len = (int)sizeof(deblocking_filter_settings_menu) / (int)sizeof(*deblocking_filter_settings_menu);
    Evas_Object *popup = settings_popup_add(deblocking_filter_settings_menu, len, settings_toggle_switch, p_view_sys, parent);
    evas_object_show(popup);
}

void
settings_toggle_switch(settings_menu_selected *selected, view_sys* p_view_sys, Evas_Object *parent)
{
    settings_toggle_set_one_by_index(selected->menu, selected->menu_len, selected->index, true, true);
    evas_object_del(parent);
}

/* Free the settings_internal_data struct when the list is deleted */
static void
gl_del_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
    settings_internal_data *sd = data;
    free(sd);
}


static char *
gl_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    settings_internal_data *sd = data;

    if (!sd || !sd->item || !part)
        return NULL;

    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(sd->item);

    if (!itc)
        return NULL;

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist headers item label */
    if (itc->item_style && !strcmp(itc->item_style, "groupindex")) {
        if (part && !strcmp(part, "elm.text.main")) {
            return strdup(sd->selected.menu[sd->selected.index].title);
        }
    }
    /* Or put this string as the genlist setting item label */
    else if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.text.main.left")) {
            return strdup(sd->selected.menu[sd->selected.index].title);
        }
    }
    return NULL;
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    settings_internal_data *sd = data;

    if (!sd || !sd->item || !part)
        return NULL;

    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(sd->item);

    if (!itc)
        return NULL;

    Evas_Object *content = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.icon.1") && sd->selected.menu[sd->selected.index].icon != NULL) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
            Evas_Object *icon = create_icon(content, sd->selected.menu[sd->selected.index].icon);
            elm_layout_content_set(content, "elm.swallow.content", icon);
        }
        if (sd->selected.menu[sd->selected.index].type == SETTINGS_TYPE_TOGGLE)
        {
            if (part && !strcmp(part, "elm.icon.right")) {
                Evas_Object *icon;
                content = elm_layout_add(obj);
                elm_layout_theme_set(content, "layout", "list/A/right.icon", "default");
                if (sd->selected.menu[sd->selected.index].toggled)
                    icon = create_icon(sd->parent, "toggle_on.png");
                else
                    icon = create_icon(sd->parent, "toggle_off.png");
                elm_layout_content_set(content, "elm.swallow.content", icon);
            }
        }
    }

    return content;
}

static void
gl_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    settings_internal_data *sd = data;
    Settings_menu_callback item_cb = sd->selected.menu[sd->selected.index].cb;

    if (item_cb != NULL)
        item_cb(&sd->selected, sd->p_view_sys, sd->parent);

    if (sd->global_cb != NULL)
        sd->global_cb(&sd->selected, sd->p_view_sys, sd->parent);
}

static void
gl_contracted_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    Elm_Object_Item *it = event_info;

    /* Free the genlist subitems when contracted */
    elm_genlist_item_subitems_clear(it);
}

Evas_Object *
settings_list_add(settings_item *menu, int len, Settings_menu_callback global_menu_cb, view_sys* p_view_sys, Evas_Object *parent)
{
    Evas_Object *genlist;
    Elm_Object_Item *it, *hit = NULL;
    int index;

    /* Set then create the Genlist object */
    Elm_Genlist_Item_Class *hitc = elm_genlist_item_class_new();
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();

    /* Set the headers item class */
    hitc->item_style = "groupindex";
    hitc->func.text_get = gl_text_get_cb;
    hitc->func.content_get = gl_content_get_cb;
    hitc->func.del = gl_del_cb;

    /* Set the settings item class */
    itc->item_style = "1line";
    itc->func.text_get = gl_text_get_cb;
    itc->func.content_get = gl_content_get_cb;
    itc->func.del = gl_del_cb;

    genlist = elm_genlist_add(parent);

    /* Set the genlist scoller mode */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    /* Enable the genlist HOMOGENEOUS mode */
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    /* Enable the genlist COMPRESS mode */
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    /* Set smart Callbacks */
    //evas_object_smart_callback_add(genlist, "realized", gl_realized_cb, NULL);
    //evas_object_smart_callback_add(genlist, "loaded", gl_loaded_cb, NULL);
    //evas_object_smart_callback_add(genlist, "longpressed", gl_longpressed_cb, NULL);
    evas_object_smart_callback_add(genlist, "contracted", gl_contracted_cb, NULL);

    /* Stop when the setting list names is all used */
    for (index = 0; index < len; index++) {
        settings_internal_data *sd = calloc(1, sizeof(*sd));

        /* Put the index in the settings_internal_data struct for callbacks */
        sd->p_view_sys = p_view_sys;
        sd->selected.index = index;
        sd->selected.id = menu[index].id;
        sd->selected.menu = menu;
        sd->selected.menu_len = len;
        sd->global_cb = global_menu_cb;

        /* Set and append headers items */
        if (menu[index].type == SETTINGS_TYPE_CATEGORY)
        {
            hit = elm_genlist_item_append(genlist,
                    hitc,                           /* genlist item class               */
                    sd,                             /* genlist item class user data     */
                    NULL,                           /* genlist parent item              */
                    ELM_GENLIST_ITEM_TREE,          /* genlist item type                */
                    NULL,                           /* genlist select smart callback    */
                    sd);                            /* genlist smart callback user data */

            /* Put genlist item in the settings_internal_data struct for callbacks */
            sd->item = hit;
            //elm_object_item_del_cb_set(hit, free_genlist_item_data);
        }
        else if (menu[index].type == SETTINGS_TYPE_ITEM || menu[index].type == SETTINGS_TYPE_TOGGLE)
        {
            /* Set and append settings items */
            sd->selected.index = index;
            it = elm_genlist_item_append(genlist,
                    itc,                           /* genlist item class               */
                    sd,                            /* genlist item class user data     */
                    hit,                           /* genlist parent item              */
                    ELM_GENLIST_ITEM_NONE,         /* genlist item type                */
                    gl_selected_cb,               /* genlist select smart callback    */
                    sd);                           /* genlist smart callback user data */

            /* Put genlist item in the settings_internal_data struct for callbacks */
            sd->parent = parent;
            sd->item = it;
            //elm_object_item_del_cb_set(it, free_genlist_item_data);
        }
    }

    /* */
    elm_genlist_item_class_free(hitc);
    elm_genlist_item_class_free(itc);

    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

    return genlist;
}

static void
popup_block_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *parent = data;
    evas_object_del(parent);
}

Evas_Object *
settings_popup_add(settings_item *menu, int menu_len, Settings_menu_callback global_menu_cb, view_sys* p_view_sys, Evas_Object *parent)
{
    Evas_Object *popup = elm_popup_add(parent);
    Evas_Object *box = elm_box_add(popup);
    Evas_Object *genlist;

    /* Set the popup Y axis value */
    if(menu_len < 6){
        evas_object_size_hint_min_set(box, EVAS_HINT_FILL, menu_len * 100);
        evas_object_size_hint_max_set(box, EVAS_HINT_FILL, menu_len * 100);
    }

    else{
        evas_object_size_hint_min_set(box, EVAS_HINT_FILL, 500);
        evas_object_size_hint_max_set(box, EVAS_HINT_FILL, 500);
    }

    genlist = settings_list_add(menu, menu_len, global_menu_cb, p_view_sys, popup);

    elm_box_pack_end(box, genlist);

    evas_object_smart_callback_add(popup, "block,clicked", popup_block_cb, popup);

    elm_object_content_set(popup, box);
    evas_object_show(genlist);

    return popup;
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

    int len = (int)sizeof(settings_menu) / (int)sizeof(*settings_menu);
    view->view = settings_list_add(settings_menu, len, NULL, view->p_view_sys, view->p_view_sys->nav);
    evas_object_show(view->view);

    return view;
}

void
destroy_setting_view(interface_view *view)
{
    free(view->p_view_sys);
    free(view);
}
