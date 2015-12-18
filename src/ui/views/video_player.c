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
#include <efl_extension.h>

#include "ui/interface.h"
#include "ui/utils.h"
#include "ui/menu/popup_menu.h"
#include "ui/settings/menu_id.h"
#include "preferences/preferences.h"
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
    Evas_Object *win;
    Evas_Object *p_evas_video;
    Evas_Object *layout;
    Evas_Object *play_pause_button, *progress_slider;
    Evas_Object *backward_button, *forward_button;
    Evas_Object *lock_button, *more_button;
    Evas_Object *p_current_popup;

};

static void video_player_stop(view_sys *p_sys);

static void
_on_slider_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;

    playback_service_seek_pos(p_sys->p_ps, elm_slider_value_get(obj));
}

static void
video_player_update_play_pause_state(view_sys *p_sys)
{
    elm_image_file_set(p_sys->play_pause_button,
            playback_service_is_playing(p_sys->p_ps) ?
                                   ICON_DIR "ic_pause_circle_normal_o.png" :
                                   ICON_DIR "ic_play_circle_normal_o.png", NULL);
}

static void
clicked_play_pause(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;

    playback_service_toggle_play_pause(p_sys->p_ps);
    video_player_update_play_pause_state(p_sys);
}

static void
clicked_backward(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;

    playback_service_seek_backward(p_sys->p_ps);
}

static void
clicked_forward(void *data, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;

    playback_service_seek_forward(p_sys->p_ps);
}

static void
clicked_lock(void *data, Evas_Object *obj, void *event_info)
{
    /* TODO lock action */
	LOGD("lock button");
}

void clear_popup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    view_sys *p_sys = data;
    p_sys->p_current_popup = NULL;
}

void channel_menu_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    popup_menu *menu = data;

    for(int i = 0; menu[i].title != NULL ; i++)
        free(menu[i].title);
    free(menu);
}

void
audio_channel_selected(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *it_parent = (Elm_Object_Item*) event_info;
    view_sys *p_sys = data;

    // elm_genlist_item_index_get starts counting at 1 (not 0)
    int index = elm_genlist_item_index_get(it_parent);

    playback_service_audio_channel_set(p_sys->p_ps, index - 1);
    evas_object_del(obj);
}

void
clicked_audio_tracks(void *data, Evas_Object *obj, void *event_info)
{
    Eina_List *channel_list, *p_it = NULL;
    view_sys *p_sys = data;
    char *channel_title;
    popup_menu *menu;

    if (p_sys->p_current_popup)
        evas_object_del(p_sys->p_current_popup);

    // Get the list of channels
    channel_list = playback_service_audio_channel_get_list(p_sys->p_ps);

    // Allocate the memory for the menu + 1 (menu + NULL terminating item)
    menu = malloc(sizeof(*menu) * (eina_list_count(channel_list) + 1));

    int i = 0;
    EINA_LIST_FOREACH(channel_list, p_it, channel_title)
    {
        if (channel_title)
            menu[i].title = strdup(channel_title);
        else
            asprintf(&menu[i].title, "Audio channel #%d", i);
        menu[i].icon = NULL;
        menu[i].cb = audio_channel_selected;
        menu[i].hidden = EINA_FALSE;
        i++;
    }

    // NULL terminating item
    menu[i].title = NULL;

    Evas_Object *popup = p_sys->p_current_popup = popup_menu_add(menu, p_sys, p_sys->p_evas_video);
    evas_object_show(popup);

    // Register a callback to free the memory allocated for the menu
    evas_object_event_callback_add(popup, EVAS_CALLBACK_FREE, channel_menu_free_cb, menu);
    evas_object_event_callback_add(popup, EVAS_CALLBACK_FREE, clear_popup_cb, p_sys);

    eina_list_free(channel_list);
}

void
spu_selected(void *data, Evas_Object *obj, void *event_info)
{
    Elm_Object_Item *it_parent = (Elm_Object_Item*) event_info;
    view_sys *p_sys = data;

    // elm_genlist_item_index_get starts counting at 1 (not 0)
    int index = elm_genlist_item_index_get(it_parent);

    // Disable subtitles is -1, first is 0 so index -2.
    playback_service_spu_channel_set(p_sys->p_ps, index - 2);
    evas_object_del(obj);
}

