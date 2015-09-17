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
#include <Edje.h>

#include "interface.h"
#include "navigation_menu.h"
#include "audio_player.h"
#include "main_popup_list.h"

#include "views/audio_view.h"
#include "views/video_view.h"
#include "views/directory_view.h"
#include "views/settings_view.h"
#include "views/about_view.h"

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
	gui_data_s *gd = data;
	/* Let window go to hide state. */
	if (!elm_object_disabled_get(gd->panel) && !elm_panel_hidden_get(gd->panel)) {
		elm_panel_toggle(gd->panel);
	} else {
		elm_win_lower(gd->win);
	}
}

static Evas_Object *
create_popup(Evas_Object *parent, gui_data_s *gd)
{
	Evas_Object *popup_list;
	gd->popup = elm_popup_add(parent);
	//elm_object_style_set(gd->popup, style);

	evas_object_show(gd->popup);
	evas_object_size_hint_min_set(gd->popup, 200, 200);
	evas_object_size_hint_max_set(gd->popup, 200, 200);

	/* Add the panel genlist in the panel */
	popup_list = create_popup_genlist(gd);
	elm_object_content_set(gd->popup, popup_list);
	evas_object_show(popup_list);

	/* */
	//evas_object_smart_callback_add(gd->popup, "clicked", cancel_cb, gd);
	/* Callback for the back key */
	eext_object_event_callback_add(gd->popup, EEXT_CALLBACK_LAST, eext_popup_back_cb, NULL);

	return gd->popup;
}

static void
list_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	gui_data_s *gd = data;
	/* Disable the panel when one of the item list is selected */
	if (!elm_object_disabled_get(gd->panel)) elm_panel_toggle(gd->panel);
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
	gui_data_s *gd = data;
	/* Disable the panel when left button is pressed */
	if (!elm_object_disabled_get(gd->panel)) elm_panel_toggle(gd->panel);
}

