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

#include <app.h>
#include <efl_extension.h>
#include <storage.h>
#include <system_settings.h>

#include "interface.h"
#include "sidebar.h"
#include "audio_player.h"
#include "menu/main_popup_list.h"

#include "views/audio_view.h"
#include "views/video_view.h"
#include "views/directory_view.h"
#include "views/settings_view.h"
#include "views/about_view.h"

typedef struct interface_priv_sys {
    Evas_Object *win;
    Evas_Object *conform;

    Evas_Object *content;
    Evas_Object *content_box;

    /* */
    Evas_Object *sidebar;
    int sidebar_idx;
    Evas_Object *sidebar_toggle_btn;

    /* */
    Evas_Object *current_view;

    /* Context popup-menu */
    Evas_Object *popup_toggle_btn;
    Evas_Object *popup;

    Evas_Object *nf_toolbar;
} interface_priv_sys;

/* TODO : A lot of size hints are Hard Coded with pixel values (using a Samsung Z1 phone) */
/* TODO : the use of Dpi or Aspect Ratio will be mandatory in the futur */

/* TODO : Some UI widget design are not perfect. */
/* TODO : Using Edje will probably be mandatory to set some of the widget and get a clean UI */
/* TODO : For examples : Headers & Toolbar Y axis scales */

/* TODO : Use Edje ColorClass to set the general widget colors of the app */
/* TODO :(VLC : #ff8800 / RGBA : 255, 136, 0, 255) */

/* TODO : Remember that the currently used player is the Tizen native one */
/* TODO : VLC haven't be linked to the current UI app */

/* TODO : Managing Hardware Key is mandatory to publish a Tizen AppStore App */
/* TODO : The Menu & Back Key aren't currently set */
/* TODO : This should be solved using the #include <efl_extension.h> */
/* TODO : Then use the void eext_object_event_callback_add func */
/* TODO : See more on https://developer.tizen.org/development/guides/native-application/ui/efl-extension */

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
    ui_app_exit();
}

static void
win_back_cb(void *data, Evas_Object *obj, void *event_info)
{
    interface_sys *intf = data;
    /* Let window go to hide state. */
    if (!elm_object_disabled_get(intf->intf_p->sidebar) && !elm_panel_hidden_get(intf->intf_p->sidebar)) {
        elm_panel_hidden_set(intf->intf_p->sidebar, EINA_TRUE);
    } else if (evas_object_visible_get(intf->intf_p->popup)) {
        evas_object_del(intf->intf_p->popup); //since elm_popup_dismiss doesn't work
    } else {
        elm_win_lower(intf->intf_p->win);
    }
}

static void
list_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
    interface_sys *intf = data;
    /* Disable the panel when one of the item list is selected */
    if (!elm_object_disabled_get(intf->intf_p->sidebar)) elm_panel_toggle(intf->intf_p->sidebar);
}

static Evas_Object *
create_button(Evas_Object *parent, char *style, char *text)
{
    Evas_Object *button;

    button = elm_button_add(parent);
    elm_object_style_set(button, style);

    /* */
    evas_object_show(button);

    return button;
}

static void
left_panel_button_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
    interface_sys *intf = data;
    /* Disable the panel when left button is pressed */
    if (!elm_object_disabled_get(intf->intf_p->sidebar)) elm_panel_toggle(intf->intf_p->sidebar);
}

static void
right_panel_button_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
    interface_sys *intf = data;

    intf->intf_p->popup = create_popup(intf->intf_p->content_box,intf);
    evas_object_size_hint_weight_set(intf->intf_p->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

}

static Evas_Object*
create_base_layout(Evas_Object *parent)
{
    Evas_Object *layout;
    layout = elm_layout_add(parent);

    /* */
    elm_layout_theme_set(layout, "layout", "drawer", "panel");
    evas_object_show(layout);

    return layout;
}


static const char*
get_type_tag(int panel){
    switch(panel)
        {
        case VIEW_AUDIO:
            return "Audio";
        case VIEW_FILES:
            return "Directory";
        case VIEW_SETTINGS:
            return "Settings";
        case VIEW_ABOUT:
            return "About";
        default:
            return "Video";
        }
}

