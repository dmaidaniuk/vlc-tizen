/*****************************************************************************
 * Copyright © 2015 VideoLAN, VideoLabs SAS
 *****************************************************************************
 *
 * Authors: Hugo Beauzée-Luyssen <hugo@beauzee.fr>
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
#include "video_controller.h"

#include "application.h"
#include "media/library/media_library.hpp"
#include "ui/views/video_view.h"
#include "ui/interface.h"

#include <assert.h>

struct video_controller
{
    application*    p_app;
    view_sys*     p_view;
    // This is the content as a video_list_item list.
    Eina_List*      p_content;
};

/* Called by the Media Library with updated video list
 * Guaranteed to be called from the main loop
 */
void
video_controller_content_update_cb(Eina_List* p_content, void* p_data)
{
    video_controller* ctrl = (video_controller*)p_data;
    if (p_content == NULL)
        return;
    Eina_List* it;
    media_item* p_item;

    assert(ctrl->p_content == NULL);
    EINA_LIST_FOREACH( p_content, it, p_item )
    {
        video_list_item* p_view_item = video_view_append_item( ctrl->p_view, p_item );
        if (p_view_item == NULL)
            continue;
        ctrl->p_content = eina_list_append(ctrl->p_content, p_view_item);
    }
}

/* Queries the media library for the updated video list */
void
video_controller_content_refresh(video_controller* ctrl)
{
    // If we already have some content, consider the view up to date
    if (ctrl->p_content != NULL)
        return;
    // otherwise, update from media library
    media_library* p_ml = (media_library*)application_get_media_library( ctrl->p_app );
    media_library_get_video_files(p_ml, &video_controller_content_update_cb, ctrl);
}

/* Called when media library signals a content change */
void
video_controller_content_changed_cb(void* p_data)
{
    video_controller* ctrl = (video_controller*)p_data;
    // Discard previous content if any, and ask ML for the
    // new content
    if (ctrl->p_content != NULL)
    {
        eina_list_free(ctrl->p_content);
        ctrl->p_content = NULL;
    }
    video_controller_content_refresh(ctrl);
}

static bool
video_controller_file_updated_cb( void* p_data, const media_item* p_new_media_item )
{
    LOGI("Updating: %s", p_new_media_item->psz_path);
    video_controller* ctrl = (video_controller*)p_data;
    if ( ctrl->p_content == NULL )
    {
        LOGI("No content, aborting");
        return false;
    }

    Eina_List* it;
    video_list_item* p_item;

    EINA_LIST_FOREACH( ctrl->p_content, it, p_item )
    {
        const media_item* p_media_item = video_list_item_get_media_item(p_item);

        if (!strcmp( p_media_item->psz_path, p_new_media_item->psz_path))
        {
            video_list_item_set_media_item(p_item, p_new_media_item);
            return true;
        }
    }
    return false;
}

video_controller*
video_controller_create(application* p_app, view_sys* p_view )
{
   video_controller* ctrl = calloc(1, sizeof(*ctrl));
   if ( ctrl == NULL )
       return NULL;
   ctrl->p_app = p_app;
   ctrl->p_view = p_view;

   /* Populate it */
   media_library* p_ml = (media_library*)application_get_media_library(p_app);
   media_library_register_on_change(p_ml, video_controller_content_changed_cb, ctrl);
   media_library_register_item_updated(p_ml, video_controller_file_updated_cb, ctrl);
   return ctrl;
}
