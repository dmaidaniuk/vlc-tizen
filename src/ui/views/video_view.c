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

#include "ui/interface.h"
#include "video_view.h"
#include "video_player.h"

void
video_gl_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    video_list_data_s *vld= data;

    struct stat sb;
    stat(vld->file_path, &sb);

    if (S_ISREG(sb.st_mode)){

        /* Launch the media player */
        Evas_Object *video_player = create_video_gui(vld->parent, vld->file_path);
        elm_object_content_set(vld->parent, video_player);
        evas_object_show(video_player);
        /* */
        dlog_print(DLOG_INFO, LOG_TAG, "VLC Player launch");
        dlog_print(DLOG_INFO, LOG_TAG, "Won't play the video at the time. Will be soon");
    }

    else if (S_ISDIR(sb.st_mode))
    {
        create_video_view(vld->file_path, vld->parent);
    }

}

static void
free_list_item_data(void *data, Evas_Object *obj, void *event_info)
{
    video_list_data_s *vld = data;
    /* Free the file path when the current genlist is deleted */
    /* For example when the player is launched or a new genlist is created */
    free(vld->file_path);
    dlog_print(DLOG_DEBUG, LOG_TAG, "Path free");

}

static Evas_Object*
create_icon(Evas_Object *parent)
{
    char buf[PATH_MAX];
    Evas_Object *img = elm_image_add(parent);

    /* Set the icon file for genlist item class */
    snprintf(buf, sizeof(buf), ICON_DIR "background_cone.png");
    elm_image_file_set(img, buf, NULL);
    /* The object will align and expand in the space the container will give him */
    evas_object_size_hint_align_set(img, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    return img;
}

static char *
gl_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    video_list_data_s *vld = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(vld->item);
    char buf[PATH_MAX];

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.text.main.left.top")) {
            snprintf(buf, 1023, "<b>%s</b>", vld->str );
            return strdup(buf);
        }
        else if (!strcmp(part, "elm.text.sub.left.bottom")) {
            snprintf(buf, 1023, "%s", "01h 32m 03s");
            return strdup(buf);
        }
        else if (!strcmp(part, "elm.text.sub.right.bottom")) {
            int w = 1920;
            int h = 1080;
            snprintf(buf, 1023, "%dx%d", w, h);
            return strdup(buf);
        }
    }
    return NULL;
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    video_list_data_s *vld = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(vld->item);
    Evas_Object *content = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.icon.1")) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/B/type.1", "default");
            Evas_Object *icon = create_icon(content);
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
gl_contracted_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    Elm_Object_Item *it = event_info;

    /* Free the genlist subitems when contracted */
    elm_genlist_item_subitems_clear(it);
}


Evas_Object*
create_video_view(char* path, Evas_Object *parent)
{
    char *buff;
    video_list_data_s *vld = malloc(sizeof(*vld));
    const char *str = NULL;
    DIR* rep = NULL;
    struct dirent* current_folder = NULL;

    /* Set then create the Genlist object */
    Evas_Object *genlist;
    Elm_Object_Item *it;
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "2line.top.3";
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
    evas_object_smart_callback_add(genlist, "realized", gl_realized_cb, NULL);
    evas_object_smart_callback_add(genlist, "loaded", gl_loaded_cb, NULL);
    evas_object_smart_callback_add(genlist, "longpressed", gl_longpressed_cb, NULL);
    evas_object_smart_callback_add(genlist, "contracted", gl_contracted_cb, NULL);

    /* Make a realpath to use a clean path in the function */
    buff = realpath(path, NULL);
    path = buff;

    /* Open the path repository then put it as a dirent variable */
    rep = opendir(path);

    if (path == NULL)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "No path");
        return NULL ;
    }

    if  (rep == NULL)
    {
        char *error;
        error = strerror(errno);
        dlog_print(DLOG_INFO, LOG_TAG, "Empty repository or Error due to %s", error);

        return NULL ;
    }

    /* Stop when the readdir have red the all repository */
    while ((current_folder = readdir(rep)) != NULL)
    {
        video_list_data_s *vld = malloc(sizeof(*vld));

        /* Put the genlist parent in the video_list_data struct for callbacks */
        vld->parent = parent;

        /* Concatenate the file path and the selected folder or file name */
        asprintf(&vld->file_path, "%s/%s", path, current_folder->d_name);
        /* Put the folder or file name as a usable string for genlist item label */
        str = current_folder->d_name;
        /* Put the folder or file name in the video_list_data struct for callbacks */
        vld->str = str;

        /* Avoid genlist item append for "." and ".." d_name */
        if (str && (strcmp(str, ".")==0 || strcmp(str, "..")==0))
        {
            continue;
        }

        /* Set and append new item in the genlist */
        it = elm_genlist_item_append(genlist,
                itc,                            /* genlist item class               */
                vld,                            /* genlist item class user data     */
                NULL,                            /* genlist parent item              */
                ELM_GENLIST_ITEM_NONE,            /* genlist item type                */
                video_gl_selected_cb,            /* genlist select smart callback    */
                vld);                            /* genlist smart callback user data */

        /* Put genlist item in the video_list_data struct for callbacks */
        vld->item = it;
        /* */
        elm_object_item_del_cb_set(vld->item, free_list_item_data);
    }
    /* */
    elm_genlist_item_class_free(itc);
    return genlist;
}





