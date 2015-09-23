#include "common.h"

#include "../interface.h"
#include "views/about_view.h"


static Evas_Object*
create_about_section(Evas_Object *nf_toolbar)
{
    // Add set set the box in the toolbar naviframe
    Evas_Object *box = elm_box_add(nf_toolbar);
    elm_box_horizontal_set(box, EINA_FALSE);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    //Add and set the label in the box
    Evas_Object *lbl_about_title = elm_label_add(box);
    evas_object_size_hint_align_set(lbl_about_title, EVAS_HINT_FILL, 0.5);
    elm_object_text_set(lbl_about_title, "<align=center><b>Building VLC for Tizen</b></align>");
    elm_box_pack_end(box, lbl_about_title);

    /* */
    evas_object_show(lbl_about_title);

    return box;
}

static Evas_Object*
create_licence_section(Evas_Object *nf_toolbar)
{
    /* Add set set the box in the toolbar naviframe */
    Evas_Object *box = elm_box_add(nf_toolbar);
    elm_box_horizontal_set(box, EINA_FALSE);
    evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Add and set the label in the box */
    Evas_Object *lbl_license_title = elm_label_add(box);
    evas_object_size_hint_align_set(lbl_license_title, EVAS_HINT_FILL, 0.5);
    elm_object_text_set(lbl_license_title, "<align=center><b>VLC License for Tizen</b></align>");
    elm_box_pack_end(box, lbl_license_title);  //Add the label in the box

    /* */
    evas_object_show(lbl_license_title);

    return box;
}

static void
tabbar_item_selected(Evas_Object *nf_toolbar, Elm_Object_Item *about_it)
{
    const char *str = NULL;
    Evas_Object *current_audio_view;
    /* Get the item selected in the toolbar */
    str = elm_object_item_text_get(about_it);

    /* Create the view depending on the item selected in the toolbar */
    if (str && !strcmp(str, "License")) {
        current_audio_view = create_licence_section(nf_toolbar);
    }
    else     {
        current_audio_view = create_about_section(nf_toolbar);
    }

    elm_object_content_set(nf_toolbar, current_audio_view ); //Add the created view in the toolbar naviframe
}

static void
tabbar_item_cb(void *data, Evas_Object *obj, void *event_info)
{
    Evas_Object *nf_toolbar = data;
    Elm_Object_Item *about_it = event_info;

    /* Call the function that creates the views */
    tabbar_item_selected(nf_toolbar, about_it);
}

static Evas_Object*
create_toolbar(Evas_Object *nf_toolbar)
{
    /* Create and set the toolbar */
    Evas_Object *tabbar = elm_toolbar_add(nf_toolbar);
    elm_object_style_set(tabbar, "tabbar");

    /* Set the toolbar shrink mode to NONE */
    elm_toolbar_shrink_mode_set(tabbar, ELM_TOOLBAR_SHRINK_NONE);
    /* Expand the content to fill the toolbar */
    elm_toolbar_transverse_expanded_set(tabbar, EINA_TRUE);
    /* Items will only call their selection func and callback when first becoming selected*/
    elm_toolbar_select_mode_set(tabbar, ELM_OBJECT_SELECT_MODE_DEFAULT);

    /* Append new entry in the toolbar with the Icon & Label wanted */
    elm_toolbar_item_append(tabbar, ICON_DIR "ic_menu_goto.png",
            "About", tabbar_item_cb, nf_toolbar);
    elm_toolbar_item_append(tabbar, ICON_DIR "ic_menu_cone.png",
            "License", tabbar_item_cb, nf_toolbar);

    return tabbar;
}

Evas_Object *
create_about_view(Evas_Object *parent)
{
    Elm_Object_Item *nf_it, *tabbar_it;
    Evas_Object *nf_toolbar,*tabbar;

    /* Toolbar Naviframe */
    nf_toolbar = elm_naviframe_add(parent);
    evas_object_size_hint_weight_set(nf_toolbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(nf_toolbar, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Toolbar Naviframe Settings */
    elm_object_part_content_set(parent, "elm.swallow.content", nf_toolbar);
    nf_it = elm_naviframe_item_push(nf_toolbar, NULL, NULL, NULL, NULL, "tabbar/icon/notitle");
    elm_naviframe_item_push(parent, "<b>About</b>", NULL, NULL, nf_toolbar, "basic");
    evas_object_show(nf_toolbar);

    /* Create the toolbar in the view */
    tabbar = create_toolbar(nf_toolbar);
    elm_object_item_part_content_set(nf_it, "tabbar", tabbar);

    /* Set the first item in the toolbar */
    tabbar_it = elm_toolbar_first_item_get(tabbar);
    elm_toolbar_item_selected_set(tabbar_it, EINA_TRUE);

    return nf_toolbar;
}
