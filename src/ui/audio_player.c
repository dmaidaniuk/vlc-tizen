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

#include <Elementary.h>
#include <math.h>

#include "playback_service.h"
#include "interface.h"
#include "audio_player.h"
#include "ui/utils.h"

struct audio_player {
    interface *intf;
    playback_service *p_ps;
    playback_service_cbs_id *p_ps_cbs_id;

    bool save_state, shuffle_state, playlist_state, more_state, fs_state;
    double slider_event_time;


    Evas_Object *layout, *fs_layout;
    Evas_Object *popup;
    Evas_Object *slider, *fs_slider;
    Evas_Object *fs_time, *fs_total_time;
    Evas_Object *fs_title, *fs_sub_title;

    Evas_Object *play_pause_img;
    Evas_Object *fs_play_pause_img, *fs_previous_img, *fs_next_img;

    Evas_Object *fs_save_btn, *fs_playlist_btn, *fs_more_btn;
    Evas_Object *fs_repeat_btn, *fs_shuffle_btn;

    Evas_Object *speed_popup_layout, *speed_popup_progress;

    Ecore_Timer *long_press_timer, *mini_player_hide_timer;
};

static Evas_Object *
audio_player_create_popup(audio_player *mpd);

static void
audio_player_stop_delay_ui(audio_player *mpd, bool delayed);

typedef struct audio_popup_data
{
    int index;
    int id;
    Evas_Object *box, *genlist;
    Elm_Object_Item *item;
    audio_player *mpd;

} audio_popup_data_s;

typedef struct more_menu {
    int id;
    const char *text;
    const char *icon;
} more_menu;

#define MORE_JUMPTO 1
#define MORE_SPEED 2
#define MORE_SLEEP 3
#define MORE_END -1

more_menu more_menu_list[] = {
        //{ MORE_JUMPTO,  "Jump to Time",    "ic_jumpto_normal.png" }, // Not implemented
        { MORE_SPEED,   "Playback Speed",  "ic_speed_normal.png"  },
        //{ MORE_SLEEP,   "Sleep in",        "ic_sleep_normal.png"  }, // Not implemented
        { MORE_END },
};

double
audio_player_convert_slider_rate(double slider_value)
{
    return  pow(4, (slider_value / 100) - 1);
}

static void
audio_player_speed_slider_drag_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
    audio_player *mpd = data;

    /* */
    double value = elm_slider_value_get(mpd->speed_popup_progress);

    /* */
    playback_service_set_play_speed(mpd->p_ps, audio_player_convert_slider_rate(value));
}

static void
audio_player_speed_slider_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
    audio_player *mpd = data;
    char buf[8];

    /* */
    double value = elm_slider_value_get(mpd->speed_popup_progress);

    /* Update the UI */
    sprintf(buf, "%.2fx", audio_player_convert_slider_rate(value));
    elm_object_part_text_set(mpd->speed_popup_layout, "swallow.value", buf);
}

static double
audio_player_get_speed_value(audio_player *mpd)
{
    // Note: 1.0 represents the normal speed, 2 double speed, 0.5 half speed and so on.
    double rate = playback_service_get_play_speed(mpd->p_ps);
    return 100 * (1 + log(rate) / log(4));
}

static void
audio_player_popup_playback_speed(audio_player *mpd)
{
    Evas_Object *layout;

    /* */
    mpd->popup = elm_popup_add(intf_get_main_naviframe(mpd->intf));
    elm_popup_orient_set(mpd->popup, ELM_POPUP_ORIENT_CENTER);

    /* */
    mpd->speed_popup_layout = layout = elm_layout_add(mpd->popup);

    /* */
    elm_layout_file_set(layout, AUDIOPLAYERSPEED_EDJ, "audio_player_speed");
    evas_object_show(layout);

    /* */
    Evas_Object *slider = mpd->speed_popup_progress = elm_slider_add(layout);
    elm_slider_horizontal_set(slider, EINA_TRUE);
    elm_slider_min_max_set(slider, 0, 200);
    elm_slider_value_set(slider, audio_player_get_speed_value(mpd));
    elm_object_part_content_set(layout, "swallow.speed", slider);
    evas_object_smart_callback_add(slider, "changed", audio_player_speed_slider_changed_cb, mpd);
    evas_object_smart_callback_add(slider, "slider,drag,stop", audio_player_speed_slider_drag_stop_cb, mpd);

    /* */
    elm_object_content_set(mpd->popup, layout);
    evas_object_show(mpd->popup);
}

