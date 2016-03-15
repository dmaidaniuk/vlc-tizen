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

#include <Elementary.h>
#include <Emotion.h>
#include <app.h>
#include <app_control.h>

#include <device/power.h>
#include <sound_manager.h>

#include "playback_service.h"
#include "media/media_list.h"
#include "preferences/preferences.h"
#include "ui/equalizer.h"

#include "ui/views/minicontrol_view.h"

#define PLAYLIST_CONTEXT_COUNT (PLAYLIST_CONTEXT_OTHERS)

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

static const char *STR_POWER_LOCKS[] = {
    "POWER_LOCK_CPU",
    "POWER_LOCK_DISPLAY",
    "POWER_LOCK_DISPLAY_DIM",
};

struct playback_service
{
    enum PLAYLIST_CONTEXT i_ctx;
    media_list *p_ml_list[PLAYLIST_CONTEXT_COUNT];
    media_list *p_ml;
    Evas_Object *p_ea;  /* emotion audio */
    Evas_Object *p_ev;  /* emotion video */
    Evas_Object *p_e;   /* emotion audio or video */
    Evas *p_ea_evas;
    Evas *p_ev_evas;

    Eina_List *p_cbs_list;

    int i_current_lock;

    bool b_started;
    bool b_seeking;

    bool b_auto_exit;
    bool b_restart_emotion;
    bool b_video_background;

    minicontrol     *p_minicontrol;
    double          i_last_notification_pos;

    ps_on_emotion_restart   emotion_restart_cb;
    void                    *emotion_restart_cb_data;
};

#define PS_SEND_CALLBACK(pf_cb, ...) do { \
    if (p_ps->p_cbs_list) { \
        Eina_List *p_el, *p_el_next; \
        playback_service_callbacks *p_cbs; \
        EINA_LIST_FOREACH_SAFE(p_ps->p_cbs_list, p_el_next, p_el, p_cbs) { \
            if (p_cbs->pf_cb && (p_cbs->i_ctx == p_ps->i_ctx || p_cbs->i_ctx == PLAYLIST_CONTEXT_NONE)) \
                p_cbs->pf_cb(p_ps, p_cbs->p_user_data, __VA_ARGS__); \
        } \
    } \
} while(0)

#define PS_SEND_VOID_CALLBACK(pf_cb) do { \
    if (p_ps->p_cbs_list) { \
        Eina_List *p_el, *p_el_next; \
        playback_service_callbacks *p_cbs; \
        EINA_LIST_FOREACH_SAFE(p_ps->p_cbs_list, p_el_next, p_el, p_cbs) { \
            if (p_cbs->pf_cb && (p_cbs->i_ctx == p_ps->i_ctx || p_cbs->i_ctx == PLAYLIST_CONTEXT_NONE)) \
                p_cbs->pf_cb(p_ps, p_cbs->p_user_data); \
        } \
    } \
} while(0)

void
ps_register_on_emotion_restart_cb(playback_service *p_ps, ps_on_emotion_restart func, void *data)
{
    p_ps->emotion_restart_cb = func;
    p_ps->emotion_restart_cb_data = data;
}

static void
ps_notification_create(playback_service *p_ps, application *p_app)
{
    p_ps->p_minicontrol = mini_control_view_create(p_ps, p_app);
}

static void
ps_notification_update_meta(playback_service *p_ps, media_item *p_mi)
{
    const char *psz_meta_title = media_item_title(p_mi);
    const char *psz_meta_artist = media_item_artist(p_mi);

    if (psz_meta_title)
        mini_control_title_set(p_ps->p_minicontrol, psz_meta_title);
    else if (psz_meta_artist)
        mini_control_title_set(p_ps->p_minicontrol, psz_meta_artist);

    mini_control_cover_set(p_ps->p_minicontrol, p_mi->psz_snapshot);
}

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
        double i_len = emotion_object_play_length_get(obj);
        double i_pos = (i_time > 0.0 && i_len > 0.0) ? i_time / i_len : 0.0;

        /* update position every 1 % */
        if (p_ps->i_ctx != PLAYLIST_CONTEXT_VIDEO
         && fabs(p_ps->i_last_notification_pos - i_pos) > 0.01f)
        {
            p_ps->i_last_notification_pos = i_pos;
            mini_control_progress_set(p_ps->p_minicontrol, i_pos);
        }

        PS_SEND_CALLBACK(pf_on_new_time, i_time, i_pos);
    }
}

