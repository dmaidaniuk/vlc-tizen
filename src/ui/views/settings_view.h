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

#ifndef SETTINGS_VIEW_H_
#define SETTINGS_VIEW_H_

#include "ui/settings/settings.h"
#include "ui/interface.h"

/* ID */
#define SETTINGS_ID_DIRECTORIES 1
#define SETTINGS_ID_HWACCELERATION 2
#define SETTINGS_ID_SUBSENC 3
#define SETTINGS_ID_VORIENTATION 4
#define SETTINGS_ID_PERFORMANCES 5
#define SETTINGS_ID_DEBLOCKING 6

#define HWACCELERATION_AUTOMATIC 1
#define HWACCELERATION_DISABLED 2
#define HWACCELERATION_DECODING 3
#define HWACCELERATION_FULL 4

#define ORIENTATION_AUTOMATIC 1
#define ORIENTATION_LOCKED 2
#define ORIENTATION_LANDSCAPE 3
#define ORIENTATION_PORTRAIT 4
#define ORIENTATION_R_LANDSCAPE 5
#define ORIENTATION_R_PORTRAIT 6

#define PERFORMANCE_FRAME_SKIP 1
#define PERFORMANCE_STRETCH 2

#define DEBLOCKING_AUTOMATIC 1
#define DEBLOCKING_FULL 2
#define DEBLOCKING_MEDIUM 3
#define DEBLOCKING_LOW 4
#define DEBLOCKING_NO 5

/* Structs */
typedef struct settings_menu_context {
    int menu_id;
} settings_menu_context;

/* Declarations */
interface_view*
create_setting_view(interface *intf, Evas_Object *parent);

void
destroy_setting_view(interface_view *);

void
menu_directories_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_hwacceleration_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_subsenc_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_vorientation_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_performance_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);
void
menu_deblocking_selected_cb(settings_menu_selected *selected, view_sys* p_view_sys, void *data, Evas_Object *parent);

#endif /* SETTINGS_VIEW_H_ */