static char *
gl_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    audio_popup_data_s *apd = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(apd->item);
    char *buf;

    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.text.main.left")) {
            asprintf(&buf, "%s", more_menu_list[apd->index].text);

            return buf;
        }
    }
    return NULL;
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    audio_popup_data_s *apd = data;
    const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(apd->item);
    Evas_Object *content = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (itc->item_style && !strcmp(itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.icon.1")) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
            Evas_Object *icon = create_icon(content, more_menu_list[apd->index].icon);
            elm_layout_content_set(content, "elm.swallow.content", icon);
        }
    }

    return content;
}

static void
popup_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_popup_data_s *apd = data;
    /* Generate the view depending on which panel genlist item is selected */
    switch(apd->id){

    case MORE_JUMPTO:
        /* */
        evas_object_del(apd->mpd->popup);
        /* */
        elm_image_file_set(apd->mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(apd->mpd->fs_more_btn);
        /* Update the button state (pressed or not) */
        apd->mpd->more_state = false;

        //TODO : Add a JumTo fcn to the current list

        break;

    case MORE_SPEED:
        /* */
        evas_object_del(apd->mpd->popup);
        /* */
        elm_image_file_set(apd->mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(apd->mpd->fs_more_btn);
        /* Update the button state (pressed or not) */
        apd->mpd->more_state = false;

        audio_player_popup_playback_speed(apd->mpd);

        break;

    case MORE_SLEEP:
        /* */
        evas_object_del(apd->mpd->popup);
        /* */
        elm_image_file_set(apd->mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(apd->mpd->fs_more_btn);
        /* Update the button state (pressed or not) */
        apd->mpd->more_state = false;

        //TODO : Add a Sleep fcn of the current list
        break;
    }
}

static void
audio_player_popup_free_cb(void *data, Evas_Object *obj, void *event_info)
{
    audio_popup_data_s *apd = data;
    free(apd);
}

static Evas_Object *
audio_player_create_popup(audio_player *mpd)
{
    Evas_Object *genlist;
    Elm_Object_Item *it;

    /* Set then create the Genlist object */
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "1line";
    itc->func.text_get = gl_text_get_cb;
    itc->func.content_get = gl_content_get_cb;

    genlist = elm_genlist_add(intf_get_main_naviframe(mpd->intf));

    /* Set the genlist scoller mode */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    /* Enable the genlist HOMOGENEOUS mode */
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    /* Enable the genlist COMPRESS mode */
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    /* Stop when the panel list names is all used */
    for (int index = 0; more_menu_list[index].id >= 0; index++) {

        audio_popup_data_s *apd = malloc(sizeof(*apd));
        /* Put the index and the gui_data in the cb_data struct for callbacks */
        apd->index = index;
        apd->id = more_menu_list[index].id;
        apd->mpd = mpd;

        it = elm_genlist_item_append(genlist,
                itc,                        /* genlist item class               */
                apd,                        /* genlist item class user data     */
                NULL,                       /* genlist parent item              */
                ELM_GENLIST_ITEM_NONE,      /* genlist item type                */
                popup_selected_cb,          /* genlist select smart callback    */
                apd);                       /* genlist smart callback user data */

        apd->item = it;
        elm_object_item_del_cb_set(it, audio_player_popup_free_cb);
    }

    elm_genlist_item_class_free(itc);

    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

    return genlist;
}


/////////////////////////////////////////////////////////


static void
time_to_string(char *psz_time, double time)
{
    int sec = ((int)time % 60);
    time /= 60;
    int min = ((int)time % 60);
    time /= 60;
    int hours = (int) time;

    if (hours)
        sprintf(psz_time, "%2.2d:%2.2d:%2.2d", hours, min, sec);
    else
        sprintf(psz_time, "%2.2d:%2.2d", min, sec);
}

static void
evas_change_time(Evas_Object *obj, double time)
{
    char psz_time[strlen("00:00:00")];
    time_to_string(psz_time, time);
    elm_object_text_set(obj, psz_time);
}


static void
player_update_sliders(audio_player *mpd, double i_pos)
{
    if (mpd->slider)
        elm_slider_value_set (mpd->slider, i_pos);
    if (mpd->fs_slider)
        elm_slider_value_set (mpd->fs_slider, i_pos);
}

static void
audio_player_reset_states(audio_player *mpd)
{
    mpd->fs_state = false;
    mpd->save_state = false;
    mpd->shuffle_state = false;
    mpd->playlist_state = false;
    mpd->more_state = false;
}

bool
audio_player_play_state(audio_player *mpd)
{
    /* Return the current play/pause state*/
    return playback_service_is_playing(mpd->p_ps);
}

bool
audio_player_save_state(audio_player *mpd)
{
    /* Return the current save button state*/
    return mpd->save_state;
}

bool
audio_player_playlist_state(audio_player *mpd)
{
    /* Return the current playlist button state*/
    return mpd->playlist_state;
}

bool
audio_player_shuffle_state(audio_player *mpd)
{
    /* Return the current shuffle state*/
    return mpd->shuffle_state;
}

bool
audio_player_more_state(audio_player *mpd)
{
    /* Return the current more button state*/
    return mpd->more_state;
}

enum PLAYLIST_REPEAT
repeat_mode(audio_player *mpd)
{
    /* Return the current repeat state*/
    return playback_service_get_repeat_mode(mpd->p_ps);
}

bool
audio_player_fs_state(audio_player *mpd)
{
    return mpd->fs_state;
}

bool
audio_player_handle_back_key(audio_player *mpd)
{
    if (mpd->popup)
    {
        mpd->more_state = false;
        elm_image_file_set(mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        evas_object_del(mpd->popup);
        mpd->popup = NULL;
        return true;
    }
    if (audio_player_fs_state(mpd) == true)
    {
        audio_player_collapse_fullscreen_player(mpd);
        return true;
    }

    return false;
}

static void
update_player_play_pause(audio_player* mpd)
{
    bool b_playing = playback_service_is_playing(mpd->p_ps);
    elm_image_file_set(mpd->play_pause_img, b_playing ? ICON_DIR "ic_pause_circle_normal_o.png" : ICON_DIR "ic_play_circle_normal_o.png", NULL);
    elm_image_file_set(mpd->fs_play_pause_img, b_playing ? ICON_DIR "ic_pause_circle_normal_o.png" : ICON_DIR "ic_play_circle_normal_o.png", NULL);
    evas_object_show(mpd->play_pause_img);
    evas_object_show(mpd->fs_play_pause_img);
}

static void
update_player_title_display(audio_player* mpd, const char *title)
{
    elm_object_part_text_set(mpd->layout, "swallow.title", title);
    elm_object_part_text_set(mpd->fs_layout, "title_text", title);
}

static void
update_player_artist_display(audio_player* mpd, const char *artist)
{
    elm_object_part_text_set(mpd->layout, "swallow.subtitle", artist);
    elm_object_part_text_set(mpd->fs_layout, "subtitle_text", artist);
}

static void
update_player_cover_display(audio_player* mpd, const char *path)
{
    evas_object_del(elm_object_part_content_get(mpd->layout, "swallow.cover"));
    evas_object_del(elm_object_part_content_get(mpd->fs_layout, "cover"));

    if (path)
    {
        elm_object_part_content_set(mpd->layout, "swallow.cover", create_image(mpd->layout, path));
        elm_object_part_content_set(mpd->fs_layout, "cover", create_image(mpd->fs_layout, path));
    }
    else
    {
        elm_object_part_content_set(mpd->layout, "swallow.cover", create_icon(mpd->layout, "background_cone.png"));
        elm_object_part_content_set(mpd->fs_layout, "cover", create_icon(mpd->fs_layout, "background_cone.png"));
    }
}

static void
update_player_display(audio_player* mpd)
{
    const char *psz_meta;
    media_item *p_mi = playback_service_list_get_item(mpd->p_ps);

    if (p_mi)
    {
        if ((psz_meta = media_item_title(p_mi)) != NULL)
            update_player_title_display(mpd, psz_meta);

        if ((psz_meta = media_item_artist(p_mi)) != NULL)
            update_player_artist_display(mpd, psz_meta);
        else
            update_player_artist_display(mpd, "Unknown Artist");

        const char *cover = p_mi->psz_snapshot;
        if (cover)
            update_player_cover_display(mpd, cover);
        else
            update_player_cover_display(mpd, NULL);
    }

    /* Change the play/pause button img */
    update_player_play_pause(mpd);
}

static void
play_pause_audio_player_cb(void *data, Evas_Object *obstopj EINA_UNUSED, void *event_info)
{
    audio_player *mpd = data;

    playback_service_toggle_play_pause(mpd->p_ps);

    update_player_display(mpd);
}

static void
play_pause_fs_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_player *mpd = data;

    playback_service_toggle_play_pause(mpd->p_ps);

    evas_object_show(mpd->fs_play_pause_img);
    update_player_display(mpd);
}

static Eina_Bool
play_pause_timer_cb(void *data)
{
   audio_player *mpd = data;
   mpd->long_press_timer = NULL;
   audio_player_stop(mpd);
   return ECORE_CALLBACK_CANCEL;
}

static void
play_pause_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    audio_player *mpd = data;
    Evas_Event_Mouse_Down *ev = event_info;
    if (ev->button != 1) return;

    mpd->long_press_timer = ecore_timer_add(1.0, play_pause_timer_cb, mpd);
}

static void
play_pause_mouse_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    audio_player *mpd = data;
    Evas_Event_Mouse_Up *ev = event_info;
    if (ev->button != 1) return;

    if (mpd->long_press_timer)
    {
        ecore_timer_del(mpd->long_press_timer);
        mpd->long_press_timer = NULL;
        // click
        play_pause_audio_player_cb(mpd, obj, event_info);
    }
}