static void
ps_emotion_play_started_cb(void *data, Evas_Object *obj, void *event)
{
    playback_service *p_ps = data;
    media_item *p_mi = media_list_get_item(p_ps->p_ml);
    const char *meta;

    for (unsigned int i = 0; i < EMOTION_META_INFO_TRACK_COUNT; ++i)
    {
        meta = emotion_object_meta_info_get(obj, i);
        if (meta != NULL)
            media_item_set_meta(p_mi, META_EMOTIOM_TO_MEDIA_ITEM[i], meta);
    }

    PS_SEND_CALLBACK(pf_on_started, p_mi);

    mini_control_playing_set(p_ps->p_minicontrol, EINA_TRUE);

    if (p_ps->i_ctx != PLAYLIST_CONTEXT_VIDEO)
    {
        p_ps->i_last_notification_pos = 0.0001f;

        ps_notification_update_meta(p_ps, p_mi);

        mini_control_visibility_set(p_ps->p_minicontrol, EINA_TRUE);
        mini_control_progress_set(p_ps->p_minicontrol, 0);
    }
    else
    {
        mini_control_visibility_set(p_ps->p_minicontrol, EINA_FALSE);
    }
}

static void
ps_emotion_play_finished_cb(void *data, Evas_Object *obj, void *event)
{
    playback_service *p_ps = data;

    LOGD("ps_emotion_play_finished_cb");

    /* play next file or stop */
    if (!media_list_set_next(p_ps->p_ml))
        playback_service_stop_notify(p_ps, true);
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
ml_on_media_selected_cb(media_list *p_ml, void *p_user_data, int i_pos,
                        media_item *p_mi)
{
    playback_service *p_ps = p_user_data;
    if (p_ml != p_ps->p_ml)
        return;

    if (p_ps->b_started)
    {
        LOGD("ml_on_media_selected_cb: %d", i_pos);

        if (i_pos < 0 || playback_service_start(p_ps, 0) != 0)
            playback_service_stop_notify(p_ps, true);
    }
    PS_SEND_CALLBACK(pf_on_media_selected, i_pos, p_mi);
}

static Evas_Object *
ps_emotion_create(playback_service *p_ps, Evas *p_evas, bool b_mute_video)
{
    char *options;

    /* Prepare libvlc options */
    if ((options = preferences_get_libvlc_options()) != NULL)
    {
        unsetenv("EMOTION_LIBVLC_ARGS");
        if (setenv("EMOTION_LIBVLC_ARGS", options, 0) != 0)
            LOGE("Failed setting environment");
        LOGD("libvlc options: %s", options);
        free(options);
    }
    else
    {
        LOGE("Unable to allocate memory");
    }

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

    if ( equalizer_is_enabled() )
    {
        float f_preamp = equalizer_get_preamp_value();
        unsigned int i_nb_bands = equalizer_get_nb_bands();
        float f_bands[i_nb_bands];
        for ( unsigned int i = 0; i < i_nb_bands; ++i )
            f_bands[i] = equalizer_get_band_value( i );
        // Don't use playback_service_eq_set since we haven't assigned p_e yet
        emotion_object_equalizer_set( p_e, f_preamp, i_nb_bands, f_bands );
    }

    return p_e;
}

static void
ps_emotion_destroy(playback_service *p_ps, Evas_Object *p_e)
{
    evas_object_smart_callback_del(p_e, "position_update",
                                   ps_emotion_position_update_cb);
    evas_object_smart_callback_del(p_e, "length_change",
                                   ps_emotion_length_change_cb);
    evas_object_smart_callback_del(p_e, "playback_started",
                                   ps_emotion_play_started_cb);
    evas_object_smart_callback_del(p_e, "playback_finished",
                                   ps_emotion_play_finished_cb);
    evas_object_del(p_e);
}

static media_list *
get_media_list(playback_service *p_ps, enum PLAYLIST_CONTEXT i_ctx)
{
    assert(i_ctx > PLAYLIST_CONTEXT_NONE || i_ctx <= PLAYLIST_CONTEXT_OTHERS);
    return p_ps->p_ml_list[i_ctx - 1];
}

playback_service *
playback_service_create(application *p_app)
{
    playback_service *p_ps = calloc(1, sizeof(playback_service));
    if (!p_ps)
        return NULL;

    p_ps->i_current_lock = -1;
    p_ps->b_auto_exit = false;

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

    p_ps->i_ctx = PLAYLIST_CONTEXT_AUDIO;
    p_ps->p_ml = get_media_list(p_ps, p_ps->i_ctx);

    p_ps->p_ea_evas = evas_new();
    if (p_ps->p_ea_evas)
    {
        p_ps->p_ea = ps_emotion_create(p_ps, p_ps->p_ea_evas, true);
        if (p_ps->p_ea)
            p_ps->p_e = p_ps->p_ea;
    }

    ps_notification_create(p_ps, p_app);

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
    if (p_ps->p_cbs_list)
    {
        EINA_LIST_FOREACH(p_ps->p_cbs_list, p_el, p_id)
          free(p_id);
        eina_list_free(p_ps->p_cbs_list);
    }

    if (p_ps->p_ea)
        ps_emotion_destroy(p_ps, p_ps->p_ea);
    if (p_ps->p_ev)
        ps_emotion_destroy(p_ps, p_ps->p_ev);

    mini_control_destroy(p_ps->p_minicontrol);

    free(p_ps);
}

int
playback_service_force_restart_emotion(playback_service *p_ps)
{
    LOGD("Restarting emotion...");
    bool is_ea = p_ps->p_e == p_ps->p_ea;

    p_ps->b_restart_emotion = false;

    playback_service_stop_notify(p_ps, true);

    p_ps->p_e = NULL;
    if (p_ps->p_ea)
    {
        ps_emotion_destroy(p_ps, p_ps->p_ea);
        p_ps->p_ea = NULL;
    }
    if (p_ps->p_ev)
    {
        ps_emotion_destroy(p_ps, p_ps->p_ev);
        p_ps->p_ev = NULL;
    }

    p_ps->p_ea = ps_emotion_create(p_ps, p_ps->p_ea_evas, true);
    if (!p_ps->p_ea)
        return -1;

    if (p_ps->p_ev_evas)
    {
        p_ps->p_ev = ps_emotion_create(p_ps, p_ps->p_ev_evas, false);
        if (!p_ps->p_ev) {
            ps_emotion_destroy(p_ps, p_ps->p_ea);
            p_ps->p_ea = NULL;
            return -1;
        }
    }

    p_ps->p_e = is_ea ? p_ps->p_ea : p_ps->p_ev;

    if (p_ps->emotion_restart_cb != NULL)
    {
        p_ps->emotion_restart_cb(p_ps->emotion_restart_cb_data);
    }

    return 0;
}

int
playback_service_restart_emotion(playback_service *p_ps, bool immediate)
{
    if (immediate || !playback_service_is_playing(p_ps))
        return playback_service_force_restart_emotion(p_ps);

    // Schedule a restart after the next playback stop
    p_ps->b_restart_emotion = true;

    LOGD("Emotion will restart after the next stop");
    return 0;
}

int
playback_service_set_context(playback_service *p_ps, enum PLAYLIST_CONTEXT i_ctx)
{
    if (i_ctx <= PLAYLIST_CONTEXT_NONE || i_ctx >  PLAYLIST_CONTEXT_OTHERS)
        return -1;
    if (get_media_list(p_ps, i_ctx) == p_ps->p_ml)
        return -1;
    if (i_ctx == PLAYLIST_CONTEXT_VIDEO && !p_ps->p_ev)
        return -1;

    playback_service_stop_notify(p_ps, true);
    p_ps->i_ctx = i_ctx;
    p_ps->p_ml = get_media_list(p_ps, i_ctx);
    p_ps->p_e = i_ctx == PLAYLIST_CONTEXT_VIDEO ? p_ps->p_ev : p_ps->p_ea;
    return 0;
}

enum PLAYLIST_CONTEXT
playback_service_get_context(playback_service *p_ps)
{
    return p_ps->i_ctx;
}

playback_service_cbs_id *
playback_service_register_callbacks(playback_service *p_ps, playback_service_callbacks *p_cbs)
{
    playback_service_callbacks *p_cbs_dup = malloc(sizeof(playback_service_callbacks));

    if (!p_cbs_dup)
    {
        LOGE("malloc failed");
        return NULL;
    }
    memcpy(p_cbs_dup, p_cbs, sizeof(playback_service_callbacks));

    p_ps->p_cbs_list = eina_list_append(p_ps->p_cbs_list, p_cbs_dup);
    if (!p_ps->p_cbs_list)
    {
        free(p_cbs_dup);
        return NULL;
    }
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
        ps_emotion_destroy(p_ps, p_ps->p_ev);
        p_ps->p_ev = NULL;
        p_ps->p_ev_evas = NULL;
    }
    if (p_evas)
    {
        p_ps->p_ev = ps_emotion_create(p_ps, p_evas, false);
        if (!p_ps->p_ev)
            return NULL;
        p_ps->p_ev_evas = p_evas;

        return p_ps->p_ev;
    }
    else
    {
        if (p_ps->i_ctx == PLAYLIST_CONTEXT_VIDEO)
            playback_service_set_context(p_ps, PLAYLIST_CONTEXT_AUDIO);
        return NULL;
    }
}

