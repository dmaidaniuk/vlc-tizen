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

#ifndef __vlc_common_H__
#define __vlc_common_H__

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>

#include <tizen.h>
#include <dlog.h>

#ifdef  LOG_TAG
# undef  LOG_TAG
#endif
#define LOG_TAG "vlc"

#if !defined(PACKAGE)
# define PACKAGE "org.videolan.vlc"
#endif

#define ICON_DIR "/opt/usr/apps/"PACKAGE"/res/images/"

#include <Evas.h>

#endif /* __vlc_common_H__ */