static void
fs_save_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_player *mpd = data;

    if(audio_player_save_state(mpd) == false)
    {
        /* Change the save button img */
        elm_image_file_set(mpd->fs_save_btn, ICON_DIR"ic_save_pressed.png", NULL);
        /* */
        evas_object_show(mpd->fs_save_btn);

        /* Update the save button state of the player */
        mpd->save_state = true;
    }
    else
    {
        /* Change the save button img */
        elm_image_file_set(mpd->fs_save_btn, ICON_DIR"ic_save_normal.png", NULL);
        /* */
        evas_object_show(mpd->fs_save_btn);

        /* Update the save button state of the player */
        mpd->save_state = false;
    }
}

static void
fs_playlist_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_player *mpd = data;

    if(audio_player_playlist_state(mpd) == false)
    {
        /* Change the playlist button img */
        elm_image_file_set(mpd->fs_playlist_btn, ICON_DIR"ic_playlist_pressed.png", NULL);
        /* */
        evas_object_show(mpd->fs_playlist_btn);

        /* Update the playlist button state of the player */
        mpd->playlist_state = true;
    }
    else
    {
        /* Change the playlist button img */
        elm_image_file_set(mpd->fs_playlist_btn, ICON_DIR"ic_playlist_normal.png", NULL);
        /* */
        evas_object_show(mpd->fs_playlist_btn);

        /* Update the playlist button state of the player */
        mpd->playlist_state = false;
    }

}

