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

#ifndef TEST_I_CAMSAVE_CALLBACKS_H
#define TEST_I_CAMSAVE_CALLBACKS_H

#include "gtk/gtk.h"

//// forward declarations
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
////struct IAMStreamConfig;
//#endif

//// helper functions
//bool load_capture_devices (GtkListStore*);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
////bool load_formats (IAMStreamConfig*, // filter handle
////bool load_formats (IMFSourceReader*, // source reader handle
//bool load_formats (IMFMediaSource*, // source handle
//                   GtkListStore*);   // return value: supported media subtypes
////bool load_resolutions (IAMStreamConfig*, // stream config handle
////bool load_resolutions (IMFSourceReader*,    // source reader handle
//bool load_resolutions (IMFMediaSource*,     // source handle
//                       const struct _GUID&, // media subtype
//                       GtkListStore*);      // return value: supported resolutions
////bool load_rates (IAMStreamConfig*, // stream config handle
////bool load_rates (IMFSourceReader*,    // source reader handle
//bool load_rates (IMFMediaSource*,     // source handle
//                 const struct _GUID&, // media subtype
//                 unsigned int,        // resolution (width)
//                 GtkListStore*);      // return value: supported rates
//#else
//int dirent_selector (const dirent*);
//int dirent_comparator (const dirent**,
//                       const dirent**);
//bool load_formats (int,            // (capture) device file descriptor
//                   GtkListStore*); // return value: supported formats (fourcc)
//bool load_resolutions (int,            // (capture) device file descriptor
//                       __u32,          // format (fourcc)
//                       GtkListStore*); // return value: supported resolutions
//bool load_rates (int,            // (capture) device file descriptor
//                 __u32,          // format (fourcc)
//                 unsigned int,   // resolution (width)
//                 unsigned int,   // resolution (height)
//                 GtkListStore*); // return value: supported rates
//#endif
//
//// thread functions
//ACE_THR_FUNC_RETURN stream_processing_function (void*);

//------------------------------------------------------------------------------

// idle routines
gboolean idle_initialize_UI_cb (gpointer);
gboolean idle_finalize_UI_cb (gpointer);
gboolean idle_session_end_cb (gpointer);
gboolean idle_update_info_display_cb (gpointer);
//gboolean idle_update_log_display_cb (gpointer);
gboolean idle_update_progress_cb (gpointer);
gboolean idle_update_video_display_cb (gpointer);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
#if GTK_CHECK_VERSION(3,0,0)
G_MODULE_EXPORT void toggleaction_record_toggled_cb (GtkToggleAction*, gpointer);
G_MODULE_EXPORT void action_snapshot_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void action_cut_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void action_report_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void action_reset_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void action_hw_settings_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void toggleaction_save_toggled_cb (GtkToggleAction*, gpointer);
//G_MODULE_EXPORT void action_settings_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void toggleaction_fullscreen_toggled_cb (GtkToggleAction*, gpointer);
#elif GTK_CHECK_VERSION(2,0,0)
G_MODULE_EXPORT void togglebutton_record_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void button_snapshot_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void button_cut_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void button_report_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void button_format_reset_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void button_hw_settings_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void togglebutton_save_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void togglebutton_display_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void togglebutton_fullscreen_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void button_display_reset_clicked_cb (GtkButton*, gpointer);
#endif // GTK_CHECK_VERSION

//G_MODULE_EXPORT gint button_clear_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_about_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_quit_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_source_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_format_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_resolution_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_rate_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gboolean dialog_main_key_press_event_cb (GtkWidget*, GdkEventKey*, gpointer);
//G_MODULE_EXPORT void drawingarea_configure_event_cb (GtkWindow*, GdkEvent*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_draw_cb (GtkWidget*, cairo_t*, gpointer);
G_MODULE_EXPORT void drawingarea_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_key_press_event_cb (GtkWidget*, GdkEventKey*, gpointer);
G_MODULE_EXPORT gboolean key_cb (GtkWidget*, GdkEventKey*, gpointer);
G_MODULE_EXPORT void filechooserbutton_cb (GtkFileChooserButton*, gpointer);
//G_MODULE_EXPORT void filechooserdialog_cb (GtkFileChooser*, gpointer);
//G_MODULE_EXPORT void textview_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif