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

#ifndef TEST_U_IMAGESCREEN_GTK_CALLBACKS_H
#define TEST_U_IMAGESCREEN_GTK_CALLBACKS_H

#include "gtk/gtk.h"

//------------------------------------------------------------------------------

// idle routines
gboolean idle_initialize_UI_cb (gpointer);
gboolean idle_finalize_UI_cb (gpointer);
//gboolean idle_update_info_display_cb (gpointer);
//gboolean idle_update_log_display_cb (gpointer);
gboolean idle_update_progress_cb (gpointer);
gboolean idle_update_video_display_cb (gpointer);
gboolean idle_session_end_cb (gpointer);

//------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
G_MODULE_EXPORT gint button_about_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gint button_quit_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gboolean dialog_main_key_press_event_cb (GtkWidget*, GdkEventKey*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_configure_event_cb (GtkWindow*, GdkEvent*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_expose_cb (GtkWidget*, GdkEvent*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_key_press_event_cb (GtkWidget*, GdkEventKey*, gpointer);
G_MODULE_EXPORT void filechooserbutton_current_folder_changed_cb (GtkFileChooser*, gpointer);
//G_MODULE_EXPORT void filechooserdialog_cb (GtkFileChooser*, gpointer);
G_MODULE_EXPORT gboolean key_cb (GtkWidget*, GdkEventKey*, gpointer);
G_MODULE_EXPORT void togglebutton_start_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void togglebutton_fullscreen_toggled_cb (GtkToggleButton*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
