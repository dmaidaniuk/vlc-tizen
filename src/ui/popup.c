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

#include "popup.h"
#include <Elementary.h>

static void
popup_del(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
}

void
popup_close(Evas_Object *popup)
{
    if (!popup)
        return;

    evas_object_smart_callback_add(popup, "dismissed", popup_del, NULL);
    elm_popup_dismiss(popup);
}
