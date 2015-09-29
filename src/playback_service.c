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
#include <Emotion.h>

#include "playback_service.h"
#include "ui/interface.h"

#define PLAYLIST_CONTEXT_COUNT (PLAYLIST_CONTEXT_OTHERS + 1)

struct playback_service
{
    media_list *p_ml_list[PLAYLIST_CONTEXT_COUNT];
    media_list *p_ml;
    Evas_Object *p_ea; /* emotion audio */
    Evas_Object *p_ev; /* emotion video */
    Evas_Object *p_emotion; /* p_ea or p_ev */
    Eina_List *p_cbs_list;
};

playback_service *
playback_service_create(application *p_app, interface *p_intf)
{
    playback_service *p_ps = calloc(1, sizeof(playback_service));
    if (!p_ps)
        return NULL;

    for (unsigned int i = 0; i < PLAYLIST_CONTEXT_COUNT; ++i)
    {
        p_ps->p_ml_list[i] = media_list_create(true);
        if (!p_ps->p_ml_list[i])
            goto error;
    }

    p_ps->p_ml = p_ps->p_ml_list[PLAYLIST_CONTEXT_AUDIO];

    emotion_init();
    p_ps->p_ea = emotion_object_add(intf_get_window(p_intf));
    if (!p_ps->p_ea)
        goto error;

    emotion_object_init(p_ps->p_ea, "libvlc");
    emotion_object_video_mute_set(p_ps->p_ea, true);
    p_ps->p_emotion = p_ps->p_ea;

    return p_ps;

error:
    playback_service_destroy(p_ps);
    return NULL;
}

void
playback_service_destroy(playback_service *p_ps)
{
    Eina_List *p_el;
    void *p_id;

    for (unsigned int i = 0; i < PLAYLIST_CONTEXT_COUNT; ++i)
    {
        if (p_ps->p_ml_list[i])
            media_list_destroy(p_ps->p_ml_list[i]);
    }

    /* Clear callback list */
    EINA_LIST_FOREACH(p_ps->p_cbs_list, p_el, p_id)
      free(p_id);
    eina_list_free(p_ps->p_cbs_list);
    p_ps->p_cbs_list = NULL;

    if (p_ps->p_ea)
        evas_object_del(p_ps->p_ea);
    if (p_ps->p_ev)
        evas_object_del(p_ps->p_ev);

    emotion_shutdown();

    free(p_ps);
}

int
playback_service_set_context(playback_service *p_ps, enum PLAYLIST_CONTEXT i_ctx)
{
    if (i_ctx < PLAYLIST_CONTEXT_VIDEO || i_ctx > PLAYLIST_CONTEXT_OTHERS)
        return -1;
    if (p_ps->p_ml_list[i_ctx] == p_ps->p_ml)
        return -1;

    playback_service_stop(p_ps);
    p_ps->p_ml = p_ps->p_ml_list[i_ctx];
    return 0;
}

media_list *
playback_service_get_ml(playback_service *p_ps)
{
    return p_ps->p_ml;
}

void *
playback_service_register_callbacks(playback_service *p_ps, playback_service_callbacks *p_cbs)
{
    Eina_List *p_el;
    playback_service_callbacks *p_cbs_dup = malloc(sizeof(playback_service_callbacks));

    if (!p_cbs_dup)
        return NULL;
    memcpy(p_cbs_dup, p_cbs, sizeof(playback_service_callbacks));

    p_el = eina_list_append(p_ps->p_cbs_list, p_cbs_dup);
    if (p_el == p_ps->p_cbs_list)
    {
        free(p_cbs_dup);
        return NULL;
    }
    p_ps->p_cbs_list = p_el;
    return p_cbs_dup;
}

void
playback_service_unregister_callbacks(playback_service *p_ps, void *p_id)
{
    p_ps->p_cbs_list = eina_list_remove(p_ps->p_cbs_list, p_id);
}

int
playback_service_set_evas_video(playback_service *p_ps, Evas *p_evas)
{
    if (p_ps->p_ev)
    {
        evas_object_del(p_ps->p_ev);
        p_ps->p_ev = NULL;
    }
    if (p_evas)
    {
        p_ps->p_ev = emotion_object_add(p_evas);
        if (!p_ps->p_ev)
            return -1;

        emotion_object_init(p_ps->p_ev, "libvlc");
        p_ps->p_emotion = p_ps->p_ev;
    }
    else
        p_ps->p_emotion = p_ps->p_ea;

    return 0;
}


int
playback_service_start(playback_service *p_ps)
{
    return -1;
}

int
playback_service_stop(playback_service *p_ps)
{
    return -1;
}

int
playback_service_play(playback_service *p_ps)
{
    return -1;
}

int
playback_service_pause(playback_service *p_ps)
{
    return -1;
}

double
playback_service_get_pos(playback_service *p_ps)
{
    return -1;
}

double
playback_service_get_len(playback_service *p_ps)
{
    return -1;
}

int
playback_service_seek(playback_service *p_ps, double f_pos)
{
    return -1;
}
