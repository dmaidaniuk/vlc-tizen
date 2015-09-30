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
#include <Emotion.h>

#include "interface.h"
#include "audio_player.h"
#include "ui/utils.h"

struct mini_player {
    interface *intf;

    bool visible_state;
    bool play_state, save_state, shuffle_state, playlist_state, more_state, fs_state;
    int repeat_state;
    double len, pos;
    Evas_Object *emotion;
    Evas_Object *parent, *table, *fs_table, *popup;
    Evas_Object *mini_player_box, *box, *fullscreen_box;
    Evas_Object *slider, *fs_slider;
    Evas_Object *cover, *fs_cover, *fs_view, *fs_time, *fs_total_time;
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
        "Jump to Time", "PLayback Speed", "Sleep in"
};

/* Set the panel list icons */
const char *audio_popup_icon_names[] = {
        "jumpto", "speed", "sleep"
};

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
player_update_sliders(mini_player *mpd)
{
    double progress = (mpd->pos > 0.0 && mpd->len > 0.0) ? mpd->pos / mpd->len : 0.0;
    if (mpd->slider)
        elm_slider_value_set (mpd->slider, progress);
    if (mpd->fs_slider)
        elm_slider_value_set (mpd->fs_slider, progress);
}

static Evas_Object*
create_icon(Evas_Object *parent, int count)
{
    return create_image(parent, audio_popup_icon_names[count]);
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
            Evas_Object *icon = create_icon(content, apd->index);
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
        elm_popup_timeout_set(apd->mpd->popup, 0.1);
        /* */
        elm_image_file_set(apd->mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(apd->mpd->fs_more_btn);
        /* Update the button state (pressed or not) */
        apd->mpd->more_state = false;

        //TODO : Add a JumTo fcn to the current list

        free(apd);
        break;

    case 1:
        /* */
        elm_popup_timeout_set(apd->mpd->popup, 0.1);
        /* */
        elm_image_file_set(apd->mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(apd->mpd->fs_more_btn);
        /* Update the button state (pressed or not) */
        apd->mpd->more_state = false;

        //TODO : Add a Playback Speed fcn of the current list

        free(apd);
        break;

    case 2:
        /* */
        elm_popup_timeout_set(apd->mpd->popup, 0.1);
        /* */
        elm_image_file_set(apd->mpd->fs_more_btn, ICON_DIR"ic_more_circle_normal_o.png", NULL);
        /* */
        evas_object_show(apd->mpd->fs_more_btn);
        /* Update the button state (pressed or not) */
        apd->mpd->more_state = false;

        //TODO : Add a Sleep fcn of the current list

        free(apd);
    }
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

    audio_popup_data_s *apd = malloc(sizeof(*apd));
    apd->box = box;
    apd->genlist = genlist;

    /* Set the genlist scoller mode */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    /* Enable the genlist HOMOGENEOUS mode */
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    /* Enable the genlist COMPRESS mode */
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    free(apd);

    /* Stop when the panel list names is all used */
    for (int index = 0; index < 3; index++) {

        audio_popup_data_s *apd = malloc(sizeof(*apd));
        /* Put the index and the gui_data in the cb_data struct for callbacks */
        apd->index = index;
        apd->mpd = mpd;

        it = elm_genlist_item_append(genlist,
                itc,                            /* genlist item class               */
                apd,                            /* genlist item class user data     */
                NULL,                            /* genlist parent item              */
                ELM_GENLIST_ITEM_NONE,            /* genlist item type                */
                popup_selected_cb,                /* genlist select smart callback    */
                apd);                            /* genlist smart callback user data */

        apd->item = it;
    }

    elm_box_pack_end(box, genlist);
    evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(genlist);

    elm_genlist_item_class_free(itc);

    return box;
}



static void
mini_player_reset_states(mini_player *mpd)
{
    mpd->len = mpd->pos = 0.0;
    mpd->fs_state = false;
    mpd->save_state = false;
    mpd->shuffle_state = false;
    mpd->playlist_state = false;
    mpd->more_state = false;
    mpd->repeat_state = 0;
}

bool
mini_player_visibility_state(mini_player *mpd)
{
    /* Return the current visibility state*/
    return mpd->visible_state;
}

bool
mini_player_play_state(mini_player *mpd)
{
    /* Return the current play/pause state*/
    return mpd->play_state;
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

int
repeat_state(mini_player *mpd)
{
    /* Return the current repeat state*/
    return mpd->repeat_state;
}

bool
mini_player_fs_state(mini_player *mp)
{
    return mp->fs_state;
}

void
mini_player_show(mini_player *mpd)
{
    /* Add the previously created mini player to the box */
    elm_box_pack_end(intf_get_root_box(mpd->intf), mpd->mini_player_box);

    /* */
    evas_object_show(mpd->mini_player_box);

    /* Switch to current visibility state */
    mpd->visible_state = true;
}

void
mini_player_hide(mini_player *mpd)
{
    /* Dismiss the previously created mini player of the box */
    elm_box_unpack(intf_get_root_box(mpd->intf), mpd->mini_player_box);
    /* */

    if (mpd->mini_player_box)
        evas_object_hide(mpd->mini_player_box);

    /* Switch to current visibility state */
    mpd->visible_state = false;

}

static void
update_player_display(mini_player* mpd)
{
    const char *meta;

    if (!mpd->emotion)
        return;
    meta = emotion_object_meta_info_get(mpd->emotion, EMOTION_META_INFO_TRACK_TITLE);
    if (meta)
    {
        elm_object_text_set(mpd->title, meta);
        elm_object_text_set(mpd->fs_title, meta);
    }

    meta = emotion_object_meta_info_get(mpd->emotion, EMOTION_META_INFO_TRACK_ARTIST);
    if (meta)
    {
        elm_object_text_set(mpd->sub_title, meta);
        elm_object_text_set(mpd->fs_sub_title, meta);
    }

    /* Change the play/pause button img */
    elm_image_file_set(mpd->play_pause_img, mpd->play_state ? ICON_DIR "ic_pause_circle_normal_o.png" : ICON_DIR "ic_play_circle_normal_o.png", NULL);
    elm_image_file_set(mpd->fs_play_pause_img, mpd->play_state ? ICON_DIR "ic_pause_circle_normal_o.png" : ICON_DIR "ic_play_circle_normal_o.png", NULL);

    evas_object_show(mpd->play_pause_img);
}

static void
play_pause_mini_player_cb(void *data, Evas_Object *obstopj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;

    if (!mpd->emotion)
        return;

    mpd->play_state = emotion_object_play_get(mpd->emotion);
    emotion_object_play_set(mpd->emotion, !mpd->play_state);
    mpd->play_state = !mpd->play_state;

    update_player_display(mpd);
}

static void
play_pause_fs_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;

    if (!mpd->emotion)
        return;

    mpd->play_state = emotion_object_play_get(mpd->emotion);
    emotion_object_play_set(mpd->emotion, !mpd->play_state);
    mpd->play_state = !mpd->play_state;

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
        elm_popup_timeout_set(mpd->popup, 0.0);

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

    if(repeat_state(mpd) == 0)
    {
        /* Change the repeat button img */
        elm_image_file_set(mpd->fs_repeat_btn, ICON_DIR"ic_repeat_pressed.png", NULL);
        /* */
        evas_object_show(mpd->fs_repeat_btn);

        /* Update the repeat button state of the player */
        mpd->repeat_state = 1;
    }
    else if(repeat_state(mpd) == 1)
    {
        /* Change the repeat button img */
        elm_image_file_set(mpd->fs_repeat_btn, ICON_DIR"ic_repeat_one_pressed.png", NULL);
        /* */
        evas_object_show(mpd->fs_repeat_btn);

        /* Update the repeat button state of the player */
        mpd->repeat_state = 2;
    }
    else
    {
        /* Change the repeat button img */
        elm_image_file_set(mpd->fs_repeat_btn, ICON_DIR"ic_repeat_normal.png", NULL);
        /* */
        evas_object_show(mpd->fs_repeat_btn);

        /* Update the repeat button state of the player */
        mpd->repeat_state = 0;
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

    if (mpd->len <= 0)
        return;
    LOGE("slider_delay_changed_cb: %f - %f - %f", mpd->len, elm_slider_value_get(obj), (elm_slider_value_get(obj) * mpd->len));

    emotion_object_position_set(mpd->emotion, elm_slider_value_get(obj) * mpd->len);
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

static Evas_Object*
swallow_mini_player(mini_player *mpd, Evas_Object *parent)
{
    Evas_Object *layout = elm_layout_add(parent);
    elm_layout_file_set(layout, AUDIOPLAYERMINIEDJ, "audio_player");

    /* set the progress bar at the top of the table */
    mpd->slider = elm_slider_add(layout);
    elm_slider_horizontal_set(mpd->slider, EINA_TRUE);
    elm_object_part_content_set(layout, "swallow.progress", mpd->slider);
    set_sliders_callbacks(mpd, mpd->slider);
    player_update_sliders(mpd);


    /* set the cover image */
    mpd->cover = create_image(layout, "background_cone.png");
    elm_object_part_content_set(layout, "swallow.cover", mpd->cover);


    /* set the title label */
    mpd->title = elm_label_add(layout);
    elm_object_text_set(mpd->title, "<b>Title</b>");
    elm_object_part_content_set(layout, "swallow.title", mpd->title);


    /* set the sub title label */
    mpd->sub_title = elm_label_add(layout);
    elm_object_text_set(mpd->sub_title, "Subtitle");
    elm_object_part_content_set(layout, "swallow.subtitle", mpd->sub_title);


    /* set the play/pause button */
    mpd->play_pause_img = create_image(layout, "ic_pause_circle_normal_o.png");
    elm_object_part_content_set(layout, "swallow.play", mpd->play_pause_img);

    return layout;
}

static void
fullscreen_player_collapse_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player *mpd = data;
    /* Pop the previous view in the content naviframe */
    intf_show_previous_view(mpd->intf);
    /* Update the fullscreen state bool */
    mpd->fs_state = false;
    /* Show the mini player */
    mini_player_show(mpd);
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
    elm_object_text_set(mpd->fs_title, "<b>Title</b>");
    evas_object_show(mpd->fs_title);
    evas_object_size_hint_min_set(mpd->fs_title, 250, 25);
    evas_object_size_hint_max_set(mpd->fs_title, 250, 25);
    evas_object_size_hint_align_set(mpd->fs_title, 0.0, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_title, 0, 0, 3, 1);

    /* */
    mpd->fs_sub_title = elm_label_add(parent);
    elm_object_text_set(mpd->fs_sub_title, "Subtitle");
    evas_object_show(mpd->fs_sub_title);
    evas_object_size_hint_min_set(mpd->fs_sub_title, 250, 25);
    evas_object_size_hint_max_set(mpd->fs_sub_title, 250, 25);
    evas_object_size_hint_align_set(mpd->fs_sub_title, 0.0, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_sub_title, 0, 1, 3, 1);

    /* */
    if (mpd->save_state == FALSE)
    {
        mpd->fs_save_btn = create_image(parent, "ic_save_normal.png");
    }
    else {
        mpd->fs_save_btn = create_image(parent, "ic_save_pressed.png");
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
        mpd->fs_playlist_btn = create_image(parent, "ic_playlist_normal.png");
    }
    else {
        mpd->fs_playlist_btn = create_image(parent, "ic_playlist_pressed.png");
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
        mpd->fs_more_btn = create_image(parent, "ic_more_circle_normal_o.png");
    }
    else {
        mpd->fs_more_btn = create_image(parent, "ic_more_circle_pressed_o.png");
    }
    evas_object_size_hint_min_set(mpd->fs_more_btn, 50, 50);
    evas_object_size_hint_max_set(mpd->fs_more_btn, 50, 50);
    evas_object_size_hint_align_set(mpd->fs_more_btn, 1.0, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_more_btn, 5, 0, 1, 2);
    evas_object_show(mpd->fs_more_btn);

    /* */
    fs_padding = create_image(parent, "");
    evas_object_size_hint_min_set(fs_padding, 400, 40);
    evas_object_size_hint_max_set(fs_padding, 400, 40);
    evas_object_size_hint_align_set(fs_padding, EVAS_HINT_FILL, EVAS_HINT_FILL);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, fs_padding, 0, 2, 6, 1);
    evas_object_show(fs_padding);

    /* */
    mpd->fs_cover = create_image(parent, "background_cone.png");
    evas_object_size_hint_min_set(mpd->fs_cover, 400, 400);
    evas_object_size_hint_max_set(mpd->fs_cover, 400, 400);
    evas_object_size_hint_align_set(mpd->fs_cover, EVAS_HINT_FILL, EVAS_HINT_FILL);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_cover, 0, 3, 6, 1);
    evas_object_show(mpd->fs_cover);

    /* */
    fs_padding = create_image(parent, "");
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
    player_update_sliders(mpd);

    /* */
    if (mpd->play_state == FALSE)
    {
        mpd->fs_play_pause_img = create_image(parent, "ic_play_circle_normal_o.png");
    }
    else {
        mpd->fs_play_pause_img = create_image(parent, "ic_pause_circle_normal_o.png");
    }
    evas_object_size_hint_min_set(mpd->fs_play_pause_img, 200, 100);
    evas_object_size_hint_max_set(mpd->fs_play_pause_img, 200, 100);
    evas_object_size_hint_align_set(mpd->fs_play_pause_img, 0.5, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_play_pause_img, 1, 6, 4, 2);
    evas_object_show(mpd->fs_play_pause_img);


    /* */
    mpd->fs_time = elm_label_add(parent);
    evas_change_time(mpd->fs_time, mpd->pos);
    evas_object_size_hint_min_set(mpd->fs_time, 100, 25);
    evas_object_size_hint_max_set(mpd->fs_time, 100, 25);
    evas_object_size_hint_align_set(mpd->fs_time, 1.0, 0.5);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_time, 0, 6, 1, 1);
    evas_object_show(mpd->fs_time);


    /* */
    mpd->fs_total_time = elm_label_add(parent);
    evas_change_time(mpd->fs_total_time, mpd->len);
    evas_object_size_hint_min_set(mpd->fs_total_time, 100, 25);
    evas_object_size_hint_max_set(mpd->fs_total_time, 100, 25);
    evas_object_size_hint_align_set(mpd->fs_total_time, 0.0, 0.5);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_total_time, 5, 6, 1, 1);
    evas_object_show(mpd->fs_total_time);


    /* */
    if (mpd->repeat_state == 0)
    {
        mpd->fs_repeat_btn = create_image(parent, "ic_repeat_normal.png");
    }
    else if (mpd->repeat_state == 1)
    {
        mpd->fs_repeat_btn = create_image(parent, "ic_repeat_pressed.png");
    }
    else
    {
        mpd->fs_repeat_btn = create_image(parent, "ic_repeat_one_pressed.png");
    }
    evas_object_size_hint_min_set(mpd->fs_repeat_btn, 100, 50);
    evas_object_size_hint_max_set(mpd->fs_repeat_btn, 100, 50);
    evas_object_size_hint_align_set(mpd->fs_repeat_btn, 0.5, 1.0);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_repeat_btn, 0, 7, 1, 1);
    evas_object_show(mpd->fs_repeat_btn);

    /* */
    if (mpd->shuffle_state == FALSE){
        mpd->fs_shuffle_btn = create_image(parent, "ic_shuffle_normal.png");
    }
    else {
        mpd->fs_shuffle_btn = create_image(parent, "ic_shuffle_pressed.png");
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
    Evas_Object *fullscreen_item_table = add_fullscreen_item_table(mpd, mpd->mini_player_box);
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
    mini_player_hide(mpd);

    /* Show the fullcreen box in the content naviframe */
    /* FIXME */
    fs_view = create_fullscreen_player_view(mpd, intf_get_main_naviframe(mpd->intf));
    elm_object_content_set(intf_get_main_naviframe(mpd->intf), fs_view);

    /* */
    evas_object_show(fs_view);
    /* Update fullscreen state bool */
    mpd->fs_state = true;
    /* */
    mpd->fs_view = fs_view;
}

