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

#include "playback_service.h"
#include "interface.h"
#include "audio_player.h"
#include "ui/utils.h"

struct mini_player {
    interface *intf;
    playback_service *p_ps;
    playback_service_cbs_id *p_ps_cbs_id;

    bool save_state, shuffle_state, playlist_state, more_state, fs_state;

    Evas_Object *fs_table, *popup;
    Evas_Object *fullscreen_box;
    Evas_Object *slider, *fs_slider;
    Evas_Object *cover, *fs_cover, *fs_time, *fs_total_time;
    Evas_Object *title, *sub_title, *fs_title, *fs_sub_title;

    Evas_Object *play_pause_img;
    Evas_Object *fs_play_pause_img;

    Evas_Object *fs_save_btn, *fs_playlist_btn, *fs_more_btn;
    Evas_Object *fs_repeat_btn, *fs_shuffle_btn;
};

static Evas_Object *
create_audio_popup(mini_player *mpd);

typedef struct audio_popup_data
{
    int index;
    Evas_Object *box, *genlist;
    Elm_Object_Item *item;
    mini_player *mpd;

} audio_popup_data_s;

/* Set the panel list labels */
const char *audio_popup_list[] = {
        "Jump to Time", "Playback Speed", "Sleep in"
};

/* Set the panel list icons */
const char *audio_popup_icon_names[] = {
        "jumpto", "speed", "sleep"
};


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
            asprintf(&buf, "%s", audio_popup_list[apd->index]);

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
            Evas_Object *icon = create_icon(content, audio_popup_icon_names[apd->index]);
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
    switch(apd->index){

    case 0:
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

    case 1:
        /* */
        evas_object_del(apd->mpd->popup);
        /* */
        elm_image_file_set(apd->mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(apd->mpd->fs_more_btn);
        /* Update the button state (pressed or not) */
        apd->mpd->more_state = false;

        //TODO : Add a Playback Speed fcn of the current list

        break;

    case 2:
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
mini_player_popup_free_cb(void *data, Evas_Object *obj, void *event_info)
{
    audio_popup_data_s *apd = data;
    free(apd);
}

static Evas_Object *
create_audio_popup(mini_player *mpd)
{
    Evas_Object *genlist;
    Elm_Object_Item *it;

    /* Set popup max size */
    Evas_Object *box = elm_box_add(mpd->popup);
    evas_object_size_hint_min_set(box, 300, 300);
    evas_object_size_hint_max_set(box, 300, 300);

    /* Set then create the Genlist object */
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "1line";
    itc->func.text_get = gl_text_get_cb;
    itc->func.content_get = gl_content_get_cb;

    genlist = elm_genlist_add(box);

    /* Set the genlist scoller mode */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    /* Enable the genlist HOMOGENEOUS mode */
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    /* Enable the genlist COMPRESS mode */
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    /* Stop when the panel list names is all used */
    for (int index = 0; index < 3; index++) {

        audio_popup_data_s *apd = malloc(sizeof(*apd));
        /* Put the index and the gui_data in the cb_data struct for callbacks */
        apd->index = index;
        apd->mpd = mpd;

        it = elm_genlist_item_append(genlist,
                itc,                        /* genlist item class               */
                apd,                        /* genlist item class user data     */
                NULL,                       /* genlist parent item              */
                ELM_GENLIST_ITEM_NONE,      /* genlist item type                */
                popup_selected_cb,          /* genlist select smart callback    */
                apd);                       /* genlist smart callback user data */

        apd->item = it;
        elm_object_item_del_cb_set(it, mini_player_popup_free_cb);
    }

    elm_box_pack_end(box, genlist);
    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(genlist);

    elm_genlist_item_class_free(itc);

    return box;
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
player_update_sliders(mini_player *mpd, double i_pos)
{
    if (mpd->slider)
        elm_slider_value_set (mpd->slider, i_pos);
    if (mpd->fs_slider)
        elm_slider_value_set (mpd->fs_slider, i_pos);
}

static void
mini_player_reset_states(mini_player *mpd)
{
    mpd->fs_state = false;
    mpd->save_state = false;
    mpd->shuffle_state = false;
    mpd->playlist_state = false;
    mpd->more_state = false;
}

bool
mini_player_play_state(mini_player *mpd)
{
    /* Return the current play/pause state*/
    return playback_service_is_playing(mpd->p_ps);
}

bool
save_state(mini_player *mpd)
{
    /* Return the current save button state*/
    return mpd->save_state;
}

bool
playlist_state(mini_player *mpd)
{
    /* Return the current playlist button state*/
    return mpd->playlist_state;
}

bool
shuffle_state(mini_player *mpd)
{
    /* Return the current shuffle state*/
    return mpd->shuffle_state;
}

bool
more_state(mini_player *mpd)
{
    /* Return the current more button state*/
    return mpd->more_state;
}

enum PLAYLIST_REPEAT
repeat_mode(mini_player *mpd)
{
    /* Return the current repeat state*/
    return playback_service_get_repeat_mode(mpd->p_ps);
}

bool
mini_player_fs_state(mini_player *mp)
{
    return mp->fs_state;
}

bool
audio_player_handle_back_key(mini_player *mp)
{
    if (mp->popup)
    {
        evas_object_del(mp->popup);
        return true;
    }
    if (mini_player_fs_state(mp) == true)
    {
        collapse_fullscreen_player(mp);
        return true;
    }

    return false;
}

static void
update_player_play_pause(mini_player* mpd)
{
    bool b_playing = playback_service_is_playing(mpd->p_ps);
    elm_image_file_set(mpd->play_pause_img, b_playing ? ICON_DIR "ic_pause_circle_normal_o.png" : ICON_DIR "ic_play_circle_normal_o.png", NULL);
    elm_image_file_set(mpd->fs_play_pause_img, b_playing ? ICON_DIR "ic_pause_circle_normal_o.png" : ICON_DIR "ic_play_circle_normal_o.png", NULL);
    evas_object_show(mpd->play_pause_img);
}

static void
update_player_display(mini_player* mpd)
{
    media_item *p_mi = playback_service_list_get_item(mpd->p_ps);

    if (p_mi)
    {
        const char *psz_meta = media_item_title(p_mi);
        if (psz_meta)
        {
            elm_object_text_set(mpd->title, psz_meta);
            elm_object_text_set(mpd->fs_title, psz_meta);
        }
        psz_meta = media_item_artist(p_mi);
        if (psz_meta)
        {
            elm_object_text_set(mpd->sub_title, psz_meta);
            elm_object_text_set(mpd->fs_sub_title, psz_meta);
        }
    }

    /* Change the play/pause button img */
    update_player_play_pause(mpd);
}

static void
play_pause_mini_player_cb(void *data, Evas_Object *obstopj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;

    playback_service_toggle_play_pause(mpd->p_ps);

    update_player_display(mpd);
}

static void
play_pause_fs_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;

    playback_service_toggle_play_pause(mpd->p_ps);

    evas_object_show(mpd->fs_play_pause_img);
    update_player_display(mpd);
}

static void
stop_mini_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;

    mini_player_stop(mpd);
}

static void
fs_save_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;

    if(save_state(mpd) == false)
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
    mini_player *mpd = data;

    if(playlist_state(mpd) == false)
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
    mini_player *mpd = data;

    mpd->more_state = false;
    elm_image_file_set(mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
    evas_object_del(obj);
}

static void
audio_player_more_popup_free_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    mini_player *mpd = data;

    mpd->popup = NULL;
}

