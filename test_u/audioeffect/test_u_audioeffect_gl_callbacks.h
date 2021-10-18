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

#ifndef TEST_U_AUDIOEFFECT_GL_CALLBACKS_H
#define TEST_U_AUDIOEFFECT_GL_CALLBACKS_H

#include "gtk/gtk.h"

void processInstructions (struct Test_U_AudioEffect_UI_CBDataBase*);

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
  G_MODULE_EXPORT void glarea_realize_cb (GtkWidget*, gpointer);
#if GTK_CHECK_VERSION(3,0,0)
#if GTK_CHECK_VERSION(3,16,0)
  G_MODULE_EXPORT GdkGLContext* glarea_create_context_cb (GtkGLArea*, gpointer);
  G_MODULE_EXPORT gboolean glarea_render_cb (GtkGLArea*, GdkGLContext*, gpointer);
  G_MODULE_EXPORT void glarea_resize_cb (GtkGLArea*, gint, gint, gpointer);
#else
#if defined (GTKGLAREA_SUPPORT)
  G_MODULE_EXPORT void glarea_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
  G_MODULE_EXPORT gboolean glarea_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
#else
  G_MODULE_EXPORT void glarea_size_allocate_event_cb (GtkWidget*, GdkRectangle*, gpointer);
  G_MODULE_EXPORT gboolean glarea_draw_cb (GtkWidget*, cairo_t*, gpointer);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
  G_MODULE_EXPORT void glarea_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
  G_MODULE_EXPORT gboolean glarea_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
#else
  G_MODULE_EXPORT void glarea_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
  G_MODULE_EXPORT gboolean glarea_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