void
clicked_spu(void *data, Evas_Object *obj, void *event_info)
{
    Eina_List *spu_list, *p_it = NULL;
    view_sys *p_sys = data;
    char *spu_title;
    popup_menu *menu;

    if (p_sys->p_current_popup)
        evas_object_del(p_sys->p_current_popup);

    // Get the list of spu
    spu_list = playback_service_spu_channel_get_list(p_sys->p_ps);

    // Prepend the "disable subtitles"
    spu_list = eina_list_prepend(spu_list, "Disable subtitles");

    // Allocate the memory for the menu + 1 (menu + NULL terminating item)
    menu = malloc(sizeof(*menu) * (eina_list_count(spu_list) + 1));

    int i = 0;
    EINA_LIST_FOREACH(spu_list, p_it, spu_title)
    {
        if (spu_title)
            menu[i].title = strdup(spu_title);
        else
            asprintf(&menu[i].title, "Subtitle track #%d", i);
        menu[i].icon = NULL;
        menu[i].cb = spu_selected;
        menu[i].hidden = EINA_FALSE;
        i++;
    }

    // NULL terminating item
    menu[i].title = NULL;

    Evas_Object *popup = p_sys->p_current_popup = popup_menu_add(menu, p_sys, p_sys->p_evas_video);
    evas_object_show(popup);

    // Register a callback to free the memory allocated for the menu
    evas_object_event_callback_add(popup, EVAS_CALLBACK_FREE, channel_menu_free_cb, menu);
    evas_object_event_callback_add(popup, EVAS_CALLBACK_FREE, clear_popup_cb, p_sys);

    eina_list_free(spu_list);
}

static popup_menu menu_more[] =
{
        {"Subtitles",       NULL, clicked_spu},
        {"Audio Tracks",    NULL, clicked_audio_tracks},
        {0}
};

static void
clicked_more(void *data, Evas_Object *obj, void *event_info)
{
    Eina_List *list;
    int spu_count, audio_tracks_count;
    view_sys *p_sys = data;

    /* SPU */
    list = playback_service_spu_channel_get_list(p_sys->p_ps);
    spu_count = eina_list_count(list);
    eina_list_free(list);

    /* Audio tracks */
    list = playback_service_audio_channel_get_list(p_sys->p_ps);
    audio_tracks_count = eina_list_count(list);
    eina_list_free(list);

    if (spu_count + audio_tracks_count == 0)
        return;

    menu_more[0].hidden = spu_count == 0;
    menu_more[1].hidden = audio_tracks_count == 0;

    Evas_Object *popup = p_sys->p_current_popup = popup_menu_orient_add(menu_more, ELM_POPUP_ORIENT_CENTER, p_sys, p_sys->p_evas_video);
    evas_object_show(popup);
    evas_object_event_callback_add(popup, EVAS_CALLBACK_FREE, clear_popup_cb, p_sys);
}

static void
ps_on_new_len_cb(playback_service *p_ps, void *p_user_data, double i_len)
{
    view_sys *p_sys = p_user_data;

    char *str = media_timetostr((int64_t)i_len);
    elm_object_part_text_set(p_sys->layout, "duration", str);
    free(str);
}

static void
ps_on_new_time_cb(playback_service *p_ps, void *p_user_data, double i_time, double i_pos)
{
    view_sys *p_sys = p_user_data;
    elm_slider_value_set(p_sys->progress_slider, i_pos);

    char *str = media_timetostr((int64_t)i_time);
    elm_object_part_text_set(p_sys->layout, "time", str);
    free(str);
}

static void
layout_touch_up_cb(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
    view_sys *p_sys = data;

    elm_object_signal_emit(p_sys->layout, "hub_background,show", "");

    LOGF("layout_touch_up_cb");
}

static void
ps_on_stop_cb(playback_service *p_ps, void *p_user_data)
{
    view_sys *p_sys = p_user_data;
    intf_show_previous_view(p_sys->intf);
}

static void
ps_on_playpause_cb(playback_service *p_ps, void *p_user_data, bool b_playing)
{
    view_sys *p_sys = p_user_data;
    elm_image_file_set(p_sys->play_pause_button,
            b_playing ?
                    ICON_DIR "ic_pause_circle_normal_o.png" :
                    ICON_DIR "ic_play_circle_normal_o.png", NULL);
}