static void
right_panel_button_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	gui_data_s *gd = data;

	gd->popup = create_popup(gd->content_box,gd);
	evas_object_size_hint_weight_set(gd->popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

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

static int internal_storage_id;
static bool storage_cb(int storage_id, storage_type_e type, storage_state_e state, const char *path, void *user_data)
{
	if (type == STORAGE_TYPE_INTERNAL)
	{
		internal_storage_id = storage_id;
		dlog_print(DLOG_DEBUG, LOG_TAG, "Storage refreshed");
		return false;
	}

	return true;
}

void
fetching_media_path(gui_data_s *gd)
{
	int error;
	char *buff;
	/* Connect to the device storage */
	error = storage_foreach_device_supported(storage_cb, NULL);
	error = storage_get_directory(internal_storage_id, STORAGE_DIRECTORY_VIDEOS, &gd->media_path);
	gd->rmp = gd->media_path ;
	/* Concatenate the media path with .. to acces the general media directory */
	asprintf(&gd->rmp,"%s/%s", gd->rmp, "..");
	/* Then do a realpath to get a "clean" path */
	buff = realpath(gd->rmp, NULL);
	gd->rmp = buff;

	if (error == STORAGE_ERROR_NONE)
	{
		dlog_print(DLOG_DEBUG, LOG_TAG, "Connected to storage");

	}
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
create_view(gui_data_s *gd, int panel)
{
	Evas_Object *content = gd->content;
	Evas_Object *view;
	gd->panel_choice = panel;
	/* Create the view depending on with panel item list is selected */
	switch(panel)
	{
	case VIEW_VIDEO:
	case VIEW_AUTO:
		view = create_video_view(gd->media_path, content);
		break;
	case VIEW_AUDIO:
		view = create_audio_view(gd, content);
		break;
	case VIEW_FILES:
		view = create_directory_view(gd->rmp, content);
		break;
	case VIEW_SETTINGS:
		view = create_setting_view(content);
		break;
	case VIEW_ABOUT:
		view = create_about_view(content);
		break;

	}
	evas_object_show(view);
	/* Push the view in the naviframe with the corresponding header */
	elm_naviframe_item_push(content, get_type_tag(panel), NULL, NULL, view, "basic");

	/* Create then set the panel toggle btn and add his callbacks */
	gd->panel_toggle_btn = create_button(gd->content, "naviframe/drawers", NULL);
	evas_object_smart_callback_add(gd->panel_toggle_btn, "clicked", left_panel_button_clicked_cb, gd);
	elm_object_part_content_set(gd->content, "title_left_btn", gd->panel_toggle_btn);

	/* */
	gd->popup_toggle_btn = create_button(gd->content, "naviframe/drawers", NULL);
	evas_object_smart_callback_add(gd->popup_toggle_btn, "clicked", right_panel_button_clicked_cb, gd);
	elm_object_part_content_set(gd->content, "title_right_btn", gd->popup_toggle_btn);


}

static Evas_Object*
create_panel(Evas_Object *layout, gui_data_s *gd)
{
	Evas_Object *panel_genlist;

	/* Create then set the panel */
	gd->panel = elm_panel_add(layout);
	elm_panel_scrollable_set(gd->panel, EINA_TRUE);
	elm_panel_hidden_set(gd->panel, EINA_TRUE);
	elm_panel_orient_set(gd->panel, ELM_PANEL_ORIENT_LEFT);

	/* Add the panel genlist in the panel */
	panel_genlist = create_panel_genlist(gd);
	evas_object_show(panel_genlist);
	evas_object_size_hint_weight_set(panel_genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(panel_genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* */
	evas_object_smart_callback_add(panel_genlist, "selected", list_clicked_cb, gd);

	/* */
	elm_object_content_set(gd->panel, panel_genlist);

	return gd->panel;
}

static Evas_Object*
create_main_content(gui_data_s *gd, Evas_Object *parent)
{
	/* Create a content box to display the content and the mini player */
	gd->content_box = elm_box_add(parent);
	elm_box_horizontal_set(gd->content_box, EINA_FALSE);

	/* Create both of the content_box subObjects */
	gd->mini_player = mini_player_create(gd, gd->content_box);
	gd->content = elm_naviframe_add(gd->content_box);

	/* Put the naviframe at the top of the content_box */
	evas_object_size_hint_align_set(gd->content, EVAS_HINT_FILL, 0.0);
	evas_object_size_hint_weight_set(gd->content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* Put the mini player at the bottom of the content_box */
	/* Then set the vertical offset of the player */
	evas_object_size_hint_align_set(gd->mini_player->mini_player_box, EVAS_HINT_FILL, 1.0);
	evas_object_size_hint_min_set(gd->mini_player->mini_player_box, 0, 100);

	/* Add the content naviframe in the content_box */
	elm_box_pack_end(gd->content_box, gd->content);
	/* */
	evas_object_show(gd->content);

	/* Ask the box to recalculate her current children dislay */
	elm_box_recalculate(gd->content_box);


	return gd->content_box;
}

static Evas_Object*
create_main_view(gui_data_s *gd)
{
	Evas_Object *layout;

	/* Add a layout to the conformant */
	layout = create_base_layout(gd->conform);

	/* Create the panel and put it in the layout */
	gd->panel = create_panel(layout, gd);
	elm_object_part_content_set(layout, "elm.swallow.left", gd->panel);

	/* Create the content box and put it in the layout */
	gd->content_box = create_main_content(gd, layout);
	elm_object_part_content_set(layout, "elm.swallow.content", gd->content_box);
	/* */
	evas_object_size_hint_weight_set(gd->content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(gd->content, EVAS_HINT_FILL, EVAS_HINT_FILL);
	/* */
	evas_object_show(gd->content_box);

	return layout;
}

static void
create_base_gui(gui_data_s *gd)
{
	Evas_Object *bg, *base_layout;

	/* Add and set the main Window */
	gd->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_autodel_set(gd->win, EINA_TRUE);

	/* Handle rotations */
	if (elm_win_wm_rotation_supported_get(gd->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(gd->win, (const int *)(&rots), 4);
	}

	/* Handle callbacks */
	evas_object_smart_callback_add(gd->win, "delete,request", win_delete_request_cb, NULL);
	eext_object_event_callback_add(gd->win, EEXT_CALLBACK_BACK, win_back_cb, gd);

	/* Add and set a conformant in the main Window */
	gd->conform = elm_conformant_add(gd->win);
	elm_win_conformant_set(gd->win, EINA_TRUE);
	evas_object_size_hint_weight_set(gd->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	/* */
	elm_win_indicator_mode_set(gd->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(gd->win, ELM_WIN_INDICATOR_OPAQUE);
	elm_win_resize_object_add(gd->win, gd->conform);
	evas_object_show(gd->conform);

	/* Add and set a bg in the conformant */
	bg = elm_bg_add(gd->conform);
	elm_object_style_set(bg, "indicator/headerbg");
	/* Add the bg in the conformant */
	elm_object_part_content_set(gd->conform, "elm.swallow.indicator_bg", bg);
	evas_object_show(bg);

	/* Create the main view in the conformant */
	base_layout = create_main_view(gd);
	elm_object_content_set(gd->conform, base_layout);

	/* Create the default view in the content naviframe */
	create_view(gd, VIEW_AUTO);

	/* Add both left and right content naviframe buttons */
	gd->panel_toggle_btn = create_button(gd->content, "naviframe/drawers", NULL);
	evas_object_smart_callback_add(gd->panel_toggle_btn, "clicked", left_panel_button_clicked_cb, gd);
	elm_object_part_content_set(gd->content, "title_left_btn", gd->panel_toggle_btn);

	gd->popup_toggle_btn = create_button(gd->content, "naviframe/drawers", NULL);
	evas_object_smart_callback_add(gd->popup_toggle_btn, "clicked", right_panel_button_clicked_cb, gd);
	elm_object_part_content_set(gd->content, "title_right_btn", gd->popup_toggle_btn);

	/* */
	evas_object_show(gd->win);
}

static bool
app_create(void *data)
{
	gui_data_s *gd = data;

	/* Fetching the media path and keep it in memory as soon as the app is create */
	fetching_media_path(gd);
	/* */
	create_base_gui(gd);

	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
app_terminate(void *data)
{
	gui_data_s *gd = data;
	free(gd->mini_player);

}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	gui_data_s gd = {0};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &gd);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &gd);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &gd);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &gd);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &gd);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &gd);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "app_main() is failed. err = %d", ret);
	}

	return ret;
}