static void
fs_more_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;

    if(more_state(mpd) == false)
    {
        /* Create the more popup list */
        Evas_Object *popup_list;

        /* FIXME */
        mpd->popup = elm_popup_add(intf_get_main_naviframe(mpd->intf));

        /* Size the popup */
        evas_object_size_hint_min_set(mpd->popup, 200, 200);
        evas_object_size_hint_max_set(mpd->popup, 200, 200);

        /* Add the more list in the popup */
        popup_list = create_audio_popup(mpd); //FIXME
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
    mini_player *mpd = data;

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
fs_shuffle_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;

    if(shuffle_state(mpd) == false)
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
slider_delay_changed_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
    mini_player *mpd = data;

    playback_service_seek_pos(mpd->p_ps, elm_slider_value_get(obj));
}

#if 0
static void
slider_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
    mini_player *mpd = data;
    double val = elm_slider_value_get(obj);
}

static void
slider_drag_start_cb(void *data, Evas_Object *obj, void *event_info)
{
    mini_player *mpd = data;
    double val = elm_slider_value_get(obj);
}

static void
slider_drag_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
    mini_player *mpd = data;
    double val = elm_slider_value_get(obj);
}
#endif

static void
set_sliders_callbacks(mini_player *mpd, Evas_Object *slider)
{
    evas_object_smart_callback_add(slider, "delay,changed", slider_delay_changed_cb, mpd);
#if 0
    evas_object_smart_callback_add(slider, "changed", slider_changed_cb, mpd);
    evas_object_smart_callback_add(slider, "slider,drag,start", slider_drag_start_cb, mpd);
    evas_object_smart_callback_add(slider, "slider,drag,stop", slider_drag_stop_cb, mpd);
#endif
}

