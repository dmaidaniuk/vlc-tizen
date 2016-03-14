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
#include "equalizer.h"
#include "playback_service.h"

#include <vlc/vlc.h>

typedef struct band_slider
{
    Evas_Object* p_slider;
    Evas_Object* p_label;
    float f_freq;
} band_slider;

struct equalizer
{
    Evas_Object* p_view;
    Evas_Object* p_layout;
    Evas_Object* p_table;
    Evas_Object* p_sliders_box;
    Evas_Object* p_preset_button;

    unsigned int i_nb_bands;
    band_slider* p_bands;

    Evas_Object* p_preamp_label;
    Evas_Object* p_preamp_slider;

    playback_service* p_ps;
};

static void
equalizer_dismiss_preset_popup(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(obj);
}

static void
equalizer_update_preamp_slider( equalizer* p_eq, float f_value )
{
    char* psz_text;
    if ( asprintf( &psz_text, "Preamp: %1.1f dB", f_value ) < 0 )
        return;
    elm_object_text_set( p_eq->p_preamp_label, psz_text );
    elm_slider_value_set( p_eq->p_preamp_slider, f_value );
    free( psz_text );
}

static void
equalizer_update_slider_label( band_slider* p_band, float f_value )
{
    // ensure we always use positive 0
    if ( f_value >= -0.001f && f_value <= .001f )
        f_value = +.0f;
    char* psz_label;
    if ( asprintf(&psz_label, "%.1f %s: %1.1f dB",
            p_band->f_freq >= 1000.f ? p_band->f_freq / 1000.f : p_band->f_freq,
            p_band->f_freq >= 1000.f ? "kHz" : "Hz",
                    f_value ) < 0 )
        return;
    elm_object_text_set( p_band->p_label, psz_label );
    free( psz_label );
}

static void
equalizer_set_preset( equalizer* p_eq, unsigned int i_preset )
{
    libvlc_equalizer_t* p_vlc_eq = libvlc_audio_equalizer_new_from_preset( i_preset );
    float f_preamp = libvlc_audio_equalizer_get_preamp( p_vlc_eq );
    equalizer_update_preamp_slider( p_eq, f_preamp );
    float f_bands[p_eq->i_nb_bands];
    for ( unsigned int i = 0; i < p_eq->i_nb_bands; ++i )
    {
        f_bands[i] = libvlc_audio_equalizer_get_amp_at_index( p_vlc_eq, i );
        elm_slider_value_set( p_eq->p_bands[i].p_slider, f_bands[i] );
        equalizer_update_slider_label( &p_eq->p_bands[i], f_bands[i] );
    }
    playback_service_eq_set( p_eq->p_ps, f_preamp, p_eq->i_nb_bands, f_bands );
    libvlc_audio_equalizer_release( p_vlc_eq );
}

static void
equalizer_preset_selected(void *data, Evas_Object *obj, void *event_info)
{
    equalizer* p_eq = (equalizer*)data;
    const char* psz_preset = elm_object_item_text_get(event_info);
    unsigned int i_nb_presets = libvlc_audio_equalizer_get_preset_count();
    for ( unsigned int i = 0; i < i_nb_presets; ++i )
    {
        const char* psz_name = libvlc_audio_equalizer_get_preset_name( i );
        if ( strcmp( psz_name, psz_preset ) == 0 )
        {
            equalizer_set_preset( p_eq, i );
            elm_object_text_set(p_eq->p_preset_button, psz_preset);
            break;
        }
    }
    evas_object_del(obj);
}

