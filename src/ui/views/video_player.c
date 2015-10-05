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
#include "ui/utils.h"
#include "video_player.h"
#include "playback_service.h"

struct view_sys
{
    interface *intf;

    bool b_fill;
    bool b_decoded;
    playback_service *p_ps;
    playback_service_cbs_id *p_ps_cbs_id;

    /* Widgets */
    Evas_Object *p_evas_video;
    Evas_Object *layout;
    Evas_Object *play_pause_button, *progress_slider;
    Evas_Object *backward_button, *forward_button;
    Evas_Object *lock_button, *more_button;

};

static void video_player_stop(view_sys *p_sys);

static void
_on_slider_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_view_sys = data;

    playback_service_seek_pos(p_view_sys->p_ps, elm_slider_value_get(obj));
}

static void
clicked_play_pause(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_view_sys = data;

    elm_image_file_set(p_view_sys->play_pause_button,
                       playback_service_toggle_play_pause(p_view_sys->p_ps) ?
                               ICON_DIR "ic_pause_circle_normal_o.png" :
                               ICON_DIR "ic_play_circle_normal_o.png", NULL);
}

static void
clicked_backward(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_view_sys = data;

    playback_service_seek_backward(p_view_sys->p_ps);
}

static void
clicked_forward(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_view_sys = data;

    playback_service_seek_forward(p_view_sys->p_ps);
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
    view_sys *p_view_sys = p_user_data;

    char *str = media_timetostr((int64_t)i_len);
    elm_object_part_text_set(p_view_sys->layout, "duration", str);
    free(str);
}

static void
ps_on_new_time_cb(playback_service *p_ps, void *p_user_data, double i_time, double i_pos)
{
    view_sys *p_view_sys = p_user_data;
    elm_slider_value_set(p_view_sys->progress_slider, i_pos);

    char *str = media_timetostr((int64_t)i_time);
    elm_object_part_text_set(p_view_sys->layout, "time", str);
    free(str);
}

static void
video_resize(view_sys *p_view_sys)
{
    Evas_Coord i_win_x, i_win_y, i_win_w, i_win_h;
    int i_video_w, video_h;

    evas_object_geometry_get(p_view_sys->p_evas_video, &i_win_x, &i_win_y,
                             &i_win_w, &i_win_h);

    if (i_win_w <= 0 || i_win_h <= 0)
        return;

    emotion_object_size_get(p_view_sys->p_evas_video, &i_video_w, &video_h);
    if (i_video_w <= 0 || video_h <= 0)
        return;

    LOGF("video_resize: win: %dx%d %dx%d, video: %dx%d", i_win_x, i_win_y, i_win_w, i_win_h, i_video_w, video_h);

    if (!p_view_sys->b_fill)
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

            i_ratio = emotion_object_ratio_get(p_view_sys->p_evas_video);
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

    evas_object_move(p_view_sys->p_evas_video, i_win_x, i_win_y);
    evas_object_resize(p_view_sys->p_evas_video, i_win_w, i_win_h);
}

static void
evas_video_decode_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
    view_sys *p_view_sys = data;
    if (!p_view_sys->b_decoded)
    {
        p_view_sys->b_decoded = true;
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
    view_sys *p_view_sys = data;

    elm_object_signal_emit(p_view_sys->layout, "hub_background,show", "");

    LOGF("layout_touch_up_cb");
}

static void
ps_on_stop_cb(playback_service *p_ps, void *p_user_data, media_item *p_mi)
{
    view_sys *p_view_sys = p_user_data;
    intf_show_previous_view(p_view_sys->intf);
}

bool
video_player_start(view_sys *p_sys, const char* file_path)
{
    elm_object_part_text_set(p_sys->layout, "duration", "--:--:--");
    elm_object_part_text_set(p_sys->layout, "time", "--:--:--");

    media_item *p_mi = media_item_create(file_path, MEDIA_ITEM_TYPE_VIDEO);
    if (!p_mi)
        return false;

    evas_object_smart_callback_add(p_sys->p_evas_video, "frame_decode", evas_video_decode_cb, p_sys);
    evas_object_event_callback_add(p_sys->p_evas_video, EVAS_CALLBACK_RESIZE, evas_video_resize_cb, p_sys);

    /* layout callbacks */
    evas_object_event_callback_add(p_sys->layout, EVAS_CALLBACK_MOUSE_UP, layout_touch_up_cb, p_sys);
    evas_object_event_callback_add(p_sys->layout, EVAS_CALLBACK_MULTI_UP, layout_touch_up_cb, p_sys);

    /* click callbacks */
    evas_object_smart_callback_add(p_sys->play_pause_button, "clicked", clicked_play_pause, p_sys);
    evas_object_smart_callback_add(p_sys->backward_button, "clicked", clicked_backward, p_sys);
    evas_object_smart_callback_add(p_sys->forward_button, "clicked", clicked_forward, p_sys);
    evas_object_smart_callback_add(p_sys->lock_button, "clicked", clicked_lock, p_sys);
    evas_object_smart_callback_add(p_sys->more_button, "clicked", clicked_more, p_sys);

    /*slider callbacks */
    evas_object_smart_callback_add(p_sys->progress_slider, "slider,drag,stop", _on_slider_changed_cb, p_sys);
    evas_object_smart_callback_add(p_sys->progress_slider, "changed", _on_slider_changed_cb, p_sys);

    playback_service_callbacks cbs = {
        .pf_on_new_len = ps_on_new_len_cb,
        .pf_on_new_time = ps_on_new_time_cb,
        .pf_on_stopped = ps_on_stop_cb,
        .p_user_data = p_sys,
    };

    p_sys->p_ps_cbs_id = playback_service_register_callbacks(p_sys->p_ps, &cbs);
    if (!p_sys->p_ps_cbs_id)
    {
        free(p_mi);
        video_player_stop(p_sys);
        return false;
    }

    LOGE("playback_service_start: %s", p_mi->psz_path);
    playback_service_set_context(p_sys->p_ps, PLAYLIST_CONTEXT_VIDEO);

    playback_service_list_append(p_sys->p_ps, p_mi);
    playback_service_start(p_sys->p_ps, 0);
    elm_object_signal_emit(p_sys->layout, "hub_background,show", "");
    return true;
}