static void
swallow_mini_player(mini_player *mpd, Evas_Object *layout)
{
    elm_layout_file_set(layout, AUDIOPLAYERMINIEDJ, "audio_player");

    /* set the progress bar at the top of the table */
    mpd->slider = elm_slider_add(layout);
    elm_slider_horizontal_set(mpd->slider, EINA_TRUE);
    elm_object_part_content_set(layout, "swallow.progress", mpd->slider);
    set_sliders_callbacks(mpd, mpd->slider);
    player_update_sliders(mpd, playback_service_get_pos(mpd->p_ps));

    /* set the cover image */
    mpd->cover = create_icon(layout, "background_cone.png");
    elm_object_part_content_set(layout, "swallow.cover", mpd->cover);

    /* set the title label */
    mpd->title = elm_label_add(layout);
    //elm_object_text_set(mpd->title, "<b>Title</b>");
    elm_object_part_content_set(layout, "swallow.title", mpd->title);

    /* set the sub title label */
    mpd->sub_title = elm_label_add(layout);
    //elm_object_text_set(mpd->sub_title, "Subtitle");
    elm_object_part_content_set(layout, "swallow.subtitle", mpd->sub_title);

    /* set the play/pause button */
    mpd->play_pause_img = create_icon(layout, "ic_pause_circle_normal_o.png");
    elm_object_part_content_set(layout, "swallow.play", mpd->play_pause_img);
}

static void
fullscreen_player_collapse_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;
    collapse_fullscreen_player(mpd);
}

void
collapse_fullscreen_player(mini_player *mpd){
    /* Pop the previous view in the content naviframe */
    intf_show_previous_view(mpd->intf);
    /* Update the fullscreen state bool */
    mpd->fs_state = false;
    /* Show the mini player */
    intf_mini_player_visible_set(mpd->intf, true);
}

