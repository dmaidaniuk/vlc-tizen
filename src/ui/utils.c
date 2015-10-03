/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
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

Evas_Object*
create_icon(Evas_Object *parent, const char *image_path)
{
    char path[PATH_MAX];

    Evas_Object *img = elm_icon_add(parent);
    snprintf(path, PATH_MAX, ICON_DIR"/%s", image_path);
    elm_image_file_set(img, path, NULL);

    /* */
    elm_image_resizable_set(img, EINA_TRUE, EINA_TRUE);
    evas_object_size_hint_align_set(img, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    return img;
}


Evas_Object*
create_image(Evas_Object *parent, const char *image_path)
{
    Evas_Object *img = elm_icon_add(parent);
    elm_image_file_set(img, image_path, NULL);

    /* */
    elm_image_resizable_set(img, EINA_TRUE, EINA_TRUE);
    evas_object_size_hint_align_set(img, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    return img;
}
