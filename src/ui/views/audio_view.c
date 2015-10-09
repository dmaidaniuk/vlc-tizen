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

#include "controller/media_controller.h"
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

typedef struct list_sys list_sys;

struct view_sys
{
    application* p_app;
    interface *p_intf;

    Evas_Object *nf_toolbar;

    list_sys* p_lists[AUDIO_VIEW_MAX];

    Elm_Genlist_Item_Class *p_default_item_class;
};

struct list_sys
{
    Evas_Object *p_list;
    media_library_controller* p_ctrl;
    view_sys* p_audio_view;
};

typedef struct audio_list_item
{
    const view_sys *p_av;

    const Elm_Genlist_Item_Class *itc;

    media_item *p_media_item;

    Elm_Object_Item* p_object_item;
} audio_list_item;

static void
genlist_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_list_item *ali = data;
    intf_create_audio_player(ali->p_av->p_intf, ali->p_media_item->psz_path);
}

static void
free_list_item_data(void *data, Evas_Object *obj, void *event_info)
{
    audio_list_item *ali = data;
    media_item_destroy(ali->p_media_item);
    free(ali);
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
            asprintf(&buf, "<b>%s</b>", media_item_title(ali->p_media_item));
            return buf;
        }
        else if (!strcmp(part, "elm.text.sub.left.bottom")) {
            const char* psz_artist = media_item_artist(ali->p_media_item);
            if (psz_artist == NULL)
                psz_artist = "Unknown Artist";
            return strdup(psz_artist);
        }
        else if (!strcmp(part, "elm.text.sub.right.bottom")) {
            return media_timetostr( ali->p_media_item->i_duration / 1000 );
        }
    }
    return NULL;
}

const media_item*
audio_list_item_get_media_item(audio_list_item* p_item)
{
    return p_item->p_media_item;
}

void
audio_list_item_set_media_item(audio_list_item* p_item, media_item* p_media_item)
{
    p_item->p_media_item = p_media_item;
    ecore_main_loop_thread_safe_call_async((Ecore_Cb)elm_genlist_item_update, p_item->p_object_item);
}

static Evas_Object*
genlist_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    audio_list_item *ali = data;
    const Elm_Genlist_Item_Class *itc = ali->itc;
    Evas_Object *layout = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "2line.top.3")) {
        if (part && !strcmp(part, "elm.icon.1")) {
            layout = elm_layout_add(obj);
            elm_layout_theme_set(layout, "layout", "list/B/type.1", "default");
            Evas_Object *icon;
            if (ali->p_media_item->psz_snapshot != NULL)
                icon = create_image(layout, ali->p_media_item->psz_snapshot);
            else
                icon = create_icon(layout, "background_cone.png");
            elm_layout_content_set(layout, "elm.swallow.content", icon);
        }
    }

    return layout;
}

audio_list_item *
audio_list_append_item(list_sys *p_sys, media_item* p_item)
{
    audio_list_item *ali = calloc(1, sizeof(*ali));
    ali->p_av = p_sys->p_audio_view;
    ali->itc = p_sys->p_audio_view->p_default_item_class;

    ali->p_media_item = p_item;

    /* Set and append new item in the genlist */
    Elm_Object_Item *it = elm_genlist_item_append(p_sys->p_list,
            p_sys->p_audio_view->p_default_item_class,  /* genlist item class               */
            ali,                                        /* genlist item class user data     */
            NULL,                                       /* genlist parent item              */
            ELM_GENLIST_ITEM_NONE,                      /* genlist item type                */
            genlist_selected_cb,                        /* genlist select smart callback    */
            ali);                                       /* genlist smart callback user data */

    /* */
    elm_object_item_del_cb_set(it, free_list_item_data);
    return ali;
}

void
audio_view_clear( list_sys* p_list )
{
    elm_genlist_clear( p_list->p_list );
}

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

static list_sys*
create_audio_list_type(view_sys *av, audio_view_type type )
{
    list_sys *p_list = av->p_lists[type];
    if(p_list == NULL)
    {
        LOGD("New View %i", type);
        p_list = calloc(1, sizeof(*p_list));
        if (p_list == NULL)
            return NULL;
        p_list->p_audio_view = av;
        p_list->p_list = genlist_create(av);
        p_list->p_ctrl = audio_controller_create(av->p_app, p_list);
        media_library_controller_refresh( p_list->p_ctrl );
        av->p_lists[type] = p_list;
    }
    else
    {
        LOGD("Recycling View %i", type);
    }

    Elm_Object_Item *it = elm_naviframe_item_push(av->nf_toolbar, "", NULL, NULL, p_list->p_list, NULL);
    elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);

    evas_object_show( p_list->p_list );
    return p_list;
}

static void
audio_list_destroy( list_sys* p_list )
{
    if (p_list == NULL)
        return;
    media_library_controller_destroy(p_list->p_ctrl);
    free(p_list);
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

    /* Item Class */
    audio_view_sys->p_default_item_class = elm_genlist_item_class_new();
    audio_view_sys->p_default_item_class->item_style = "2line.top.3";
    audio_view_sys->p_default_item_class->func.text_get = genlist_text_get_cb;
    audio_view_sys->p_default_item_class->func.content_get = genlist_content_get_cb;

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
        audio_list_destroy( p_sys->p_lists[i]);
    }
    elm_genlist_item_class_free(p_sys->p_default_item_class);
    free(p_sys);
    free(view);
}
