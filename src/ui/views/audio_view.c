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
#include "system_storage.h"

#include "ui/interface.h"
#include "ui/views/audio_view.h"
#include "ui/audio_player.h"

#include "ui/utils.h"

#include "media/media_item.h"

#include <Elementary.h>

typedef enum audio_view_type
{
    AUDIO_VIEW_ARTIST,
    AUDIO_VIEW_ALBUM,
    AUDIO_VIEW_SONG,
    AUDIO_VIEW_GENRE,
    //AUDIO_VIEW_PLAYLIST,
    AUDIO_VIEW_MAX,
} audio_view_type;

typedef struct audio_view
{
    interface *p_intf;

    Evas_Object *nf_toolbar;

    Evas_Object *audio_lists[AUDIO_VIEW_MAX];
} audio_view;

typedef struct audio_list_item
{
    audio_view *p_av;

    const Elm_Genlist_Item_Class *itc;

    media_item *p_media_item;
} audio_list_item;

static Evas_Object*
create_audio_list(audio_view *av);

static void
genlist_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_list_item *ali = data;
    struct stat sb;
    stat(ali->p_media_item->psz_path, &sb);

    if (S_ISREG(sb.st_mode))
    {
        /* Launch the media player */
        create_base_player(intf_get_mini_player(ali->p_av->p_intf), ali->p_media_item->psz_path);
        LOGI("VLC Player launch");
    }

    else if (S_ISDIR(sb.st_mode))
    {
        /* Continue to browse media folder */
        create_audio_list(ali->p_av);
    }
    else
    {
        LOGI("This is not a file or a folder");
    }
}

static void
free_list_item_data(void *data, Evas_Object *obj, void *event_info)
{
    LOGD("CLEANING");
    audio_list_item *ali = data;
    free(ali->p_media_item->psz_path);
    free(ali);
}

static Evas_Object*
create_icon(Evas_Object *parent)
{
    return create_image(parent, "background_cone.png" );
}

static char *
genlist_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    audio_list_item *ali = data;
    const Elm_Genlist_Item_Class *itc = ali->itc;
    char *buf;

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.text.main.left.top")) {
            asprintf(&buf, "<b>%s</b>", ali->p_media_item->psz_title);
            return buf;
        }
        else if (!strcmp(part, "elm.text.sub.left.bottom")) {
            return strdup("Artist");
        }
        else if (!strcmp(part, "elm.text.sub.right.bottom")) {
            int m = 4;
            int s = 56;
            asprintf(&buf,"%d m %d s", m, s);
            return buf;
        }
    }
    return NULL;
}

static Evas_Object*
genlist_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    audio_list_item *ali = data;
    const Elm_Genlist_Item_Class *itc = ali->itc;
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

static audio_list_item *
genlist_audio_item_create(audio_view *av, Evas_Object *parent_genlist, char *psz_path, char *psz_title, Elm_Genlist_Item_Class *itc)
{
    audio_list_item *ali = malloc(sizeof(*ali));
    ali->p_av = av;
    ali->itc = itc;

    ali->p_media_item = media_item_create(psz_path, MEDIA_ITEM_TYPE_AUDIO);
    ali->p_media_item->psz_title = psz_title;

    /* Set and append new item in the genlist */
    Elm_Object_Item *it = elm_genlist_item_append(parent_genlist,
            itc,                            /* genlist item class               */
            ali,                            /* genlist item class user data     */
            NULL,                           /* genlist parent item              */
            ELM_GENLIST_ITEM_NONE,          /* genlist item type                */
            genlist_selected_cb,            /* genlist select smart callback    */
            ali);                           /* genlist smart callback user data */

    /* */
    //elm_object_item_del_cb_set(it, free_list_item_data);
    return ali;
}

static Evas_Object*
genlist_create(audio_view *av, Elm_Genlist_Item_Class **itc )
{
     Evas_Object *genlist = elm_genlist_add(av->nf_toolbar);
     /* Set the genlist scoller mode */
     elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
     /* Enable the genlist HOMOGENEOUS mode */
     elm_genlist_homogeneous_set(genlist, EINA_TRUE);
     /* Enable the genlist COMPRESS mode */
     elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

     /* Item Class */
     *itc = elm_genlist_item_class_new();
     (*itc)->item_style = "2line.top.3";
     (*itc)->func.text_get = genlist_text_get_cb;
     (*itc)->func.content_get = genlist_content_get_cb;

     return genlist;
}

