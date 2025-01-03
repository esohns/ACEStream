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

#ifndef TEST_I_AVSAVE_CALLBACKS_H
#define TEST_I_AVSAVE_CALLBACKS_H

#include "gtk/gtk.h"

//------------------------------------------------------------------------------

// idle routines
gboolean idle_initialize_UI_cb (gpointer);
gboolean idle_finalize_UI_cb (gpointer);
gboolean idle_session_end_cb (gpointer);
gboolean idle_update_display_cb (gpointer);
gboolean idle_update_info_display_cb (gpointer);
gboolean idle_update_progress_cb (gpointer);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
G_MODULE_EXPORT void button_snapshot_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void button_cut_clicked_cb (GtkButton*, gpointer);
//G_MODULE_EXPORT void button_display_reset_clicked_cb (GtkButton*, gpointer);
//G_MODULE_EXPORT void button_format_reset_clicked_cb (GtkButton*, gpointer);
//G_MODULE_EXPORT void button_hw_settings_clicked_cb (GtkButton*, gpointer);
//G_MODULE_EXPORT void button_report_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void togglebutton_display_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void togglebutton_fullscreen_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void togglebutton_record_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void togglebutton_save_toggled_cb (GtkToggleButton*, gpointer);

//G_MODULE_EXPORT gint button_clear_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_about_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_quit_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_video_source_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_video_format_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_video_resolution_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_video_framerate_changed_cb (GtkWidget*, gpointer);
//G_MODULE_EXPORT gboolean dialog_main_key_press_event_cb (GtkWidget*, GdkEventKey*, gpointer);
//G_MODULE_EXPORT void drawingarea_configure_event_cb (GtkWindow*, GdkEvent*, gpointer);
#if GTK_CHECK_VERSION (3,0,0)
G_MODULE_EXPORT gboolean drawingarea_audio_draw_cb (GtkWidget*, cairo_t*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_video_draw_cb (GtkWidget*, cairo_t*, gpointer);
#else
G_MODULE_EXPORT gboolean drawingarea_audio_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_video_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
#endif // GTK_CHECK_VERSION (3,0,0)
G_MODULE_EXPORT void drawingarea_audio_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);
G_MODULE_EXPORT void drawingarea_video_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_audio_key_press_event_cb (GtkWidget*, GdkEventKey*, gpointer);
//G_MODULE_EXPORT gboolean key_cb (GtkWidget*, GdkEventKey*, gpointer);
//G_MODULE_EXPORT void filechooserbutton_save_cb (GtkFileChooserButton*, gpointer);
//G_MODULE_EXPORT void filechooserdialog_cb (GtkFileChooser*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
