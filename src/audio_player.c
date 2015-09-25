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

#include <player.h>
#include <Elementary.h>
#include <glib.h>

#include "interface.h"
#include "audio_player.h"
#include "audio_popup.h"

bool
mini_player_visibility_state(mini_player_data_s *mpd)
{
    /* Return the current visibility state*/
    return mpd->visible_state;
}

bool
play_state(mini_player_data_s *mpd)
{
    /* Return the current play/pause state*/
    return mpd->play_state;
}

bool
save_state(mini_player_data_s *mpd)
{
    /* Return the current save button state*/
    return mpd->save_state;
}

bool
playlist_state(mini_player_data_s *mpd)
{
    /* Return the current playlist button state*/
    return mpd->playlist_state;
}

bool
shuffle_state(mini_player_data_s *mpd)
{
    /* Return the current shuffle state*/
    return mpd->shuffle_state;
}

bool
more_state(mini_player_data_s *mpd)
{
    /* Return the current more button state*/
    return mpd->more_state;
}

int
repeat_state(mini_player_data_s *mpd)
{
    /* Return the current repeat state*/
    return mpd->repeat_state;
}

void
mini_player_show(mini_player_data_s *mpd)
{
    /* Add the previously created mini player to the box */
    elm_box_pack_end(mpd->gd->content_box, mpd->mini_player_box);
    /* */
    evas_object_show(mpd->mini_player_box);

    /* Switch to current visibility state */
    mpd->visible_state = true;
}

void
mini_player_hide(mini_player_data_s *mpd)
{
    /* Dismiss the previously created mini player of the box */
    elm_box_unpack(mpd->gd->content_box, mpd->mini_player_box);
    /* */
    evas_object_hide(mpd->mini_player_box);

    /* Switch to current visibility state */
    mpd->visible_state = false;

}

static Evas_Object*
create_image(Evas_Object *parent, const char *image_path)
{
    /* Add and set images for buttons */
    char path[strlen(ICON_DIR)+strlen(image_path)+2];
    sprintf(path, ICON_DIR"/%s", image_path);
    Evas_Object *ic = elm_image_add(parent);

    /* */
    elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
    elm_image_file_set(ic, path, NULL);

    return ic;
}

static void
update_player_display(mini_player_data_s* mpd) {
    char *song_title, *song_artist;
    int error_code = player_get_content_info(mpd->player, PLAYER_CONTENT_INFO_TITLE,
            &song_title);
    if (error_code == 0) {
        elm_object_text_set(mpd->title, song_title);
        elm_object_text_set(mpd->fs_title, song_title);
    }
    error_code = player_get_content_info(mpd->player, PLAYER_CONTENT_INFO_ARTIST,
            &song_artist);
    if (error_code == 0) {
            elm_object_text_set(mpd->sub_title, song_artist);
            elm_object_text_set(mpd->fs_sub_title, song_artist);
    }

    free(song_title);
    free(song_artist);

    /* Change the play/pause button img */
    elm_image_file_set(mpd->play_pause_img, mpd->play_state ? ICON_DIR "ic_play_circle_normal_o.png" : ICON_DIR "ic_pause_circle_normal_o.png", NULL);
    elm_image_file_set(mpd->fs_play_pause_img, mpd->play_state ? ICON_DIR "ic_play_circle_normal_o.png" : ICON_DIR "ic_pause_circle_normal_o.png", NULL);

    evas_object_show(mpd->play_pause_img);
}

static void
play_pause_mini_player_cb(void *data, Evas_Object *obstopj EINA_UNUSED, void *event_info)
{
    mini_player_data_s *mpd = data;
    int error_code = 0;

    if(play_state(mpd) == true)
    {
    	/* Pause the player */
    	error_code = player_pause(mpd->player);
    }
    else
    {
    	/* Start the player */
    	error_code = player_start(mpd->player);
    }
    /* Update the play/pause state of the player */
    mpd->play_state = !mpd->play_state;
    update_player_display(mpd);
}

static void
play_pause_fs_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player_data_s *mpd = data;
    int error_code = 0;

    if(play_state(mpd) == true)
    {
    	/* Pause the player */
    	error_code = player_pause(mpd->player);
    } else {
    	/* Start the player */
    	error_code = player_start(mpd->player);
    }
    /* Update the play/pause state of the player */
    mpd->play_state = !mpd->play_state;
    evas_object_show(mpd->fs_play_pause_img);
    update_player_display(mpd);
}

