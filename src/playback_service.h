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
#include "media/media_list.h"

enum PLAYLIST_CONTEXT {
    PLAYLIST_CONTEXT_AUDIO,
    PLAYLIST_CONTEXT_VIDEO,
    PLAYLIST_CONTEXT_OTHERS,
};

typedef struct playback_service_callbacks playback_service_callbacks;
struct playback_service_callbacks
{
    void (*pf_on_started)(playback_service *p_ps, void *p_user_data, media_item *p_mi);
    void (*pf_on_stopped)(playback_service *p_ps, void *p_user_data, media_item *p_mi);
    void (*pf_on_new_len)(playback_service *p_ps, void *p_user_data, double i_len);
    void (*pf_on_new_pos)(playback_service *p_ps, void *p_user_data, double i_pos);
    void (*pf_on_seek_done)(playback_service *p_ps, void *p_user_data);
    void (*pf_on_new_title)(playback_service *p_ps, void *p_user_data, const char *psz_title);
    void *p_user_data;
};

playback_service *
playback_service_create(application *p_app, interface *p_intf);

void
playback_service_destroy(playback_service *p_ps);

int
playback_service_set_context(playback_service *p_ps, enum PLAYLIST_CONTEXT i_ctx);

media_list *
playback_service_get_ml(playback_service *p_ps);

void *
playback_service_register_callbacks(playback_service *p_ps, playback_service_callbacks *p_cbs);

void
playback_service_unregister_callbacks(playback_service *p_ps, void *p_id);

Evas_Object *
playback_service_set_evas_video(playback_service *p_ps, Evas *p_evas);

int
playback_service_start(playback_service *p_ps);

int
playback_service_stop(playback_service *p_ps);

bool
playback_service_is_started(playback_service *p_ps);

int
playback_service_play(playback_service *p_ps);

int
playback_service_pause(playback_service *p_ps);

bool
playback_service_is_playing(playback_service *p_ps);

double
playback_service_get_pos(playback_service *p_ps);

double
playback_service_get_len(playback_service *p_ps);

int
playback_service_seek(playback_service *p_ps, double i_pos);


#endif /* PLAYBACK_SERVICE_H */