static void
audio_player_more_popup_close_cb(void *data, Evas_Object *obj, void *event_info)
{
    audio_player *mpd = data;

    mpd->more_state = false;
    elm_image_file_set(mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
    evas_object_del(obj);
}

static void
audio_player_more_popup_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    audio_player *mpd = data;

    mpd->popup = NULL;
}

static void
fs_more_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_player *mpd = data;

    if(audio_player_more_state(mpd) == false)
    {
        /* Create the more popup list */
        Evas_Object *popup_list;

        /* FIXME */
        mpd->popup = elm_popup_add(intf_get_main_naviframe(mpd->intf));

        /* Size the popup */
        evas_object_size_hint_min_set(mpd->popup, 200, 200);
        evas_object_size_hint_max_set(mpd->popup, 200, 200);

        /* Add the more list in the popup */
        popup_list = audio_player_create_popup(mpd); //FIXME

        if (application_get_system_version() >= TIZEN_VERSION_2_4_0)
        {
            // Workaround: popup menu are blank when shown in landscape
            // on devices running Tizen 2.4 and compiled with the 2.3 SDK.
            evas_object_size_hint_min_set(popup_list, 600, 100);
        }

        elm_object_content_set(mpd->popup, popup_list);
        evas_object_show(popup_list);
        /* */
        evas_object_show(mpd->popup);

        evas_object_smart_callback_add(mpd->popup, "block,clicked", audio_player_more_popup_close_cb, mpd);
        evas_object_event_callback_add(mpd->popup, EVAS_CALLBACK_FREE, audio_player_more_popup_free_cb, mpd);

        /* Change the more button img */
        elm_image_file_set(mpd->fs_more_btn, ICON_DIR"ic_more_circle_pressed_o.png", NULL);
        /* */
        evas_object_show(mpd->fs_more_btn);

        /* Update the more button state of the player */
        mpd->more_state = true;
    }
    else
    {
        /* */
        evas_object_del(mpd->popup);

        /* Change the more button img */
        elm_image_file_set(mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(mpd->fs_more_btn);

        /* Update the more button state of the player */
        mpd->more_state = false;
    }
}