static Evas_Object*
add_fullscreen_item_table(mini_player *mpd, Evas_Object *parent)
{
    Evas_Object *fs_slider, *fs_padding;

    /* */
    mpd->fs_table = elm_table_add(parent);
    /* */
    evas_object_size_hint_align_set(mpd->fs_table, 0.5, 0.5);
    /* */
    elm_table_padding_set(mpd->fs_table, ELM_SCALE_SIZE(0), ELM_SCALE_SIZE(0));
    /* */
    evas_object_show(mpd->fs_table);

    /* */
    mpd->fs_title = elm_label_add(parent);
    evas_object_show(mpd->fs_title);
    evas_object_size_hint_min_set(mpd->fs_title, 250, 25);
    evas_object_size_hint_max_set(mpd->fs_title, 250, 25);
    evas_object_size_hint_align_set(mpd->fs_title, 0.0, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_title, 0, 0, 3, 1);

    /* */
    mpd->fs_sub_title = elm_label_add(parent);
    evas_object_show(mpd->fs_sub_title);
    evas_object_size_hint_min_set(mpd->fs_sub_title, 250, 25);
    evas_object_size_hint_max_set(mpd->fs_sub_title, 250, 25);
    evas_object_size_hint_align_set(mpd->fs_sub_title, 0.0, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_sub_title, 0, 1, 3, 1);

    /* */
    if (mpd->save_state == FALSE)
    {
        mpd->fs_save_btn = create_icon(parent, "ic_save_normal.png");
    }
    else {
        mpd->fs_save_btn = create_icon(parent, "ic_save_pressed.png");
    }
    evas_object_size_hint_min_set(mpd->fs_save_btn, 50, 50);
    evas_object_size_hint_max_set(mpd->fs_save_btn, 50, 50);
    evas_object_size_hint_align_set(mpd->fs_save_btn, 1.0, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_save_btn, 3, 0, 1, 2);
    evas_object_show(mpd->fs_save_btn);

    /* */
    if (mpd->playlist_state == FALSE)
    {
        mpd->fs_playlist_btn = create_icon(parent, "ic_playlist_normal.png");
    }
    else {
        mpd->fs_playlist_btn = create_icon(parent, "ic_playlist_pressed.png");
    }
    evas_object_size_hint_min_set(mpd->fs_playlist_btn, 50, 50);
    evas_object_size_hint_max_set(mpd->fs_playlist_btn, 50, 50);
    evas_object_size_hint_align_set(mpd->fs_playlist_btn, 1.0, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_playlist_btn, 4, 0, 1, 2);
    evas_object_show(mpd->fs_playlist_btn);

    /* */
    if (mpd->more_state == FALSE)
    {
        mpd->fs_more_btn = create_icon(parent, "ic_more_circle_normal_o.png");
    }
    else {
        mpd->fs_more_btn = create_icon(parent, "ic_more_circle_pressed_o.png");
    }
    evas_object_size_hint_min_set(mpd->fs_more_btn, 50, 50);
    evas_object_size_hint_max_set(mpd->fs_more_btn, 50, 50);
    evas_object_size_hint_align_set(mpd->fs_more_btn, 1.0, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_more_btn, 5, 0, 1, 2);
    evas_object_show(mpd->fs_more_btn);

    /* */
    fs_padding = create_icon(parent, "");
    evas_object_size_hint_min_set(fs_padding, 400, 40);
    evas_object_size_hint_max_set(fs_padding, 400, 40);
    evas_object_size_hint_align_set(fs_padding, EVAS_HINT_FILL, EVAS_HINT_FILL);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, fs_padding, 0, 2, 6, 1);
    evas_object_show(fs_padding);

    /* */
    mpd->fs_cover = create_icon(parent, "background_cone.png");
    evas_object_size_hint_min_set(mpd->fs_cover, 400, 400);
    evas_object_size_hint_max_set(mpd->fs_cover, 400, 400);
    evas_object_size_hint_align_set(mpd->fs_cover, EVAS_HINT_FILL, EVAS_HINT_FILL);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_cover, 0, 3, 6, 1);
    evas_object_show(mpd->fs_cover);

    /* */
    fs_padding = create_icon(parent, "");
    evas_object_size_hint_min_set(fs_padding, 400, 40);
    evas_object_size_hint_max_set(fs_padding, 400, 40);
    evas_object_size_hint_align_set(fs_padding, EVAS_HINT_FILL, EVAS_HINT_FILL);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, fs_padding, 0, 4, 6, 1);
    evas_object_show(fs_padding);

    /* */
    fs_slider = elm_slider_add(mpd->fs_table);
    elm_slider_horizontal_set(fs_slider, EINA_TRUE);

    evas_object_size_hint_min_set(fs_slider, 400, 3);
    evas_object_size_hint_max_set(fs_slider, 400, 3);
    evas_object_size_hint_align_set(fs_slider, EVAS_HINT_FILL, 0.5);
    evas_object_size_hint_weight_set(fs_slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    evas_object_show(fs_slider);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, fs_slider, 0, 5, 6, 1);
    mpd->fs_slider = fs_slider;
    set_sliders_callbacks(mpd, mpd->fs_slider);
    double i_pos = playback_service_get_pos(mpd->p_ps);
    double i_time = playback_service_get_time(mpd->p_ps);
    double i_len = playback_service_get_len(mpd->p_ps);

    player_update_sliders(mpd, i_pos);

    /* */
    mpd->fs_play_pause_img = create_icon(mpd->fs_table, "ic_pause_circle_normal_o.png");
    update_player_play_pause(mpd);
    evas_object_size_hint_min_set(mpd->fs_play_pause_img, 200, 100);
    evas_object_size_hint_max_set(mpd->fs_play_pause_img, 200, 100);
    evas_object_size_hint_align_set(mpd->fs_play_pause_img, 0.5, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_play_pause_img, 1, 6, 4, 2);
    evas_object_show(mpd->fs_play_pause_img);

    /* */
    mpd->fs_time = elm_label_add(parent);
    evas_change_time(mpd->fs_time, i_time);
    evas_object_size_hint_min_set(mpd->fs_time, 100, 25);
    evas_object_size_hint_max_set(mpd->fs_time, 100, 25);
    evas_object_size_hint_align_set(mpd->fs_time, 1.0, 0.5);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_time, 0, 6, 1, 1);
    evas_object_show(mpd->fs_time);

    /* */
    mpd->fs_total_time = elm_label_add(parent);
    evas_change_time(mpd->fs_total_time, i_len);
    evas_object_size_hint_min_set(mpd->fs_total_time, 100, 25);
    evas_object_size_hint_max_set(mpd->fs_total_time, 100, 25);
    evas_object_size_hint_align_set(mpd->fs_total_time, 0.0, 0.5);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_total_time, 5, 6, 1, 1);
    evas_object_show(mpd->fs_total_time);

    /* */
    if (repeat_mode(mpd) == REPEAT_NONE)
    {
        mpd->fs_repeat_btn = create_icon(parent, "ic_repeat_normal.png");
    }
    else if (repeat_mode(mpd) == REPEAT_ALL)
    {
        mpd->fs_repeat_btn = create_icon(parent, "ic_repeat_pressed.png");
    }
    else
    {
        mpd->fs_repeat_btn = create_icon(parent, "ic_repeat_one_pressed.png");
    }
    evas_object_size_hint_min_set(mpd->fs_repeat_btn, 100, 50);
    evas_object_size_hint_max_set(mpd->fs_repeat_btn, 100, 50);
    evas_object_size_hint_align_set(mpd->fs_repeat_btn, 0.5, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_repeat_btn, 0, 7, 1, 1);
    evas_object_show(mpd->fs_repeat_btn);

    /* */
    if (mpd->shuffle_state == FALSE){
        mpd->fs_shuffle_btn = create_icon(parent, "ic_shuffle_normal.png");
    }
    else {
        mpd->fs_shuffle_btn = create_icon(parent, "ic_shuffle_pressed.png");
    }
    evas_object_size_hint_min_set(mpd->fs_shuffle_btn, 100, 50);
    evas_object_size_hint_max_set(mpd->fs_shuffle_btn, 100, 50);
    evas_object_size_hint_align_set(mpd->fs_shuffle_btn, 0.5, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_shuffle_btn, 5, 7, 1, 1);
    evas_object_show(mpd->fs_shuffle_btn);

    /* Add callbacks */
    evas_object_smart_callback_add(mpd->fs_title, "clicked", fullscreen_player_collapse_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_sub_title, "clicked", fullscreen_player_collapse_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_play_pause_img, "clicked", play_pause_fs_player_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_shuffle_btn, "clicked", fs_shuffle_player_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_repeat_btn, "clicked", fs_repeat_player_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_save_btn, "clicked", fs_save_player_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_playlist_btn, "clicked", fs_playlist_player_cb, mpd);
    evas_object_smart_callback_add(mpd->fs_more_btn, "clicked", fs_more_player_cb, mpd);
    LOGE("add_fullscreen_item_table!");

    return mpd->fs_table;
}


