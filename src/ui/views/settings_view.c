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
#include "ui/menu/popup_genlist.h"
#include "ui/utils.h"

#include <app_preference.h>

typedef struct setting_data
{
    int id;
    int index;
    Elm_Object_Item *item;
    Evas_Object *parent;
    Evas_Object *genlist_test;

} setting_data;

typedef struct settings_item
{
    int id;
    const char* title;
    const char* icon;
    int type;

} settings_item;


settings_item settings_menu[] =
{
        {0,                             "General",                      "",                                 SETTINGS_TYPE_CATEGORY},
        {SETTINGS_ID_DIRECTORIES,       "Directories",                  "ic_menu_folder.png",               SETTINGS_TYPE_ITEM},
        {SETTINGS_ID_HWACCELERATION,    "Hardware acceleration",        "ic_menu_preferences.png",          SETTINGS_TYPE_ITEM},
        {SETTINGS_ID_SUBSENC,           "Subtitles text encoding",      "ic_browser_subtitle_normal.png",   SETTINGS_TYPE_ITEM},
        {SETTINGS_ID_VORIENTATION,      "Video orientation",            "ic_menu_video.png",                SETTINGS_TYPE_ITEM},
        {0,                             "Extra settings",               "",                                 SETTINGS_TYPE_CATEGORY},
        {SETTINGS_ID_PERFORMANCES,      "Performances",                 "ic_menu_preferences.png",          SETTINGS_TYPE_ITEM},
        {SETTINGS_ID_DEBLOCKING,        "Deblocking filter settings",   "ic_menu_preferences.png",          SETTINGS_TYPE_ITEM}
};

/* Create the setting submenu items */
/* TODO : Add callbacks for each one and set the logic behind them */

popup_menu_item_s directory_menu[] =
{
        {"Internal memory", "toggle_off.png"},
        {"Add repository", "call_button_add_call_press.png"}
};

popup_menu_item_s hardware_acceleration_menu[] =
{
        {"Automatic", "toggle_on.png"},
        {"Disabled", "toggle_off.png"},
        {"Decoding acceleration", "toggle_off.png"},
        {"Full acceleration", "toggle_off.png"}
};

popup_menu_item_s video_orientation_menu[] =
{
        {"Automatic (sensor)", "toggle_on.png"},
        {"Locked at start", "toggle_off.png"},
        {"Landscape", "toggle_off.png"},
        {"Portrait", "toggle_off.png"},
        {"Reverse landscape", "toggle_off.png"},
        {"Reverse portrait", "toggle_off.png"}
};

