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

#include <assert.h>
#include <eina_list.h>

#include "media_list.h"

struct media_list
{
    Eina_List *p_cbs_list;
    Eina_List *p_item_list;
    media_item *p_mi;
    unsigned int i_pos;
    unsigned int i_count;
    bool b_free_media;
};

media_list *
media_list_create(bool b_free_media)
{
    media_list *p_ml = calloc(1, sizeof(media_list));
    if (p_ml)
        p_ml->b_free_media = b_free_media;
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
    free(p_ml);
}

static void
media_list_on_media_added(media_list *p_ml, media_item *p_mi)
{
    Eina_List *p_el;
    media_list_callbacks *cbs;

    p_ml->i_count++;
    EINA_LIST_FOREACH(p_ml->p_cbs_list, p_el, cbs)
        cbs->pf_on_media_added(p_ml, cbs->p_user_data, p_mi);
}

static void
media_list_on_media_removed(media_list *p_ml, media_item *p_mi)
{
    Eina_List *p_el;
    media_list_callbacks *cbs;

    p_ml->i_count--;
    if (p_ml->i_pos >= p_ml->i_count)
    {
        p_ml->i_pos = p_ml->i_count -1;
        p_ml->p_mi = eina_list_nth(p_ml->p_item_list, p_ml->i_pos);
        assert(p_ml->p_mi);
    }

    EINA_LIST_FOREACH(p_ml->p_cbs_list, p_el, cbs)
        cbs->pf_on_media_removed(p_ml, cbs->p_user_data, p_mi);

    if (p_ml->b_free_media)
        media_item_destroy(p_mi);
}


void *
media_list_register_callbacks(media_list *p_ml, media_list_callbacks *p_cbs)
{
    Eina_List *p_el;
    media_list_callbacks *p_cbs_dup = malloc(sizeof(media_list_callbacks));

    if (!p_cbs_dup)
        return NULL;
    memcpy(p_cbs_dup, p_cbs, sizeof(media_list_callbacks));

    p_el = eina_list_append(p_ml->p_cbs_list, p_cbs_dup);
    if (p_el == p_ml->p_cbs_list)
    {
        free(p_cbs_dup);
        return NULL;
    }
    p_ml->p_cbs_list = p_el;
    return p_cbs_dup;
}

void
media_list_unregister_callbacks(media_list *p_ml, void *p_id)
{
    p_ml->p_cbs_list = eina_list_remove(p_ml->p_cbs_list, p_id);
    free(p_id);
}

int
media_list_insert(media_list *p_ml, int i_index, media_item *p_mi)
{
    Eina_List *p_el;

    if (i_index < 0)
        p_el = eina_list_append(p_ml->p_item_list, p_mi);
    else if (i_index == 0)
        p_el = eina_list_prepend(p_ml->p_item_list, p_mi);
    else
    {
        p_el = eina_list_nth_list(p_ml->p_item_list, i_index);
        if (p_el)
            p_el = eina_list_append_relative_list(p_ml->p_item_list,
                                                  p_mi, p_el);
    }
    if (!p_el || p_el == p_ml->p_cbs_list)
        return -1;
    p_ml->p_cbs_list = p_el;
    media_list_on_media_added(p_ml, p_mi);

    return 0;
}

int
media_list_remove(media_list *p_ml, media_item *p_mi)
{
    /* Don't use eina_list_remove since there is no way to check if it
     * succeeded to remove p_mi */

    Eina_List *p_el = eina_list_data_find_list(p_ml->p_item_list, p_mi);
    if (p_el)
    {
        p_ml->p_item_list = eina_list_remove_list(p_ml->p_item_list, p_el);
        media_list_on_media_removed(p_ml, p_mi);
        return 0;
    }
    else
        return -1;
}

int
media_list_remove_index(media_list *p_ml, unsigned int i_index)
{
    Eina_List *p_el = eina_list_nth_list(p_ml->p_item_list, i_index);
    if (p_el)
    {
        media_item *p_mi = eina_list_data_get(p_el);
        p_ml->p_item_list = eina_list_remove_list(p_ml->p_item_list, p_el);

        media_list_on_media_removed(p_ml, p_mi);
        return 0;
    }
    else
        return -1;
}

void
media_list_clear(media_list *p_ml)
{
    Eina_List *p_el;
    media_item *p_mi;

    p_ml->i_pos = 0;
    p_ml->p_mi = NULL;
    EINA_LIST_FOREACH(p_ml->p_item_list, p_el, p_mi)
        media_list_on_media_removed(p_ml, p_mi);
    eina_list_free(p_ml->p_item_list);
    p_ml->p_cbs_list = NULL;
}

unsigned int
media_list_get_count(media_list *p_ml)
{
    return p_ml->i_count;
}

unsigned int
media_list_get_pos(media_list *p_ml)
{
    return p_ml->i_pos;
}

void
media_list_set_pos(media_list *p_ml, unsigned int i_index)
{
    if (i_index >= p_ml->i_count)
        p_ml->i_pos = p_ml->i_count - 1;
    else
        p_ml->i_pos = i_index;
    p_ml->p_mi = eina_list_nth(p_ml->p_item_list, p_ml->i_pos);
    assert(p_ml->p_mi);
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

    if (i_index >= p_ml->i_count)
        i_index = p_ml->i_count - 1;
    p_mi = eina_list_nth(p_ml->p_item_list, i_index);
    assert(p_mi);
    return p_mi;
}