static void
stop_mini_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player_data_s *mpd = data;
    int error_code = 0;

    /* Stop the player */
    error_code = player_stop(mpd->player);
    /* */
    player_destroy (mpd->player);

    /* Hide the player */
    mini_player_hide(mpd);

    /* And free player_data struct */
    mpd->player = NULL;
    free(mpd);
}

static void
fs_save_player_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player_data_s *mpd = data;

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
    mini_player_data_s *mpd = data;

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
    mini_player_data_s *mpd = data;

    if(more_state(mpd) == false)
    {
        /* Create the more popup list */
        Evas_Object *popup_list;

        /* */
        mpd->popup = elm_popup_add(mpd->gd->content);
        /* Size the popup */
        evas_object_size_hint_min_set(mpd->popup, 200, 200);
        evas_object_size_hint_max_set(mpd->popup, 200, 200);

        /* Add the more list in the popup */
        popup_list = create_audio_popup(mpd);
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
    mini_player_data_s *mpd = data;

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
    mini_player_data_s *mpd = data;

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

static Evas_Object*
add_item_table(mini_player_data_s *mpd, Evas_Object *parent)
{
    Evas_Object *content_table;
    Evas_Object *title, *sub_title;
    Evas_Object *progress_bar;

    /* */
    content_table = elm_table_add(parent);
    /* Put the content table in the audio player structure */
    mpd->table = content_table;
    /* Align then set the table padding */
    evas_object_size_hint_align_set(content_table, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_table_padding_set(content_table, ELM_SCALE_SIZE(5), ELM_SCALE_SIZE(5));
    /* */
    evas_object_show(content_table);


    /* Add then set the progress bar at the top of the table */
    progress_bar = elm_progressbar_add(content_table);
    elm_progressbar_horizontal_set(progress_bar, EINA_TRUE);
    elm_progressbar_value_set (progress_bar, 0.6);
    /* Scale the progress bar */
    evas_object_size_hint_max_set(progress_bar, 449, 1);
    evas_object_size_hint_align_set(progress_bar, 0.0, EVAS_HINT_FILL);
    /* */
    evas_object_show(progress_bar);
    elm_table_pack(content_table, progress_bar, 0, 0, 4, 1);


    /* Add then set the cover image */
    mpd->cover = create_image(parent, "background_cone.png");
    /* Scale and align the cover image */
    evas_object_size_hint_min_set(mpd->cover, 100, 100);
    evas_object_size_hint_max_set(mpd->cover, 100, 100);
    evas_object_size_hint_align_set(mpd->cover, 0.0, EVAS_HINT_FILL);
    /* */
    evas_object_show(mpd->cover);
    elm_table_pack(content_table, mpd->cover, 0, 1, 1, 2);


    /* Add then set the title label */
    title = elm_label_add(parent);
    elm_object_text_set(title, "<b>Title</b>");
    /* Scale and align the title label */
    evas_object_size_hint_min_set(title, 250, 0);
    evas_object_size_hint_max_set(title, 250, 50);
    evas_object_size_hint_align_set(title, 0.0, EVAS_HINT_FILL);
    /* */
    evas_object_show(title);
    elm_table_pack(content_table, title, 1, 1, 2, 1);
    /* Put the title label in the audio player structure */
    mpd->title = title;


    /* Add then set the sub title label */
    sub_title = elm_label_add(parent);
    elm_object_text_set(sub_title, "Subtitle");
    /* Scale and align the sub title label */
    evas_object_size_hint_min_set(sub_title, 250, 0);
    evas_object_size_hint_max_set(sub_title, 250, 50);
    evas_object_size_hint_align_set(sub_title, 0.0, EVAS_HINT_FILL);
    /* */
    evas_object_show(sub_title);
    elm_table_pack(content_table, sub_title, 1, 2, 2, 1);
    /* Put the sub title label in the audio player structure */
    mpd->sub_title = sub_title;


    /* Add then set the play/pause button */
    mpd->play_pause_img = create_image(parent, "ic_pause_circle_normal_o.png");
    /* Scale and align the play/pause button */
    evas_object_size_hint_min_set(mpd->play_pause_img, 100, 100);
    evas_object_size_hint_max_set(mpd->play_pause_img, 100, 100);
    evas_object_size_hint_align_set(mpd->play_pause_img, 1.0, EVAS_HINT_FILL);
    /* */
    evas_object_show(mpd->play_pause_img);
    elm_table_pack(content_table, mpd->play_pause_img, 3, 1, 1, 2);

    return content_table;
}

static void
fullscreen_player_collapse_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    mini_player_data_s *mpd = data;
    /* Pop the previous view in the content naviframe */
    create_view(mpd->gd, mpd->gd->panel_choice);
    /* Update the fullscreen state bool */
    mpd->fs_state = false;
    /* Show the mini player */
    mini_player_show(mpd);
}

static Evas_Object*
add_fullscreen_item_table(mini_player_data_s *mpd, Evas_Object *parent)
{
    Evas_Object *fs_progress_bar, *fs_padding;

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
    fs_progress_bar = elm_progressbar_add(mpd->fs_table);
    elm_progressbar_horizontal_set(fs_progress_bar, EINA_TRUE);
    elm_progressbar_value_set (fs_progress_bar, 0.5);
    evas_object_size_hint_min_set(fs_progress_bar, 400, 3);
    evas_object_size_hint_max_set(fs_progress_bar, 400, 3);
    evas_object_size_hint_align_set(fs_progress_bar, 0.5, 0.5);
    evas_object_show(fs_progress_bar);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, fs_progress_bar, 0, 5, 6, 1);


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
    elm_object_text_set(mpd->fs_time, "02:26");
    evas_object_size_hint_min_set(mpd->fs_time, 100, 25);
    evas_object_size_hint_max_set(mpd->fs_time, 100, 25);
    evas_object_size_hint_align_set(mpd->fs_time, 1.0, 0.5);
    /* Put the object in the chosen slot of the item table */
    elm_table_pack(mpd->fs_table, mpd->fs_time, 0, 6, 1, 1);
    evas_object_show(mpd->fs_time);


    /* */
    mpd->fs_total_time = elm_label_add(parent);
    elm_object_text_set(mpd->fs_total_time, "04:52");
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

    return mpd->fs_table;
}