void
create_view(interface_sys *intf, int sidebar_idx)
{
    Evas_Object *content = intf->intf_p->content;
    Evas_Object *view;
    intf->intf_p->sidebar_idx = sidebar_idx;

    /* Create the view depending on with panel item list is selected */
    switch(sidebar_idx)
    {
    case VIEW_VIDEO:
    case VIEW_AUTO:
        view = create_video_view(intf->app->media_path, content);
        break;
    case VIEW_AUDIO:
        intf->intf_p->nf_toolbar = view = create_audio_view(intf, content);
        break;
    case VIEW_FILES:
        view = create_directory_view(intf->app->media_path, content);
        break;
    case VIEW_SETTINGS:
        view = create_setting_view(content);
        break;
    case VIEW_ABOUT:
        view = create_about_view(content);
        break;

    }
    /* Push the view in the naviframe with the corresponding header */
    elm_naviframe_item_push(content, get_type_tag(sidebar_idx), NULL, NULL, view, "basic");

    /* Create then set the panel toggle btn and add his callbacks */
    intf->intf_p->sidebar_toggle_btn = create_button(intf->intf_p->content, "naviframe/drawers", NULL);
    evas_object_smart_callback_add(intf->intf_p->sidebar_toggle_btn, "clicked", left_panel_button_clicked_cb, intf);
    elm_object_part_content_set(intf->intf_p->content, "title_left_btn", intf->intf_p->sidebar_toggle_btn);

    /* */
    intf->intf_p->popup_toggle_btn = create_button(intf->intf_p->content, "naviframe/drawers", NULL);
    evas_object_smart_callback_add(intf->intf_p->popup_toggle_btn, "clicked", right_panel_button_clicked_cb, intf);
    elm_object_part_content_set(intf->intf_p->content, "title_right_btn", intf->intf_p->popup_toggle_btn);


}

