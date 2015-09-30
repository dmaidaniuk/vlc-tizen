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

#include <Evas.h>
#include <Elementary.h>

#include "ui/interface.h"
#include "video_player.h"
#include "playback_service.h"

typedef struct videodata
{
    playback_service *p_ps;
    media_list *p_ml;
    Evas_Object *parent, *layout;
    Evas_Object *play_pause_button, *progress_slider;
    Evas_Object *canvas;
    bool play_state;
} videodata_s;

static void
_on_slider_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
    videodata_s *vd = data;
    if(vd)
    {
        double val = elm_slider_value_get(obj);
        //TODO plug to VLC
    }
}

static void
clicked_play_pause(void *data, Evas_Object *obj, void *event_info)
{
   //TODO link with vlc state
    videodata_s *vd = data;
        if(vd)
        {
            elm_image_file_set(vd->play_pause_button, vd->play_state ? ICON_DIR "ic_pause_circle_normal_o.png" : ICON_DIR "ic_play_circle_normal_o.png", NULL);
            vd->play_state = !vd->play_state;
        }

}

static void
clicked_backward(void *data, Evas_Object *obj, void *event_info)
{
   //TODO backward action
	LOGD("backward button");
}

static void
clicked_forward(void *data, Evas_Object *obj, void *event_info)
{
   //TODO forward action
	LOGD("forward button");
}

static void
clicked_lock(void *data, Evas_Object *obj, void *event_info)
{
   //TODO lock action
	LOGD("lock button");
}

static void
clicked_more(void *data, Evas_Object *obj, void *event_info)
{
   //TODO more action
	LOGD("more button");
}

Evas_Object*
create_video_gui(playback_service *p_ps, Evas_Object *parent, const char* file_path)
{
    media_item *p_mi;
    videodata_s *vd = malloc(sizeof(*vd));
    if (!vd)
        return NULL;

    p_mi = media_item_create(file_path, MEDIA_ITEM_TYPE_VIDEO);
    if (!p_mi)
    {
        free(vd);
        return NULL;
    }

    vd->p_ps = p_ps;

    vd->parent = parent;

    vd->layout = elm_layout_add(parent);

    elm_layout_file_set(vd->layout, VIDEOPLAYEREDJ, "media_player_renderer");

    evas_object_size_hint_weight_set(vd->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(vd->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(vd->layout);

    /* create and assign canvas*/
    Evas *evas = evas_object_evas_get(vd->layout);
    Evas_Object *evas_video = playback_service_set_evas_video(vd->p_ps, evas);
    evas_object_show(evas_video);
    elm_object_part_content_set(vd->layout, "swallow.visualization", evas_video);

    //create play/pause button
    vd->play_pause_button = elm_image_add(vd->layout);
    elm_image_file_set(vd->play_pause_button, ICON_DIR"ic_play_circle_normal_o.png", NULL);
    //attach to edje layout
    elm_object_part_content_set(vd->layout, "swallow.play_button", vd->play_pause_button);
    //click callback
    evas_object_smart_callback_add(vd->play_pause_button, "clicked", clicked_play_pause, vd);

    //create backward button
    Evas_Object *backward_button = elm_image_add(vd->layout);
    elm_image_file_set(backward_button, ICON_DIR"ic_backward_circle_normal_o.png", NULL);
    elm_object_part_content_set(vd->layout, "swallow.backward_button", backward_button);
    evas_object_smart_callback_add(backward_button, "clicked", clicked_backward, vd);

    //create forward button
    Evas_Object *forward_button = elm_image_add(vd->layout);
    elm_image_file_set(forward_button, ICON_DIR"ic_forward_circle_normal_o.png", NULL);
    elm_object_part_content_set(vd->layout, "swallow.forward_button", forward_button);
    evas_object_smart_callback_add(forward_button, "clicked", clicked_forward, vd);

    //create lock button
    Evas_Object *lock_button = elm_image_add(vd->layout);
    elm_image_file_set(lock_button, ICON_DIR"ic_lock_circle_normal_o.png", NULL);
    elm_object_part_content_set(vd->layout, "swallow.lock_button", lock_button);
    evas_object_smart_callback_add(lock_button, "clicked", clicked_lock, vd);

    //create more button
    Evas_Object *more_button = elm_image_add(vd->layout);
    elm_image_file_set(more_button, ICON_DIR"ic_more_circle_normal_o.png", NULL);
    elm_object_part_content_set(vd->layout, "swallow.more_button", more_button);
    evas_object_smart_callback_add(more_button, "clicked", clicked_more, vd);

    //progress slider
    vd->progress_slider = elm_slider_add(vd->layout);
    elm_slider_horizontal_set(vd->progress_slider, EINA_TRUE);
    elm_object_part_content_set(vd->layout, "swallow.progress", vd->progress_slider);

    //slider callbacks
    evas_object_smart_callback_add(vd->progress_slider, "slider,drag,stop", _on_slider_changed_cb, vd);
    evas_object_smart_callback_add(vd->progress_slider, "changed", _on_slider_changed_cb, vd);

    Elm_Object_Item *it = elm_naviframe_item_push(parent, NULL, NULL, NULL, vd->layout, NULL);
    elm_naviframe_item_title_enabled_set(it, EINA_FALSE, EINA_FALSE);

    LOGE("playback_service_start: %s", p_mi->psz_path);
    playback_service_set_context(vd->p_ps, PLAYLIST_CONTEXT_VIDEO);
    vd->p_ml = playback_service_get_ml(vd->p_ps);

    media_list_clear(vd->p_ml);
    media_list_append(vd->p_ml, p_mi);
    playback_service_start(vd->p_ps);

    return vd->layout;
}
