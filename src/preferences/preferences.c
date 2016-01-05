/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Ludovic Fauvet <etix@videolan.org>
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

#include "preferences/preferences.h"
#include "ui/settings/menu_id.h"
#include <app_preference.h>

typedef union pref_id {
    pref_enum t_enum;
    pref_index t_index;
    pref_bool t_bool;
    int t_int;
} pref_id;

typedef struct pref_key_id_map
{
    pref_id id;
    const char* key;
} pref_key_id_map;

pref_key_id_map mapping[] =
{
        // type enum
        {{.t_enum = PREF_HWACCELERATION}, "HWACCELERATION"},
        {{.t_enum = PREF_ORIENTATION}, "ORIENTATION"},
        {{.t_enum = PREF_DEBLOCKING}, "DEBLOCKING"},

        // type index
        {{.t_index = PREF_SUBSENC}, "SUBSENC"},
        {{.t_index = PREF_CURRENT_VIEW}, "CURRENT_VIEW"},

        // type bool
        {{.t_bool = PREF_FRAME_SKIP}, "FRAME_SKIP"},
        {{.t_bool = PREF_AUDIO_STRETCH}, "AUDIO_STRETCH"},
        {{.t_bool = PREF_DIRECTORIES_INTERNAL}, "DIRECTORIES_INTERNAL"},
        {{.t_bool = PREF_DIRECTORIES_EXTERNAL}, "DIRECTORIES_EXTERNAL"},
        {{.t_bool = PREF_DEVELOPER_VERBOSE}, "PREF_DEVELOPER_VERBOSE"},
        {{0}}
};

const char *
preferences_get_key(pref_id key_id)
{
    for (int i = 0; mapping[i].id.t_int != 0; i++)
    {
        if (mapping[i].id.t_int == key_id.t_int)
            return mapping[i].key;
    }
    return NULL;
}

void
preferences_set_enum(pref_enum key, menu_id value)
{
    pref_id key_id;
    key_id.t_enum = key;

    preference_set_int(preferences_get_key(key_id), value);
}

void
preferences_set_index(pref_index key, int value)
{
    pref_id key_id;
    key_id.t_index = key;

    preference_set_int(preferences_get_key(key_id), value);
}

void
preferences_set_bool(pref_bool key, bool value)
{
    pref_id key_id;
    key_id.t_bool = key;

    preference_set_boolean(preferences_get_key(key_id), value);
}

menu_id
preferences_get_enum(pref_enum key, menu_id default_value)
{
    pref_id key_id;
    key_id.t_enum = key;

    int value;
    if (preference_get_int(preferences_get_key(key_id), &value) != PREFERENCE_ERROR_NONE)
        return default_value;

    return (menu_id)value;
}

int
preferences_get_index(pref_index key, int default_value)
{
    pref_id key_id;
    key_id.t_index = key;

    int value;
    if (preference_get_int(preferences_get_key(key_id), &value) != PREFERENCE_ERROR_NONE)
        return default_value;

    return value;
}

bool
preferences_get_bool(pref_bool key, bool default_value)
{
    pref_id key_id;
    key_id.t_bool = key;

    bool value;
    if (preference_get_boolean(preferences_get_key(key_id), &value) != PREFERENCE_ERROR_NONE)
        return default_value;

    return value;
}

char *
preferences_get_libvlc_options()
{
    char *buf = calloc(512, sizeof(char));
    if (buf == NULL)
        return NULL;

    if (preferences_get_bool(PREF_AUDIO_STRETCH, false))
        strcat(buf, "--audio-time-stretch ");
    else
        strcat(buf, "--no-audio-time-stretch ");


    strcat(buf, "--avcodec-skiploopfilter ");
    menu_id deblocking = preferences_get_enum(PREF_DEBLOCKING, DEBLOCKING_AUTOMATIC);
    switch (deblocking)
    {
    case DEBLOCKING_FULL:
        strcat(buf, "1 ");
        break;
    case DEBLOCKING_MEDIUM:
        strcat(buf, "2 ");
        break;
    case DEBLOCKING_LOW:
        strcat(buf, "3 ");
        break;
    case DEBLOCKING_NO:
        strcat(buf, "4 ");
        break;
    case DEBLOCKING_AUTOMATIC:
    default:
        //TODO add auto-detection based on device performances
        strcat(buf, "3 ");
        break;
    }

    if (preferences_get_bool(PREF_FRAME_SKIP, false))
        strcat(buf, "--avcodec-skip-frame 2 --avcodec-skip-idct 2 ");
    else
        strcat(buf, "--avcodec-skip-frame 0 --avcodec-skip-idct 0 ");

    strcat(buf, "--subsdec-encoding system "); //TODO find a way to pass the value
    strcat(buf, "--stats ");
    if (preferences_get_bool(PREF_DEVELOPER_VERBOSE, false))
        strcat(buf, "-vvv");
    else
        strcat(buf, "-v");

    return buf;
}