bool
video_player_start(view_sys *p_sys, const char* file_path)
{
    /* Screen orientation */
    if (elm_win_wm_rotation_supported_get(p_sys->win)) {
        menu_id orientation = preferences_get_enum(PREF_ORIENTATION, ORIENTATION_AUTOMATIC);

        int rotation = -1;

        switch (orientation) {
        case ORIENTATION_LANDSCAPE:
            rotation = 270;
            break;
        case ORIENTATION_PORTRAIT:
            rotation = 0;
            break;
        case ORIENTATION_R_LANDSCAPE:
            rotation = 90;
            break;
        case ORIENTATION_R_PORTRAIT:
            rotation = 180;
            break;
        case ORIENTATION_LOCKED:
            rotation = elm_win_rotation_get(p_sys->win);
            break;
        case ORIENTATION_AUTOMATIC:
        default:
            break;
        }

        if (rotation >= 0)
            elm_win_wm_rotation_available_rotations_set(p_sys->win, &rotation, 1);
    }

    /* */
    elm_object_part_text_set(p_sys->layout, "duration", "--:--:--");
    elm_object_part_text_set(p_sys->layout, "time", "--:--:--");

    media_item *p_mi = media_item_create(file_path, MEDIA_ITEM_TYPE_VIDEO);
    if (!p_mi)
        return false;

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
        .pf_on_playpause = ps_on_playpause_cb,
        .p_user_data = p_sys,
        .i_ctx = PLAYLIST_CONTEXT_VIDEO,
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
        p_sys->p_ps_cbs_id = NULL;
    }

    if (elm_win_wm_rotation_supported_get(p_sys->win)) {
        int rots[4] = { 0, 90, 180, 270 };
        elm_win_wm_rotation_available_rotations_set(p_sys->win, (const int *)(&rots), 4);
    }
}

Evas_Object*
video_player_create_ui(view_sys *p_sys, Evas_Object *parent)
{
    /* Create the layout */
    Evas_Object *layout = p_sys->layout = elm_layout_add(parent);
    elm_layout_file_set(layout, VIDEOPLAYER_EDJ, "media_player_renderer");
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(layout);

    /* Create the evas */
    Evas *evas = evas_object_evas_get(layout);
    p_sys->p_evas_video = playback_service_set_evas_video(p_sys->p_ps, evas);

    elm_object_part_content_set(layout, "swallow.visualization", p_sys->p_evas_video);
    emotion_object_keep_aspect_set(p_sys->p_evas_video, EMOTION_ASPECT_KEEP_BOTH);
    evas_object_show(p_sys->p_evas_video);

    /* create play/pause button */
    p_sys->play_pause_button = elm_image_add(layout);
    elm_image_file_set(p_sys->play_pause_button, ICON_DIR"ic_pause_circle_normal_o.png", NULL);
    /* attach to edje layout */
    elm_object_part_content_set(layout, "swallow.play_button", p_sys->play_pause_button);

    /* create backward button */
    p_sys->backward_button = elm_image_add(layout);
    elm_image_file_set(p_sys->backward_button, ICON_DIR"ic_backward_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.backward_button", p_sys->backward_button);

    /* create forward button */
    p_sys->forward_button = elm_image_add(layout);
    elm_image_file_set(p_sys->forward_button, ICON_DIR"ic_forward_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.forward_button", p_sys->forward_button);

    /* create lock button */
    p_sys->lock_button = elm_image_add(layout);
    elm_image_file_set(p_sys->lock_button, ICON_DIR"ic_lock_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.lock_button", p_sys->lock_button);

    /* create more button */
    p_sys->more_button = elm_image_add(layout);
    elm_image_file_set(p_sys->more_button, ICON_DIR"ic_more_circle_normal_o.png", NULL);
    elm_object_part_content_set(layout, "swallow.more_button", p_sys->more_button);

    /* progress slider */
    p_sys->progress_slider = elm_slider_add(layout);
    elm_slider_horizontal_set(p_sys->progress_slider, EINA_TRUE);
    elm_object_part_content_set(layout, "swallow.progress", p_sys->progress_slider);

    return layout;
}

static bool
video_player_callback(view_sys *p_view_sys, interface_view_event event)
{
    switch (event) {
    case INTERFACE_VIEW_EVENT_MENU:
    case INTERFACE_VIEW_EVENT_RESUME:
    {
        layout_touch_up_cb(p_view_sys, NULL, NULL, NULL);
        return true;
    }
    case INTERFACE_VIEW_EVENT_BACK:
        if (p_view_sys->p_current_popup) {
            evas_object_del(p_view_sys->p_current_popup);
            return true;
        }
        return false;
    default:
        break;
    }
    return true;
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
    p_sys->win = intf_get_window(intf);

    view->view = video_player_create_ui(p_sys, parent);
    view->p_view_sys = p_sys;
    view->pf_event = video_player_callback;
    view->pf_stop = video_player_stop;

    return view;
}

void
destroy_video_player(interface_view *view)
{
    video_player_stop(view->p_view_sys);
    playback_service_set_evas_video(view->p_view_sys->p_ps, NULL);
    free(view->p_view_sys);
    free(view);
}
