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

#include "ui/interface.h"
#include "video_player.h"

typedef struct videodata
{
    Evas_Object *parent, *layout;
    Evas_Object *play_pause_button, *progress_slider;
    Evas_Object *canvas;
    bool play_state;
    char *file_path;

} videodata_s;

static void
_on_slider_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
    videodata_s *vd = data;
    if(vd)
    {
        double val = elm_slider_value_get(obj);
        //TODO plug to VLC
    }
}

static void
clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
   //TODO
}

Evas_Object*
create_video_gui(Evas_Object *parent, const char* file_path)
{
    videodata_s *vd = malloc(sizeof(*vd));
    vd->parent = parent;
    vd->file_path = file_path;

    vd->layout = elm_layout_add(parent);

    elm_layout_file_set(vd->layout, VIDEOPLAYEREDJ, "media_player_renderer");

    evas_object_size_hint_weight_set(vd->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(vd->layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(vd->layout);

    /* Initialize Evas Library & Function */
    evas_init();
    /* create and assign canvas*/
    Evas *evas = evas_new();
    vd->canvas = evas_object_image_filled_add(vd->layout);
    elm_object_part_content_set(vd->layout, "swallow.visualization", vd->canvas);
    //TODO vd->canvas = ??

    //create play/pause image
    vd->play_pause_button = elm_image_add(vd->layout);
    elm_image_file_set(vd->play_pause_button, ICON_DIR"ic_play_circle_normal_o.png", NULL);
    //attach to edje layout
    elm_object_part_content_set(vd->layout, "swallow.play", vd->play_pause_button);
    //click callback
    evas_object_smart_callback_add(vd->play_pause_button, "clicked", clicked_cb, vd);

    //progress slider
    vd->progress_slider = elm_slider_add(vd->layout);
    elm_slider_horizontal_set(vd->progress_slider, EINA_TRUE);
    elm_object_part_content_set(vd->layout, "swallow.progress", vd->progress_slider);

    //slider callbacks
    evas_object_smart_callback_add(vd->progress_slider, "slider,drag,stop", _on_slider_changed_cb, vd);
    evas_object_smart_callback_add(vd->progress_slider, "changed", _on_slider_changed_cb, vd);

    elm_naviframe_item_push(parent, "", NULL, NULL, vd->layout, NULL);

    return vd->layout;
}
