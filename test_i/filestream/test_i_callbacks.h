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

#ifndef TEST_I_CALLBACKS_H
#define TEST_I_CALLBACKS_H

#include "ace/config-lite.h"

#include <gtk/gtk.h>

// thread functions
ACE_THR_FUNC_RETURN stream_processing_function (void*);

//------------------------------------------------------------------------------

// idle routines
gboolean idle_initialize_source_UI_cb (gpointer);
gboolean idle_end_source_UI_cb (gpointer);
gboolean idle_update_info_display_source_cb (gpointer);
gboolean idle_update_progress_source_cb (gpointer);

//////////////////////////////////////////

gboolean idle_initialize_target_UI_cb (gpointer);
gboolean idle_start_target_UI_cb (gpointer);
gboolean idle_end_target_UI_cb (gpointer);
gboolean idle_reset_target_UI_cb (gpointer);
gboolean idle_update_info_display_target_cb (gpointer);
gboolean idle_update_progress_target_cb (gpointer);

//////////////////////////////////////////

gboolean idle_finalize_UI_cb (gpointer);

gboolean idle_update_log_display_cb (gpointer);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
// callbacks
G_MODULE_EXPORT void action_stop_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void checkbutton_loop_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void filechooserbutton_source_cb (GtkFileChooserButton*, gpointer);
G_MODULE_EXPORT void toggle_action_start_toggled_cb (GtkToggleAction*, gpointer);

//////////////////////////////////////////

G_MODULE_EXPORT void action_close_all_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void action_listen_activate_cb (GtkAction*, gpointer);
G_MODULE_EXPORT void filechooserbutton_target_cb (GtkFileChooserButton*, gpointer);
G_MODULE_EXPORT void filechooser_target_cb (GtkFileChooser*, gpointer);

//////////////////////////////////////////

G_MODULE_EXPORT void action_report_activate_cb (GtkAction*, gpointer);
//G_MODULE_EXPORT void radiobutton_protocol_toggled_cb (GtkToggleButton*, gpointer);
//G_MODULE_EXPORT void spinbutton_port_value_changed_cb (GtkWidget*, gpointer);

//////////////////////////////////////////

G_MODULE_EXPORT gint button_clear_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_about_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_quit_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void textview_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);

//////////////////////////////////////////

G_MODULE_EXPORT void filechooserdialog_cb (GtkFileChooser*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