static void
fs_repeat_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_player *mpd = data;

    if(repeat_mode(mpd) == REPEAT_NONE)
    {
        /* Change the repeat button img */
        elm_image_file_set(mpd->fs_repeat_btn, ICON_DIR"ic_repeat_pressed.png", NULL);
        /* */
        evas_object_show(mpd->fs_repeat_btn);

        playback_service_set_repeat_mode(mpd->p_ps, REPEAT_ALL);
    }
    else if(repeat_mode(mpd) == REPEAT_ALL)
    {
        /* Change the repeat button img */
        elm_image_file_set(mpd->fs_repeat_btn, ICON_DIR"ic_repeat_one_pressed.png", NULL);
        /* */
        evas_object_show(mpd->fs_repeat_btn);

        playback_service_set_repeat_mode(mpd->p_ps, REPEAT_ONE);
    }
    else
    {
        /* Change the repeat button img */
        elm_image_file_set(mpd->fs_repeat_btn, ICON_DIR"ic_repeat_normal.png", NULL);
        /* */
        evas_object_show(mpd->fs_repeat_btn);

        playback_service_set_repeat_mode(mpd->p_ps, REPEAT_NONE);
    }
}

static void
fs_previous_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_player *mpd = data;

    playback_service_list_set_prev(mpd->p_ps);
}

static void
fs_next_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_player *mpd = data;

    playback_service_list_set_next(mpd->p_ps);
}

static void
fs_shuffle_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    audio_player *mpd = data;

    if(audio_player_shuffle_state(mpd) == false)
    {
        /* Change the shuffle button img */
        elm_image_file_set(mpd->fs_shuffle_btn, ICON_DIR"ic_shuffle_pressed.png", NULL);
        /* */
        evas_object_show(mpd->fs_shuffle_btn);

        /* Update the shuffle button state of the player */
        mpd->shuffle_state = true;
    }
    else
    {
        /* Change the shuffle button img */
        elm_image_file_set(mpd->fs_shuffle_btn, ICON_DIR"ic_shuffle_normal.png", NULL);
        /* */
        evas_object_show(mpd->fs_shuffle_btn);

        /* Update the shuffle button state of the player */
        mpd->shuffle_state = false;
    }
}

static void
slider_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
    audio_player *mpd = data;

    // The smart callback "delay,changed" only works properly when
    // seeking without releasing the finger/mouse but fail, more often
    // than not, on single tap/click.
    // Let's fix it by filtering events ourselves.

    if (ecore_time_get() - mpd->slider_event_time > 0.5)
    {
        playback_service_seek_pos(mpd->p_ps, elm_slider_value_get(obj));
        mpd->slider_event_time = ecore_time_get();
    }
}