static void
ps_acquire_lock(playback_service *p_ps, power_lock_e i_new_lock)
{
    if (device_power_request_lock(i_new_lock, 0) == DEVICE_ERROR_NONE)
    {
        p_ps->i_current_lock = i_new_lock;
        LOGD("ps_acquire_lock: get %s", STR_POWER_LOCKS[i_new_lock]);
    }
    else
        LOGE("ps_acquire_lock: failed to get %s", STR_POWER_LOCKS[i_new_lock]);
}

static void
ps_release_lock(playback_service *p_ps)
{
    if (p_ps->i_current_lock != -1)
    {
        device_power_release_lock(p_ps->i_current_lock);
        LOGD("ps_release_lock: release %s", STR_POWER_LOCKS[p_ps->i_current_lock]);
        p_ps->i_current_lock = -1;
    }
}

void
sound_session_interrupted_cb2(sound_session_interrupted_code_e code, void *user_data)
{
    playback_service *p_ps = user_data;

    switch(code) {
    case SOUND_SESSION_INTERRUPTED_COMPLETED:
        LOGD("SOUND_SESSION_INTERRUPTED_COMPLETED");
        break;
    case SOUND_SESSION_INTERRUPTED_BY_MEDIA:
        LOGD("SOUND_SESSION_INTERRUPTED_BY_MEDIA");
        break;
    case SOUND_SESSION_INTERRUPTED_BY_CALL:
        LOGD("SOUND_SESSION_INTERRUPTED_BY_CALL");
        break;
    case SOUND_SESSION_INTERRUPTED_BY_EARJACK_UNPLUG:
        LOGD("SOUND_SESSION_INTERRUPTED_BY_EARJACK_UNPLUG");
        break;
    case SOUND_SESSION_INTERRUPTED_BY_RESOURCE_CONFLICT:
        LOGD("SOUND_SESSION_INTERRUPTED_BY_RESOURCE_CONFLICT");
        break;
    case SOUND_SESSION_INTERRUPTED_BY_ALARM:
        LOGD("SOUND_SESSION_INTERRUPTED_BY_ALARM");
        break;
    case SOUND_SESSION_INTERRUPTED_BY_EMERGENCY:
        LOGD("SOUND_SESSION_INTERRUPTED_BY_EMERGENCY");
        break;
    case SOUND_SESSION_INTERRUPTED_BY_NOTIFICATION:
        LOGD("SOUND_SESSION_INTERRUPTED_BY_NOTIFICATION");
        break;
    default:
        LOGD("DEFAULT");
    }

    switch(code) {
    case SOUND_SESSION_INTERRUPTED_COMPLETED:
        playback_service_play(p_ps);
        break;
    case SOUND_SESSION_INTERRUPTED_BY_MEDIA:
    case SOUND_SESSION_INTERRUPTED_BY_CALL:
    case SOUND_SESSION_INTERRUPTED_BY_ALARM:
    case SOUND_SESSION_INTERRUPTED_BY_EMERGENCY:
    case SOUND_SESSION_INTERRUPTED_BY_NOTIFICATION:
        playback_service_pause(p_ps);
        break;
    case SOUND_SESSION_INTERRUPTED_BY_RESOURCE_CONFLICT:
    case SOUND_SESSION_INTERRUPTED_BY_EARJACK_UNPLUG:
        playback_service_pause(p_ps);
        break;
    }
}

