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
#include "media/media_list.h"

#include "ui/interface.h"

#define PLAYLIST_CONTEXT_COUNT (PLAYLIST_CONTEXT_OTHERS + 1)

static const int META_EMOTIOM_TO_MEDIA_ITEM[] = {
    MEDIA_ITEM_META_TITLE,
    MEDIA_ITEM_META_ARTIST,
    MEDIA_ITEM_META_ALBUM,
    MEDIA_ITEM_META_YEAR,
    MEDIA_ITEM_META_GENRE,
    MEDIA_ITEM_META_COMMENT,
    MEDIA_ITEM_META_DISC_ID,
    MEDIA_ITEM_META_COUNT,
};

struct playback_service
{
    media_list *p_ml_list[PLAYLIST_CONTEXT_COUNT];
    media_list *p_ml;
    Evas_Object *p_ea;  /* emotion audio */
    Evas_Object *p_ev;  /* emotion video */
    Evas_Object *p_e;   /* emotion audio or video */
    Evas *p_ea_evas;
    Eina_List *p_cbs_list;

    bool b_started;
    bool b_seeking;
};

#define PS_SEND_CALLBACK(pf_cb, ...) do { \
    Eina_List *p_el; \
    playback_service_callbacks *p_cbs; \
    EINA_LIST_FOREACH(p_ps->p_cbs_list, p_el, p_cbs) \
        if (p_cbs->pf_cb) \
            p_cbs->pf_cb(p_ps, p_cbs->p_user_data, __VA_ARGS__); \
} while(0)

#define PS_SEND_VOID_CALLBACK(pf_cb) do { \
    Eina_List *p_el; \
    playback_service_callbacks *p_cbs; \
    EINA_LIST_FOREACH(p_ps->p_cbs_list, p_el, p_cbs) \
        if (p_cbs->pf_cb) \
            p_cbs->pf_cb(p_ps, p_cbs->p_user_data); \
} while(0)

static void
ps_emotion_length_change_cb(void *data, Evas_Object *obj, void *event)
{
    playback_service *p_ps = data;
    double i_len = emotion_object_play_length_get(obj);

    PS_SEND_CALLBACK(pf_on_new_len, i_len);
}

static void
ps_emotion_position_update_cb(void *data, Evas_Object *obj, void *event)
{
    playback_service *p_ps = data;

    if (p_ps->b_seeking)
    {
        p_ps->b_seeking = false;
        PS_SEND_VOID_CALLBACK(pf_on_seek_done);
    }
    else
    {
        double i_time = emotion_object_position_get(obj);
        PS_SEND_CALLBACK(pf_on_new_time, i_time);
    }
}

static void
ps_emotion_play_started_cb(void *data, Evas_Object *obj, void *event)
{
    playback_service *p_ps = data;
    media_item *p_mi = media_list_get_item(p_ps->p_ml);

    for (unsigned int i = 0; i < EMOTION_META_INFO_TRACK_COUNT; ++i)
        media_item_set_meta(p_mi, META_EMOTIOM_TO_MEDIA_ITEM[i],
                            emotion_object_meta_info_get(obj, i));

    PS_SEND_CALLBACK(pf_on_started, media_list_get_item(p_ps->p_ml));
}

static void
ps_emotion_play_finished_cb(void *data, Evas_Object *obj, void *event)
{
    playback_service *p_ps = data;

    PS_SEND_CALLBACK(pf_on_stopped, media_list_get_item(p_ps->p_ml));

    /* play next file */
    media_list_set_next(p_ps->p_ml);
}

static void
ml_on_media_added_cb(media_list *p_ml, void *p_user_data, unsigned int i_pos,
                     media_item *p_mi)
{
    playback_service *p_ps = p_user_data;

    PS_SEND_CALLBACK(pf_on_media_added, i_pos, p_mi);
}

static void
ml_on_media_removed_cb(media_list *p_ml, void *p_user_data, unsigned int i_pos,
                        media_item *p_mi)
{
    playback_service *p_ps = p_user_data;

    PS_SEND_CALLBACK(pf_on_media_removed, i_pos, p_mi);
}

