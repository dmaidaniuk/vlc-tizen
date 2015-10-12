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
#include "ui/views/audio_list_song_view.h"
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

struct view_sys
{
    application*            p_app;
    interface*              p_intf;
    Evas_Object*            nf_toolbar;
    list_view*              p_lists[AUDIO_VIEW_MAX];
};

static Evas_Object*
genlist_create(const view_sys *av)
{
    Evas_Object *genlist = elm_genlist_add(av->nf_toolbar);
    if (genlist == NULL)
        return NULL;
    /* Set the genlist scoller mode */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    /* Enable the genlist HOMOGENEOUS mode */
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    /* Enable the genlist COMPRESS mode */
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    return genlist;
}

static list_view*
create_audio_list_type(view_sys *av, audio_view_type type )
{
    list_view* p_view = av->p_lists[type];
    if(p_view == NULL)
    {
        Evas_Object* p_genlist = genlist_create(av);
        if (p_genlist == NULL)
            return NULL;
        switch (type)
        {
        case AUDIO_VIEW_SONG:
        default:
            p_view = audio_list_song_view_create(av->p_app, av->p_intf, p_genlist);
//        default:
//            p_list = NULL;
        }
        av->p_lists[type] = p_view;
    }
    else
    {
        LOGD("Recycling View %i", type);
    }

    p_view->pf_show(p_view->p_sys, av->nf_toolbar);
    return p_view;
}

static void
tabbar_item_cb(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *av = data;

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
create_toolbar(view_sys *av, Evas_Object *parent)
{
    /* Create and set the toolbar */
    Evas_Object *tabbar = elm_toolbar_add(parent);

    /* Set the toolbar shrink mode to NONE */
    elm_toolbar_shrink_mode_set(tabbar, ELM_TOOLBAR_SHRINK_SCROLL);
    /* Expand the content to fill the toolbar */
    elm_toolbar_transverse_expanded_set(tabbar, EINA_TRUE);
    /* Items will only call their selection func and callback when first becoming selected*/
    elm_toolbar_select_mode_set(tabbar, ELM_OBJECT_SELECT_MODE_DEFAULT);

    evas_object_size_hint_weight_set(tabbar, EVAS_HINT_EXPAND, 0.0);
    evas_object_size_hint_align_set(tabbar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    evas_object_size_hint_min_set(tabbar, 450, 400);
    evas_object_size_hint_max_set(tabbar, 450, 400);

    /* Append new entry in the toolbar with the Icon & Label wanted */
    elm_toolbar_item_append(tabbar, NULL, "Artists",  tabbar_item_cb, av);
    elm_toolbar_item_append(tabbar, NULL, "Albums",   tabbar_item_cb, av);
    elm_toolbar_item_append(tabbar, NULL, "Songs",    tabbar_item_cb, av);
    elm_toolbar_item_append(tabbar, NULL, "Genre",    tabbar_item_cb, av);

    return tabbar;
}

interface_view *
create_audio_view(interface *intf, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));

    /* Setup the audio_view */
    view_sys *audio_view_sys = calloc(1, sizeof(*audio_view_sys));
    audio_view_sys->p_intf = intf;
    audio_view_sys->p_app = intf_get_application(intf);

    view->p_view_sys = audio_view_sys;

    /* Content box */
    Evas_Object *audio_box = elm_box_add(parent);
    evas_object_size_hint_weight_set(audio_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(audio_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Create the toolbar in the view */
    Evas_Object *tabbar = create_toolbar(audio_view_sys, audio_box);
    elm_box_pack_end(audio_box, tabbar);
    evas_object_show(tabbar);

    /* Toolbar Naviframe */
    audio_view_sys->nf_toolbar = elm_naviframe_add(audio_box);
    evas_object_size_hint_weight_set(audio_view_sys->nf_toolbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(audio_view_sys->nf_toolbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_box_pack_end(audio_box, audio_view_sys->nf_toolbar );
    evas_object_show(audio_view_sys->nf_toolbar);

    /* Set the first item in the toolbar */
    elm_toolbar_item_selected_set(elm_toolbar_first_item_get(tabbar), EINA_TRUE);

    /*  */
    evas_object_show(audio_box);
    view->view = audio_box;

    return view;
}

void
destroy_audio_view(interface_view *view)
{
    view_sys* p_sys = view->p_view_sys;
    for ( unsigned int i = 0; i < AUDIO_VIEW_MAX; ++i )
    {
        if (p_sys->p_lists[i] != NULL)
        {
            p_sys->p_lists[i]->pf_del(p_sys->p_lists[i]->p_sys);
            free(p_sys->p_lists[i]);
        }
    }
    free(p_sys);
    free(view);
}