static Evas_Object*
create_audio_list(audio_view *av)
{
    Elm_Genlist_Item_Class *itc;
    Evas_Object *genlist = genlist_create(av, &itc);

    application *p_app = intf_get_application(av->p_intf);
    const char *path = application_get_media_path(p_app, MEDIA_DIRECTORY_MUSIC);

    /* Make a realpath to use a clean path in the function */
    char *buff = realpath(path, NULL);
    path = buff;

    /* Open the path repository then put it as a dirent variable */
    DIR* rep = opendir(path);

    LOGI("Audio View %s ", path);
    if (path == NULL)
    {
        LOGI("No path");
        return NULL ;
    }

    if  (rep == NULL)
    {
        char *error;
        error = strerror(errno);
        LOGI("Empty repository or Error due to %s", error);

        return NULL ;
    }

    struct dirent* current_folder = NULL;
    /* Stop when the readdir have red the all repository */
    while ((current_folder = readdir(rep)) != NULL)
    {
        char *psz_path;
        char *str = current_folder->d_name;

        /* Concatenate the file path and the selected folder or file name */
        asprintf(&psz_path, "%s/%s", path, str);

        /* Avoid genlist item append for "." and ".." d_name */
        if (str && (strcmp(str, ".")==0 || strcmp(str, "..")==0))
        {
            continue;
        }

        genlist_audio_item_create(av, genlist, psz_path, str, itc);
    }
    /* */
    elm_genlist_item_class_free(itc);
    return genlist;
}

static Evas_Object*
create_audio_list_type(audio_view *av, audio_view_type type )
{
    Evas_Object *genlist = av->audio_lists[type];
    if(genlist == NULL)
    {
        LOGD("New View %i", type);
        av->audio_lists[type] = genlist = create_audio_list(av);
    }
    else
    {
        LOGD("Recycling View %i %i", type, elm_genlist_items_count(genlist));
    }

    elm_object_content_set(av->nf_toolbar, av->audio_lists[type] );
    return genlist;
}

static void
tabbar_item_cb(void *data, Evas_Object *obj, void *event_info)
{
    audio_view *av = data;

    /* Get the item selected in the toolbar */
    const char *str = elm_object_item_text_get((Elm_Object_Item *)event_info);

    /* Create the view depending on the item selected in the toolbar */
    if (str && !strcmp(str, "Songs")) {
        create_audio_list_type(av, AUDIO_VIEW_SONG);
    }
    else if (str && !strcmp(str, "Artists")) {
        create_audio_list_type(av, AUDIO_VIEW_ARTIST);
    }
    else if (str && !strcmp(str, "Albums")) {
        create_audio_list_type(av, AUDIO_VIEW_ALBUM);
    }
    else {
        create_audio_list_type(av, AUDIO_VIEW_GENRE);
    }
}

static Evas_Object*
create_toolbar(audio_view *av)
{
    /* Create and set the toolbar */
    Evas_Object *tabbar = elm_toolbar_add(av->nf_toolbar);
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
    elm_toolbar_item_append(tabbar, NULL, "Artists",  tabbar_item_cb, av);
    elm_toolbar_item_append(tabbar, NULL, "Albums",   tabbar_item_cb, av);
    elm_toolbar_item_append(tabbar, NULL, "Songs",    tabbar_item_cb, av);
    elm_toolbar_item_append(tabbar, NULL, "Genre", tabbar_item_cb, av);

    return tabbar;
}

Evas_Object *
create_audio_view(interface *intf, Evas_Object *parent)
{
    Elm_Object_Item *tabbar_it;
    Evas_Object *tabbar;

    /* setup the audio_view */
    audio_view *av = calloc(1, sizeof(*av));
    av->p_intf = intf;
    for(int i = 0; i < AUDIO_VIEW_MAX; i++)
        av->audio_lists[i] = NULL;

    /* Toolbar Naviframe */
    av->nf_toolbar = elm_naviframe_add(parent);
    evas_object_size_hint_weight_set(av->nf_toolbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(av->nf_toolbar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Toolbar Naviframe Settings */
    elm_object_part_content_set(parent, "elm.swallow.content", av->nf_toolbar);
    evas_object_show(av->nf_toolbar);

    /* Create the toolbar in the view */
    tabbar = create_toolbar(av);
    Elm_Object_Item *nf_it = elm_naviframe_item_push(av->nf_toolbar, NULL, NULL, NULL, NULL, "tabbar/icon/notitle");
    elm_object_item_part_content_set(nf_it, "tabbar", tabbar);

    /* Set the first item in the toolbar */
    tabbar_it = elm_toolbar_first_item_get(tabbar);
    elm_toolbar_item_selected_set(tabbar_it, EINA_TRUE);

    /* Push on the parent view */
    elm_naviframe_item_push(parent, "<b>Audio</b>", NULL, NULL, av->nf_toolbar, "basic");

    return av->nf_toolbar;
}
