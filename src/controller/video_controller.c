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
#include "media/media_library.hpp"
#include "ui/views/video_view.h"
#include "ui/interface.h"

struct video_controller
{
    application*    p_app;
    video_view*     p_view;
    Eina_List*      p_content;
};

/* Called by the Media Library with updated video list
 * Guaranteed to be called from the main loop
 */
void
video_controller_content_update_cb(Eina_List* p_content, void* p_data)
{
    video_controller* ctrl = (video_controller*)p_data;
    ctrl->p_content = p_content;
    video_view_update( ctrl->p_view, p_content );
}

/* Queries the media library for the updated video list */
void
video_controller_content_refresh(video_controller* ctrl)
{
    // If we already have some content, simply send it to the view
    if (ctrl->p_content != NULL)
    {
        video_view_update( ctrl->p_view, ctrl->p_content );
        return;
    }
    // otherwise, update from media library
    const media_library* p_ml = application_get_media_library( ctrl->p_app );
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

video_controller*
video_controller_create( application* p_app, video_view* p_view )
{
   video_controller* ctrl = calloc(1, sizeof(*ctrl));
   if ( ctrl == NULL )
       return NULL;
   ctrl->p_app = p_app;
   ctrl->p_view = p_view;

   /* Populate it */
   media_library* p_ml = application_get_media_library(p_app);
   media_library_register_on_change(p_ml, video_controller_content_changed_cb, ctrl);
   return ctrl;
}
