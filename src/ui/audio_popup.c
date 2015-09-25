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
#include "interface.h"
#include "audio_popup.h"
#include "audio_player.h"

#include <Elementary.h>

/* Set the panel list labels */
const char *audio_popup_list[] = {
        "Jump to Time", "PLayback Speed", "Sleep in"
};

/* Set the panel list icons */
const char *audio_popup_icon_names[] = {
        "jumpto", "speed", "sleep"
};

static Evas_Object*
create_icon(Evas_Object *parent, int count)
{
    char buf[PATH_MAX];
    Evas_Object *img = elm_image_add(parent);

    /* Create then set panel genlist used icones */
    snprintf(buf, sizeof(buf), "%s/ic_%s_normal.png", ICON_DIR, audio_popup_icon_names[count]);
    elm_image_file_set(img, buf, NULL);

    /* The object will align and expand in the space the container will give him */
    evas_object_size_hint_align_set(img, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    return img;
}

static char *
gl_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    audio_popup_data_s *apd = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(apd->item);
    char *buf;

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.text.main.left")) {
            asprintf(&buf, "%s", audio_popup_list[apd->index]);

            return buf;
        }
    }
    return NULL;
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    audio_popup_data_s *apd = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(apd->item);
    Evas_Object *content = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.icon.1")) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
            Evas_Object *icon = create_icon(content, apd->index);
            elm_layout_content_set(content, "elm.swallow.content", icon);
        }
    }

    return content;
}


static void
popup_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_popup_data_s *apd = data;
    /* Generate the view depending on which panel genlist item is selected */
    switch(apd->index){

    case 0:
        /* */
        elm_popup_timeout_set(apd->mpd->popup, 0.1);
        /* */
        elm_image_file_set(apd->mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(apd->mpd->fs_more_btn);
        /* Update the button state (pressed or not) */
        apd->mpd->more_state = false;

        //TODO : Add a JumTo fcn to the current list

        free(apd);
        break;

    case 1:
        /* */
        elm_popup_timeout_set(apd->mpd->popup, 0.1);
        /* */
        elm_image_file_set(apd->mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(apd->mpd->fs_more_btn);
        /* Update the button state (pressed or not) */
        apd->mpd->more_state = false;

        //TODO : Add a Playback Speed fcn of the current list

        free(apd);
        break;

    case 2:
        /* */
        elm_popup_timeout_set(apd->mpd->popup, 0.1);
        /* */
        elm_image_file_set(apd->mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(apd->mpd->fs_more_btn);
        /* Update the button state (pressed or not) */
        apd->mpd->more_state = false;

        //TODO : Add a Sleep fcn of the current list

        free(apd);
    }
}

Evas_Object *
create_audio_popup(mini_player_data_s *mpd)
{

    Evas_Object *genlist;
    Elm_Object_Item *it;

    /* Set popup max size */
    Evas_Object *box = elm_box_add(mpd->popup);
    evas_object_size_hint_min_set(box, 300, 300);
    evas_object_size_hint_max_set(box, 300, 300);

    /* Set then create the Genlist object */
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "1line";
    itc->func.text_get = gl_text_get_cb;
    itc->func.content_get = gl_content_get_cb;

    genlist = elm_genlist_add(box);

    audio_popup_data_s *apd = malloc(sizeof(*apd));
    apd->box = box;
    apd->genlist = genlist;

    /* Set the genlist scoller mode */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    /* Enable the genlist HOMOGENEOUS mode */
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    /* Enable the genlist COMPRESS mode */
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    free(apd);

    /* Stop when the panel list names is all used */
    for (int index = 0; index < 3; index++) {

        audio_popup_data_s *apd = malloc(sizeof(*apd));
        /* Put the index and the gui_data in the cb_data struct for callbacks */
        apd->index = index;
        apd->mpd = mpd;

        it = elm_genlist_item_append(genlist,
                itc,                            /* genlist item class               */
                apd,                            /* genlist item class user data     */
                NULL,                            /* genlist parent item              */
                ELM_GENLIST_ITEM_NONE,            /* genlist item type                */
                popup_selected_cb,                /* genlist select smart callback    */
                apd);                            /* genlist smart callback user data */

        apd->item = it;
    }

    elm_box_pack_end(box, genlist);
    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(genlist);

    elm_genlist_item_class_free(itc);

    return box;
}
