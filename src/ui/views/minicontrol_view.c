/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Ludovic Fauvet <etix@videolan.org>
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
#include "minicontrol_view.h"
#include "playback_service.h"
#include "ui/utils.h"

#include <minicontrol-provider.h>
#include <sys/time.h>

struct minicontrol
{
    playback_service *p_ps;
    application *p_app;
    Evas_Object *win;

    Evas_Object *layout;
    Evas_Object *play_button;
    Evas_Object *next_button;
    Evas_Object *prev_button;
    Evas_Object *cover;
    Evas_Object *progress;

    struct timeval button_last_event;
};

void
mini_control_visibility_set(minicontrol *mc, Eina_Bool visible)
{
    visible ? evas_object_show(mc->win) : evas_object_hide(mc->win);
}

void
mini_control_playing_set(minicontrol *mc, Eina_Bool playing)
{
    elm_image_file_set(mc->play_button, playing ? ICON_DIR "ic_pause_circle_normal_o.png" : ICON_DIR "ic_play_circle_normal_o.png", NULL);
}

void
mini_control_title_set(minicontrol *mc, const char* title)
{
    elm_object_part_text_set(mc->layout, "swallow.title", title);
}

void
mini_control_progress_set(minicontrol *mc, double progress)
{
    elm_slider_value_set(mc->progress, progress);
}

void
mini_control_cover_set(minicontrol *mc, const char* path)
{
    if (!path)
        elm_object_part_content_set(mc->layout, "swallow.cover", create_icon(mc->layout, "background_cone.png"));
    else
        elm_object_part_content_set(mc->layout, "swallow.cover", create_image(mc->layout, path));
}

long
elapsed_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec) +
            ((t1.tv_usec - t0.tv_usec)/1000000.0);
}

static void
mini_control_background_action_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    minicontrol *mc = data;

    struct timeval t;
    gettimeofday(&t, 0);

    if (elapsed_msec(mc->button_last_event, t) <= 5)
        return;

    interface *p_intf = application_get_interface(mc->p_app);

    intf_raise(p_intf);
    playback_service_set_auto_exit(mc->p_ps, false);
}

static void
mini_control_action_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    minicontrol *mc = data;

    if (obj == mc->play_button)
    {
        playback_service_toggle_play_pause(mc->p_ps);
    }
    else if (obj == mc->next_button)
    {
        playback_service_list_set_next(mc->p_ps);
    }
    else if (obj == mc->prev_button)
    {
        playback_service_list_set_prev(mc->p_ps);
    }
    else
    {
        return;
    }

    gettimeofday(&mc->button_last_event, 0);
}

// mini_control_event_cb doesn't provide an embedded opaque object...
static playback_service* ps = NULL;

static void
mini_control_event_cb(minicontrol_viewer_event_e event_type, bundle *event_arg)
{
    if (event_type == MINICONTROL_VIEWER_EVENT_HIDE || event_type == MINICONTROL_EVENT_REQUEST_HIDE)
        playback_service_stop_notify(ps, true);
}

minicontrol*
mini_control_view_create(playback_service *p_ps, application *p_app)
{
    minicontrol *mc = calloc(1, sizeof(*mc));
    mc->p_ps = p_ps;
    mc->p_app = p_app;
    ps = p_ps;

    const Evas_Coord width = 720;
    const Evas_Coord height = 120;

    /* main widget */
    mc->win = minicontrol_create_window("quickpanel", MINICONTROL_TARGET_VIEWER_QUICK_PANEL, mini_control_event_cb);
    evas_object_resize(mc->win, width, height);
    evas_object_hide(mc->win);

    /* Create layout and set the theme */
    Evas_Object *wlayout = elm_layout_add(mc->win);
    evas_object_resize(wlayout, width, height);
    //elm_win_resize_object_add(mc->win, layout);
    elm_layout_theme_set(wlayout, "layout", "application", "default");
    evas_object_show(wlayout);

    /* Create the background */
    Evas_Object *bg = elm_bg_add(wlayout);
    elm_bg_color_set(bg, 255, 255, 255);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(bg);

    /* Set the background to the theme */
    elm_object_part_content_set(wlayout, "elm.swallow.bg", bg);

    /* */
    Evas_Object *layout = mc->layout = elm_layout_add(mc->win);
    elm_object_part_content_set(wlayout, "elm.swallow.content", layout);

    /* */
    elm_layout_file_set(layout, QUICKPANELCONTROLS_EDJ, "quick_panel_controls");
    evas_object_show(layout);

    mc->play_button = create_icon(layout, "ic_play_circle_normal_o.png");
    mc->next_button = create_icon(layout, "ic_widget_next_normal.png");
    mc->prev_button = create_icon(layout, "ic_widget_previous_normal.png");
    mc->cover = create_icon(layout, "background_cone.png");

    elm_object_part_content_set(layout, "swallow.play", mc->play_button);
    elm_object_part_content_set(layout, "swallow.cover", mc->cover);
    elm_object_part_content_set(layout, "swallow.previous", mc->prev_button);
    elm_object_part_content_set(layout, "swallow.next", mc->next_button);

    evas_object_event_callback_add(mc->play_button, EVAS_CALLBACK_MOUSE_UP, mini_control_action_cb, mc);
    evas_object_event_callback_add(mc->next_button, EVAS_CALLBACK_MOUSE_UP, mini_control_action_cb, mc);
    evas_object_event_callback_add(mc->prev_button, EVAS_CALLBACK_MOUSE_UP, mini_control_action_cb, mc);

    Evas_Object *edje = elm_layout_edje_get(layout);
    edje_object_signal_callback_add(edje, "mouse,clicked,1", "hub_background", mini_control_background_action_cb, mc);

    /* */
    Evas_Object *progress = mc->progress = elm_slider_add(layout);
    elm_slider_horizontal_set(progress, EINA_TRUE);
    elm_object_disabled_set(progress, EINA_TRUE);
    elm_object_part_content_set(layout, "swallow.progress", progress);

    return mc;
}

void
mini_control_destroy(minicontrol *mc)
{
    evas_object_del(mc->win);
    free(mc);
}
