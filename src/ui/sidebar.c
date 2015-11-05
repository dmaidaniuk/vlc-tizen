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

typedef struct sidebar {
    Evas_Object *sidebar_panel;
    Evas_Object *sidebar_genlist;
    bool initialized;
} sidebar;

typedef struct menu_cb_data
{
    interface *intf;

    int index;
    const Elm_Genlist_Item_Class *itc;
    Elm_Object_Item *it;
    sidebar *sb;
} menu_cb_data_s;

typedef struct {
    const char* label;
    const char* icon;
    const char* icon_selected;
} menu_entry;
static const menu_entry menu_entries[] = {
        { "Video",      "ic_menu_video.png",        "ic_menu_video_selected.png" },
        { "Audio",      "ic_menu_audio.png",        "ic_menu_audio_selected.png" },
        { "Directory",  "ic_menu_folder.png",       "ic_menu_folder_selected.png" },
        { "Settings",   "ic_menu_preferences.png",  "ic_menu_preferences_selected.png" },
        { "About",      "ic_menu_cone.png",         "ic_menu_cone_selected.png" }
};

static char *
sidebar_text_get_cb(void *data, Evas_Object *obj, const char *part)
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
sidebar_content_get_cb(void *data, Evas_Object *obj, const char *part)
{
    menu_cb_data_s *cd = data;
    Evas_Object *content = NULL;
    const char *icon_str;

    /* Check the item class style and add the object needed in the item class*/
    /* Here, puts the icon in the item class to add it to genlist items */
    if (cd->itc->item_style && !strcmp(cd->itc->item_style, "1line")) {
        if (part && !strcmp(part, "elm.icon.1")) {
            content = elm_layout_add(obj);
            elm_layout_theme_set(content, "layout", "list/B/type.3", "default");

            if (elm_genlist_item_selected_get(cd->it))
                icon_str = menu_entries[cd->index].icon_selected;
            else
                icon_str = menu_entries[cd->index].icon;

            Evas_Object *icon = create_icon(content, icon_str);
            elm_layout_content_set(content, "elm.swallow.content", icon);
        }
    }

    return content;
}

void
sidebar_selected_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    menu_cb_data_s *cd = data;
    /* Generate the view depending on which sidebar genlist item is selected */

    if (!cd->sb->initialized)
    {
        // Don't forward selection until we're ready
        return;
    }

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
    }
}

static void
sidebar_item_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
    menu_cb_data_s *cd = data;
    free(cd);
}

void sidebar_delete_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
    sidebar *sb = data;
    free(sb);
}

static Evas_Object *
sidebar_create_panel_genlist(interface *intf, Evas_Object *parent, sidebar *sb)
{
    /* Set then create the Genlist object */
    Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
    itc->item_style = "1line";
    itc->func.text_get = sidebar_text_get_cb;
    itc->func.content_get = sidebar_content_get_cb;

    /* */
    Evas_Object *genlist = elm_genlist_add(parent);

    /* Set the genlist scoller mode */
    elm_scroller_single_direction_set(genlist, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
    /* Enable the genlist HOMOGENEOUS mode */
    elm_genlist_homogeneous_set(genlist, EINA_TRUE);
    /* Enable the genlist COMPRESS mode */
    elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
    /* Avoid the weird unselect when clicking on the same item twice */
    elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);

    /* */
    for (int index = 0; index < VIEW_MAX; index++) {

        menu_cb_data_s *cd = malloc(sizeof(*cd));

        Elm_Object_Item *it = elm_genlist_item_append(genlist,
                itc,                            /* item class               */
                cd,                             /* item class user data     */
                NULL,                           /* parent item              */
                ELM_GENLIST_ITEM_NONE,          /* item type                */
                sidebar_selected_cb,                 /* select smart callback    */
                cd);                            /* smart callback user data */

        elm_object_item_del_cb_set(it, sidebar_item_delete_cb);

        /* Put the index and the gui_data in the cb_data struct for callbacks */
        cd->index = index;
        cd->intf = intf;
        cd->itc = itc;
        cd->it = it;
        cd->sb = sb;
    }

    elm_genlist_item_class_free(itc);
    return genlist;
}

static void
sidebar_list_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *sidebar_panel = data;
    /* Disable the sidebar when one of the item list is selected */
    if (!elm_object_disabled_get(sidebar_panel))
        elm_panel_toggle(sidebar_panel);
}

void
sidebar_set_selected_view(sidebar *sb, view_e view_type)
{
    elm_genlist_item_selected_set(elm_genlist_nth_item_get(sb->sidebar_genlist, view_type), EINA_TRUE);

    // For an unknown reason, elm_genlist_item_selected_set unhide its parent object
    // so we have to hide it again manually.
    elm_panel_hidden_set(sb->sidebar_panel, EINA_TRUE);
}

Evas_Object*
sidebar_get_widget(sidebar *sb)
{
    return sb->sidebar_panel;
}

sidebar*
create_sidebar(interface *intf, Evas_Object *layout, view_e view_type)
{
    sidebar *sb = calloc(1, sizeof(*sb));

    /* Create then set the sidebar */
    sb->sidebar_panel = elm_panel_add(layout);
    elm_panel_scrollable_set(sb->sidebar_panel, EINA_TRUE);
    elm_panel_hidden_set(sb->sidebar_panel, EINA_TRUE);
    elm_panel_orient_set(sb->sidebar_panel, ELM_PANEL_ORIENT_LEFT);

    /* Add the sidebar genlist in the sidebar */
    sb->sidebar_genlist = sidebar_create_panel_genlist(intf, sb->sidebar_panel, sb);
    evas_object_show(sb->sidebar_genlist);
    evas_object_size_hint_weight_set(sb->sidebar_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(sb->sidebar_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* */
    elm_object_content_set(sb->sidebar_panel, sb->sidebar_genlist);

    /* Select the nth item by default */
    elm_genlist_item_selected_set(elm_genlist_nth_item_get(sb->sidebar_genlist, view_type), EINA_TRUE);
    sb->initialized = true;

    /* */
    evas_object_smart_callback_add(sb->sidebar_genlist, "selected", sidebar_list_clicked_cb, sb->sidebar_panel);
    evas_object_event_callback_add(sb->sidebar_genlist, EVAS_CALLBACK_FREE, sidebar_delete_cb, sb);

    return sb;
}