static void
slider_drag_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
    audio_player *mpd = data;
    playback_service_seek_pos(mpd->p_ps, elm_slider_value_get(obj));
}

static void
set_sliders_callbacks(audio_player *mpd, Evas_Object *slider)
{
    evas_object_smart_callback_add(slider, "changed", slider_changed_cb, mpd);
    evas_object_smart_callback_add(slider, "slider,drag,stop", slider_drag_stop_cb, mpd);
}

static void
swallow_mini_player(audio_player *mpd, Evas_Object *layout)
{
    elm_layout_file_set(layout, AUDIOPLAYERMINI_EDJ, "audio_player");

    /* set the progress bar at the top of the table */
    mpd->slider = elm_slider_add(layout);
    elm_slider_horizontal_set(mpd->slider, EINA_TRUE);
    elm_object_part_content_set(layout, "swallow.progress", mpd->slider);
    set_sliders_callbacks(mpd, mpd->slider);
    player_update_sliders(mpd, playback_service_get_pos(mpd->p_ps));

    /* set the cover image */
    Evas_Object *placeholder = create_icon(layout, "background_cone.png");
    elm_object_part_content_set(layout, "swallow.cover", placeholder);

    /* set the play/pause button */
    mpd->play_pause_img = create_icon(layout, "ic_pause_circle_normal_o.png");
    elm_object_part_content_set(layout, "swallow.play", mpd->play_pause_img);
}

void
audio_player_collapse_fullscreen_player(audio_player *mpd){
    /* Pop the previous view in the content naviframe */
    intf_show_previous_view(mpd->intf);
    /* Update the fullscreen state bool */
    mpd->fs_state = false;
    /* Show the mini player */
    intf_mini_player_visible_set(mpd->intf, true);
}

static Evas_Object*
add_fullscreen_item_table(audio_player *mpd, Evas_Object *parent)
{
    Evas_Object *layout = elm_layout_add(parent);;
    evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    elm_layout_file_set(layout, AUDIOPLAYER_EDJ, "audio_player");

    /* More button */
    if (mpd->more_state == FALSE)
    {
        mpd->fs_more_btn = create_icon(parent, "ic_more_circle_normal_o.png");
    }
    else {
        mpd->fs_more_btn = create_icon(parent, "ic_more_circle_pressed_o.png");
    }
    evas_object_show(mpd->fs_more_btn);
    elm_object_part_content_set(layout, "more_button", mpd->fs_more_btn);

    /* Cover */
    Evas_Object *placeholder = create_icon(parent, "background_cone.png");
    evas_object_show(placeholder);
    elm_object_part_content_set(layout, "cover", placeholder);

    /* Seek */
    mpd->fs_slider = elm_slider_add(parent);
    elm_slider_horizontal_set(mpd->fs_slider, EINA_TRUE);
    evas_object_show(mpd->fs_slider);
    elm_object_part_content_set(layout, "seek", mpd->fs_slider);
    set_sliders_callbacks(mpd, mpd->fs_slider);

    double i_pos = playback_service_get_time(mpd->p_ps);
    double i_len = playback_service_get_len(mpd->p_ps);

    player_update_sliders(mpd, i_pos);

    /* Play / Pause */
    mpd->fs_play_pause_img = create_icon(parent, "ic_pause_circle_normal_o.png");
    evas_object_show(mpd->fs_play_pause_img);
    elm_object_part_content_set(layout, "play_button", mpd->fs_play_pause_img);
    update_player_play_pause(mpd);

    /* Next / Previous */
    mpd->fs_previous_img = create_icon(parent, "ic_widget_previous_normal.png");
    elm_object_part_content_set(layout, "previous_button", mpd->fs_previous_img);
    mpd->fs_next_img = create_icon(parent, "ic_widget_next_normal.png");
    elm_object_part_content_set(layout, "next_button", mpd->fs_next_img);

    /* Elapsed time */
    mpd->fs_time = elm_label_add(parent);
    evas_object_show(mpd->fs_time);
    //elm_object_part_content_set(layout, "?", mpd->fs_time); // TODO
    evas_change_time(mpd->fs_time, i_pos);

    /* Total time */
    mpd->fs_total_time = elm_label_add(parent);
    evas_object_show(mpd->fs_total_time);
    //elm_object_part_content_set(layout, "?", mpd->fs_total_time); // TODO
    evas_change_time(mpd->fs_total_time, i_len);

    /* Repeat */
    enum PLAYLIST_REPEAT repeat_state = repeat_mode(mpd);
    if (repeat_state == REPEAT_NONE)
    {
        mpd->fs_repeat_btn = create_icon(parent, "ic_repeat_normal.png");
    }
    else if (repeat_state == REPEAT_ALL)
    {
        mpd->fs_repeat_btn = create_icon(parent, "ic_repeat_pressed.png");
    }
    else
    {
        mpd->fs_repeat_btn = create_icon(parent, "ic_repeat_one_pressed.png");
    }
    evas_object_show(mpd->fs_repeat_btn);
    elm_object_part_content_set(layout, "repeat_button", mpd->fs_repeat_btn);

    /* Shuffle */
    if (mpd->shuffle_state == FALSE){
        mpd->fs_shuffle_btn = create_icon(parent, "ic_shuffle_normal.png");
    }
    else {
        mpd->fs_shuffle_btn = create_icon(parent, "ic_shuffle_pressed.png");
    }
    evas_object_show(mpd->fs_shuffle_btn);
    elm_object_part_content_set(layout, "shuffle_button", mpd->fs_shuffle_btn);

    /* Callbacks */
    evas_object_smart_callback_add(mpd->fs_play_pause_img, "clicked", play_pause_fs_player_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_shuffle_btn, "clicked", fs_shuffle_player_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_repeat_btn, "clicked", fs_repeat_player_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_previous_img, "clicked", fs_previous_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_next_img, "clicked", fs_next_cb, mpd);
    //    evas_object_smart_callback_add(mpd->fs_save_btn, "clicked", fs_save_player_cb, mpd);
    //    evas_object_smart_callback_add(mpd->fs_playlist_btn, "clicked", fs_playlist_player_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_more_btn, "clicked", fs_more_player_cb, mpd);

    return layout;
}