static Evas_Object*
create_fullscreen_player_view(mini_player_data_s *mpd, Evas_Object *parent)
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
    mini_player_data_s *mpd = data;
    Evas_Object *fs_view;

    /* */
    mini_player_hide(mpd);

    /* Show the fullcreen box in the content naviframe */
    fs_view = create_fullscreen_player_view(mpd, mpd->gd->content);
    elm_object_content_set(mpd->gd->content, fs_view);
    /* */
    evas_object_show(fs_view);
    /* Update fullscreen state bool */
    mpd->fs_state = true;
    /* */
    mpd->fs_view = fs_view;

}

void
create_base_player(mini_player_data_s *mpd, char *file_path)
{
    /* Add the given file path in the audio player structure */
    mpd->file_path = file_path;
    int error_code = 0;
    /* */
    error_code = player_create(&mpd->player);
    error_code = player_set_uri(mpd->player, mpd->file_path);
    error_code = player_prepare(mpd->player);

    /* Start the player instantly when the file is selected in the list */
    error_code = player_start(mpd->player);
    update_player_display(mpd);
    mpd->play_state = true;

    /* Set all button to their default state */
    mpd->fs_state = false;
    mpd->save_state = false;
    mpd->shuffle_state = false;
    mpd->playlist_state = false;
    mpd->more_state = false;
    mpd->repeat_state = 0;

    /* Show the mini player only if it isn't already shown */
    if (mini_player_visibility_state(mpd) == false){

        mini_player_show(mpd);
    }

}

mini_player_data_s*
mini_player_create(gui_data_s *gd, Evas_Object *parent)
{
    Evas_Object *item_table;
    mini_player_data_s *mpd = malloc(sizeof(*mpd));

    /* */
    mpd->player = NULL;
    mpd->gd = gd;
    mpd->play_state = false;
    mpd->visible_state = false;

    /* Create the mini_player UI */
    mpd->mini_player_box = elm_box_add(parent);
    elm_box_horizontal_set(mpd->mini_player_box, EINA_TRUE);

    /* Add the table layout of the mini_player */
    item_table = add_item_table(mpd, mpd->mini_player_box);
    evas_object_show(item_table);
    elm_box_pack_end(mpd->mini_player_box, item_table);
    elm_box_recalculate(mpd->mini_player_box);

    /* Add button callbacks */
    evas_object_smart_callback_add(mpd->play_pause_img, "clicked", play_pause_mini_player_cb, mpd);
    evas_object_smart_callback_add(mpd->cover, "clicked", stop_mini_player_cb, mpd);
    evas_object_smart_callback_add(mpd->title, "clicked", mini_player_fullscreen_cb, mpd);
    evas_object_smart_callback_add(mpd->sub_title, "clicked", mini_player_fullscreen_cb, mpd);

    return mpd;
}