static void
equalizer_list_presets(void *data, Evas_Object *obj, void *event_info)
{
    equalizer* p_eq = (equalizer*)data;
    Evas_Object* p_parent = p_eq->p_layout;

    Evas_Object* p_popup = elm_ctxpopup_add(p_parent);

    elm_object_style_set(p_popup, "dropdown/list");
    evas_object_smart_callback_add(p_popup, "dismissed", equalizer_dismiss_preset_popup, NULL);
    evas_object_size_hint_weight_set(p_popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p_popup, EVAS_HINT_FILL, EVAS_HINT_FILL);

    //add some items
    unsigned int i_nb_presets = libvlc_audio_equalizer_get_preset_count();
    for ( unsigned int i = 0; i < i_nb_presets; ++i )
    {
        const char* psz_name = libvlc_audio_equalizer_get_preset_name( i );
        elm_ctxpopup_item_append(p_popup, psz_name, NULL, equalizer_preset_selected, p_eq);
    }

    elm_ctxpopup_direction_priority_set(p_popup, ELM_CTXPOPUP_DIRECTION_DOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN,
            ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);

    //change position of the popup
    Evas_Coord x, y;
    evas_pointer_canvas_xy_get(evas_object_evas_get(p_parent), &x, &y);
    evas_object_move(p_popup, x, y);

    evas_object_show(p_popup);
}