static Evas_Object*
create_fullscreen_player_view(mini_player *mpd, Evas_Object *parent)
{
    /* Add the box for the fullscreen player view */
    Evas_Object *fullscreen_box = elm_box_add(parent);
    /* Add the fullscreen table layout in the fullscreen box */
    Evas_Object *fullscreen_item_table = add_fullscreen_item_table(mpd, parent);
    /* */
    elm_box_pack_end(fullscreen_box, fullscreen_item_table);
    evas_object_show(fullscreen_item_table);
    /* The fullscreen box recalculate the layout of her children */
    elm_box_recalculate(fullscreen_box);
    /* Put the fullscreen box in the audio player structure */
    mpd->fullscreen_box = fullscreen_box;

    update_player_display(mpd);
    return mpd->fullscreen_box;
}

static void
mini_player_fullscreen_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;
    Evas_Object *fs_view;

    /* */
    intf_mini_player_visible_set(mpd->intf, false);

    /* Show the fullcreen box in the content naviframe */
    /* FIXME */
    fs_view = create_fullscreen_player_view(mpd, intf_get_main_naviframe(mpd->intf));
    elm_naviframe_item_simple_push(intf_get_main_naviframe(mpd->intf), fs_view);

    /* */
    evas_object_show(fs_view);
    /* Update fullscreen state bool */
    mpd->fs_state = true;
}

