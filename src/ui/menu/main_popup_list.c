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

#include <Elementary.h>
#include <efl_extension.h>

#include "main_popup_list.h"

#include "ui/utils.h"

enum
{
    ACTION_REFRESH,
    ACTION_EQUALIZER,
    ACTION_MAX,
};

typedef struct general_popup_data
{
    int index;
    Evas_Object *box, *genlist;
    Elm_Object_Item *item;

} general_popup_data_s;

typedef struct action_item
{
    const char* title;
    const char* icon;

} action_item_s;

action_item_s action_items[] =
{
        { "Refresh", "ic_repeat_normal.png" },
        { "Equalizer", "ic_equalizer_normal.png" },
};

static char *
gl_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    general_popup_data_s *gpd = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(gpd->item);

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.text.main.left")) {
            return strdup(action_items[gpd->index].title);
        }
    }
    return NULL;
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    general_popup_data_s *gpd = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(gpd->item);
    Evas_Object *content = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.icon.1")) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
            Evas_Object *icon = create_image(content, action_items[gpd->index].icon);
            elm_layout_content_set(content, "elm.swallow.content", icon);
        }
    }

    return content;
}

static void
popup_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    general_popup_data_s *gpd = data;
    /* Generate the view depending on which panel genlist item is selected */
    switch(gpd->index){

    case ACTION_REFRESH:
        //evas_object_del(gpd->intf->popup);

        //TODO : Add a refresh function

        free(gpd);
        break;

    case ACTION_EQUALIZER:
        //evas_object_del(gpd->intf->popup);

        //TODO : Add an equalizer function of the current list

        free(gpd);
        break;
    case ACTION_MAX:
    default:
        break;
    }
}

static void
popup_block_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *popup = data;
    evas_object_del(popup);
}

Evas_Object *
create_popup_genlist(Evas_Object* popup)
{

    Evas_Object *genlist;
    Elm_Object_Item *it;

    Evas_Object *box = elm_box_add(popup);
    evas_object_size_hint_min_set(box, 200, 200);
    evas_object_size_hint_max_set(box, 200, 200);

    /* Set then create the Genlist object */
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "1line";
    itc->func.text_get = gl_text_get_cb;
    itc->func.content_get = gl_content_get_cb;

    genlist = elm_genlist_add(box);

    /* Set the genlist scoller mode */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    /* Enable the genlist HOMOGENEOUS mode */
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    /* Enable the genlist COMPRESS mode */
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    /* Stop when the panel list names is all used */
    for (int index = 0; index < ACTION_MAX; index++) {

        general_popup_data_s *gpd = malloc(sizeof(*gpd));
        /* Put the index and the gui_data in the cb_data struct for callbacks */
        gpd->index = index;

        it = elm_genlist_item_append(genlist,
                itc,                            /* genlist item class               */
                gpd,                            /* genlist item class user data     */
                NULL,                           /* genlist parent item              */
                ELM_GENLIST_ITEM_NONE,          /* genlist item type                */
                popup_selected_cb,              /* genlist select smart callback    */
                gpd);                           /* genlist smart callback user data */

        gpd->item = it;
    }

    elm_box_pack_end(box, genlist);
    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_smart_callback_add(popup, "block,clicked", popup_block_cb, popup);
    evas_object_show(genlist);

    elm_genlist_item_class_free(itc);

    return box;
}

Evas_Object *
create_popup(Evas_Object *parent, interface *intf)
{
    Evas_Object *popup_list;
    Evas_Object *popup = elm_popup_add(parent);
    //elm_object_style_set(popup, style);

    evas_object_show(popup);
    evas_object_size_hint_min_set(popup, 200, 200);
    evas_object_size_hint_max_set(popup, 200, 200);

    /* Add the panel genlist in the panel */
    popup_list = create_popup_genlist(popup);
    elm_object_content_set(popup, popup_list);
    evas_object_show(popup_list);

    /* */
    //evas_object_smart_callback_add(popup, "clicked", cancel_cb, intf); // FIXME

    /* Callback for the back key */
    eext_object_event_callback_add(popup, EEXT_CALLBACK_LAST, eext_popup_back_cb, NULL);

    evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    return popup;
}
