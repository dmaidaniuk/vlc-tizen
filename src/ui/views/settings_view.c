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
#include "ui/views/settings.h"

#include <app_preference.h>

/* ID */
#define SETTINGS_ID_DIRECTORIES 1
#define SETTINGS_ID_HWACCELERATION 2
#define SETTINGS_ID_SUBSENC 3
#define SETTINGS_ID_VORIENTATION 4
#define SETTINGS_ID_PERFORMANCES 5
#define SETTINGS_ID_DEBLOCKING 6

/* Types */
#define SETTINGS_TYPE_CATEGORY 0
#define SETTINGS_TYPE_ITEM 1
#define SETTINGS_TYPE_TOGGLE 2

static void
menu_directories_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
static void
menu_hwacceleration_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
static void
menu_subsenc_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
static void
menu_vorientation_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
static void
menu_performance_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);
static void
menu_deblocking_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);

static Evas_Object *
settings_list_add(settings_item *menu, int len, Settings_menu_callback global_menu_cb, Evas_Object *parent);

static Evas_Object *
settings_popup_add(settings_item *menu, int menu_len, Settings_menu_callback global_menu_cb, Evas_Object *parent);

static void
settings_toggle_switch(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent);

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
        {42, "Automatic", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Disabled", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Decoding acceleration", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Full acceleration", NULL, SETTINGS_TYPE_TOGGLE}
};

settings_item video_orientation_menu[] =
{
        {42, "Automatic (sensor)", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Locked at start", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Landscape", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Portrait", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Reverse landscape", NULL, SETTINGS_TYPE_TOGGLE},
        {42, "Reverse portrait", NULL, SETTINGS_TYPE_TOGGLE}
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

static void
menu_directories_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent)
{
    int len = (int)sizeof(directory_menu) / (int)sizeof(*directory_menu);
    Evas_Object *genlist = settings_list_add(directory_menu, len, NULL, parent);
    elm_object_content_set(parent, genlist);
    evas_object_show(genlist);
}

static void
menu_hwacceleration_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent)
{
    int len = (int)sizeof(hardware_acceleration_menu) / (int)sizeof(*hardware_acceleration_menu);
    Evas_Object *popup = settings_popup_add(hardware_acceleration_menu, len, settings_toggle_switch, parent);
    evas_object_show(popup);
}

static void
menu_subsenc_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent)
{
    int len = (int)sizeof(subtitles_text_encoding_menu) / (int)sizeof(*subtitles_text_encoding_menu);
    Evas_Object *popup = settings_popup_add(subtitles_text_encoding_menu, len, NULL, parent);
    evas_object_show(popup);
}

static void
menu_vorientation_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent)
{
    int len = (int)sizeof(video_orientation_menu) / (int)sizeof(*video_orientation_menu);
    Evas_Object *popup = settings_popup_add(video_orientation_menu, len, NULL, parent);
    evas_object_show(popup);
}

static void
menu_performance_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent)
{
    int len = (int)sizeof(performance_menu) / (int)sizeof(*performance_menu);
    Evas_Object *popup = settings_popup_add(performance_menu, len, NULL, parent);
    evas_object_show(popup);
}

static void
menu_deblocking_selected_cb(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent)
{
    int len = (int)sizeof(deblocking_filter_settings_menu) / (int)sizeof(*deblocking_filter_settings_menu);
    Evas_Object *popup = settings_popup_add(deblocking_filter_settings_menu, len, NULL, parent);
    evas_object_show(popup);
}

static void
settings_toggle_switch(int id, int index, settings_item *menu, int menu_len, Evas_Object *parent)
{
    //TODO
    evas_object_del(parent);
}

/* Free the setting_data struct when the list is deleted */
static void
gl_del_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
    setting_data *sd = data;
    free(sd);
}


static char *
gl_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    setting_data *sd = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(sd->item);

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist headers item label */
    if (itc->item_style && !strcmp(itc->item_style, "groupindex")) {
        if (part && !strcmp(part, "elm.text.main")) {
            return strdup(sd->menu[sd->index].title);
        }
    }
    /* Or put this string as the genlist setting item label */
    else if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.text.main.left")) {
            return strdup(sd->menu[sd->index].title);
        }
    }
    return NULL;
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    setting_data *sd = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(sd->item);
    Evas_Object *content = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.icon.1") && sd->menu[sd->index].icon != NULL) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
            Evas_Object *icon = create_icon(content, sd->menu[sd->index].icon);
            elm_layout_content_set(content, "elm.swallow.content", icon);
        }
        if (sd->menu[sd->index].type == SETTINGS_TYPE_TOGGLE)
        {
            if (part && !strcmp(part, "elm.icon.right")) {
                Evas_Object *icon;
                content = elm_layout_add(obj);
                elm_layout_theme_set(content, "layout", "list/A/right.icon", "default");
                if (sd->menu[sd->index].toggled)
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
    setting_data *sd = data;
    Settings_menu_callback item_cb = sd->menu[sd->index].cb;

    if (item_cb != NULL)
        item_cb(sd->id, sd->index, sd->menu, sd->menu_len, sd->parent);

    if (sd->global_cb != NULL)
        sd->global_cb(sd->id, sd->index, sd->menu, sd->menu_len, sd->parent);
}

static void
gl_contracted_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    Elm_Object_Item *it = event_info;

    /* Free the genlist subitems when contracted */
    elm_genlist_item_subitems_clear(it);
}

Evas_Object *
settings_list_add(settings_item *menu, int len, Settings_menu_callback global_menu_cb, Evas_Object *parent)
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
        setting_data *sd = calloc(1, sizeof(*sd));

        /* Put the index in the setting_data struct for callbacks */
        sd->index = index;
        sd->id = menu[index].id;
        sd->menu = menu;
        sd->menu_len = len;
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

            /* Put genlist item in the setting_data struct for callbacks */
            sd->item = hit;
            //elm_object_item_del_cb_set(hit, free_genlist_item_data);
        }
        else if (menu[index].type == SETTINGS_TYPE_ITEM || menu[index].type == SETTINGS_TYPE_TOGGLE)
        {
            /* Set and append settings items */
            sd->index = index;
            it = elm_genlist_item_append(genlist,
                    itc,                           /* genlist item class               */
                    sd,                            /* genlist item class user data     */
                    hit,                           /* genlist parent item              */
                    ELM_GENLIST_ITEM_NONE,         /* genlist item type                */
                    gl_selected_cb,               /* genlist select smart callback    */
                    sd);                           /* genlist smart callback user data */

            /* Put genlist item in the setting_data struct for callbacks */
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
settings_popup_add(settings_item *menu, int menu_len, Settings_menu_callback global_menu_cb, Evas_Object *parent)
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

    genlist = settings_list_add(menu, menu_len, global_menu_cb, popup);

    elm_box_pack_end(box, genlist);

    evas_object_smart_callback_add(popup, "block,clicked", popup_block_cb, popup);

    elm_object_content_set(popup, box);
    evas_object_show(genlist);

    return popup;
}

interface_view*
create_setting_view(interface *intf, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));
    //view->view = create_setting_list(parent);

    int len = (int)sizeof(settings_menu) / (int)sizeof(*settings_menu);
    view->view = settings_list_add(settings_menu, len, NULL, parent);
    evas_object_show(view->view);

    return view;
}

void
destroy_setting_view(interface_view *view)
{
    free(view);
}
