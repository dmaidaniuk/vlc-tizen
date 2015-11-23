/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
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

#ifndef SETTINGS_MENU_ID_H_
#define SETTINGS_MENU_ID_H_

/* ID */
typedef enum menu_id
{
    /* Main menu */
    SETTINGS_ID_DIRECTORIES = 100,
    SETTINGS_ID_HWACCELERATION,
    SETTINGS_ID_SUBSENC,
    SETTINGS_ID_VORIENTATION,
    SETTINGS_ID_PERFORMANCES,
    SETTINGS_ID_DEBLOCKING,
    SETTINGS_ID_DEVELOPER,

    /* Submenu */
    DIRECTORIES_INTERNAL = 1000,
    DIRECTORIES_EXTERNAL,
    DIRECTORIES_ADDLOCATION,

    HWACCELERATION_AUTOMATIC = 2000,
    HWACCELERATION_DISABLED,
    HWACCELERATION_DECODING,
    HWACCELERATION_FULL,

    ORIENTATION_AUTOMATIC = 3000,
    ORIENTATION_LOCKED,
    ORIENTATION_LANDSCAPE,
    ORIENTATION_PORTRAIT,
    ORIENTATION_R_LANDSCAPE,
    ORIENTATION_R_PORTRAIT,

    PERFORMANCE_FRAME_SKIP = 4000,
    PERFORMANCE_STRETCH,

    DEBLOCKING_AUTOMATIC = 5000,
    DEBLOCKING_FULL,
    DEBLOCKING_MEDIUM,
    DEBLOCKING_LOW,
    DEBLOCKING_NO,

    DEVELOPER_VERBOSE = 6000,

} menu_id;

#endif