static void
audio_player_show_fullscreen(audio_player *mpd)
{
    /* */
    intf_mini_player_visible_set(mpd->intf, false);

    /* Show the fullcreen box in the content naviframe */
    update_player_display(mpd);
    elm_naviframe_item_simple_push(intf_get_main_naviframe(mpd->intf), mpd->fs_layout);

    /* Update fullscreen state bool */
    mpd->fs_state = true;
}

static void
audio_player_fullscreen_edge_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
    audio_player_show_fullscreen(data);
}

static void
ps_on_new_time_cb(playback_service *p_ps, void *p_user_data, double i_time, double i_pos)
{
    audio_player *mpd = p_user_data;

    if (mpd->fs_time)
        evas_change_time(mpd->fs_time, i_time);
    player_update_sliders(mpd, i_pos);
}

static void
ps_on_new_len_cb(playback_service *p_ps, void *p_user_data, double i_len)
{
    audio_player *mpd = p_user_data;
    if (mpd->fs_total_time)
        evas_change_time(mpd->fs_total_time, i_len);
}

static void
ps_on_media_selected_cb(playback_service *p_ps, void *p_user_data, unsigned int i_pos, media_item *p_mi)
{
    audio_player *mpd = p_user_data;

    update_player_display(mpd);
}

static void
ps_on_started_cb(playback_service *p_ps, void *p_user_data, media_item *p_mi)
{
    audio_player *mpd = p_user_data;

    if (mpd->mini_player_hide_timer)
        ecore_timer_del(mpd->mini_player_hide_timer);

    update_player_display(mpd);

    /* Show the mini player only if it isn't already shown */
    if (intf_mini_player_visible_get(mpd->intf) == false && audio_player_fs_state(mpd) == false){
        intf_mini_player_visible_set(mpd->intf, true);
    }
}

static void
ps_on_playpause_cb(playback_service *p_ps, void *p_user_data, bool b_playing)
{
    audio_player *mpd = p_user_data;
    update_player_play_pause(mpd);
}

