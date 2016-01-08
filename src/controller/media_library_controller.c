/*****************************************************************************
 * Copyright © 2015-2016 VideoLAN, VideoLabs SAS
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
#include "media_library_controller.h"

#include "application.h"
#include "media/library/media_library.hpp"
#include "ui/views/video_view.h"
#include "ui/interface.h"

#include <assert.h>

#include "media_library_controller_private.h"

static void
media_library_controller_add_item(media_library_controller* ctrl, media_item* p_item)
{
    void* p_view_item = ctrl->p_list_view->pf_append_item( ctrl->p_list_view->p_sys, p_item );
    if (p_view_item == NULL)
        return;
    ctrl->p_content = eina_list_append(ctrl->p_content, p_view_item);
}

bool
media_library_controller_file_update( media_library_controller* ctrl, const library_item* p_library_item )
{
    if ( ctrl->pf_accept_item( p_library_item ) == false )
        return false;

    void* p_new_library_item = ctrl->pf_item_duplicate( p_library_item );
    if (p_new_library_item == NULL)
        return true;

    if ( ctrl->p_content != NULL )
    {
        Eina_List* it;
        void* p_item;
        EINA_LIST_FOREACH( ctrl->p_content, it, p_item )
        {
            const void* p_media_item = ctrl->p_list_view->pf_get_item(p_item);

            if ( ctrl->pf_item_compare( p_media_item, p_new_library_item) )
            {
                ctrl->p_list_view->pf_set_item(p_item, p_new_library_item);
                return true;
            }
        }
    }
    media_library_controller_add_item( ctrl, p_new_library_item );
    return true;
}

static bool
media_library_controller_file_updated_cb(void* p_data, const library_item* p_new_media_item, bool b_added )
{
    (void)b_added;
    media_library_controller* ctrl = (media_library_controller*)p_data;
    return media_library_controller_file_update(ctrl, p_new_media_item);
}

/* Called by the Media Library with updated video list
 * Guaranteed to be called from the main loop
 */
void
media_library_controller_content_update_cb(Eina_List* p_content, void* p_data)
{
    if (p_content == NULL)
        return;
    media_library_controller* ctrl = (media_library_controller*)p_data;
    Eina_List* it;
    void* p_item;

    EINA_LIST_FOREACH( p_content, it, p_item )
    {
        media_library_controller_file_update(ctrl, p_item);
    }
}

/*
 * Called when media library signals a content change (currently, only after reload)
 * Guaranteed to be called from the main loop
 */
void
media_library_controller_content_changed_cb(void* p_data)
{
    media_library_controller* ctrl = (media_library_controller*)p_data;

    // Discard previous content if any, and ask ML for the new content
    if (ctrl->p_content != NULL)
    {
        eina_list_free(ctrl->p_content);
        ctrl->p_list_view->pf_clear(ctrl->p_list_view->p_sys);
        ctrl->p_content = NULL;
    }
    media_library* p_ml = (media_library*)application_get_media_library( ctrl->p_app );
    ctrl->pf_media_library_get_content(p_ml, &media_library_controller_content_update_cb, ctrl->p_user_data);
}

void
media_library_controller_refresh(media_library_controller* p_ctrl)
{
    ecore_main_loop_thread_safe_call_async(&media_library_controller_content_changed_cb, p_ctrl);
}

void
media_library_controller_set_content_callback(media_library_controller* p_ctrl, pf_media_library_get_content_cb cb, void* p_user_data)
{
    p_ctrl->pf_media_library_get_content = cb;
    p_ctrl->p_user_data = p_user_data;
}

media_library_controller*
media_library_controller_create(application* p_app, list_view* p_list_view )
{
   media_library_controller* ctrl = calloc(1, sizeof(*ctrl));
   if ( ctrl == NULL )
       return NULL;
   ctrl->p_app = p_app;
   ctrl->p_list_view = p_list_view;
   /* Default the user data to ourselves. This is when the callbacks are our defaults ones */
   ctrl->p_user_data = ctrl;

   /* Populate it */
   media_library* p_ml = (media_library*)application_get_media_library(p_app);
   media_library_register_on_change(p_ml, media_library_controller_content_changed_cb, ctrl);
   media_library_register_item_updated(p_ml, media_library_controller_file_updated_cb, ctrl);
   return ctrl;
}

void
media_library_controller_destroy(media_library_controller *ctrl)
{
    eina_list_free(ctrl->p_content);
    media_library* p_ml = (media_library*)application_get_media_library(ctrl->p_app);
    media_library_unregister_on_change(p_ml, &media_library_controller_content_changed_cb, ctrl);
    media_library_unregister_item_updated(p_ml, &media_library_controller_file_updated_cb, ctrl);
    free(ctrl);
}
