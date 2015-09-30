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

#include "ui/utils.h"

#include "media/media_item.h"

#include <Elementary.h>

typedef struct video_view {
    interface *p_intf;

    Evas_Object *p_parent;
    Evas_Object *p_genlist;
} video_view;

typedef struct video_list_item {
    video_view *videoview;

    const Elm_Genlist_Item_Class *itc;

    media_item *item;

} video_list_item;

void
genlist_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
    video_list_item *vli = data;

    struct stat sb;
    stat(vli->item->psz_path, &sb);

    if (S_ISREG(sb.st_mode))
    {
        intf_create_video_player(vli->videoview->p_intf, vli->item->psz_path);
    }
    else if (S_ISDIR(sb.st_mode))
    {
        create_video_view(vli->videoview->p_intf, vli->videoview->p_parent, vli->item->psz_path);
    }
}

static void
free_list_item(void *data, Evas_Object *obj, void *event_info)
{
    video_list_item *vli = data;
    media_item_destroy(vli->item);
    free(vli);
}

static Evas_Object*
create_default_icon(Evas_Object *parent)
{
    return create_image(parent, "background_cone.png");
}

static char *
duration_string(int64_t duration)
{
    lldiv_t d;
    long long sec;
    char *str;

    duration /= 1000;
    d = lldiv(duration, 60);
    sec = d.rem;
    d = lldiv(d.quot, 60);
    asprintf(&str, "%02lld:%02lld:%02lld", d.quot, d.rem, sec);
    return str;
}

static char *
genlist_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    video_list_item *vli = data;
    const Elm_Genlist_Item_Class *itc = vli->itc;

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.text.main.left.top")) {
            char buf[PATH_MAX];
            snprintf(buf, 1023, "<b>%s</b>", vli->item->psz_title);
            return strdup(buf);
        }
        else if (!strcmp(part, "elm.text.sub.left.bottom")) {
            if(vli->item->i_duration < 0)
                return NULL;
            else
                return duration_string(vli->item->i_duration);
        }
        else if (!strcmp(part, "elm.text.sub.right.bottom")) {
            if (vli->item->i_w <= 0 || vli->item->i_h <= 0)
                return NULL;
            char *str_resolution;
            asprintf( &str_resolution, "%dx%d", vli->item->i_w, vli->item->i_h);
            return str_resolution;
        }
    }
    return NULL;
}

static Evas_Object*
genlist_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    video_list_item *vli = data;
    const Elm_Genlist_Item_Class *itc = vli->itc;

    Evas_Object *layout = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.icon.1")) {
            layout = elm_layout_add(obj);
            elm_layout_theme_set(layout, "layout", "list/B/type.1", "default");
            Evas_Object *icon = create_default_icon(layout);
            elm_layout_content_set(layout, "elm.swallow.content", icon);
        }
    }

    return layout;
}

static void
genlist_loaded_cb(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
    /* Set the callbacks when one of the genlist item is loaded */
}

static void
genlist_realized_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    /* Set the callbacks when one of the genlist item is realized */
}

static void
genlist_longpressed_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    /* Set the callbacks when one of the genlist item is longpress */
}

static void
genlist_contracted_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    Elm_Object_Item *it = event_info;

    /* Free the genlist subitems when contracted */
    elm_genlist_item_subitems_clear(it);
}

void
genlist_item_create(video_view *videoview, char *psz_path, char *psz_title, Elm_Genlist_Item_Class *itc)
{
    /* */
    video_list_item *vli = calloc(1, sizeof(*vli));
    vli->videoview = videoview;
    vli->itc = itc;

    /* Item instantiation */
    vli->item = media_item_create(psz_path, MEDIA_ITEM_TYPE_VIDEO);
    vli->item->psz_title = psz_title;

    /* Set and append new item in the genlist */
    Elm_Object_Item *it = elm_genlist_item_append(videoview->p_genlist,
            itc,                            /* genlist item class               */
            vli,                            /* genlist item class user data     */
            NULL,                           /* genlist parent item for trees    */
            ELM_GENLIST_ITEM_NONE,          /* genlist item type                */
            genlist_item_selected_cb,       /* genlist select smart callback    */
            vli);                           /* genlist smart callback user data */

    /* */
    elm_object_item_del_cb_set(it, free_list_item);
}

void
generate_video_list(video_view *videoview, const char *root_path, Elm_Genlist_Item_Class *itc)
{
    struct dirent* current_folder = NULL;
    char *str_title;

    if (root_path == NULL)
    {
        LOGI("No video path");
        return;
    }

    /* Open the path repository then put it as a dirent variable */
    DIR *rep = opendir(root_path);
    if  (rep == NULL)
    {
        char *error = strerror(errno);
        LOGI("Empty repository or Error due to %s", error);

        return;
    }

    /* Stop when the readdir have red the all repository */
    while ((current_folder = readdir(rep)) != NULL)
    {
        str_title = current_folder->d_name;

        /* Avoid appending for "." and ".." d_name */
        if (str_title && (strcmp(str_title, ".")==0 || strcmp(str_title, "..")==0))
        {
            continue;
        }

        /* Concatenate the file path and the selected folder or file name */
        char *psz_path;
        asprintf(&psz_path, "%s/%s", root_path, str_title);

        genlist_item_create(videoview, psz_path, str_title, itc);
    }
}

Evas_Object*
create_video_view(interface *intf, Evas_Object *parent, const char* path )
{
    video_view *vv = malloc(sizeof(*vv));
    vv->p_intf = intf;
    vv->p_parent = parent;

    /* Set then create the Genlist object */
    Evas_Object *genlist = vv->p_genlist = elm_genlist_add(parent);

    /* Genlist class */
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "2line.top.3";
    itc->func.text_get = genlist_text_get_cb;
    itc->func.content_get = genlist_content_get_cb;

    /* Set the genlist modes */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    /* Set smart Callbacks */
    evas_object_smart_callback_add(genlist, "realized", genlist_realized_cb, NULL);
    evas_object_smart_callback_add(genlist, "loaded", genlist_loaded_cb, NULL);
    evas_object_smart_callback_add(genlist, "longpressed", genlist_longpressed_cb, NULL);
    evas_object_smart_callback_add(genlist, "contracted", genlist_contracted_cb, NULL);

    /* Populate it */
    generate_video_list(vv, path, itc);

    /* */
    elm_genlist_item_class_free(itc);
    return genlist;
}
