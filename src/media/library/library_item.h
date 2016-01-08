/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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

 #ifndef LIBRARY_ITEM_H_
 # define LIBRARY_ITEM_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct library_item library_item;

typedef enum library_item_type
{
    LIBRARY_ITEM_MEDIA = 1,
    LIBRARY_ITEM_ALBUM,
    LIBRARY_ITEM_ARTIST,
    LIBRARY_ITEM_GENRE
} library_item_type;

#define LIBRARY_ITEM_COMMON \
    library_item_type i_library_item_type;

#ifdef __cplusplus
}
#endif

#endif // LIBRARY_ITEM_H_
