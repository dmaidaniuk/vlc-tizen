/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
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

#ifndef MEDIA_LIST_H
#define MEDIA_LIST_H

#include "application.h"

#include "media_item.h"

typedef struct media_list_cbs_id media_list_cbs_id;
typedef struct media_list_callbacks media_list_callbacks;
struct media_list_callbacks
{
    void (*pf_on_media_added)(media_list *p_ml, void *p_user_data, unsigned int i_pos, media_item *p_mi);
    void (*pf_on_media_removed)(media_list *p_ml, void *p_user_data, unsigned int i_pos, media_item *p_mi);
    void (*pf_on_media_selected)(media_list *p_ml, void *p_user_data, int i_pos, media_item *p_mi);
    void *p_user_data;
};

enum PLAYLIST_REPEAT {
    REPEAT_NONE,
    REPEAT_ONE,
    REPEAT_ALL,
};

media_list *
media_list_create(bool b_free_media);

void
media_list_destroy(media_list *p_ml);

media_list_cbs_id *
media_list_register_callbacks(media_list *p_ml, media_list_callbacks *p_cbs);

void
media_list_unregister_callbacks(media_list *p_ml, media_list_cbs_id *p_id);

int
media_list_insert(media_list *p_ml, int i_index, media_item *p_mi);

int
media_list_remove(media_list *p_ml, media_item *p_mi);

int
media_list_remove_index(media_list *p_ml, unsigned int i_index);

void
media_list_clear(media_list *p_ml);

unsigned int
media_list_get_count(media_list *p_ml);

int
media_list_get_pos(media_list *p_ml);

bool
media_list_set_pos(media_list *p_ml, int i_index);

bool
media_list_set_next(media_list *p_ml);

bool
media_list_set_prev(media_list *p_ml);

media_item *
media_list_get_item(media_list *p_ml);

media_item *
media_list_get_item_at(media_list *p_ml,  unsigned int i_index);

void
media_list_set_repeat_mode(media_list *p_ml, enum PLAYLIST_REPEAT i_repeat);

enum PLAYLIST_REPEAT
media_list_get_repeat_mode(media_list *p_ml);

int
media_list_copy_list(media_list *p_ml_src, media_list *p_ml_dst);

/*****************/
/* static helpers */
/*****************/

static inline int
media_list_append(media_list *p_ml, media_item *p_mi)
{
    return media_list_insert(p_ml, -1, p_mi);
}

#endif /* media_list_H */