static void
equalizer_add_presets(equalizer* p_eq)
{
    Evas_Object* p_label = elm_label_add(p_eq->p_table);
    elm_object_text_set(p_label, "Presets");
    evas_object_size_hint_weight_set(p_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p_label, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(p_label);
    elm_table_pack( p_eq->p_table, p_label, 0, 0, 1, 1 );

    p_eq->p_preset_button = elm_button_add(p_eq->p_table);
    elm_object_style_set(p_eq->p_preset_button, "dropdown");
    evas_object_smart_callback_add(p_eq->p_preset_button, "clicked", equalizer_list_presets, p_eq);
    elm_object_text_set(p_eq->p_preset_button, libvlc_audio_equalizer_get_preset_name(0));
    evas_object_size_hint_weight_set(p_eq->p_preset_button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p_eq->p_preset_button, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(p_eq->p_preset_button);
    elm_table_pack( p_eq->p_table, p_eq->p_preset_button, 1, 0, 1, 1 );
}

static void
equalizer_slider_changed_cb( void *data, Evas_Object *obj, void *event_info )
{
    equalizer* p_eq = (equalizer*)data;
    float f_preamp = elm_slider_value_get( p_eq->p_preamp_slider );
    equalizer_update_preamp_slider( p_eq, f_preamp );
    float f_bands[p_eq->i_nb_bands];
    for ( unsigned int i = 0; i < p_eq->i_nb_bands; ++i )
    {
        f_bands[i] = elm_slider_value_get( p_eq->p_bands[i].p_slider );
        equalizer_update_slider_label( &p_eq->p_bands[i], f_bands[i] );
    }
    playback_service_eq_set( p_eq->p_ps, f_preamp, p_eq->i_nb_bands, f_bands );
}

static void
equalizer_slider_changed_update_label_cb( void *data, Evas_Object *obj, void *event_info )
{
    equalizer_update_slider_label( (band_slider*)data, elm_slider_value_get( obj ) );
}

static void
equalizer_preamp_slider_changed_update_label_cb( void *data, Evas_Object *obj, void *event_info )
{
    equalizer_update_preamp_slider( (equalizer*)data, elm_slider_value_get(obj) );
}

static void
equalizer_init_sliders(equalizer* p_eq)
{
    Evas_Object* p_box = p_eq->p_sliders_box = elm_box_add( p_eq->p_table );
    evas_object_size_hint_weight_set( p_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND );
    evas_object_size_hint_align_set( p_box, EVAS_HINT_FILL, EVAS_HINT_FILL );

    Evas_Object* p_preamp_label = p_eq->p_preamp_label = elm_label_add( p_box );
    evas_object_size_hint_weight_set( p_preamp_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND );
    evas_object_size_hint_align_set( p_preamp_label, EVAS_HINT_FILL, EVAS_HINT_FILL );
    evas_object_show( p_preamp_label );
    elm_box_pack_end( p_box, p_preamp_label );

    Evas_Object* p_preamp_slider = p_eq->p_preamp_slider = elm_slider_add( p_box );
    elm_slider_value_set( p_preamp_slider, 0.0 );
    elm_slider_min_max_set( p_preamp_slider, -20.0, 20.0 );
    evas_object_size_hint_weight_set( p_preamp_slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND );
    evas_object_size_hint_align_set( p_preamp_slider, EVAS_HINT_FILL, EVAS_HINT_FILL );
    evas_object_smart_callback_add( p_preamp_slider, "delay,changed", equalizer_slider_changed_cb, p_eq );
    evas_object_smart_callback_add( p_preamp_slider, "changed", equalizer_preamp_slider_changed_update_label_cb, p_eq );
    evas_object_show( p_preamp_slider );
    elm_box_pack_end( p_box, p_preamp_slider );

    equalizer_update_preamp_slider( p_eq, 0.0f );

    p_eq->i_nb_bands = libvlc_audio_equalizer_get_band_count();
    p_eq->p_bands = malloc( p_eq->i_nb_bands * sizeof( *p_eq->p_bands ) );
    for ( unsigned int i = 0; i < p_eq->i_nb_bands; ++i )
    {
        p_eq->p_bands[i].f_freq = libvlc_audio_equalizer_get_band_frequency( i );

        Evas_Object* p_label = p_eq->p_bands[i].p_label = elm_label_add( p_box );
        evas_object_size_hint_weight_set( p_label, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND );
        evas_object_size_hint_align_set( p_label, EVAS_HINT_FILL, EVAS_HINT_FILL );
        evas_object_show( p_label );
        elm_box_pack_end( p_box, p_label );

        Evas_Object* p_slider = p_eq->p_bands[i].p_slider = elm_slider_add( p_box );
        evas_object_size_hint_weight_set( p_slider, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND );
        evas_object_size_hint_align_set( p_slider, EVAS_HINT_FILL, EVAS_HINT_FILL );
        elm_slider_value_set( p_slider, 0.0 );
        elm_slider_min_max_set(p_slider, -20.0, 20.0);
        evas_object_smart_callback_add( p_slider, "delay,changed", equalizer_slider_changed_cb, p_eq );
        evas_object_smart_callback_add( p_slider, "changed", equalizer_slider_changed_update_label_cb, &p_eq->p_bands[i] );
        evas_object_show( p_slider );
        elm_box_pack_end( p_box, p_slider );

        equalizer_update_slider_label( &p_eq->p_bands[i], 0.0 );
    }
    evas_object_show( p_box );
    elm_table_pack( p_eq->p_table, p_box, 0, 1, 2, 1 );
}

equalizer*
equalizer_create(interface* p_intf, playback_service* p_ps)
{
    equalizer* p_eq = calloc(1, sizeof(*p_eq));
    if ( p_eq == NULL )
        return NULL;

    p_eq->p_ps = p_ps;
    p_eq->p_view = elm_popup_add(intf_get_main_naviframe(p_intf));
    elm_popup_orient_set(p_eq->p_view, ELM_POPUP_ORIENT_CENTER);

    /* */
    Evas_Object* p_layout = p_eq->p_layout = elm_layout_add(p_eq->p_view);
    elm_layout_theme_set(p_layout, "layout", "application", "default");
    evas_object_size_hint_weight_set(p_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(p_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

    /* Create the background */
    Evas_Object *bg = elm_bg_add(p_layout);
    elm_bg_color_set(bg, 255, 255, 255);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(bg, EVAS_HINT_FILL, EVAS_HINT_FILL);
    evas_object_show(bg);

    /* Set the background to the theme */
    elm_object_part_content_set(p_layout, "elm.swallow.bg", bg);

    p_eq->p_table = elm_table_add(p_layout);
    evas_object_show(p_eq->p_table);
    elm_object_part_content_set(p_layout, "elm.swallow.content", p_eq->p_table);

    equalizer_add_presets(p_eq);
    equalizer_init_sliders(p_eq);

    evas_object_show(p_layout);
    elm_object_content_set(p_eq->p_view, p_layout);
    evas_object_show(p_eq->p_view);

    return p_eq;
}

void
equalizer_destroy(equalizer* p_eq)
{
    evas_object_del(p_eq->p_view);
    free(p_eq->p_bands);
    free(p_eq);
}
