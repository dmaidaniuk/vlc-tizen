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

#ifndef PLAYBACK_SERVICE_H
#define PLAYBACK_SERVICE_H

#include "application.h"
#include "media/media_item.h"
#include "media/media_list.h"

enum PLAYLIST_CONTEXT {
    PLAYLIST_CONTEXT_NONE,
    PLAYLIST_CONTEXT_AUDIO,
    PLAYLIST_CONTEXT_VIDEO,
    PLAYLIST_CONTEXT_OTHERS,
};

typedef struct playback_service_cbs_id playback_service_cbs_id;
typedef struct playback_service_callbacks playback_service_callbacks;
struct playback_service_callbacks
{
    void (*pf_on_media_added)(playback_service *p_ps, void *p_user_data, unsigned int i_pos, media_item *p_mi);
    void (*pf_on_media_removed)(playback_service *p_ps, void *p_user_data, unsigned int i_pos, media_item *p_mi);
    void (*pf_on_media_selected)(playback_service *p_ps, void *p_user_data, unsigned int i_pos, media_item *p_mi);
    void (*pf_on_started)(playback_service *p_ps, void *p_user_data, media_item *p_mi);
    void (*pf_on_playpause)(playback_service *p_ps, void *p_user_data, bool b_playing);
    void (*pf_on_stopped)(playback_service *p_ps, void *p_user_data);
    void (*pf_on_new_len)(playback_service *p_ps, void *p_user_data, double i_len);
    void (*pf_on_new_time)(playback_service *p_ps, void *p_user_data, double i_time, double i_pos);
    void (*pf_on_seek_done)(playback_service *p_ps, void *p_user_data);
    void *p_user_data;
    enum PLAYLIST_CONTEXT i_ctx;
};

playback_service *
playback_service_create(application *p_app);

void
playback_service_destroy(playback_service *p_ps);

int
playback_service_restart_emotion(playback_service *p_ps, bool immediate);

int
playback_service_set_context(playback_service *p_ps, enum PLAYLIST_CONTEXT i_ctx);

playback_service_cbs_id *
playback_service_register_callbacks(playback_service *p_ps, playback_service_callbacks *p_cbs);

void
playback_service_unregister_callbacks(playback_service *p_ps, playback_service_cbs_id *p_id);

Evas_Object *
playback_service_set_evas_video(playback_service *p_ps, Evas *p_evas);

int
playback_service_start(playback_service *p_ps, double i_time);

int
playback_service_stop(playback_service *p_ps);

bool
playback_service_is_started(playback_service *p_ps);

int
playback_service_play(playback_service *p_ps);

int
playback_service_pause(playback_service *p_ps);

bool
playback_service_toggle_play_pause(playback_service *p_ps);

int
playback_service_pause(playback_service *p_ps);

bool
playback_service_is_playing(playback_service *p_ps);

double
playback_service_get_time(playback_service *p_ps);

double
playback_service_get_pos(playback_service *p_ps);

double
playback_service_get_len(playback_service *p_ps);

int
playback_service_seek_time(playback_service *p_ps, double i_time);

int
playback_service_seek_pos(playback_service *p_ps, double i_percent);

int
playback_service_seek_forward(playback_service *p_ps);

int
playback_service_seek_backward(playback_service *p_ps);

int
playback_service_list_insert(playback_service *p_ps, int i_index, media_item *p_mi);

static inline int
playback_service_list_append(playback_service *p_ps, media_item *p_mi)
{
    return playback_service_list_insert(p_ps, -1, p_mi);
}

int
playback_service_list_remove(playback_service *p_ps, media_item *p_mi);

int
playback_service_list_remove_index(playback_service *p_ps, unsigned int i_index);

void
playback_service_list_clear(playback_service *p_ps);

unsigned int
playback_service_list_get_count(playback_service *p_ps);

unsigned int
playback_service_list_get_pos(playback_service *p_ps);

void
playback_service_list_set_pos(playback_service *p_ps, unsigned int i_index);

void
playback_service_list_set_next(playback_service *p_ps);

void
playback_service_list_set_prev(playback_service *p_ps);

media_item *
playback_service_list_get_item(playback_service *p_ps);

media_item *
playback_service_list_get_item_at(playback_service *p_ps,  unsigned int i_index);

Eina_List*
playback_service_spu_channel_get_list(playback_service *p_ps);

void
playback_service_spu_channel_set(playback_service *p_ps, int spu);

Eina_List*
playback_service_audio_channel_get_list(playback_service *p_ps);

void
playback_service_audio_channel_set(playback_service *p_ps, int channel);

void
playback_service_set_auto_exit(playback_service *p_ps, bool value);

void
playback_service_set_repeat_mode(playback_service *p_ps, enum PLAYLIST_REPEAT i_repeat);

enum PLAYLIST_REPEAT
playback_service_get_repeat_mode(playback_service *p_ps);

#endif /* PLAYBACK_SERVICE_H */
