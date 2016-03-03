/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
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
#include "system_storage.h"

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
        intf_video_player_play(dd->dv->p_intf, dd->file_path, 0);
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
browse_directory(view_sys *dv, const char* path);

void
browse_main(view_sys *dv, const char *path_internal, Eina_List *path_external);

bool
browse(view_sys *dv, const char* path)
{
    char *cpath;
    bool ret = false;

    /* Use realpath to get the canonicalized absolute path */
    cpath = realpath(path, NULL);

    if (cpath == NULL)
    {
        LOGE("realpath failed");
        return false;
    }

    /* Path the the internal memory */
    const char *internal = application_get_media_path(intf_get_application(dv->p_intf), MEDIA_DIRECTORY);
    /* List of paths to the external memory */
    Eina_List *ext = media_storage_external_list_get(application_get_media_storage(intf_get_application(dv->p_intf)));

    if (strncmp(cpath, internal, strlen(internal)) == 0)
    {
        /* Valid path to the internal memory */
        ret = browse_directory(dv, cpath);
    }
    else
    {
        Eina_List *l;
        char *path;

        EINA_LIST_FOREACH(ext, l, path)
        {
            if (strncmp(cpath, path, strlen(path)) == 0)
            {
                /* Valid path to an external memory */
                ret = browse_directory(dv, cpath);
                break;
            }
        }
    }

    if (!ret)
    {
        ret = !ret;
        /* Show the entry points (internal and external memory) */
        browse_main(dv, internal, ext);
    }

    eina_list_free(ext);
    free(cpath);
    return ret;
}

directory_data*
new_directory_data(view_sys *dv, const char* path)
{
    directory_data *dd = malloc(sizeof(*dd));
    dd->dv = dv;
    dd->is_file = false;
    dd->file_path = strdup(path);

    return dd;
}

void
browse_main(view_sys *dv, const char *path_internal, Eina_List *path_external)
{
    Evas_Object *file_list;
    directory_data *dd;
    Elm_Object_Item *item;

    // Set the current path to 'main'
    strcpy(dv->current_path, "main");

    /* Clear the layout */
    elm_box_clear(dv->p_box);

    /* Create the list */
    file_list = elm_list_add(dv->p_box);

    // Item: User's media directory
    dd = new_directory_data(dv, path_internal);

    /* Set and append new item in the list */
    item = elm_list_item_append(file_list, "Internal memory", NULL, NULL, list_selected_cb, dd);

    elm_object_item_del_cb_set(item, free_list_item_data);

    Eina_List *l;
    char *path;

    EINA_LIST_FOREACH(path_external, l, path)
    {
        dd = new_directory_data(dv, path);
        /* Set and append new item in the list */
        item = elm_list_item_append(file_list, "External memory", NULL, NULL, list_selected_cb, dd);
        elm_object_item_del_cb_set(item, free_list_item_data);
    }

    /* */

    elm_list_go(file_list);

    elm_box_pack_end(dv->p_box, file_list);
    evas_object_size_hint_weight_set(file_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    /* The next line is required or the list won't show up */
    evas_object_size_hint_align_set(file_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(file_list);
}

bool
browse_directory(view_sys *dv, const char* path)
{
    Elm_Object_Item *item;
    Evas_Object *file_list;
    directory_data *dd;
    DIR* rep = NULL;
    struct dirent* current_folder = NULL;
    struct stat st;
    char tmppath[PATH_MAX];
    bool is_file;

    if (path == NULL)
    {
        LOGE("browse_directory: given path is null");
        return false;
    }
    else if (strlen(path) + 1 > PATH_MAX - 1)
    {
        LOGE("Given path exceeds the maximum length of %d", PATH_MAX);
        return false;
    }

    strcpy(dv->current_path, path);

    /* Open the path repository then put it as a dirent variable */
    rep = opendir(path);

    if  (rep == NULL)
    {
        char *error;
        error = strerror(errno);
        LOGI("Empty repository or Error due to %s", error);

        if (strcmp(path, "/") == 0)
        {
            /* We're already on the root directory don't open the parent directory */
            return false;
        }

        /* Try to open the parent directory */
        if (strlen(path) + 1 + 3 > PATH_MAX - 1)
            return false;
        sprintf(tmppath, "%s/..", path);
        return browse(dv, tmppath);
    }

    /* Clear the layout */
    elm_box_clear(dv->p_box);

    /* Create the current directory label */
    Evas_Object *directory =  elm_label_add(dv->p_box);
    elm_object_text_set(directory, path);
    elm_box_pack_end(dv->p_box, directory);
    evas_object_show(directory);

    /* Create the list */
    file_list = elm_list_add(dv->p_box);

    /* Browse the current directory */
    while ((current_folder = readdir(rep)) != NULL)
    {
        /* Avoid genlist item append for "." and ".." d_name */
	    if (strcmp(current_folder->d_name, ".") == 0 || strcmp(current_folder->d_name, "..") == 0)
        {
            continue;
        }

	    char *file_path;

        /* Concatenate the file path and the selected folder or file name */
        asprintf(&file_path, "%s/%s", path, current_folder->d_name);

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

    /* */
    elm_list_go(file_list);

    elm_box_pack_end(dv->p_box, file_list);
    evas_object_size_hint_weight_set(file_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    /* The next line is required or the list won't show up */
    evas_object_size_hint_align_set(file_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_show(file_list);

    return true;
}

static bool
directory_event(view_sys *p_view_sys, interface_view_event event)
{
    if(event == INTERFACE_VIEW_EVENT_BACK && strcmp(p_view_sys->current_path, "main") != 0)
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

    /* Create layout and set the theme */
    Evas_Object *layout = elm_layout_add(parent);
    elm_layout_theme_set(layout, "layout", "application", "default");

    /* Create the background */
    Evas_Object *bg = elm_bg_add(layout);
    elm_bg_color_set(bg, 255, 255, 255);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(bg);

    /* Set the background to the theme */
    elm_object_part_content_set(layout, "elm.swallow.bg", bg);

    /* Create the box container */
    Evas_Object *box = elm_box_add(layout);
    evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(box);

    /* Set the content to the theme */
    elm_object_part_content_set(layout, "elm.swallow.content", box);

    dv->p_box = box;
    view->view = layout;

    browse(dv, "/");
    return view;
}

void
destroy_directory_view(interface_view *view)
{
    free(view->p_view_sys);
    free(view);
}