static void
emotion_position_update_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
    mini_player *mpd = data;

    mpd->pos = emotion_object_position_get(mpd->emotion);
    if (mpd->fs_time)
        evas_change_time(mpd->fs_time, mpd->pos);
    player_update_sliders(mpd);
}

static void
emotion_length_change_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
    mini_player *mpd = data;
    mpd->len = emotion_object_play_length_get(mpd->emotion);
    if (mpd->fs_total_time)
        evas_change_time(mpd->fs_total_time, mpd->len);
    player_update_sliders(mpd);
}

static void
emotion_title_change_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
    update_player_display(data);
}

static void
emotion_playback_started_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
    update_player_display(data);
}

static void
emotion_playback_finished_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event EINA_UNUSED)
{
    mini_player_stop(data);
}

void
create_base_player(mini_player *mpd, const char *file_path)
{
    mini_player_reset_states(mpd);

    if (!mpd->emotion)
    {
        setenv("EMOTION_LIBVLC_DEBUG", "1", 1);
        mpd->emotion =  emotion_object_add(intf_get_window(mpd->intf));
        emotion_object_init(mpd->emotion, "libvlc");
        evas_object_smart_callback_add(mpd->emotion, "position_update", emotion_position_update_cb, mpd);
        evas_object_smart_callback_add(mpd->emotion, "length_change", emotion_length_change_cb, mpd);
        evas_object_smart_callback_add(mpd->emotion, "title_change", emotion_title_change_cb, mpd);
        evas_object_smart_callback_add(mpd->emotion, "playback_started", emotion_playback_started_cb, mpd);
        evas_object_smart_callback_add(mpd->emotion, "playback_finished", emotion_playback_finished_cb, mpd);
        //evas_object_smart_callback_add(mpd->emotion, "audio_level_change", emotion_audio_level_change_cb, mpd);
        //evas_object_smart_callback_add(mpd->emotion, "channels_change", emotion_channels_change_cb, mpd);
    }
    emotion_object_file_set(mpd->emotion, file_path);
    emotion_object_play_set(mpd->emotion, EINA_TRUE);

    update_player_display(mpd);
    mpd->play_state = true;

    /* Show the mini player only if it isn't already shown */
    if (mini_player_visibility_state(mpd) == false){

        mini_player_show(mpd);
    }

}

