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
#include <storage.h>
#include <app.h>

#include "interface.h"
#include "views/audio_view.h"
#include "audio_player.h"

static void
audio_gl_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_list_data_s *ald = data;
    struct stat sb;
    stat(ald->file_path, &sb);

    if (S_ISREG(sb.st_mode))
    {
        /* Launch the media player */
        create_base_player(ald->gd->mini_player, ald->file_path);
        dlog_print(DLOG_INFO, LOG_TAG, "VLC Player launch");
    }

    else if (S_ISDIR(sb.st_mode))
    {
        /* Continue to browse media folder */
        create_audio_list(ald->file_path, ald->gd);
    }
    else
    {
        dlog_print(DLOG_INFO, LOG_TAG, "This is not a file or a folder");
    }

}

static void
free_list_item_data(void *data, Evas_Object *obj, void *event_info)
{
    audio_list_data_s *ald = data;
    /* Free the file path when the current genlist is deleted */
    /* For example when the player is launched or a new genlist is created */
    free(ald->file_path);
    dlog_print(DLOG_DEBUG, LOG_TAG, "Path free");

}

static Evas_Object*
create_icon(Evas_Object *parent)
{
    char buf[PATH_MAX];
    Evas_Object *img = elm_image_add(parent);

    /* Set the icon file for genlist item class */
    snprintf(buf, sizeof(buf), ICON_DIR"/background_cone.png");
    elm_image_file_set(img, buf, NULL);

    /* The object will align and expand in the space the container will give him */
    evas_object_size_hint_align_set(img, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    return img;
}

static char *
gl_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    audio_list_data_s *ald = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(ald->item);
    char *buf;

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.text.main.left.top")) {
            asprintf(&buf, "<b>%s</b>", ald->str );
            return buf;
        }
        else if (!strcmp(part, "elm.text.sub.left.bottom")) {
            asprintf(&buf,"%s", "Artist");
            return strdup(buf);
        }
        else if (!strcmp(part, "elm.text.sub.right.bottom")) {
            int m = 4;
            int s = 56;
            asprintf(&buf," %d m %d s", m, s);
            return strdup(buf);
        }
    }
    return NULL;
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    audio_list_data_s *ald = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(ald->item);
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

Evas_Object*
create_audio_list(char* path, gui_data_s *gd)
{
    char *buff;
    audio_list_data_s *ald = malloc(sizeof(*ald));
    ald->gd = gd;
    const char *str = NULL;
    DIR* rep = NULL;
    struct dirent* current_folder = NULL;

    /* Set then create the Genlist object */
    Evas_Object *parent = gd->nf_toolbar;
    Evas_Object *genlist;
    Elm_Object_Item *it;
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "2line.top.3";
    itc->func.text_get = gl_text_get_cb;
    itc->func.content_get = gl_content_get_cb;
    dlog_print(DLOG_INFO, LOG_TAG, "Coucou");
    genlist = elm_genlist_add(parent);

    /* Set the genlist scoller mode */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    /* Enable the genlist HOMOGENEOUS mode */
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    /* Enable the genlist COMPRESS mode */
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

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
        audio_list_data_s *ald = malloc(sizeof(*ald));
        ald->gd = gd;

        /* Put the genlist parent in the audio_list_data struct for callbacks */
        ald->parent = parent;

        /* Concatenate the file path and the selected folder or file name */
        asprintf(&ald->file_path, "%s/%s", path, current_folder->d_name);
        /* Put the folder or file name as a usable string for genlist item label */
        str = current_folder->d_name;
        /* Put the folder or file name in the audio_list_data struct for callbacks */
        ald->str = str;

        /* Avoid genlist item append for "." and ".." d_name */
        if (str && (strcmp(str, ".")==0 || strcmp(str, "..")==0))
        {
            continue;
        }

        /* Set and append new item in the genlist */
        it = elm_genlist_item_append(genlist,
                itc,                            /* genlist item class               */
                ald,                            /* genlist item class user data     */
                NULL,                            /* genlist parent item              */
                ELM_GENLIST_ITEM_NONE,            /* genlist item type                */
                audio_gl_selected_cb,            /* genlist select smart callback    */
                ald);                            /* genlist smart callback user data */

        /* Put genlist item in the audio_list_data struct for callbacks */
        ald->item = it;
        /* */
        elm_object_item_del_cb_set(ald->item, free_list_item_data);
    }
    /* */
    elm_genlist_item_class_free(itc);
    return genlist;
}

