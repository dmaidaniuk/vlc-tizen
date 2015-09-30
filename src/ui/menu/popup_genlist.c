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
#include <efl_extension.h>

#include "ui/interface.h"

#include "ui/utils.h"

#include "popup_genlist.h"


typedef struct popup_genlist_data
{
    Evas_Object *parent;

    popup_menu_item_s *menu_item;
    int index;
    int nb_item;

    Elm_Object_Item *item;
} popup_genlist_data_s;

static char *
gl_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    popup_genlist_data_s *pgd = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(pgd->item);

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.text.main.left")) {
            return strdup(pgd->menu_item[pgd->index].title);
        }
    }
    return NULL;
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    popup_genlist_data_s *pgd = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(pgd->item);
    Evas_Object *content = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.icon.right")) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/A/right.icon", "default");
            Evas_Object *icon = create_image(pgd->parent, pgd->menu_item[pgd->index].icon);
            elm_layout_content_set(content, "elm.swallow.content", icon);
        }
    }

    return content;
}


static void
popup_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    popup_genlist_data_s *pgd = data;
    int count;

    /* Change the icon depending on the setting state (pressed or not) */

    if(strcmp (pgd->menu_item[pgd->index].icon, "toggle_on.png") == 0)
    {
        pgd->menu_item[pgd->index].icon = "toggle_off.png";
    }

    else if(strcmp (pgd->menu_item[pgd->index].icon, "toggle_off.png") == 0)
    {
        for(count = 0; count < pgd->nb_item; count ++)
        {
            if (strcmp(pgd->menu_item[count].icon, "toggle_on.png") == 0)
            {
                pgd->menu_item[count].icon = "toggle_off.png";
            }
        }

        pgd->menu_item[pgd->index].icon = "toggle_on.png";
    }

    /* */
    evas_object_del(pgd->parent);
}

Evas_Object *
create_settings_popup_genlist(Evas_Object *parent, popup_menu_item_s *directory_menu, int nb_item)
{
    Evas_Object *genlist;
    Elm_Object_Item *it;
    Evas_Object *box = elm_box_add(parent);

    /* Set the popup Y axis value */
    if(nb_item < 6){
        evas_object_size_hint_min_set(box, EVAS_HINT_FILL, nb_item*100);
        evas_object_size_hint_max_set(box, EVAS_HINT_FILL, nb_item*100);
    }

    else{
        evas_object_size_hint_min_set(box, EVAS_HINT_FILL, 500);
        evas_object_size_hint_max_set(box, EVAS_HINT_FILL, 500);
    }

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
    for (int index = 0; index < nb_item; index++) {

        popup_genlist_data_s *pgd = malloc(sizeof(*pgd));
        /* Put the index and the gui_data in the cb_data struct for callbacks */
        pgd->index = index;
        pgd->parent = parent;
        pgd->menu_item = directory_menu;
        pgd->nb_item = nb_item;

        it = elm_genlist_item_append(genlist,
                itc,                            /* genlist item class               */
                pgd,                            /* genlist item class user data     */
                NULL,                            /* genlist parent item              */
                ELM_GENLIST_ITEM_NONE,            /* genlist item type                */
                popup_selected_cb,                /* genlist select smart callback    */
                pgd);                            /* genlist smart callback user data */

        eext_object_event_callback_add(pgd->parent, EEXT_CALLBACK_BACK, popup_selected_cb, pgd);
        pgd->item = it;
    }


    elm_box_pack_end(box, genlist);
    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(genlist);

    elm_genlist_item_class_free(itc);

    return box;
}
