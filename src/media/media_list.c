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

#include "common.h"

#include <assert.h>
#include <eina_array.h>
#include <eina_list.h>

#include "media_list.h"

struct media_list
{
    Eina_List *p_cbs_list;
    Eina_Array *p_item_array;
    media_item *p_mi;
    int i_pos;
    bool b_free_media;

    enum PLAYLIST_REPEAT i_repeat;
};

#define ML_SEND_CALLBACK(pf_cb, ...) do { \
    Eina_List *p_el; \
    media_list_callbacks *p_cbs; \
    EINA_LIST_FOREACH(p_ml->p_cbs_list, p_el, p_cbs) \
        if (p_cbs->pf_cb) \
            p_cbs->pf_cb(p_ml, p_cbs->p_user_data, __VA_ARGS__); \
} while (0)

#define ML_CLIP_POS(i_pos) do { \
    if (i_pos >= eina_array_count(p_ml->p_item_array)) \
        i_pos = eina_array_count(p_ml->p_item_array) - 1; \
} while (0)

static void
media_list_on_new_pos(media_list *p_ml)
{
    ML_SEND_CALLBACK(pf_on_media_selected, p_ml->i_pos, p_ml->p_mi);
}

static void
media_list_on_media_added(media_list *p_ml, unsigned int i_index, media_item *p_mi)
{
    if (p_ml->i_pos >= 0 && p_ml->i_pos >= i_index)
    {
        // A media has been inserted before the current media
        // we need to shift its position.
        p_ml->i_pos++;
    }

    ML_SEND_CALLBACK(pf_on_media_added, i_index, p_mi);
}

static void
media_list_on_media_removed(media_list *p_ml, unsigned int i_index, media_item *p_mi)
{
    if (p_ml->i_pos >= 0 && p_ml->i_pos >= i_index)
    {
        // A media has been removed before the current media
        // we need to shift its position.
        p_ml->i_pos--;
    }

    ML_SEND_CALLBACK(pf_on_media_removed, i_index, p_mi);

    if (p_ml->b_free_media)
        media_item_destroy(p_mi);
}

media_list *
media_list_create(bool b_free_media)
{
    media_list *p_ml = calloc(1, sizeof(media_list));
    if (!p_ml)
        return NULL;
    p_ml->p_item_array = eina_array_new(30);
    if (!p_ml->p_item_array)
    {
        free(p_ml);
        return NULL;
    }

    p_ml->b_free_media = b_free_media;
    p_ml->i_pos = -1;
    p_ml->i_repeat = REPEAT_NONE;
    return p_ml;
}

void
media_list_destroy(media_list *p_ml)
{
    Eina_List *p_el;
    void *p_id;

    /* Clear callback list */
    EINA_LIST_FOREACH(p_ml->p_cbs_list, p_el, p_id)
      free(p_id);
    eina_list_free(p_ml->p_cbs_list);
    p_ml->p_cbs_list = NULL;

    media_list_clear(p_ml);
    eina_array_free(p_ml->p_item_array);
    free(p_ml);
}

media_list_cbs_id *
media_list_register_callbacks(media_list *p_ml, media_list_callbacks *p_cbs)
{
    media_list_callbacks *p_cbs_dup = malloc(sizeof(media_list_callbacks));

    if (!p_cbs_dup)
        return NULL;
    memcpy(p_cbs_dup, p_cbs, sizeof(media_list_callbacks));

    p_ml->p_cbs_list = eina_list_append(p_ml->p_cbs_list, p_cbs_dup);
    if (!p_ml->p_cbs_list)
    {
        free(p_cbs_dup);
        return NULL;
    }
    return (media_list_cbs_id *) p_cbs_dup;
}

void
media_list_unregister_callbacks(media_list *p_ml, media_list_cbs_id *p_id)
{
    p_ml->p_cbs_list = eina_list_remove(p_ml->p_cbs_list, p_id);
    free(p_id);
}

int
media_list_insert(media_list *p_ml, int i_index, media_item *p_mi)
{
    if (i_index < 0)
    {
        if (!eina_array_push(p_ml->p_item_array, p_mi))
            return -1;
    }
    else
    {
        unsigned int i_count;
        /* increase array size */
        if (!eina_array_push(p_ml->p_item_array, p_mi)) // dummy data
            return -1;

        /* "memmove" */
        i_count = eina_array_count(p_ml->p_item_array);
        for (unsigned int i = i_count - 1; i > i_index; --i)
        {
            media_item *p_move_mi = eina_array_data_get(p_ml->p_item_array, i - 1);
            eina_array_data_set(p_ml->p_item_array, i, p_move_mi);
        }
        eina_array_data_set(p_ml->p_item_array, i_index, p_mi);
    }

    media_list_on_media_added(p_ml, i_index, p_mi);

    if (p_ml->i_pos == -1)
        media_list_set_pos(p_ml, 0);

    return 0;
}