popup_menu_item_s subtitles_text_encoding_menu[] =
{
        {"Default (Windows-1252)", "toggle_on.png"},
        {"Universal (UTF-8)", "toggle_off.png"},
        {"Universal (UTF-16)", "toggle_off.png"},
        {"Universal (big endian UTF-16)", "toggle_off.png"},
        {"Universal (little endian UTF-16)", "toggle_off.png"},
        {"Universal, Chinese (GB18030)", "toggle_off.png"},
        {"Western European (Latin-9)", "toggle_off.png"},
        {"Western European (Windows-1252)", "toggle_off.png"},
        {"Western European (IBM 00850)", "toggle_off.png"},
        {"Eastern European (Latin-2)", "toggle_off.png"},
        {"Eastern European (Windows-1250)", "toggle_off.png"},
        {"Esperanto (Latin-3)", "toggle_off.png"},
        {"Nordic (Latin-6)", "toggle_off.png"},
        {"Cyrillic (Windows-1251)", "toggle_off.png"},
        {"Russian (KOI8-R)", "toggle_off.png"},
        {"Ukrainian (KOI8-U)", "toggle_off.png"},
        {"Arabic (ISO 8859-6)", "toggle_off.png"},
        {"Arabic (Windows-1256)", "toggle_off.png"},
        {"Greek (ISO 8859-7)", "toggle_off.png"},
        {"Greek (Windows-1253)", "toggle_off.png"},
        {"Hebrew (ISO 8859-8)", "toggle_off.png"},
        {"Hebrew (Windows-1255)", "toggle_off.png"},
        {"Turkish (ISO 8859-9)", "toggle_off.png"},
        {"Turkish (Windows-1254)", "toggle_off.png"},
        {"Thai (TIS 620-2533/ISO 8859-11)", "toggle_off.png"},
        {"Thai (Windows-874)", "toggle_off.png"},
        {"Baltic (Latin-7)", "toggle_off.png"},
        {"Baltic (Windows-1257)", "toggle_off.png"},
        {"Celtic (Latin-8)", "toggle_off.png"},
        {"South-Estearn (Latin-10)", "toggle_off.png"},
        {"Simplified Chinese (ISO-2022-CN-EXT)", "toggle_off.png"},
        {"Simplified Chinese Unix (EUC-CN)", "toggle_off.png"},
        {"Japanese (7-bits JIS/ISO-2022-JP-2)", "toggle_off.png"},
        {"Japanese (Shift JIS)", "toggle_off.png"},
        {"Korean (EUC-KR/CP949)", "toggle_off.png"},
        {"Korean (ISO-2022-KR)", "toggle_off.png"},
        {"Traditional Chinese (Big5)", "toggle_off.png"},
        {"Traditional Chinese Unix (EUC-TW)", "toggle_off.png"},
        {"Hong-Kong Supplementary (HKSCS)", "toggle_off.png"},
        {"Vietnamese (VISCII)", "toggle_off.png"},
        {"Vietnamese (Windows-1258)", "toggle_off.png"}

};

popup_menu_item_s performance_menu[] =
{
        {"Enable frame skip", "toggle_off.png"},
        {"Time-stretching audio", "toggle_off.png"}

};

popup_menu_item_s deblocking_filter_settings_menu[] =
{
        {"Automatic", "toggle_on.png"},
        {"Full deblocking (slowest)", "toggle_off.png"},
        {"Medium deblocking", "toggle_off.png"},
        {"Low deblocking", "toggle_off.png"},
        {"No deblocking (fastest)", "toggle_off.png"}

};

static void
popup_directories_selected_cb(int id, int index, popup_menu_item_s *menu_item, Evas_Object *parent, void *data)
{
    /* Change the icon depending on the setting state (pressed or not) */

    if(strcmp (menu_item[index].icon, "toggle_on.png") == 0)
    {
        menu_item[index].icon = "toggle_off.png";
    }

    else if(strcmp (menu_item[index].icon, "toggle_off.png") == 0)
    {
        for(int count = 0; count < ((int)sizeof(directory_menu) / (int)sizeof(*directory_menu)); count ++)
        {
            if (strcmp(menu_item[count].icon, "toggle_on.png") == 0)
            {
                menu_item[count].icon = "toggle_off.png";
            }
        }

        menu_item[index].icon = "toggle_on.png";
    }

    /* */
    evas_object_del(parent);
}

/* Free the setting_data struct when the list is deleted */
static void
gl_del_cb(void *data, Evas_Object *obj EINA_UNUSED)
{
    setting_data *sd = data;
    free(sd);
}

static Evas_Object*
create_icon(Evas_Object *parent, int count)
{
    return create_image(parent, settings_menu[count].icon);
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
            return strdup(settings_menu[sd->index].title);
        }
    }
    /* Or put this string as the genlist setting item label */
    else if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.text.main.left")) {
            return strdup(settings_menu[sd->index].title);
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
        if (part && !strcmp(part, "elm.icon.1")) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
            Evas_Object *icon = create_icon(content, sd->index);
            elm_layout_content_set(content, "elm.swallow.content", icon);
        }
    }

    return content;
}

static void
gl_loaded_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
    /* Set the callbacks when one of the genlist item is loaded */
}

static void
gl_realized_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    /* Set the callbacks when one of the genlist item is realized */
}

static void
gl_longpressed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    /* Set the callbacks when one of the genlist item is longpress */
}