static void
ml_on_media_selected_cb(media_list *p_ml, void *p_user_data, unsigned int i_pos,
                        media_item *p_mi)
{
    playback_service *p_ps = p_user_data;
    if (p_ml != p_ps->p_ml)
        return;

    if (p_ps->b_started)
    {
        /* play new media */
        playback_service_stop(p_ps);
        if (i_pos >= 0)
            playback_service_start(p_ps);
    }
    PS_SEND_CALLBACK(pf_on_media_selected, i_pos, p_mi);
}

static Evas_Object *
ps_emotion_create(playback_service *p_ps, Evas *p_evas, bool b_mute_video)
{
    setenv("EMOTION_LIBVLC_DEBUG", "1", 1);
    Evas_Object *p_e = emotion_object_add(p_evas);
    if (!p_e)
    {
        LOGE("emotion_object_add(evas) failed");
        return NULL;
    }

    if (!emotion_object_init(p_e, "libvlc"))
    {
        LOGE("emotion_object_init failed");
        return NULL;
    }

    if (b_mute_video)
        emotion_object_video_mute_set(p_e, b_mute_video);

    evas_object_smart_callback_add(p_e, "position_update",
                                   ps_emotion_position_update_cb, p_ps);
    evas_object_smart_callback_add(p_e, "length_change",
                                   ps_emotion_length_change_cb, p_ps);
    evas_object_smart_callback_add(p_e, "playback_started",
                                   ps_emotion_play_started_cb, p_ps);
    evas_object_smart_callback_add(p_e, "playback_finished",
                                   ps_emotion_play_finished_cb, p_ps);
    //evas_object_smart_callback_add(p_e, "decode_stop",ps_emotion_stop_cb, p_ps);
    //evas_object_smart_callback_add(p_e, "progress_change", ps_emotion_progress_change_cb, p_ps);
    //evas_object_smart_callback_add(p_e, "audio_level_change", ps_emotion_audio_change, p_ps);
    //evas_object_smart_callback_add(p_e, "channels_change", ps_emotion_channels_change, p_ps);

    return p_e;
}

playback_service *
playback_service_create(application *p_app)
{
    playback_service *p_ps = calloc(1, sizeof(playback_service));
    if (!p_ps)
        return NULL;

    for (unsigned int i = 0; i < PLAYLIST_CONTEXT_COUNT; ++i)
    {
        media_list_callbacks cbs = {
                .pf_on_media_added = ml_on_media_added_cb,
                .pf_on_media_removed = ml_on_media_removed_cb,
                .pf_on_media_selected = ml_on_media_selected_cb,
                .p_user_data = p_ps,
        };
        p_ps->p_ml_list[i] = media_list_create(true);
        if (!p_ps->p_ml_list[i])
            goto error;
        if (media_list_register_callbacks(p_ps->p_ml_list[i], &cbs) == NULL)
            goto error;
    }

    p_ps->p_ml = p_ps->p_ml_list[PLAYLIST_CONTEXT_AUDIO];

    p_ps->p_ea_evas = evas_new();
    if (!p_ps->p_ea_evas)
        goto error;

    p_ps->p_ea = ps_emotion_create(p_ps, p_ps->p_ea_evas, true);
    if (!p_ps->p_ea)
        goto error;
    p_ps->p_e = p_ps->p_ea;

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
    if (p_ps->p_ea_evas)
        evas_free(p_ps->p_ea_evas);

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

playback_service_cbs_id *
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
    return (playback_service_cbs_id *) p_cbs_dup;
}

void
playback_service_unregister_callbacks(playback_service *p_ps, playback_service_cbs_id *p_id)
{
    p_ps->p_cbs_list = eina_list_remove(p_ps->p_cbs_list, p_id);
    free(p_id);
}

Evas_Object *
playback_service_set_evas_video(playback_service *p_ps, Evas *p_evas)
{
    if (p_ps->p_ev)
    {
        evas_object_del(p_ps->p_ev);
        p_ps->p_ev = NULL;
    }
    if (p_evas)
    {
        p_ps->p_ev = ps_emotion_create(p_ps, p_evas, false);
        if (!p_ps->p_ev)
            return NULL;
        p_ps->p_e = p_ps->p_ev;

        return p_ps->p_ev;
    }
    else
    {
        p_ps->p_e = p_ps->p_ea;
        return NULL;
    }
}


