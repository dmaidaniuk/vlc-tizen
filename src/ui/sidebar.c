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

#include <Elementary.h>

#include "sidebar.h"

#include "interface.h"
#include "audio_player.h"
#include "ui/utils.h"

typedef struct menu_cb_data
{
    interface *intf;

    int index;
    const Elm_Genlist_Item_Class *itc;
} menu_cb_data_s;

typedef struct {
    const char* label;
    const char* icon_name;
} menu_entry;
static const menu_entry menu_entries[] = {
        { "Video",      "ic_menu_video.png" },
        { "Audio",      "ic_menu_audio.png" },
        { "Directory",  "ic_menu_folder.png" },
        { "Settings",   "ic_menu_preferences.png" },
        { "About",      "ic_menu_cone.png" }
};

static char *
gl_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
    menu_cb_data_s *cd = data;
    /* Check the item class style and put the current folder or file name as a string */
    /* Then put this string as the genlist item label */
    if (cd->itc->item_style && !strcmp(cd->itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.text.main.left")) {
            return strdup(menu_entries[cd->index].label);
        }
    }
    return NULL;
}

static Evas_Object*
gl_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    menu_cb_data_s *cd = data;
    Evas_Object *content = NULL;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (cd->itc->item_style && !strcmp(cd->itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.icon.1")) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
            Evas_Object *icon = create_icon(content, menu_entries[cd->index].icon_name);
            elm_layout_content_set(content, "elm.swallow.content", icon);
        }
    }

    return content;
}

void
gl_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    menu_cb_data_s *cd = data;
    /* Generate the view depending on which sidebar genlist item is selected */

    intf_update_mini_player(cd->intf);

    switch(cd->index){

    case VIEW_VIDEO:
        intf_show_view(cd->intf, VIEW_VIDEO);
        break;

    case VIEW_AUDIO:
        intf_show_view(cd->intf, VIEW_AUDIO);
        break;

    case VIEW_FILES:
        intf_show_view(cd->intf, VIEW_FILES);
        break;

    case VIEW_SETTINGS:
        intf_show_view(cd->intf, VIEW_SETTINGS);
        break;

    case VIEW_ABOUT:
        intf_show_view(cd->intf, VIEW_ABOUT);
        break;

        free(cd);
    }
}

static Evas_Object *
create_panel_genlist(interface *intf, Evas_Object *sidebar)
{
    /* Set then create the Genlist object */
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "1line";
    itc->func.text_get = gl_text_get_cb;
    itc->func.content_get = gl_content_get_cb;

    /* */
    Evas_Object *genlist = elm_genlist_add(sidebar);

    /* Set the genlist scoller mode */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    /* Enable the genlist HOMOGENEOUS mode */
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    /* Enable the genlist COMPRESS mode */
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

    /* */
    for (int index = 0; index < VIEW_MAX; index++) {

        menu_cb_data_s *cd = malloc(sizeof(*cd));

        elm_genlist_item_append(genlist,
                itc,                            /* item class               */
                cd,                             /* item class user data     */
                NULL,                           /* parent item              */
                ELM_GENLIST_ITEM_NONE,          /* item type                */
                gl_selected_cb,                 /* select smart callback    */
                cd);                            /* smart callback user data */

        /* Put the index and the gui_data in the cb_data struct for callbacks */
        cd->index = index;
        cd->intf = intf;
        cd->itc = itc;
    }

    elm_genlist_item_class_free(itc);
    return genlist;
}

static void
list_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *sidebar = data;
    /* Disable the sidebar when one of the item list is selected */
    if (!elm_object_disabled_get(sidebar))
        elm_panel_toggle(sidebar);
}

Evas_Object*
create_sidebar(interface *intf, Evas_Object *layout)
{
    Evas_Object *sidebar_list;
    Evas_Object *sidebar;

    /* Create then set the sidebar */
    sidebar = elm_panel_add(layout);
    elm_panel_scrollable_set(sidebar, EINA_TRUE);
    elm_panel_hidden_set(sidebar, EINA_TRUE);
    elm_panel_orient_set(sidebar, ELM_PANEL_ORIENT_LEFT);

    /* Add the sidebar genlist in the sidebar */
    sidebar_list = create_panel_genlist(intf, sidebar);
    evas_object_show(sidebar_list);
    evas_object_size_hint_weight_set(sidebar_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(sidebar_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* */
    evas_object_smart_callback_add(sidebar_list, "selected", list_clicked_cb, sidebar);

    /* */
    elm_object_content_set(sidebar, sidebar_list);
    elm_genlist_item_selected_set(elm_genlist_first_item_get(sidebar), EINA_TRUE);

    return sidebar;
}