static void
ps_on_new_time_cb(playback_service *p_ps, void *p_user_data, double i_time, double i_pos)
{
    mini_player *mpd = p_user_data;

    if (mpd->fs_time)
        evas_change_time(mpd->fs_time, i_time);
    player_update_sliders(mpd, i_pos);
}

static void
ps_on_new_len_cb(playback_service *p_ps, void *p_user_data, double i_len)
{
    mini_player *mpd = p_user_data;
    if (mpd->fs_total_time)
        evas_change_time(mpd->fs_total_time, i_len);
}

static void
ps_on_started_cb(playback_service *p_ps, void *p_user_data, media_item *p_mi)
{
    update_player_display(p_user_data);
}

static void
ps_on_stopped_cb(playback_service *p_ps, void *p_user_data)
{
    mini_player_stop(p_user_data);
}

void
create_base_player(mini_player *mpd, Eina_Array *array, int pos)
{
    mini_player_reset_states(mpd);

    if (!mpd->p_ps_cbs_id)
    {
        playback_service_callbacks cbs = {
            .pf_on_media_added = NULL,
            .pf_on_media_removed = NULL,
            .pf_on_media_selected = NULL,
            .pf_on_started = ps_on_started_cb,
            .pf_on_stopped = ps_on_stopped_cb,
            .pf_on_new_len = ps_on_new_len_cb,
            .pf_on_new_time = ps_on_new_time_cb,
            .pf_on_seek_done = NULL,
            .p_user_data = mpd,
        };
        mpd->p_ps_cbs_id = playback_service_register_callbacks(mpd->p_ps, &cbs);
    }
    playback_service_set_context(mpd->p_ps, PLAYLIST_CONTEXT_AUDIO);
    playback_service_set_evas_video(mpd->p_ps, NULL);
    playback_service_list_clear(mpd->p_ps);

    media_item *p_mi;
    Eina_Array_Iterator iterator;
    unsigned int i;

    EINA_ARRAY_ITER_NEXT(array, i, p_mi, iterator)
    {
        playback_service_list_append(mpd->p_ps, p_mi);
    }

    eina_array_free(array);

    //media_item *p_mi = media_item_create(file_path, MEDIA_ITEM_TYPE_AUDIO);
    //playback_service_list_append(mpd->p_ps, p_mi);
    playback_service_list_set_pos(mpd->p_ps, pos);
    playback_service_start(mpd->p_ps, 0);

    update_player_display(mpd);

    /* Show the mini player only if it isn't already shown */
    if (intf_mini_player_visible_get(mpd->intf) == false){
        intf_mini_player_visible_set(mpd->intf, true);
    }

}

static void
mini_player_free(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    mini_player *mpd = data;

    playback_service_unregister_callbacks(mpd->p_ps, mpd->p_ps_cbs_id);
    free(mpd);
}

mini_player*
mini_player_create(interface *intf, playback_service *p_ps, Evas_Object *layout)
{
    mini_player *mpd = calloc(1, sizeof(*mpd));
    mpd->intf = intf;
    mpd->p_ps = p_ps;

    swallow_mini_player(mpd, layout);

    /* Add button callbacks */
    evas_object_smart_callback_add(mpd->play_pause_img, "clicked", play_pause_mini_player_cb, mpd);
    evas_object_smart_callback_add(mpd->cover, "clicked", stop_mini_player_cb, mpd);
    evas_object_smart_callback_add(mpd->title, "clicked", mini_player_fullscreen_cb, mpd);
    evas_object_smart_callback_add(mpd->sub_title, "clicked", mini_player_fullscreen_cb, mpd);

    /* Put the mini player at the bottom of the content_box */
    evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, 1.0);

    evas_object_event_callback_add(layout, EVAS_CALLBACK_FREE, mini_player_free, mpd);

    update_player_display(mpd);
    return mpd;
}

void
mini_player_stop(mini_player *mpd)
{
    /* Stop the player */
    playback_service_stop(mpd->p_ps);

    mpd->fs_state = false;

    /* Hide the player */
    intf_mini_player_visible_set(mpd->intf, false);
}
