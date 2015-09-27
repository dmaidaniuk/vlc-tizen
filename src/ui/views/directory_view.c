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

#include "directory_view.h"

typedef struct directory_data {
    Evas_Object *parent;
    char *file_path;
} directory_data_s;

void
list_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    directory_data_s *dd= data;
    struct stat sb;
    stat(dd->file_path, &sb);

    if (S_ISREG(sb.st_mode)){

        /* Launch the media player */
        dlog_print(DLOG_INFO, LOG_TAG, "VLC Player launch");
        dlog_print(DLOG_INFO, LOG_TAG, "Won't play the video at the time. Will be soon");

    }

    else if (S_ISDIR(sb.st_mode))
    {
        /* Continue to browse media folder */
        create_directory_view(dd->file_path, dd->parent);
    }

}

static void
free_list_item_data(void *data, Evas_Object *obj, void *event_info)
{
    directory_data_s *dd = data;
    /* Free the file path when the current list is deleted */
    /* For example when the player is launched or a new list is created */
    free(dd->file_path);
    dlog_print(DLOG_DEBUG, LOG_TAG, "Path free");

}

Evas_Object*
create_directory_view(char* path, Evas_Object *parent)
{
    char *buff;
    directory_data_s *dd = malloc(sizeof(*dd));
    const char *str = NULL;
    Evas_Object *file_list;
    DIR* rep = NULL;
    struct dirent* current_folder = NULL;

    /* Make a realpath to use a clean path in the function */
    buff = realpath(path, NULL);
    path = buff;

    if (path == NULL)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "No path");
        return NULL ;
    }

    /* Open the path repository then put it as a dirent variable */
    rep = opendir(path);

    if  (rep == NULL)
    {
        char *error;
        error = strerror(errno);
        dlog_print(DLOG_INFO, LOG_TAG, "Empty repository or Error due to %s", error);

        return NULL ;
    }

    /* Add a first list append with ".." to go back in directory */
    file_list = elm_list_add(parent);
    dd->parent = parent;
    asprintf(&dd->file_path, "%s/..", path);
    Elm_Object_Item *item = elm_list_item_append(file_list, dd->file_path, NULL, NULL, list_selected_cb, dd);
    elm_object_item_del_cb_set(item, free_list_item_data);

    while ((current_folder = readdir(rep)) != NULL)
    {
        directory_data_s *dd = malloc(sizeof(*dd));
        /* Put the list parent in the directory_data struct for callbacks */
        dd->parent = parent;

        /* Concatenate the file path and the selected folder or file name */
        asprintf(&dd->file_path, "%s/%s", path, current_folder->d_name);
        /* Put the folder or file name as a usable string for list item label */
        str = current_folder->d_name;

        /* Avoid genlist item append for "." and ".." d_name */
        if (str && (strcmp(str, ".") == 0 || strcmp(str, "..") == 0))
        {
            continue;
        }

        /* Set and append new item in the list */
        Elm_Object_Item *item = elm_list_item_append(file_list, str, NULL, NULL, list_selected_cb, dd);
        /* */
        elm_object_item_del_cb_set(item, free_list_item_data);
    }

    /* */
    evas_object_show(file_list);
    elm_object_content_set(parent, file_list);

    return file_list;
}