static void
gl_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    Evas_Object *list;
    Evas_Object *popup;
    setting_data *sd = data;

    popup_menu_item_s *menu = NULL;
    int len = 0;
    Menu_item_callback callback = NULL;
    void *callback_data = NULL;

    switch(sd->id)
    {
    case SETTINGS_ID_DIRECTORIES:
        menu = directory_menu;
        len = (int)sizeof(directory_menu) / (int)sizeof(*directory_menu);
        callback = popup_directories_selected_cb;
        break;
    case SETTINGS_ID_HWACCELERATION:
        menu = hardware_acceleration_menu;
        len = (int)sizeof(hardware_acceleration_menu) / (int)sizeof(*hardware_acceleration_menu);
        break;
    case SETTINGS_ID_SUBSENC:
        menu = subtitles_text_encoding_menu;
        len = (int)sizeof(subtitles_text_encoding_menu) / (int)sizeof(*subtitles_text_encoding_menu);
        break;
    case SETTINGS_ID_VORIENTATION:
        menu = video_orientation_menu;
        len = (int)sizeof(video_orientation_menu) / (int)sizeof(*video_orientation_menu);
        break;
    case SETTINGS_ID_PERFORMANCES:
        menu = performance_menu;
        len = (int)sizeof(performance_menu) / (int)sizeof(*performance_menu);
        break;
    case SETTINGS_ID_DEBLOCKING:
        menu = deblocking_filter_settings_menu;
        len = (int)sizeof(deblocking_filter_settings_menu) / (int)sizeof(*deblocking_filter_settings_menu);
        break;
    default:
        break;
    }

    if (menu != NULL) {
        popup = elm_popup_add(sd->parent);
        list = create_settings_popup_genlist(popup, menu, len, callback, callback_data);
        elm_object_content_set(popup, list);
        evas_object_show(list);
        evas_object_show(popup);
    }
}

static void
gl_contracted_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    Elm_Object_Item *it = event_info;

    /* Free the genlist subitems when contracted */
    elm_genlist_item_subitems_clear(it);
}

static Evas_Object *
create_setting_list(Evas_Object *parent)
{
    Evas_Object *genlist;
    Elm_Object_Item *hit, *it;
    int n_items = (int) sizeof(settings_menu) / (int) sizeof (*settings_menu);
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
    evas_object_smart_callback_add(genlist, "realized", gl_realized_cb, NULL);
    evas_object_smart_callback_add(genlist, "loaded", gl_loaded_cb, NULL);
    evas_object_smart_callback_add(genlist, "longpressed", gl_longpressed_cb, NULL);
    evas_object_smart_callback_add(genlist, "contracted", gl_contracted_cb, NULL);

    /* Stop when the setting list names is all used */
    for (index = 0; index < n_items; index++) {
        setting_data *sd = calloc(1, sizeof(*sd));

        /* Put the index in the setting_data struct for callbacks */
        sd->index = index;
        sd->id = settings_menu[index].id;

        /* Set and append headers items */
        if (settings_menu[index].type == SETTINGS_TYPE_CATEGORY)
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
        }
        else if (settings_menu[index].type == SETTINGS_TYPE_ITEM)
        {
            /* Set and append settings items */
            sd->index = index;
            it = elm_genlist_item_append(genlist,
                    itc,                           /* genlist item class               */
                    sd,                            /* genlist item class user data     */
                    hit,                           /* genlist parent item              */
                    ELM_GENLIST_ITEM_NONE,         /* genlist item type                */
                    gl_selected_cb,                /* genlist select smart callback    */
                    sd);                           /* genlist smart callback user data */

            /* Put genlist item in the setting_data struct for callbacks */
            sd->parent = parent;
            sd->item = it;
        }
    }

    /* */
    elm_genlist_item_class_free(hitc);
    elm_genlist_item_class_free(itc);

    /* */
    evas_object_show(genlist);

    return genlist;
}

Evas_Object*
create_setting_view(interface *intf, Evas_Object *parent)
{
    return create_setting_list(parent);
}
