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

#ifndef HTTP_GET_CALLBACKS_H
#define HTTP_GET_CALLBACKS_H

#include "gtk/gtk.h"

#include "ace/config-macros.h"

// thread functions
ACE_THR_FUNC_RETURN stream_processing_function (void*);

//------------------------------------------------------------------------------

gboolean idle_finalize_ui_cb (gpointer);
gboolean idle_initialize_ui_cb (gpointer);
gboolean idle_session_end_cb (gpointer);
gboolean idle_session_start_cb (gpointer);
gboolean idle_update_info_display_cb (gpointer);
//gboolean idle_update_log_display_cb (gpointer);
gboolean idle_update_progress_cb (gpointer);

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
// -----------------------------------------------------------------------------
G_MODULE_EXPORT void button_execute_activate_cb (GtkWidget*, gpointer);
G_MODULE_EXPORT void checkbutton_save_toggled_cb (GtkToggleButton*, gpointer);
G_MODULE_EXPORT void entry_url_delete_text_cb (GtkEditable*,
                                               gint,
                                               gint,
                                               gpointer);
G_MODULE_EXPORT void entry_url_insert_text_cb (GtkEditable*,
                                               gchar*,
                                               gint,
                                               gpointer,
                                               gpointer);
//------------------------------------------------------------------------------

G_MODULE_EXPORT void button_about_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void button_cancel_clicked_cb (GtkButton*, gpointer);
//G_MODULE_EXPORT void button_clear_clicked_cb (GtkButton*, gpointer);
G_MODULE_EXPORT void button_quit_clicked_cb (GtkButton*, gpointer);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
