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
#include <Emotion.h>

#include "ui/interface.h"
#include "video_player.h"
#include "playback_service.h"

struct view_sys
{
    bool b_fill;
    bool b_decoded;
    playback_service *p_ps;
    playback_service_cbs_id *p_ps_cbs_id;

    /* Widgets */
    Evas_Object *p_evas_video;
    Evas_Object *play_pause_button, *progress_slider;
};

static void
_on_slider_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *vd = data;

    playback_service_seek_pos(vd->p_ps, elm_slider_value_get(obj));
}

static void
clicked_play_pause(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *vd = data;

    elm_image_file_set(vd->play_pause_button,
                       playback_service_toggle_play_pause(vd->p_ps) ?
                               ICON_DIR "ic_pause_circle_normal_o.png" :
                               ICON_DIR "ic_play_circle_normal_o.png", NULL);
}

static void
clicked_backward(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *vd = data;

    playback_service_seek_backward(vd->p_ps);
}

static void
clicked_forward(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *vd = data;

    playback_service_seek_forward(vd->p_ps);
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

static void
ps_on_new_len_cb(playback_service *p_ps, void *p_user_data, double i_len)
{
    view_sys *vd = p_user_data;
    /* TODO */
}

static void
ps_on_new_time_cb(playback_service *p_ps, void *p_user_data, double i_time, double i_pos)
{
    view_sys *vd = p_user_data;
    elm_slider_value_set(vd->progress_slider, i_pos);
}

static void
video_resize(view_sys *vd)
{
    Evas_Coord i_win_x, i_win_y, i_win_w, i_win_h;
    int i_video_w, video_h;

    evas_object_geometry_get(vd->p_evas_video, &i_win_x, &i_win_y,
                             &i_win_w, &i_win_h);

    if (i_win_w <= 0 || i_win_h <= 0)
        return;

    emotion_object_size_get(vd->p_evas_video, &i_video_w, &video_h);
    if (i_video_w <= 0 || video_h <= 0)
        return;

    LOGF("video_resize: win: %dx%d %dx%d, video: %dx%d", i_win_x, i_win_y, i_win_w, i_win_h, i_video_w, video_h);

    if (!vd->b_fill)
    {
        if ((i_win_w <= 0) || (i_win_h <= 0) || (i_video_w <= 0) || (video_h <= 0))
        {
            i_win_w = 1;
            i_win_h = 1;
        }
        else
        {
            int i_w = 1, i_h = 1;
            double i_ratio;

            i_ratio = emotion_object_ratio_get(vd->p_evas_video);
            if (i_ratio > 0.0)
                i_video_w = (video_h * i_ratio);
            else
                i_ratio = i_video_w / (double)video_h;

            i_w = i_win_w;
            i_h = ((double)i_win_w + 1.0) / i_ratio;
            if (i_h > i_win_h)
            {
                i_h = i_win_h;
                i_w = i_win_h * i_ratio;
                if (i_w > i_win_w)
                    i_w = i_win_w;
            }
            i_win_x += ((i_win_w - i_w) / 2);
            i_win_y += ((i_win_h - i_h) / 2);
            i_win_w = i_w;
            i_win_h = i_h;
       }
    }

    LOGF("video_resize: move to: %dx%d %dx%d", i_win_x, i_win_y, i_win_w, i_win_h);

    evas_object_move(vd->p_evas_video, i_win_x, i_win_y);
    evas_object_resize(vd->p_evas_video, i_win_w, i_win_h);
}

static void
evas_video_decode_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
    view_sys *vd = data;
    if (!vd->b_decoded)
    {
        vd->b_decoded = true;
        video_resize(data);
    }
}

static void
evas_video_resize_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
    video_resize(data);
}

static void
layout_touch_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
    view_sys *vd = data;

    LOGF("layout_touch_up_cb");
}

bool
video_player_start(interface_view *view, const char* file_path)
{
    media_item *p_mi = media_item_create(file_path, MEDIA_ITEM_TYPE_VIDEO);
    if (!p_mi)
        return false;

    LOGE("playback_service_start: %s", p_mi->psz_path);
    playback_service_set_context(view->p_view_sys->p_ps, PLAYLIST_CONTEXT_VIDEO);

    playback_service_list_clear(view->p_view_sys->p_ps);
    playback_service_list_append(view->p_view_sys->p_ps, p_mi);
    playback_service_start(view->p_view_sys->p_ps, 0);
    return true;
}