static void
ps_on_stopped_cb(playback_service *p_ps, void *p_user_data)
{
    audio_player_stop_delay_ui(p_user_data, true);
}

void
audio_player_start(audio_player *mpd, Eina_Array *array, int pos)
{
    audio_player_reset_states(mpd);

    playback_service_set_context(mpd->p_ps, PLAYLIST_CONTEXT_AUDIO);
    playback_service_list_clear(mpd->p_ps);

    media_item *p_mi;
    Eina_Array_Iterator iterator;
    unsigned int i;

    EINA_ARRAY_ITER_NEXT(array, i, p_mi, iterator)
    {
        playback_service_list_append(mpd->p_ps, p_mi);
    }

    eina_array_free(array);

    playback_service_list_set_pos(mpd->p_ps, pos);
    playback_service_start(mpd->p_ps, 0);

    update_player_display(mpd);
}

void
destroy_audio_player(audio_player *mpd)
{
    // Unregister from the playback service
    playback_service_unregister_callbacks(mpd->p_ps, mpd->p_ps_cbs_id);

    free(mpd);
}

audio_player*
audio_player_create(interface *intf, playback_service *p_ps, Evas_Object *layout)
{
    audio_player *mpd = calloc(1, sizeof(*mpd));
    mpd->intf = intf;
    mpd->p_ps = p_ps;
    mpd->layout = layout;

    playback_service_callbacks cbs = {
        .pf_on_media_added = NULL,
        .pf_on_media_removed = NULL,
        .pf_on_media_selected = ps_on_media_selected_cb,
        .pf_on_started = ps_on_started_cb,
        .pf_on_playpause = ps_on_playpause_cb,
        .pf_on_stopped = ps_on_stopped_cb,
        .pf_on_new_len = ps_on_new_len_cb,
        .pf_on_new_time = ps_on_new_time_cb,
        .pf_on_seek_done = NULL,
        .p_user_data = mpd,
        .i_ctx = PLAYLIST_CONTEXT_AUDIO,
    };
    mpd->p_ps_cbs_id = playback_service_register_callbacks(mpd->p_ps, &cbs);

    if (mpd->p_ps_cbs_id == NULL)
        LOGE("Unable to register the audio player");

    swallow_mini_player(mpd, layout);
    mpd->fs_layout = add_fullscreen_item_table(mpd, intf_get_main_naviframe(mpd->intf));

    Evas_Object *edje = elm_layout_edje_get(layout);

    /* Add button callbacks */
    evas_object_event_callback_add(mpd->play_pause_img, EVAS_CALLBACK_MOUSE_DOWN, play_pause_mouse_down_cb, mpd);
    evas_object_event_callback_add(mpd->play_pause_img, EVAS_CALLBACK_MOUSE_UP, play_pause_mouse_up_cb, mpd);

    edje_object_signal_callback_add(edje, "*clicked*", "expand_region", audio_player_fullscreen_edge_cb, mpd);

    /* Put the mini player at the bottom of the content_box */
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, 1.0);

    update_player_display(mpd);
    return mpd;
}

Eina_Bool
audio_player_mini_hide_cb(void *data)
{
    audio_player *mpd = data;

    /* Hide the player */
    intf_mini_player_visible_set(mpd->intf, false);

    return ECORE_CALLBACK_CANCEL;
}

static void
audio_player_stop_delay_ui(audio_player *mpd, bool delayed)
{
    /* Stop the player */
    playback_service_stop(mpd->p_ps);

    /* Reset the fullscreen state */
    mpd->fs_state = false;

    if (!delayed)
    {
        /* Hide the player */
        intf_mini_player_visible_set(mpd->intf, false);
    }
    else
    {
        if (mpd->mini_player_hide_timer)
            ecore_timer_del(mpd->mini_player_hide_timer);

        /* Hide the player after a small delay to avoid flickering between tracks */
        mpd->mini_player_hide_timer = ecore_timer_add(0.5, audio_player_mini_hide_cb, mpd);
    }
}

void
audio_player_stop(audio_player *mpd)
{
    audio_player_stop_delay_ui(mpd, false);
}
