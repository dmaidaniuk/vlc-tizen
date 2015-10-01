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
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include "directory_view.h"
#include "ui/interface.h"
#include "ui/utils.h"

void
list_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    directory_data_s *dd = data;

    if (dd->isFile)
    {
        /* Start the playback of the given file */
        intf_create_video_player(dd->dv->p_intf, dd->file_path);
    }
    else
    {
        /* Continue to browse media folder */
        browse(dd->dv, dd->file_path);
    }
}

static void
free_list_item_data(void *data, Evas_Object *obj, void *event_info)
{
    directory_data_s *dd = data;
    /* Free the file path when the current list is deleted */
    /* For example when the player is launched or a new list is created */
    free(dd->file_path);
    free(dd);
}

static int compare_sort_items(const void *data1, const void *data2)
{
	const char *label1, *label2;
	const Elm_Object_Item *li_it1 = data1;
	const Elm_Object_Item *li_it2 = data2;
	const directory_data_s *li_data1, *li_data2;

	label1 = elm_object_item_text_get(li_it1);
	label2 = elm_object_item_text_get(li_it2);

	if (strcmp(label1, "..") == 0) {
		return -1;
	} else if (strcmp(label2, "..") == 0) {
		return 1;
	}

	li_data1 = elm_object_item_data_get(data1);
	li_data2 = elm_object_item_data_get(data2);

	if (!li_data1->isFile && li_data2->isFile)
		return -1;
	else if (li_data1->isFile && !li_data2->isFile)
		return 1;

	return strcasecmp(label1, label2);
}

Evas_Object*
browse(directory_view *dv, const char* path)
{
    char *cpath;
    directory_data_s *dd;
    Evas_Object *file_list;
    DIR* rep = NULL;
    struct dirent* current_folder = NULL;
    struct stat st;
    char tmppath[PATH_MAX];
    bool isFile;

    /* Make a realpath to use a clean path in the function */
    cpath = realpath(path, NULL);

    if (cpath == NULL)
    {
        LOGI("No path");
        return NULL ;
    }

    /* Open the path repository then put it as a dirent variable */
    rep = opendir(cpath);

    if  (rep == NULL)
    {
        char *error;
        error = strerror(errno);
        LOGI("Empty repository or Error due to %s", error);

        if (strcmp(cpath, "/") == 0)
        {
            /* We're already on the root directory don't open the parent directory */
            free(cpath);
            return NULL;
        }

        free(cpath);

        /* Try to open the parent directory */
        sprintf(tmppath, "%s/..", path);
        return browse(dv, tmppath);
    }

    /* Create the box container */
    Evas_Object *box = elm_box_add(dv->p_parent);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(box);

    /* Create the current directory label */
    Evas_Object *directory =  elm_label_add(dv->p_parent);
    elm_object_text_set(directory, cpath);
    elm_box_pack_end(box, directory);
    evas_object_show(directory);

    /* Add a first list append with ".." to go back in directory */
    file_list = elm_list_add(dv->p_parent);
    dd = malloc(sizeof(*dd));
    dd->dv = dv;
    dd->isFile = false;
    asprintf(&dd->file_path, "%s/..", cpath);
    Elm_Object_Item *item = elm_list_item_append(file_list, "..", NULL, NULL, list_selected_cb, dd);
    elm_object_item_del_cb_set(item, free_list_item_data);

    while ((current_folder = readdir(rep)) != NULL)
    {
        if (!current_folder->d_name)
            continue;

        /* Avoid genlist item append for "." and ".." d_name */
	    if (strcmp(current_folder->d_name, ".") == 0 || strcmp(current_folder->d_name, "..") == 0)
        {
            continue;
        }

        /* Concatenate the file path and the selected folder or file name */
        sprintf(tmppath, "%s/%s", cpath, current_folder->d_name);

        if (stat(tmppath, &st) != 0)
            continue;

        if (S_ISREG(st.st_mode))
            isFile = true;
        else if (S_ISDIR(st.st_mode))
            isFile = false;
        else
            continue;

        dd = malloc(sizeof(*dd));
        dd->dv = dv;
        dd->isFile = isFile;
        dd->file_path = strdup(tmppath);

        /* Set and append new item in the list */
        Elm_Object_Item *item = elm_list_item_sorted_insert(file_list, current_folder->d_name, NULL, NULL, list_selected_cb, dd, compare_sort_items);

        /* */
        elm_object_item_del_cb_set(item, free_list_item_data);
    }

    closedir(rep);

    elm_list_go(file_list);

    elm_box_pack_end(box, file_list);
    evas_object_size_hint_weight_set(file_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    /* The next line is required or the list won't show up */
    evas_object_size_hint_align_set(file_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(file_list);

    elm_object_content_set(dv->p_parent, box);
    free(cpath);

    return box;
}

Evas_Object*
create_directory_view(interface *intf, const char* path, Evas_Object *parent)
{
    directory_view *dv = malloc(sizeof(*dv));
    dv->p_intf = intf;
    dv->p_parent = parent;

    return browse(dv, path);
}