int
playback_service_start(playback_service *p_ps, double i_time)
{
    media_item *p_mi;
    power_lock_e i_new_lock;

    if (!p_ps->p_e)
    {
        LOGE("no emotion object set");
        return -1;
    }

    p_mi = media_list_get_item(p_ps->p_ml);
    if (!p_mi || !p_mi->psz_path)
    {
        LOGE("playback_service_start without item");
        return -1;
    }
    LOGD("playback_service_start: %s", p_mi->psz_path);

    // Unset the current file. Because emotion_object_file_set returns EINA_FALSE
    // when reloading the same file, we need to unset it first to allow the REPEAT_ONE
    // function to work.
    emotion_object_file_set(p_ps->p_e, NULL);

    if (!emotion_object_file_set(p_ps->p_e, p_mi->psz_path))
    {
        LOGE("emotion_object_file_set failed");
        return -1;
    }
    if (i_time > 0)
        emotion_object_position_set(p_ps->p_e, i_time);

    i_new_lock = p_ps->p_e == p_ps->p_ev ? POWER_LOCK_DISPLAY : POWER_LOCK_CPU;

    if (p_ps->i_current_lock != i_new_lock)
    {
        ps_release_lock(p_ps);
        ps_acquire_lock(p_ps, i_new_lock);
    }

    sound_manager_set_session_interrupted_cb(sound_session_interrupted_cb2, p_ps);

    p_ps->b_started = true;
    return playback_service_play(p_ps);
}

