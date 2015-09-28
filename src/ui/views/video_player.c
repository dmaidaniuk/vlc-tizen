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
#include <Evas_GL.h>
#include <Elementary.h>
#include <player.h>

#include "ui/interface.h"
#include "video_player.h"

typedef struct videodata
{
    player_h player;
    Evas_Object *parent, *layout;
    Evas_Object *play_pause_button;
    bool play_state;
    char *file_path;

} videodata_s;

static void
init_base_player(void *data, Evas_Object *obj, void *event_info)
{
    videodata_s *vd = data;

    /* Create an Evas image object for the video surface */
    Evas *canvas;
    /* Initializate Evas Library & Function that comes with it */
    evas_init();
    /* */
    canvas = evas_new();
    /* Chose a rendering method by looking with are available on the Evas library */
    /* See more with the evas_render_method_list() int the create_video_gui func */
    int render_method = evas_render_method_lookup("software_generic");
    LOGD("%i", render_method);
    /* */
    evas_output_method_set(canvas, render_method);
    evas_output_size_set(canvas, 320, 176);
    evas_output_viewport_set(canvas, 0, 0, 320, 176);
    /* */
    LOGI("Evas set");

    /* Create an Evas with the previously set canvas */
    Evas_Object *img = evas_object_image_add(canvas);

    int error_code = 0;
    /* */
    error_code = player_create(&vd->player);
    error_code = player_set_uri(vd->player, vd->file_path);
    /* */
    error_code = player_set_display(vd->player, PLAYER_DISPLAY_TYPE_EVAS, GET_DISPLAY(img));
    error_code = player_set_display_mode(vd->player, PLAYER_DISPLAY_MODE_FULL_SCREEN) ;

    /* Enable and check display visibility */
    bool is_visible;
    error_code = player_is_display_visible(vd->player, &is_visible);
    /* */
    LOGE("player_is_display_visible = %d", error_code);

    if (!is_visible)
    {
        error_code = player_set_display_visible(vd->player, true);
        /* */
        LOGE("player_set_display_visible = %d", error_code);
        LOGI("DISPLAY IS VISIBLE");
    }
    /* */
    error_code = player_prepare(vd->player);

    /* Check player state */
    player_state_e state;
    error_code = player_get_state(vd->player, &state);
    LOGE("player STATE ERROR CODE = %d", error_code);
    if (error_code != PLAYER_STATE_READY)
    {
        LOGI("PLAYER_STATE = READY");
    }

    /* TODO : Not sure if this is useful because we previsouly enable display visibility */
    /* after initialize the img as the player diplay */
    evas_object_show(img);

    /* */
    error_code = player_start(vd->player);

    /* Check player state */
    player_state_e state2;
    error_code = player_get_state(vd->player, &state2);
    LOGE("player STATE ERROR CODE = %d", error_code);
    if (error_code != PLAYER_STATE_PLAYING)
    {
        LOGI("PLAYER_STATE = PLAYING");
    }

    /* Retrieving file data */
    /* Was used to see if the file was correctly decoded */
    int width, height;
    error_code = player_get_video_size(vd->player, &width, &height);
    LOGE("player_get_video_size = %d", error_code);
    LOGI("height = %i", height);
    LOGI("Width = %i", width);

    int duration;
    error_code = player_get_duration(vd->player, &duration);
    LOGE("player_get_duration = %d", error_code);
    LOGI("Duration = %i", duration);

    int fps, bit_rate;
    error_code = player_get_video_stream_info(vd->player, &fps, &bit_rate);
    LOGE("player_ get_video_stream_info = %d", error_code);
    LOGI("fps = %i", fps);
    LOGI("Bit Rate = %i", bit_rate);

    /* Start the player instantly when the file is selected in the list */
    vd->play_state = true;

}

static void
release_base_player(void *data, Evas_Object *obj, void *event_info)
{
    videodata_s *vd = data;
    int error_code = 0;

    /* */
    error_code = player_stop(vd->player);
    error_code = player_unprepare(vd->player);
    error_code = player_destroy(vd->player);
    evas_object_del(vd->parent);

}

Evas_Object*
create_video_gui(Evas_Object *parent, char* file_path)
{
    videodata_s *vd = malloc(sizeof(*vd));
    vd->parent = parent;
    vd->file_path = file_path;

    vd->layout = elm_layout_add(parent);

    elm_layout_file_set(vd->layout, VIDEOPLAYEREDJ, "media_player_renderer");

    evas_object_size_hint_weight_set(vd->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(vd->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(vd->layout);

    //create play/pause image
    vd->play_pause_button = elm_image_add(vd->layout);
    elm_image_file_set(vd->play_pause_button, ICON_DIR"ic_play_circle_normal_o.png", NULL);
    //attach to edje layout
    elm_object_part_content_set(vd->layout, "swallow.play", vd->play_pause_button);
    //show
    evas_object_show(vd->play_pause_button);

    elm_naviframe_item_push(parent, "Video Player Test", NULL, NULL, vd->layout, NULL);

    return vd->layout;
}