mini_player*
mini_player_create(interface *intf, Evas_Object *parent)
{
    mini_player *mpd = calloc(1, sizeof(*mpd));

    mpd->intf = intf;
    mpd->play_state = false;
    mpd->visible_state = false;

    mpd->mini_player_box = swallow_mini_player(mpd, parent);

    /* Add button callbacks */
    evas_object_smart_callback_add(mpd->play_pause_img, "clicked", play_pause_mini_player_cb, mpd);
    evas_object_smart_callback_add(mpd->cover, "clicked", stop_mini_player_cb, mpd);
    evas_object_smart_callback_add(mpd->title, "clicked", mini_player_fullscreen_cb, mpd);
    evas_object_smart_callback_add(mpd->sub_title, "clicked", mini_player_fullscreen_cb, mpd);

    /* Put the mini player at the bottom of the content_box */
    evas_object_size_hint_align_set(mpd->mini_player_box, EVAS_HINT_FILL, 1.0);
    evas_object_show(mpd->mini_player_box);

    update_player_display(mpd);
    return mpd;
}

void
mini_player_stop(mini_player *mpd)
{
    /* Stop the player */
    if (mpd->emotion)
    {
        emotion_object_play_set(mpd->emotion, EINA_FALSE);
        emotion_object_file_set(mpd->emotion, NULL);
        evas_object_del(mpd->emotion);
        mpd->emotion = NULL;
    }

    mpd->fs_state = false;

    /* Hide the player */
    mini_player_hide(mpd);
}
