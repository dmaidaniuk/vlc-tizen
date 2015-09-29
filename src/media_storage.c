/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Nicolas Rechatin [nicolas@videolabs.io]
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
#include "media_storage.h"

#include <storage.h>

struct media_storage {
    int i_internal_storage_id;
    char *psz_paths[MEDIA_DIRECTORY_MAX];
};

static bool storage_cb(int storage_id, storage_type_e type, storage_state_e state, const char *path, void *user_data)
{
    media_storage *p_ms = user_data;
    if (type == STORAGE_TYPE_INTERNAL)
    {
        p_ms->i_internal_storage_id = storage_id;
        LOGD("Storage refreshed");
        return false;
    }

    return true;
}

media_storage *
media_storage_create(application *p_app)
{
    media_storage *p_ms = calloc(1, sizeof(media_storage));
    if (!p_ms)
        return NULL;

    /* Connect to the device storage */
    if (storage_foreach_device_supported(storage_cb, p_ms))
    {
        free(p_ms);
        return NULL;
    }
    return p_ms;
}

void
media_storage_destroy(media_storage *p_ms)
{
    for (unsigned int i = 0; i < MEDIA_DIRECTORY_MAX; ++i)
        free(p_ms->psz_paths[i]);
    free(p_ms);
}

const char*
media_storage_get_path(media_storage *p_ms, media_directory_e type)
{
    int error, storage_type;

    if (type < 0 || type >= MEDIA_DIRECTORY_MAX)
        type = MEDIA_DIRECTORY;

    /* Return precomputed values, it should not change at runtime */
    if (p_ms->psz_paths[type])
        return p_ms->psz_paths[type];

    switch(type)
    {
    case MEDIA_DIRECTORY:
    case MEDIA_DIRECTORY_VIDEOS:
        storage_type = STORAGE_DIRECTORY_VIDEOS;
        break;
    case MEDIA_DIRECTORY_MUSIC:
        storage_type = STORAGE_DIRECTORY_MUSIC;
        break;
    case MEDIA_DIRECTORY_CAMERA:
        storage_type = STORAGE_DIRECTORY_CAMERA;
        break;
    default:
        LOGW("Storage error, unknown type");
        return NULL;
    }

    /* */
    char *directory;
    error = storage_get_directory(p_ms->i_internal_storage_id, storage_type, &directory);

    if (error != STORAGE_ERROR_NONE)
    {
        LOGD("Failed storage");
        return NULL;
    }

    /* Special case for Directory view */
    if (type == MEDIA_DIRECTORY) {
        char *device_storage_path;
        /* Concatenate the media path with .. to access the general media directory */
        asprintf(&device_storage_path,"%s/%s", directory, "..");
        free(directory);
        directory = device_storage_path;
    }

    /* Store, Log out and return */
    p_ms->psz_paths[type] = directory;
    LOGD("Storage type %d: %s", type, directory);
    return directory;
}
