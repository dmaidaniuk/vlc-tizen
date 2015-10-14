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

#include "popup_menu.h"
#include "ui/utils.h"

typedef struct popup_menu_internal
{
    int index;
    popup_menu *menu;
    void *data;

    Evas_Object *popup;
    Elm_Object_Item *item;
} popup_menu_internal;

static char *
popup_menu_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    popup_menu_internal *pmi = data;

    if (!pmi || !pmi->item || !part)
        return NULL;

    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(pmi->item);

    if (!itc)
        return NULL;

    /* Put this string as the genlist setting item label */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.text.main.left")) {
            return strdup(pmi->menu[pmi->index].title);
        }
    }
    return NULL;
}

static Evas_Object*
popup_menu_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    popup_menu_internal *pmi = data;

    if (!pmi || !pmi->item || !part)
        return NULL;

    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(pmi->item);

    if (!itc)
        return NULL;

    Evas_Object *content = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.icon.1") && pmi->menu[pmi->index].icon != NULL) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
            Evas_Object *icon = create_icon(content, pmi->menu[pmi->index].icon);
            elm_layout_content_set(content, "elm.swallow.content", icon);
        }
    }

    return content;
}

static void
popup_menu_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    popup_menu_internal *pmi = data;

    pmi->menu[pmi->index].cb(pmi->data, pmi->popup, event_info);
}

static void
popup_menu_del_cb(void *data, Evas_Object *obj, void *event_info)
{
    popup_menu_internal *pmi = data;
    free(pmi);
}

static void
popup_menu_close_cb(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
}

Evas_Object *
popup_menu_add(popup_menu *menu, void *data, Evas_Object *parent)
{
    return popup_menu_orient_add(menu, ELM_POPUP_ORIENT_BOTTOM, data, parent);
}

Evas_Object *
popup_menu_orient_add(popup_menu *menu, Elm_Popup_Orient orient, void *data, Evas_Object *parent)
{
    Evas_Object *popup = elm_popup_add(parent);
    Evas_Object *box = elm_box_add(popup);
    Elm_Object_Item *it, *hit = NULL;
    Evas_Object *genlist;
    int index;

    elm_popup_orient_set(popup, orient);

    /* */
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();

    /* Set the settings item class */
    itc->item_style = "1line";
    itc->func.text_get = popup_menu_text_get_cb;
    itc->func.content_get = popup_menu_content_get_cb;

    /* */
    genlist = elm_genlist_add(parent);
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    for (index = 0; menu[index].title != NULL; index++) {
        popup_menu_internal *pmi = calloc(1, sizeof(*pmi));

        pmi->index = index;
        pmi->menu = menu;
        pmi->data = data;
        pmi->popup = popup;

        it = elm_genlist_item_append(genlist,
                itc,                           /* genlist item class               */
                pmi,                           /* genlist item class user data     */
                hit,                           /* genlist parent item              */
                ELM_GENLIST_ITEM_NONE,         /* genlist item type                */
                popup_menu_selected_cb,        /* genlist select smart callback    */
                pmi);                          /* genlist smart callback user data */

        pmi->item = it;

        /* Put genlist item in the settings_internal_data struct for callbacks */
        elm_object_item_del_cb_set(it, popup_menu_del_cb);
    }

    /* */
    elm_genlist_item_class_free(itc);

    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);


    /* Set the popup Y axis value */
    if (index < 6) {
        evas_object_size_hint_min_set(box, EVAS_HINT_FILL, index * 100);
        evas_object_size_hint_max_set(box, EVAS_HINT_FILL, index * 100);
    } else {
        evas_object_size_hint_min_set(box, EVAS_HINT_FILL, 500);
        evas_object_size_hint_max_set(box, EVAS_HINT_FILL, 500);
    }

    elm_box_pack_end(box, genlist);

    evas_object_smart_callback_add(popup, "block,clicked", popup_menu_close_cb, NULL);

    elm_object_content_set(popup, box);
    evas_object_show(genlist);

    return popup;
}
