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
    /**
     * Content Management
     */
    application*    p_app;
    view_sys*     p_view;
    // This is the content as a video_list_item list.
    Eina_List*      p_content;

    /**
     * Callbacks & settings
     */
    video_list_item*    (*pf_view_append_media_item)( view_sys* p_view, media_item* p_item );
    void                (*pf_view_clear)( view_sys* videoview );
    const media_item*   (*pf_get_media_item)( video_list_item* p_view );
    void                (*pf_set_media_item)( video_list_item* p_view, media_item* p_item );
    void                (*pf_media_library_get_content)( media_library* p_ml, media_library_list_cb cb, void* p_user_data );

};

static void
video_controller_add_item(video_controller* ctrl, media_item* p_item)
{
    video_list_item* p_view_item = ctrl->pf_view_append_media_item( ctrl->p_view, p_item );
    if (p_view_item == NULL)
        return;
    ctrl->p_content = eina_list_append(ctrl->p_content, p_view_item);
}

bool
video_controller_file_update( video_controller* ctrl, const media_item* p_media_item )
{
    if ( p_media_item->i_type != MEDIA_ITEM_TYPE_VIDEO )
        return false;

    media_item* p_new_media_item = media_item_copy( p_media_item );
    if (p_new_media_item == NULL)
        return true;

    if ( ctrl->p_content != NULL )
    {
        Eina_List* it;
        video_list_item* p_item;
        EINA_LIST_FOREACH( ctrl->p_content, it, p_item )
        {
            const media_item* p_media_item = ctrl->pf_get_media_item(p_item);

            if (!strcmp( p_media_item->psz_path, p_new_media_item->psz_path))
            {
                ctrl->pf_set_media_item(p_item, p_new_media_item);
                return true;
            }
        }
    }
    video_controller_add_item( ctrl, p_new_media_item );
    return true;
}

static bool
video_controller_file_updated_cb(void* p_data, const media_item* p_new_media_item, bool b_added )
{
    (void)b_added;
    video_controller* ctrl = (video_controller*)p_data;
    return video_controller_file_update(ctrl, p_new_media_item);
}

/* Called by the Media Library with updated video list
 * Guaranteed to be called from the main loop
 */
void
video_controller_content_update_cb(Eina_List* p_content, void* p_data)
{
    if (p_content == NULL)
        return;
    video_controller* ctrl = (video_controller*)p_data;
    Eina_List* it;
    media_item* p_item;

    EINA_LIST_FOREACH( p_content, it, p_item )
    {
        video_controller_file_update(ctrl, p_item);
    }
}

/*
 * Called when media library signals a content change (currently, only after reload)
 * Guaranteed to be called from the main loop
 */
void
video_controller_content_changed_cb(void* p_data)
{
    video_controller* ctrl = (video_controller*)p_data;

    // Discard previous content if any, and ask ML for the new content
    if (ctrl->p_content != NULL)
    {
        eina_list_free(ctrl->p_content);
        ctrl->pf_view_clear(ctrl->p_view);
        ctrl->p_content = NULL;
    }
    media_library* p_ml = (media_library*)application_get_media_library( ctrl->p_app );
    ctrl->pf_media_library_get_content(p_ml, &video_controller_content_update_cb, ctrl);
}

video_controller*
video_controller_create(application* p_app, view_sys* p_view )
{
   video_controller* ctrl = calloc(1, sizeof(*ctrl));
   if ( ctrl == NULL )
       return NULL;
   ctrl->p_app = p_app;
   ctrl->p_view = p_view;
   ctrl->pf_view_append_media_item = &video_view_append_item;
   ctrl->pf_get_media_item = &video_list_item_get_media_item;
   ctrl->pf_set_media_item = &video_list_item_set_media_item;
   ctrl->pf_media_library_get_content = &media_library_get_video_files;
   ctrl->pf_view_clear = &video_view_clear;

   /* Populate it */
   media_library* p_ml = (media_library*)application_get_media_library(p_app);
   media_library_register_on_change(p_ml, video_controller_content_changed_cb, ctrl);
   media_library_register_item_updated(p_ml, video_controller_file_updated_cb, ctrl);
   return ctrl;
}

void
video_controller_destroy(video_controller *ctrl)
{
    eina_list_free(ctrl->p_content);
    media_library* p_ml = (media_library*)application_get_media_library(ctrl->p_app);
    media_library_unregister_on_change(p_ml, &video_controller_content_changed_cb, ctrl);
    media_library_unregister_item_updated(p_ml, &video_controller_file_updated_cb, ctrl);
    free(ctrl);
}
