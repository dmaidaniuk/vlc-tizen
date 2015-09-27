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
    Evas_Object *parent, *box;
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
    dlog_print(DLOG_DEBUG, LOG_TAG, "%i", render_method);
    /* */
    evas_output_method_set(canvas, render_method);
    evas_output_size_set(canvas, 320, 176);
    evas_output_viewport_set(canvas, 0, 0, 320, 176);
    /* */
    dlog_print(DLOG_INFO, LOG_TAG, "Evas set");

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
    dlog_print(DLOG_ERROR, LOG_TAG, "player_is_display_visible = %d", error_code);

    if (!is_visible)
    {
        error_code = player_set_display_visible(vd->player, true);
        /* */
        dlog_print(DLOG_ERROR, LOG_TAG, "player_set_display_visible = %d", error_code);
        dlog_print(DLOG_INFO, LOG_TAG, "DISPLAY IS VISIBLE");
    }
    /* */
    error_code = player_prepare(vd->player);

    /* Check player state */
    player_state_e state;
    error_code = player_get_state(vd->player, &state);
    dlog_print(DLOG_ERROR, LOG_TAG, "player STATE ERROR CODE = %d", error_code);
    if (error_code != PLAYER_STATE_READY)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "PLAYER_STATE = READY");
    }

    /* TODO : Not sure if this is useful because we previsouly enable display visibility */
    /* after initialize the img as the player diplay */
    evas_object_show(img);

    /* */
    error_code = player_start(vd->player);

    /* Check player state */
    player_state_e state2;
    error_code = player_get_state(vd->player, &state2);
    dlog_print(DLOG_ERROR, LOG_TAG, "player STATE ERROR CODE = %d", error_code);
    if (error_code != PLAYER_STATE_PLAYING)
    {
        dlog_print(DLOG_INFO, LOG_TAG, "PLAYER_STATE = PLAYING");
    }

    /* Retrieving file data */
    /* Was used to see if the file was correctly decoded */
    int width, height;
    error_code = player_get_video_size(vd->player, &width, &height);
    dlog_print(DLOG_ERROR, LOG_TAG, "player_get_video_size = %d", error_code);
    dlog_print(DLOG_INFO, LOG_TAG, "height = %i", height);
    dlog_print(DLOG_INFO, LOG_TAG, "Width = %i", width);

    int duration;
    error_code = player_get_duration(vd->player, &duration);
    dlog_print(DLOG_ERROR, LOG_TAG, "player_get_duration = %d", error_code);
    dlog_print(DLOG_INFO, LOG_TAG, "Duration = %i", duration);

    int fps, bit_rate;
    error_code = player_get_video_stream_info(vd->player, &fps, &bit_rate);
    dlog_print(DLOG_ERROR, LOG_TAG, "player_ get_video_stream_info = %d", error_code);
    dlog_print(DLOG_INFO, LOG_TAG, "fps = %i", fps);
    dlog_print(DLOG_INFO, LOG_TAG, "Bit Rate = %i", bit_rate);

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

}

Evas_Object*
create_video_gui(Evas_Object *parent, char* file_path)
{
    Eina_List *engine_list, *l;
    char *engine_name;

    /* Retrieve render method list available in all Evas Library */
    engine_list = evas_render_method_list();
    if (!engine_list)
    {
        dlog_print(DLOG_DEBUG, LOG_TAG, "ERROR : No available Evas Engines :\n");
        exit(-1);
    }
    /* */
    dlog_print(DLOG_DEBUG, LOG_TAG, "Available Evas Engines :\n");
    /* Acces the Eina_list* created by evas_render_method_list() */
    /* Return strig list */
    EINA_LIST_FOREACH(engine_list, l, engine_name)
    dlog_print(DLOG_DEBUG, LOG_TAG, "%s", engine_name);
    evas_render_method_list_free(engine_list);

    /* TODO : This is a very simple an ugly UI */
    /* That was just create to debug & test video player func & display */
    /* When this will work correctly, a proper player UI must be added */
    Evas_Object *box, *button_init, *button_end, *lbl_init, *lbl_end;
    videodata_s *vd = malloc(sizeof(*vd));
    vd->parent = parent;
    vd->file_path = file_path;

    /* */
    box = elm_box_add(parent);
    /* */
    button_init = elm_button_add(box);
    button_end = elm_button_add(box);
    /* */
    lbl_init = elm_label_add(button_init);
    lbl_end = elm_label_add(button_end);
    elm_object_text_set(lbl_init, "INIT PLAYER");
    elm_object_text_set(lbl_end, "END PLAYER");
    /* */
    elm_object_content_set(button_init, lbl_init);
    elm_object_content_set(button_end, lbl_end);
    /* */
    evas_object_size_hint_align_set(button_init, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_align_set(button_end, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(button_init, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_weight_set(button_end, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    /* */
    elm_box_pack_end(box, button_init);
    elm_box_pack_end(box, button_end);
    /* */
    evas_object_show(lbl_init);
    evas_object_show(lbl_end);
    evas_object_show(button_end);
    evas_object_show(button_init);
    evas_object_show(box);
    /* */
    elm_naviframe_item_push(parent, "Video Player Test", NULL, NULL, box, NULL);
    vd->box = box;

    // Add callback to button
    evas_object_smart_callback_add(button_init, "clicked", init_base_player, vd);
    evas_object_smart_callback_add(button_end, "clicked", release_base_player, vd);

    return box;
}