static Eina_Bool
media_list_array_keep_cb(void *p_data, void *p_user_data)
{
   return (p_data == p_user_data) ? EINA_FALSE : EINA_TRUE;
}

static int
media_list_remove_common(media_list *p_ml, unsigned int i_index, media_item *p_mi)
{
    int i_prev_count = eina_array_count(p_ml->p_item_array);

    if (!eina_array_remove(p_ml->p_item_array, media_list_array_keep_cb, p_mi))
        return -1;

    assert(eina_array_count(p_ml->p_item_array) != i_prev_count);

    media_list_on_media_removed(p_ml, i_index, p_mi);

    if (eina_array_count(p_ml->p_item_array) == 0)
    {
        /* notify there if no more current media */
        p_ml->i_pos = -1;
        p_ml->p_mi = NULL;
        media_list_on_new_pos(p_ml);
    }
    else if (p_ml->i_pos == i_index)
    {
        /* notify current media changed */
        p_ml->p_mi = eina_array_data_get(p_ml->p_item_array, p_ml->i_pos);
        media_list_on_new_pos(p_ml);
    }
    else if (i_index < p_ml->i_pos)
    {
        p_ml->i_pos--;
    }

    return 0;
}

int
media_list_remove(media_list *p_ml, media_item *p_mi)
{
    media_item *p_iter_mi;
    Eina_Array_Iterator iterator;
    unsigned int i;

    EINA_ARRAY_ITER_NEXT(p_ml->p_item_array, i, p_iter_mi, iterator)
    {
        if (p_iter_mi == p_mi)
            return media_list_remove_common(p_ml, i, p_mi);
    }
    return -1;
}

int
media_list_remove_index(media_list *p_ml, unsigned int i_index)
{
    ML_CLIP_POS(i_index);
    media_item *p_mi = eina_array_data_get(p_ml->p_item_array, i_index);
    if (!p_mi)
        return -1;
    return media_list_remove_common(p_ml, i_index, p_mi);
}

void
media_list_clear(media_list *p_ml)
{
    media_item *p_mi;
    Eina_Array_Iterator iterator;
    unsigned int i;

    EINA_ARRAY_ITER_NEXT(p_ml->p_item_array, i, p_mi, iterator)
        media_list_on_media_removed(p_ml, i, p_mi);
    eina_array_flush(p_ml->p_item_array);

    if (p_ml->i_pos != -1)
    {
        p_ml->i_pos = -1;
        p_ml->p_mi = NULL;
        media_list_on_new_pos(p_ml);
    }
}

unsigned int
media_list_get_count(media_list *p_ml)
{
    return eina_array_count(p_ml->p_item_array);
}

int
media_list_get_pos(media_list *p_ml)
{
    return p_ml->i_pos;
}

bool
media_list_set_pos(media_list *p_ml, int i_index)
{
    ML_CLIP_POS(i_index);
    if (i_index != p_ml->i_pos || p_ml->i_repeat == REPEAT_ONE)
    {
        p_ml->i_pos = i_index;
        p_ml->p_mi = p_ml->i_pos >= 0 ? eina_array_data_get(p_ml->p_item_array, p_ml->i_pos) : NULL;
        media_list_on_new_pos(p_ml);
        return true;
    } else {
        if (p_ml->i_repeat == REPEAT_ALL)
        {
            p_ml->i_pos = 0;
            p_ml->p_mi = eina_array_data_get(p_ml->p_item_array, p_ml->i_pos);
            media_list_on_new_pos(p_ml);
            return true;
        }
        return false;
    }
}

bool
media_list_set_next(media_list *p_ml)
{
    if (p_ml->i_repeat == REPEAT_ONE)
    {
        return media_list_set_pos(p_ml, p_ml->i_pos);
    }
    else
    {
        return media_list_set_pos(p_ml, p_ml->i_pos + 1);
    }
}

bool
media_list_set_prev(media_list *p_ml)
{
    if (p_ml->i_repeat == REPEAT_ONE)
    {
        return media_list_set_pos(p_ml, p_ml->i_pos);
    }
    else
    {
        return media_list_set_pos(p_ml, p_ml->i_pos - 1);
    }
}

media_item *
media_list_get_item(media_list *p_ml)
{
    return p_ml->p_mi;
}

media_item *
media_list_get_item_at(media_list *p_ml, unsigned int i_index)
{
    media_item *p_mi;

    ML_CLIP_POS(i_index);

    p_mi = eina_array_data_get(p_ml->p_item_array, i_index);
    assert(p_mi);
    return p_mi;
}

void
media_list_set_repeat_mode(media_list *p_ml, enum PLAYLIST_REPEAT i_repeat)
{
    p_ml->i_repeat = i_repeat;
}

enum PLAYLIST_REPEAT
media_list_get_repeat_mode(media_list *p_ml)
{
    return p_ml->i_repeat;
}
