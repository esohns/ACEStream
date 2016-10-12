/***************************************************************************
*   Copyright (C) 2010 by Erik Sohns   *
*   erik.sohns@web.de   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/

#ifndef TEST_U_AUDIOEFFECT_CALLBACKS_H
#define TEST_U_AUDIOEFFECT_CALLBACKS_H

#include <ace/config-lite.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfidl.h>
#include <mfreadwrite.h>
#endif

#include <gtk/gtk.h>
#if GTK_CHECK_VERSION (3,0,0)
#include <gtkgl/gdkgl.h>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#include <gtkglarea/gtkglarea.h>
#else
#include <gtkgl/gtkglarea.h>
#endif
#endif

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct IAMStreamConfig;
#endif

// helper functions
bool load_capture_devices (GtkListStore*);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
bool load_formats (IAMStreamConfig*, // filter handle
                   GtkListStore*);   // return value: supported media subtypes
bool load_formats (IMFMediaSource*, // source handle
                   GtkListStore*);   // return value: supported media subtypes
bool load_resolutions (IAMStreamConfig*, // stream config handle
                       const struct _GUID&, // media subtype
                       GtkListStore*);      // return value: supported resolutions
bool load_resolutions (IMFMediaSource*,     // source handle
                       const struct _GUID&, // media subtype
                       GtkListStore*);      // return value: supported resolutions
bool load_rates (IAMStreamConfig*, // stream config handle
                 const struct _GUID&, // media subtype
                 unsigned int,        // resolution (width)
                 GtkListStore*);      // return value: supported rates
bool load_rates (IMFMediaSource*,     // source handle
                 const struct _GUID&, // media subtype
                 unsigned int,        // resolution (width)
                 GtkListStore*);      // return value: supported rates
#else
bool load_formats (int,            // (capture) device file descriptor
                   GtkListStore*); // return value: supported formats (fourcc)
bool load_resolutions (int,            // (capture) device file descriptor
                       __u32,          // format (fourcc)
                       GtkListStore*); // return value: supported resolutions
bool load_rates (int,            // (capture) device file descriptor
                 __u32,          // format (fourcc)
                 unsigned int,   // resolution (width)
                 unsigned int,   // resolution (height)
                 GtkListStore*); // return value: supported rates
#endif

// thread functions
ACE_THR_FUNC_RETURN stream_processing_function (void*);

//------------------------------------------------------------------------------

// idle routines
gboolean idle_initialize_UI_cb (gpointer);
gboolean idle_finalize_UI_cb (gpointer);
gboolean idle_session_end_cb (gpointer);
gboolean idle_update_info_display_cb (gpointer);
gboolean idle_update_display_cb (gpointer);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  // callbacks
  G_MODULE_EXPORT void action_cut_activate_cb (GtkAction*, gpointer);
  G_MODULE_EXPORT void action_report_activate_cb (GtkAction*, gpointer);
  G_MODULE_EXPORT void button_about_clicked_cb (GtkButton*, gpointer);
  G_MODULE_EXPORT void button_quit_clicked_cb (GtkButton*, gpointer);
  G_MODULE_EXPORT void button_reset_clicked_cb (GtkButton*, gpointer);
  G_MODULE_EXPORT void button_settings_clicked_cb (GtkButton*, gpointer);
  G_MODULE_EXPORT void combobox_effect_changed_cb (GtkWidget*, gpointer);
  G_MODULE_EXPORT void combobox_source_changed_cb (GtkWidget*, gpointer);
  G_MODULE_EXPORT void combobox_format_changed_cb (GtkWidget*, gpointer);
  G_MODULE_EXPORT void combobox_frequency_changed_cb (GtkWidget*, gpointer);
  G_MODULE_EXPORT void combobox_resolution_changed_cb (GtkWidget*, gpointer);
  G_MODULE_EXPORT void combobox_channels_changed_cb (GtkWidget*, gpointer);
  G_MODULE_EXPORT gboolean drawingarea_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
  G_MODULE_EXPORT gboolean drawingarea_2d_draw_cb (GtkWidget*, cairo_t*, gpointer);
  G_MODULE_EXPORT gboolean drawingarea_3d_draw_cb (GtkWidget*, cairo_t*, gpointer);
  G_MODULE_EXPORT gboolean drawingarea_tooltip_cb (GtkWidget*, gint, gint, gboolean, GtkTooltip*, gpointer);
  G_MODULE_EXPORT void drawingarea_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);
  G_MODULE_EXPORT void filechooserbutton_cb (GtkFileChooserButton*, gpointer);
  G_MODULE_EXPORT void filechooserdialog_cb (GtkFileChooser*, gpointer);
#if GTK_CHECK_VERSION (3,16,0)
  //G_MODULE_EXPORT GdkGLContext* glarea_create_context_cb (GtkGLArea*, gpointer);
  G_MODULE_EXPORT GdkGLContext* glarea_realize_cb (GtkGLArea*, gpointer);
  G_MODULE_EXPORT gboolean glarea_render_cb (GtkGLArea*, GdkGLContext*, gpointer);
  G_MODULE_EXPORT void glarea_resize_cb (GtkGLArea*, gint, gint, gpointer);
#else
  G_MODULE_EXPORT void glarea_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
  G_MODULE_EXPORT gboolean glarea_draw_cb (GtkWidget*, cairo_t*, gpointer);
  G_MODULE_EXPORT void glarea_realize_cb (GtkWidget*, gpointer);
#endif
  G_MODULE_EXPORT void radiobutton_2d_toggled_cb (GtkToggleButton*, gpointer);
  G_MODULE_EXPORT void scale_sinus_frequency_value_changed_cb (GtkRange*, gpointer);
  G_MODULE_EXPORT void toggleaction_record_toggled_cb (GtkToggleAction*, gpointer);
  G_MODULE_EXPORT void togglebutton_3d_toggled_cb (GtkToggleButton*, gpointer);
  G_MODULE_EXPORT void togglebutton_effect_toggled_cb (GtkToggleButton*, gpointer);
  G_MODULE_EXPORT void togglebutton_mute_toggled_cb (GtkToggleButton*, gpointer);
  G_MODULE_EXPORT void togglebutton_save_toggled_cb (GtkToggleButton*, gpointer);
  G_MODULE_EXPORT void togglebutton_sinus_toggled_cb (GtkToggleButton*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