static int internal_storage_id;
static bool audio_storage_cb(int storage_id, storage_type_e type, storage_state_e state, const char *path, void *user_data)
{
    if (type == STORAGE_TYPE_INTERNAL)
    {
        internal_storage_id = storage_id;
        dlog_print(DLOG_DEBUG, LOG_TAG, "Storage refreshed");
        return false;
    }

    return true;
}

static void
tabbar_item_selected(gui_data_s *gd, Elm_Object_Item *audio_it)
{
    int error;
    char *audio_path;
    const char *str = NULL;
    Evas_Object *current_audio_view;

    /* Get the item selected in the toolbar */
    str = elm_object_item_text_get(audio_it);

    /* Connect to the device storage */
    error = storage_foreach_device_supported(audio_storage_cb, NULL);
    error = storage_get_directory(internal_storage_id, STORAGE_DIRECTORY_MUSIC, &audio_path);

    /* Create the view depending on the item selected in the toolbar */
    if (str && !strcmp(str, "Songs")) {
        current_audio_view = create_audio_list(audio_path, gd);
    }
    else if (str && !strcmp(str, "Artists")) {
        current_audio_view = create_audio_list(audio_path, gd);
    }
    else if (str && !strcmp(str, "Albums")) {
        current_audio_view = create_audio_list(audio_path, gd);
    }
    else     {
        current_audio_view = create_audio_list(audio_path, gd);
    }

    elm_object_content_set(gd->nf_toolbar, current_audio_view );
}

static void
tabbar_item_cb(void *data, Evas_Object *obj, void *event_info)
{
    gui_data_s *gd = data;
    Elm_Object_Item *audio_it = event_info;

    /* Call the function that creates the views */
    tabbar_item_selected(gd, audio_it);
}

static Evas_Object*
create_toolbar(gui_data_s *gd)
{
    /* Create and set the toolbar */
    Evas_Object *tabbar = elm_toolbar_add(gd->nf_toolbar);
    elm_object_style_set(tabbar, "tabbar");

    /* Set the toolbar shrink mode to NONE */
    elm_toolbar_shrink_mode_set(tabbar, ELM_TOOLBAR_SHRINK_SCROLL);
    /* Expand the content to fill the toolbar */
    elm_toolbar_transverse_expanded_set(tabbar, EINA_TRUE);
    /* Items will only call their selection func and callback when first becoming selected*/
    elm_toolbar_select_mode_set(tabbar, ELM_OBJECT_SELECT_MODE_DEFAULT);

    evas_object_size_hint_min_set(tabbar, 450, 400);
    evas_object_size_hint_max_set(tabbar, 450, 400);

    /* Append new entry in the toolbar with the Icon & Label wanted */
    elm_toolbar_item_append(tabbar, NULL, "Songs", tabbar_item_cb, gd);
    elm_toolbar_item_append(tabbar, NULL, "Artists", tabbar_item_cb, gd);
    elm_toolbar_item_append(tabbar, NULL, "Albums", tabbar_item_cb, gd);
    elm_toolbar_item_append(tabbar, NULL, "Playlist", tabbar_item_cb, gd);

    return tabbar;
}

Evas_Object *
create_audio_view(gui_data_s *gd, Evas_Object *parent)
{
    Elm_Object_Item *nf_it, *tabbar_it;
    Evas_Object *tabbar;

    /* Toolbar Naviframe */
    gd->nf_toolbar = elm_naviframe_add(parent);
    evas_object_size_hint_weight_set(gd->nf_toolbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(gd->nf_toolbar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Toolbar Naviframe Settings */
    elm_object_part_content_set(parent, "elm.swallow.content", gd->nf_toolbar);
    evas_object_show(gd->nf_toolbar);
    nf_it = elm_naviframe_item_push(gd->nf_toolbar, NULL, NULL, NULL, NULL, "tabbar/icon/notitle");
    elm_naviframe_item_push(parent, _("<b>Audio</b>"), NULL, NULL, gd->nf_toolbar, "basic");

    /* Create the toolbar in the view */
    tabbar = create_toolbar(gd);
    elm_object_item_part_content_set(nf_it, "tabbar", tabbar);

    /* Set the first item in the toolbar */
    tabbar_it = elm_toolbar_first_item_get(tabbar);
    elm_toolbar_item_selected_set(tabbar_it, EINA_TRUE);

    return gd->nf_toolbar;
}
