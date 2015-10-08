/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Nicolas Rechatin <nicolas@videolabs.io>
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

#include "settings.h"
#include "ui/utils.h"
#include <app_preference.h>

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
settings_toggle_set_one_by_id(settings_item *menu, int menu_len, menu_id id, bool value, bool toggle_others)
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

static void
gl_del_cb(void *data, Evas_Object *obj, void *event_info)
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
        item_cb(&sd->selected, sd->p_view_sys, sd->data, sd->parent);

    if (sd->global_cb != NULL)
        sd->global_cb(&sd->selected, sd->p_view_sys, sd->data, sd->parent);
}

static void
gl_contracted_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    Elm_Object_Item *it = event_info;

    /* Free the genlist subitems when contracted */
    elm_genlist_item_subitems_clear(it);
}

Evas_Object *
settings_list_add(settings_item *menu, int len, Settings_menu_callback global_menu_cb, void *data, view_sys* p_view_sys, Evas_Object *parent)
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

    /* Set the settings item class */
    itc->item_style = "1line";
    itc->func.text_get = gl_text_get_cb;
    itc->func.content_get = gl_content_get_cb;

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
        sd->data = data;

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
            sd->item = sd->selected.item = hit;
            elm_object_item_del_cb_set(hit, gl_del_cb);
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
            sd->parent = genlist;
            sd->item = sd->selected.item = it;
            elm_object_item_del_cb_set(it, gl_del_cb);
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
popup_close_cb(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
}

Evas_Object *
settings_popup_add(settings_item *menu, int menu_len, Settings_menu_callback global_menu_cb, void *data, view_sys* p_view_sys, Evas_Object *parent)
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

    genlist = settings_list_add(menu, menu_len, global_menu_cb, data, p_view_sys, popup);

    elm_box_pack_end(box, genlist);

    evas_object_smart_callback_add(popup, "block,clicked", popup_close_cb, NULL);

    elm_object_content_set(popup, box);
    evas_object_show(genlist);

    return popup;
}