interface_view*
create_video_player(playback_service *p_ps, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));

    view_sys *p_sys = view->p_view_sys = calloc(1, sizeof(*p_sys));
    if (!p_sys)
        return NULL;

    p_sys->p_ps = p_ps;
    playback_service_callbacks cbs = {
        .pf_on_new_len = ps_on_new_len_cb,
        .pf_on_new_time = ps_on_new_time_cb,
        .p_user_data = p_sys,
    };

    p_sys->p_ps_cbs_id = playback_service_register_callbacks(p_sys->p_ps, &cbs);
    if (!p_sys->p_ps_cbs_id)
    {
        free(p_sys);
        return NULL;
    }

    /* Create the layout */
    Evas_Object *layout = elm_layout_add(parent);
    elm_layout_file_set(layout, VIDEOPLAYEREDJ, "media_player_renderer");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_event_callback_add(layout, EVAS_CALLBACK_MOUSE_UP, layout_touch_up_cb, p_sys);
    evas_object_event_callback_add(layout, EVAS_CALLBACK_MULTI_UP, layout_touch_up_cb, p_sys);
    evas_object_show(layout);

    /* Create the evas */
    Evas *evas = evas_object_evas_get(layout);
    p_sys->p_evas_video = playback_service_set_evas_video(p_sys->p_ps, evas);
    evas_object_smart_callback_add(p_sys->p_evas_video, "frame_decode", evas_video_decode_cb, p_sys);
    evas_object_event_callback_add(p_sys->p_evas_video, EVAS_CALLBACK_RESIZE, evas_video_resize_cb, p_sys);

    elm_object_part_content_set(layout, "swallow.visualization", p_sys->p_evas_video);
    evas_object_show(p_sys->p_evas_video);

    /* create play/pause button */
    p_sys->play_pause_button = elm_image_add(layout);
    elm_image_file_set(p_sys->play_pause_button, ICON_DIR"ic_pause_circle_normal_o.png", NULL);
    //attach to edje layout
    elm_object_part_content_set(layout, "swallow.play_button", p_sys->play_pause_button);
    //click callback
    evas_object_smart_callback_add(p_sys->play_pause_button, "clicked", clicked_play_pause, p_sys);

    //create backward button
    Evas_Object *backward_button = elm_image_add(layout);
    elm_image_file_set(backward_button, ICON_DIR"ic_backward_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.backward_button", backward_button);
    evas_object_smart_callback_add(backward_button, "clicked", clicked_backward, p_sys);

    //create forward button
    Evas_Object *forward_button = elm_image_add(layout);
    elm_image_file_set(forward_button, ICON_DIR"ic_forward_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.forward_button", forward_button);
    evas_object_smart_callback_add(forward_button, "clicked", clicked_forward, p_sys);

    //create lock button
    Evas_Object *lock_button = elm_image_add(layout);
    elm_image_file_set(lock_button, ICON_DIR"ic_lock_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.lock_button", lock_button);
    evas_object_smart_callback_add(lock_button, "clicked", clicked_lock, p_sys);

    //create more button
    Evas_Object *more_button = elm_image_add(layout);
    elm_image_file_set(more_button, ICON_DIR"ic_more_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.more_button", more_button);
    evas_object_smart_callback_add(more_button, "clicked", clicked_more, p_sys);

    //progress slider
    p_sys->progress_slider = elm_slider_add(layout);
    elm_slider_horizontal_set(p_sys->progress_slider, EINA_TRUE);
    elm_object_part_content_set(layout, "swallow.progress", p_sys->progress_slider);

    //slider callbacks
    evas_object_smart_callback_add(p_sys->progress_slider, "slider,drag,stop", _on_slider_changed_cb, p_sys);
    evas_object_smart_callback_add(p_sys->progress_slider, "changed", _on_slider_changed_cb, p_sys);

    view->view = layout;
    return view;
}

void
destroy_video_player(interface_view *view)
{
    evas_object_event_callback_del(view->p_view_sys->p_evas_video, EVAS_CALLBACK_RESIZE, evas_video_resize_cb);
    evas_object_smart_callback_del(view->p_view_sys->p_evas_video, "frame_decode", evas_video_decode_cb);
    free(view->p_view_sys);
    free(view);
}
