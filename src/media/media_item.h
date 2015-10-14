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

#ifndef MEDIA_ITEM_H_
#define MEDIA_ITEM_H_

#include "media/library/library_item.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum MEDIA_ITEM_TYPE {
    MEDIA_ITEM_TYPE_UNKNOWN,
    MEDIA_ITEM_TYPE_VIDEO,
    MEDIA_ITEM_TYPE_AUDIO,
    MEDIA_ITEM_TYPE_SUBTITLE,
    MEDIA_ITEM_TYPE_DIRECTORY,
    MEDIA_ITEM_TYPE_ARCHIVE,
};

enum MEDIA_ITEM_META {
    MEDIA_ITEM_META_TITLE,
    MEDIA_ITEM_META_ARTIST,
    MEDIA_ITEM_META_ALBUM,
    MEDIA_ITEM_META_YEAR,
    MEDIA_ITEM_META_GENRE,
    MEDIA_ITEM_META_COMMENT,
    MEDIA_ITEM_META_DISC_ID,
    MEDIA_ITEM_META_COUNT,
};

typedef struct media_item {
    LIBRARY_ITEM_COMMON

    char *psz_path;                 /* Normalized path on the device */
    enum MEDIA_ITEM_TYPE i_type;    /* Video, Audio, Subs, etc... */

    char *psz_metas[MEDIA_ITEM_META_COUNT];
    int64_t i_duration;             /* in ms */

    //FIXME replace with a union
    int i_w, i_h;                   /* in pixels */

    char* psz_snapshot;             /* Path to a snapshot file */
    uint32_t i_id;                  /* Opaque file type specific ID, provided by the media library */
} media_item;

media_item *
media_item_create(const char *psz_path, enum MEDIA_ITEM_TYPE i_type);

media_item*
media_item_copy(const media_item* p_item);

void
media_item_destroy(media_item *p_mi);

bool
media_item_identical(const media_item* p_left, const media_item* p_right);

int
media_item_set_meta(media_item *p_mi, enum MEDIA_ITEM_META i_meta, const char *psz_meta);

#define media_item_title(p_mi) (p_mi)->psz_metas[MEDIA_ITEM_META_TITLE]
#define media_item_artist(p_mi) (p_mi)->psz_metas[MEDIA_ITEM_META_ARTIST]
#define media_item_album(p_mi) (p_mi)->psz_metas[MEDIA_ITEM_META_ALBUM]
#define media_item_year(p_mi) (p_mi)->psz_metas[MEDIA_ITEM_META_YEAR]
#define media_item_genre(p_mi) (p_mi)->psz_metas[MEDIA_ITEM_META_GENRE]
#define media_item_comment(p_mi) (p_mi)->psz_metas[MEDIA_ITEM_META_COMMENT]
#define media_item_disc_id(p_mi) (p_mi)->psz_metas[MEDIA_ITEM_META_DISC_ID]

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* MEDIA_ITEM_H_ */