int
playback_service_start(playback_service *p_ps)
{
    media_item *p_mi;

    if (p_ps->b_started)
        return -1;

    p_mi = media_list_get_item(p_ps->p_ml);
    if (!p_mi || !p_mi->psz_path)
    {
        LOGE("playback_service_start without item");
        return -1;
    }
    LOGD("playback_service_start: %s", p_mi->psz_path);

    if (!emotion_object_file_set(p_ps->p_e, p_mi->psz_path))
    {
        LOGE("emotion_object_file_set failed");
        return -1;
    }

    p_ps->b_started = true;
    return playback_service_play(p_ps);
}

int
playback_service_stop(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return -1;

    playback_service_pause(p_ps);
    emotion_object_file_set(p_ps->p_e, NULL);
    p_ps->b_started = false;
    return 0;
}

bool
playback_service_is_started(playback_service *p_ps)
{
    return !!p_ps->b_started;
}

int
playback_service_play(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return -1;

    emotion_object_play_set(p_ps->p_e, true);
    return 0;
}

int
playback_service_pause(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return -1;

    emotion_object_play_set(p_ps->p_e, false);
    return 0;
}

bool
playback_service_toggle_play_pause(playback_service *p_ps)
{
    bool b_new_state;
    if (!p_ps->b_started)
        return false;

    b_new_state = !emotion_object_play_get(p_ps->p_e);
    emotion_object_play_set(p_ps->p_e, b_new_state);
    return b_new_state;
}

bool
playback_service_is_playing(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return false;

    return emotion_object_play_get(p_ps->p_e);
}

double
playback_service_get_pos(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return 0.0;

    return emotion_object_position_get(p_ps->p_e);
}

double
playback_service_get_len(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return 0.0;

    return emotion_object_play_length_get(p_ps->p_e);
}

int
playback_service_seek_time(playback_service *p_ps, double i_time)
{
    if (!p_ps->b_started)
        return -1;

    emotion_object_position_set(p_ps->p_e, i_time);
    p_ps->b_seeking = true;
    return 0;
}

int
playback_service_seek_pos(playback_service *p_ps, double i_percent)
{
    if (!p_ps->b_started)
        return -1;

    double i_time = emotion_object_play_length_get(p_ps->p_e) * i_percent;
    emotion_object_position_set(p_ps->p_e, i_time);
    p_ps->b_seeking = true;
    return 0;
}

int
playback_service_seek_forward(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return -1;

    /* TODO increase step by step */
    double i_time = emotion_object_position_get(p_ps->p_e);
    emotion_object_position_set(p_ps->p_e, i_time + 5);
    p_ps->b_seeking = true;
    return 0;
}

int
playback_service_seek_backward(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return -1;

    /* TODO increase step by step */
    double i_time = emotion_object_position_get(p_ps->p_e);
    emotion_object_position_set(p_ps->p_e, i_time - 5);
    p_ps->b_seeking = true;
    return 0;
}

int
playback_service_list_insert(playback_service *p_ps, int i_index, media_item *p_mi)
{
    return media_list_insert(p_ps->p_ml, i_index, p_mi);
}

int
playback_service_list_remove(playback_service *p_ps, media_item *p_mi)
{
    return media_list_remove(p_ps->p_ml, p_mi);
}

int
playback_service_list_remove_index(playback_service *p_ps, unsigned int i_index)
{
    return media_list_remove_index(p_ps->p_ml, i_index);
}

void
playback_service_list_clear(playback_service *p_ps)
{
    return media_list_clear(p_ps->p_ml);
}

unsigned int
playback_service_list_get_count(playback_service *p_ps)
{
    return media_list_get_count(p_ps->p_ml);
}

unsigned int
playback_service_list_get_pos(playback_service *p_ps)
{
    return media_list_get_pos(p_ps->p_ml);
}

void
playback_service_list_set_pos(playback_service *p_ps, unsigned int i_index)
{
    media_list_set_pos(p_ps->p_ml, i_index);
}

void
playback_service_list_set_next(playback_service *p_ps)
{
    media_list_set_next(p_ps->p_ml);
}

void
playback_service_list_set_prev(playback_service *p_ps)
{
    media_list_set_prev(p_ps->p_ml);
}

media_item *
playback_service_list_get_item(playback_service *p_ps)
{
    return media_list_get_item(p_ps->p_ml);
}

media_item *
playback_service_list_get_item_at(playback_service *p_ps,  unsigned int i_index)
{
    return media_list_get_item_at(p_ps->p_ml, i_index);
}