int
playback_service_stop_notify(playback_service *p_ps, bool b_notify)
{
    if (!p_ps->b_started)
        return -1;

    playback_service_pause(p_ps);
    emotion_object_file_set(p_ps->p_e, NULL);
    p_ps->b_started = false;
    p_ps->b_video_background = false;
    ps_release_lock(p_ps);

    sound_manager_unset_session_interrupted_cb();

    if (p_ps->b_restart_emotion && !p_ps->b_auto_exit)
        playback_service_force_restart_emotion(p_ps);

    if (b_notify)
        PS_SEND_VOID_CALLBACK(pf_on_stopped);

    mini_control_playing_set(p_ps->p_minicontrol, EINA_FALSE);
    mini_control_visibility_set(p_ps->p_minicontrol, EINA_FALSE);

    if (p_ps->b_auto_exit)
        ui_app_exit();

    return 0;
}

int
playback_service_stop(playback_service *p_ps)
{
    return playback_service_stop_notify(p_ps, false);
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
    PS_SEND_CALLBACK(pf_on_playpause, true);
    mini_control_playing_set(p_ps->p_minicontrol, EINA_TRUE);
    return 0;
}

int
playback_service_pause(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return -1;

    emotion_object_play_set(p_ps->p_e, false);
    PS_SEND_CALLBACK(pf_on_playpause, false);
    mini_control_playing_set(p_ps->p_minicontrol, EINA_FALSE);
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
    PS_SEND_CALLBACK(pf_on_playpause, b_new_state);
    mini_control_playing_set(p_ps->p_minicontrol, b_new_state);
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
playback_service_get_time(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return 0.0;

    return emotion_object_position_get(p_ps->p_e);
}

double
playback_service_get_pos(playback_service *p_ps)
{
    if (!p_ps->b_started)
        return 0.0;

    double i_time = emotion_object_position_get(p_ps->p_e);
    double i_len = emotion_object_play_length_get(p_ps->p_e);
    return (i_time > 0.0 && i_len > 0.0) ? i_time / i_len : 0.0;
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

Eina_List*
playback_service_spu_channel_get_list(playback_service *p_ps)
{
    Eina_List *list = NULL;
    int count = emotion_object_spu_channel_count(p_ps->p_e);

    for (int i = 0; i < count; i++)
    {
        list = eina_list_append(list, emotion_object_spu_channel_name_get(p_ps->p_e, i));
    }

    return list;
}

void
playback_service_spu_channel_set(playback_service *p_ps, int spu)
{
    emotion_object_spu_channel_set(p_ps->p_e, spu);
}

Eina_List*
playback_service_audio_channel_get_list(playback_service *p_ps)
{
    Eina_List *list = NULL;
    int count = emotion_object_audio_channel_count(p_ps->p_e);

    for (int i = 0; i < count; i++)
        list = eina_list_append(list, emotion_object_spu_channel_name_get(p_ps->p_e, i));

    return list;
}

void
playback_service_audio_channel_set(playback_service *p_ps, int channel)
{
    emotion_object_audio_channel_set(p_ps->p_e, channel);
}

void
playback_service_set_auto_exit(playback_service *p_ps, bool value)
{
    p_ps->b_auto_exit = value;
}

void
playback_service_set_repeat_mode(playback_service *p_ps, enum PLAYLIST_REPEAT i_repeat)
{
    media_list_set_repeat_mode(get_media_list(p_ps, PLAYLIST_CONTEXT_AUDIO), i_repeat);
}

enum PLAYLIST_REPEAT
playback_service_get_repeat_mode(playback_service *p_ps)
{
    return media_list_get_repeat_mode(get_media_list(p_ps, PLAYLIST_CONTEXT_AUDIO));
}

double
playback_service_get_play_speed(playback_service *p_ps)
{
    return emotion_object_play_speed_get(p_ps->p_e);
}

void
playback_service_set_play_speed(playback_service *p_ps, double rate)
{
    emotion_object_play_speed_set(p_ps->p_e, rate);
}

void
playback_service_enable_background_playback(playback_service *p_ps)
{
    if (p_ps->i_ctx != PLAYLIST_CONTEXT_VIDEO)
        return;

    double time = playback_service_get_time(p_ps);

    if (!media_list_copy_list(get_media_list(p_ps, PLAYLIST_CONTEXT_VIDEO), get_media_list(p_ps, PLAYLIST_CONTEXT_AUDIO)))
        LOGE("Copying video playlist to audio failed");
    if (!playback_service_set_context(p_ps, PLAYLIST_CONTEXT_AUDIO))
        LOGE("Switching from video context to audio failed");

    p_ps->b_video_background = true;
    playback_service_start(p_ps, time);
}

void
playback_service_disable_background_playback(playback_service *p_ps)
{
    if (p_ps->i_ctx != PLAYLIST_CONTEXT_AUDIO || p_ps->b_video_background == false)
        return;

    p_ps->b_video_background = false;
    playback_service_stop(p_ps);
}

bool
playback_service_is_background_playback(playback_service *p_ps)
{
    return p_ps->b_video_background;
}

const char*
playback_service_current_file_path_get(playback_service *p_ps)
{
    return emotion_object_file_get(p_ps->p_e);
}

void
playback_service_eq_set(playback_service* p_ps, float f_preamp, unsigned int i_nb_bands, float* f_bands )
{
    emotion_object_equalizer_set( p_ps->p_e, f_preamp, i_nb_bands, f_bands );
}

void
playback_service_eq_get(playback_service* p_ps, float* f_preamp, unsigned int* i_nb_bands, float** f_bands )
{
    emotion_object_equalizer_get( p_ps->p_e, f_preamp, i_nb_bands, f_bands );
}
