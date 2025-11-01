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

#ifndef TEST_I_GTK_CALLBACKS_H
#define TEST_I_GTK_CALLBACKS_H

#if defined (GLEW_SUPPORT)
#include "GL/glew.h"
#endif // GLEW_SUPPORT
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "gl/GL.h"
#include "gl/GLU.h"
#else
#include "GL/gl.h"
#include "GL/glu.h"
#endif // ACE_WIN32 || ACE_WIN64

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
G_MODULE_EXPORT void button_about_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void button_clear_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void button_reset_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void button_settings_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void button_quit_clicked_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void combobox_source_changed_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_query_tooltip_cb (GtkWidget*, gint, gint, gboolean, GtkTooltip*, gpointer);
G_MODULE_EXPORT void drawingarea_realize_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void drawingarea_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);
#if GTK_CHECK_VERSION (3,0,0)
G_MODULE_EXPORT gboolean drawingarea_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
G_MODULE_EXPORT gboolean drawingarea_draw_cb (GtkWidget*, cairo_t*, gpointer);
#else
G_MODULE_EXPORT gboolean drawingarea_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
#endif // GTK_CHECK_VERSION(3,0,0)
G_MODULE_EXPORT void filechooserbutton_save_current_folder_changed_cb (GtkFileChooserButton*, gpointer);
G_MODULE_EXPORT gboolean hscale_boost_change_value_cb (GtkRange*, GtkScrollType*, gdouble, gpointer);
G_MODULE_EXPORT void hscale_boost_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void hscale_volume_value_changed_cb (GtkRange*, gpointer);
G_MODULE_EXPORT void filechooserbutton_model_file_set_cb (GtkFileChooserButton*, gpointer);
//G_MODULE_EXPORT void filechooserbutton_scorer_file_set_cb (GtkFileChooserButton*, gpointer);
G_MODULE_EXPORT void radiobutton_2d_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void textview_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);
G_MODULE_EXPORT void togglebutton_3d_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void togglebutton_record_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void togglebutton_save_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void togglebutton_visualization_toggled_cb (GtkToggleButton*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
