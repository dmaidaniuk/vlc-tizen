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

struct view_sys {
    interface *p_intf;
    Evas_Object *p_box;
    char current_path[PATH_MAX];
};

typedef struct directory_data {
    view_sys *dv;
    char *file_path;
    bool is_file;
} directory_data;

bool
browse(view_sys *dv, const char* path);

void
list_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    directory_data *dd = data;

    if (dd->is_file)
    {
        /* Start the playback of the given file */
        intf_video_player_play(dd->dv->p_intf, dd->file_path);
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
    directory_data *dd = data;
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
	const directory_data *li_data1, *li_data2;

	label1 = elm_object_item_text_get(li_it1);
	label2 = elm_object_item_text_get(li_it2);

	li_data1 = elm_object_item_data_get(data1);
	li_data2 = elm_object_item_data_get(data2);

	if (!li_data1->is_file && li_data2->is_file)
		return -1;
	else if (li_data1->is_file && !li_data2->is_file)
		return 1;

	return strcasecmp(label1, label2);
}

bool
browse(view_sys *dv, const char* path)
{
    Elm_Object_Item *item;
    Evas_Object *file_list;
    directory_data *dd;
    DIR* rep = NULL;
    struct dirent* current_folder = NULL;
    struct stat st;
    char tmppath[PATH_MAX];
    char *cpath;
    bool is_file;

    /* Make a realpath to use a clean path in the function */
    cpath = realpath(path, NULL);

    if (cpath == NULL)
    {
        LOGE("realpath failed");
        return false;
    }
    else if (MAX(strlen(path), strlen(cpath)) + 1 > PATH_MAX - 1)
    {
        free(cpath);
        LOGE("Given path exceeds the maximum length of %d", PATH_MAX);
        return false;
    }

    strcpy(dv->current_path, cpath);

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
            return false;
        }

        free(cpath);

        /* Try to open the parent directory */
        if (strlen(path) + 1 + 3 > PATH_MAX - 1)
            return false;
        sprintf(tmppath, "%s/..", path);
        return browse(dv, tmppath);
    }

    /* Clear the layout */
    elm_box_clear(dv->p_box);

    Evas_Object *label_layout = elm_layout_add(dv->p_box);
    elm_layout_file_set(label_layout, BASICTEXTEDJ, "basic_text");
    evas_object_size_hint_align_set(label_layout, EVAS_HINT_FILL, 0.0);
    evas_object_show(label_layout);

    /* Create the current directory label */
    Evas_Object *directory =  elm_label_add(dv->p_box);
    elm_object_text_set(directory, cpath);
    elm_object_part_content_set(label_layout, "swallow.main", directory);

    elm_box_pack_end(dv->p_box, label_layout);
    evas_object_show(directory);

    /* Create the list */
    file_list = elm_list_add(dv->p_box);

    /* Browse the current directory */
    while ((current_folder = readdir(rep)) != NULL)
    {
        if (!current_folder->d_name)
            continue;

        /* Avoid genlist item append for "." and ".." d_name */
	    if (strcmp(current_folder->d_name, ".") == 0 || strcmp(current_folder->d_name, "..") == 0)
        {
            continue;
        }

	    char *file_path;

        /* Concatenate the file path and the selected folder or file name */
        asprintf(&file_path, "%s/%s", cpath, current_folder->d_name);

        if (stat(file_path, &st) != 0)
        {
            free(file_path);
            continue;
        }

        if (S_ISREG(st.st_mode))
            is_file = true;
        else if (S_ISDIR(st.st_mode))
            is_file = false;
        else
        {
            free(file_path);
            continue;
        }

        dd = malloc(sizeof(*dd));
        dd->dv = dv;
        dd->is_file = is_file;
        dd->file_path = file_path;

        /* Set and append new item in the list */
        item = elm_list_item_sorted_insert(file_list, current_folder->d_name, NULL, NULL, list_selected_cb, dd, compare_sort_items);

        /* */
        elm_object_item_del_cb_set(item, free_list_item_data);
    }
    closedir(rep);

    /* Prepend the '..' item */
    dd = malloc(sizeof(*dd));
    dd->dv = dv;
    dd->is_file = false;
    asprintf(&dd->file_path, "%s/..", cpath);
    item = elm_list_item_prepend(file_list, "..", NULL, NULL, list_selected_cb, dd);
    elm_object_item_del_cb_set(item, free_list_item_data);

    elm_list_go(file_list);

    elm_box_pack_end(dv->p_box, file_list);
    evas_object_size_hint_weight_set(file_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    /* The next line is required or the list won't show up */
    evas_object_size_hint_align_set(file_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(file_list);

    free(cpath);

    return true;
}

static bool
directory_event(view_sys *p_view_sys, interface_view_event event)
{
    if(event == INTERFACE_VIEW_EVENT_BACK && strcmp(p_view_sys->current_path, "/") != 0)
    {
        char *parent_dir;
        if (asprintf(&parent_dir, "%s/..", p_view_sys->current_path) == -1)
            return false;
        browse(p_view_sys, parent_dir);
        free(parent_dir);
        return true;
    }
    return false;
}

interface_view*
create_directory_view(interface *intf, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));

    view_sys *dv = malloc(sizeof(*dv));
    dv->p_intf = intf;
    view->p_view_sys = dv;
    view->pf_event = directory_event;

    /* Create the box container */
    Evas_Object *box = elm_box_add(parent);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(box);

    view->view = dv->p_box = box;

    const char *psz_path = application_get_media_path(intf_get_application(intf), MEDIA_DIRECTORY);
    browse(dv, psz_path);
    return view;
}

void
destroy_directory_view(interface_view *view)
{
    free(view->p_view_sys);
    free(view);
}