static void
video_player_stop(view_sys *p_sys)
{
    evas_object_smart_callback_del(p_sys->p_evas_video, "frame_decode", evas_video_decode_cb);
    evas_object_event_callback_del(p_sys->p_evas_video, EVAS_CALLBACK_RESIZE, evas_video_resize_cb);

    /* layout callbacks */
    evas_object_event_callback_del(p_sys->layout, EVAS_CALLBACK_MOUSE_UP, layout_touch_up_cb);
    evas_object_event_callback_del(p_sys->layout, EVAS_CALLBACK_MULTI_UP, layout_touch_up_cb);

    /* click callbacks */
    evas_object_smart_callback_del(p_sys->play_pause_button, "clicked", clicked_play_pause);
    evas_object_smart_callback_del(p_sys->backward_button, "clicked", clicked_backward);
    evas_object_smart_callback_del(p_sys->forward_button, "clicked", clicked_forward);
    evas_object_smart_callback_del(p_sys->lock_button, "clicked", clicked_lock);
    evas_object_smart_callback_del(p_sys->more_button, "clicked", clicked_more);

    /*slider callbacks */
    evas_object_smart_callback_del(p_sys->progress_slider, "slider,drag,stop", _on_slider_changed_cb);
    evas_object_smart_callback_del(p_sys->progress_slider, "changed", _on_slider_changed_cb);

    if (p_sys->p_ps_cbs_id)
    {
        playback_service_unregister_callbacks(p_sys->p_ps, p_sys->p_ps_cbs_id);
        playback_service_list_clear(p_sys->p_ps);
        playback_service_stop(p_sys->p_ps);
    }
}

interface_view*
create_video_player(interface *intf, playback_service *p_ps, Evas_Object *parent)
{
    interface_view *view = calloc(1, sizeof(*view));

    view_sys *p_sys = view->p_view_sys = calloc(1, sizeof(*p_sys));
    if (!p_sys)
        return NULL;

    p_sys->intf = intf;
    p_sys->p_ps = p_ps;

    /* Create the layout */
    Evas_Object *layout = p_sys->layout = elm_layout_add(parent);
    elm_layout_file_set(layout, VIDEOPLAYEREDJ, "media_player_renderer");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(layout);

    /* Create the evas */
    Evas *evas = evas_object_evas_get(layout);
    p_sys->p_evas_video = playback_service_set_evas_video(p_sys->p_ps, evas);

    elm_object_part_content_set(layout, "swallow.visualization", p_sys->p_evas_video);
    evas_object_show(p_sys->p_evas_video);

    /* create play/pause button */
    p_sys->play_pause_button = elm_image_add(layout);
    elm_image_file_set(p_sys->play_pause_button, ICON_DIR"ic_pause_circle_normal_o.png", NULL);
    //attach to edje layout
    elm_object_part_content_set(layout, "swallow.play_button", p_sys->play_pause_button);

    //create backward button
    p_sys->backward_button = elm_image_add(layout);
    elm_image_file_set(p_sys->backward_button, ICON_DIR"ic_backward_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.backward_button", p_sys->backward_button);

    //create forward button
    p_sys->forward_button = elm_image_add(layout);
    elm_image_file_set(p_sys->forward_button, ICON_DIR"ic_forward_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.forward_button", p_sys->forward_button);

    //create lock button
    p_sys->lock_button = elm_image_add(layout);
    elm_image_file_set(p_sys->lock_button, ICON_DIR"ic_lock_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.lock_button", p_sys->lock_button);

    //create more button
    p_sys->more_button = elm_image_add(layout);
    elm_image_file_set(p_sys->more_button, ICON_DIR"ic_more_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.more_button", p_sys->more_button);

    //progress slider
    p_sys->progress_slider = elm_slider_add(layout);
    elm_slider_horizontal_set(p_sys->progress_slider, EINA_TRUE);
    elm_object_part_content_set(layout, "swallow.progress", p_sys->progress_slider);

    view->view = layout;

    view->pf_stop = video_player_stop;
    return view;
}

void
destroy_video_player(interface_view *view)
{
    free(view->p_view_sys);
    free(view);
}