static Evas_Object*
create_panel(Evas_Object *layout, interface_sys *intf)
{
    Evas_Object *sidebar_list;

    /* Create then set the panel */
    intf->intf_p->sidebar = elm_panel_add(layout);
    elm_panel_scrollable_set(intf->intf_p->sidebar, EINA_TRUE);
    elm_panel_hidden_set(intf->intf_p->sidebar, EINA_TRUE);
    elm_panel_orient_set(intf->intf_p->sidebar, ELM_PANEL_ORIENT_LEFT);

    /* Add the panel genlist in the panel */
    sidebar_list = create_panel_genlist(intf);
    evas_object_show(sidebar_list);
    evas_object_size_hint_weight_set(sidebar_list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(sidebar_list, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* */
    evas_object_smart_callback_add(sidebar_list, "selected", list_clicked_cb, intf);

    /* */
    elm_object_content_set(intf->intf_p->sidebar, sidebar_list);

    return intf->intf_p->sidebar;
}

static Evas_Object*
create_main_content(interface_sys *intf, Evas_Object *parent)
{
    /* Create a content box to display the content and the mini player */
    intf->intf_p->content_box = elm_box_add(parent);
    elm_box_horizontal_set(intf->intf_p->content_box, EINA_FALSE);

    /* Create both of the content_box subObjects */
    intf->mini_player = mini_player_create(intf, intf->intf_p->content_box);
    intf->intf_p->content = elm_naviframe_add(intf->intf_p->content_box);

    /* Put the naviframe at the top of the content_box */
    evas_object_size_hint_align_set(intf->intf_p->content, EVAS_HINT_FILL, 0.0);
    evas_object_size_hint_weight_set(intf->intf_p->content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

    /* Add the content naviframe in the content_box */
    elm_box_pack_end(intf->intf_p->content_box, intf->intf_p->content);

    /* */
    evas_object_show(intf->intf_p->content);

    /* Ask the box to recalculate her current children dislay */
    elm_box_recalculate(intf->intf_p->content_box);


    return intf->intf_p->content_box;
}

static Evas_Object*
create_main_view(interface_sys *intf)
{
    Evas_Object *layout;

    /* Add a layout to the conformant */
    layout = create_base_layout(intf->intf_p->conform);

    /* Create the panel and put it in the layout */
    intf->intf_p->sidebar = create_panel(layout, intf);
    elm_object_part_content_set(layout, "elm.swallow.left", intf->intf_p->sidebar);

    /* Create the content box and put it in the layout */
    intf->intf_p->content_box = create_main_content(intf, layout);
    elm_object_part_content_set(layout, "elm.swallow.content", intf->intf_p->content_box);
    /* */
    evas_object_size_hint_weight_set(intf->intf_p->content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(intf->intf_p->content, EVAS_HINT_FILL, EVAS_HINT_FILL);
    /* */
    evas_object_show(intf->intf_p->content_box);

    return layout;
}

Evas*
get_window(interface_sys *intf)
{
    return evas_object_evas_get(intf->intf_p->win);
}

Evas_Object*
get_sidebar(interface_sys *intf)
{
    return intf->intf_p->sidebar;
}

Evas_Object *
get_miniplayer_content_box(interface_sys *intf)
{
    return intf->intf_p->content_box;
}

Evas_Object *
get_content(interface_sys *intf)
{
    return intf->intf_p->content;
}

Evas_Object *
get_toolbar(interface_sys *intf)
{
    return intf->intf_p->nf_toolbar;
}

void
show_previous_view(interface_sys *intf)
{
    create_view(intf, intf->intf_p->sidebar_idx);
}


void
create_base_gui(application_sys *app)
{
    interface_sys *intf = calloc(1, sizeof(*intf));
    intf->app = app;

    interface_priv_sys *intf_p = calloc(1, sizeof(*intf_p));
    intf->intf_p = intf_p;


    Evas_Object *bg, *base_layout;

    /* Add and set the main Window */
    intf->intf_p->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
    elm_win_autodel_set(intf->intf_p->win, EINA_TRUE);

    /* Handle rotations */
    if (elm_win_wm_rotation_supported_get(intf->intf_p->win)) {
        int rots[4] = { 0, 90, 180, 270 };
        elm_win_wm_rotation_available_rotations_set(intf->intf_p->win, (const int *)(&rots), 4);
    }

    /* Handle callbacks */
    evas_object_smart_callback_add(intf->intf_p->win, "delete,request", win_delete_request_cb, NULL);
    eext_object_event_callback_add(intf->intf_p->win, EEXT_CALLBACK_BACK, win_back_cb, intf);

    /* Add and set a conformant in the main Window */
    intf->intf_p->conform = elm_conformant_add(intf->intf_p->win);
    elm_win_conformant_set(intf->intf_p->win, EINA_TRUE);
    evas_object_size_hint_weight_set(intf->intf_p->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    /* */
    elm_win_indicator_mode_set(intf->intf_p->win, ELM_WIN_INDICATOR_SHOW);
    elm_win_indicator_opacity_set(intf->intf_p->win, ELM_WIN_INDICATOR_OPAQUE);
    elm_win_resize_object_add(intf->intf_p->win, intf->intf_p->conform);
    evas_object_show(intf->intf_p->conform);

    /* Add and set a bg in the conformant */
    bg = elm_bg_add(intf->intf_p->conform);
    elm_object_style_set(bg, "indicator/headerbg");
    /* Add the bg in the conformant */
    elm_object_part_content_set(intf->intf_p->conform, "elm.swallow.indicator_bg", bg);
    evas_object_show(bg);

    /* Create the main view in the conformant */
    base_layout = create_main_view(intf);
    elm_object_content_set(intf->intf_p->conform, base_layout);

    /* Create the default view in the content naviframe */
    create_view(intf, VIEW_AUTO);

    /* Add both left and right content naviframe buttons */
    intf->intf_p->sidebar_toggle_btn = create_button(intf->intf_p->content, "naviframe/drawers", NULL);
    evas_object_smart_callback_add(intf->intf_p->sidebar_toggle_btn, "clicked", left_panel_button_clicked_cb, intf);
    elm_object_part_content_set(intf->intf_p->content, "title_left_btn", intf->intf_p->sidebar_toggle_btn);

    intf->intf_p->popup_toggle_btn = create_button(intf->intf_p->content, "naviframe/drawers", NULL);
    evas_object_smart_callback_add(intf->intf_p->popup_toggle_btn, "clicked", right_panel_button_clicked_cb, intf);
    elm_object_part_content_set(intf->intf_p->content, "title_right_btn", intf->intf_p->popup_toggle_btn);

    /* */
    evas_object_show(intf->intf_p->win);
}

void
update_mini_player(interface_sys *intf)
{
    if((mini_player_play_state(intf->mini_player) == true) && (mini_player_visibility_state(intf->mini_player) == false))
    {
        mini_player_show(intf->mini_player);
    }

    if((mini_player_play_state(intf->mini_player) == false) && (mini_player_fs_state(intf->mini_player) == true))
    {
        mini_player_stop(intf->mini_player);
    }
}
