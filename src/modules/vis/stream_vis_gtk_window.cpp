/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
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
#include "stdafx.h"

#include "stream_vis_gtk_window.h"

#include "ace/config-macros.h"

#include "stream_vis_defines.h"

const char libacestream_default_vis_gtk_window_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_WINDOW_DEFAULT_NAME_STRING);

//////////////////////////////////////////

#if GTK_CHECK_VERSION (3,0,0)
gboolean
acestream_gtk_window_draw_cb (GtkWidget* widget_in,
                              cairo_t* context_in,
                              gpointer userData_in)
{
  ACE_ASSERT (widget_in);
  ACE_ASSERT (context_in);
  GdkPixbuf* pixbuf_p = static_cast<GdkPixbuf*> (userData_in);
  ACE_ASSERT (pixbuf_p);

  gdk_cairo_set_source_pixbuf (context_in, pixbuf_p, 0.0, 0.0);
  cairo_paint (context_in);

  return TRUE; // do NOT propagate event
}
#else
gboolean
acestream_gtk_window_expose_event_cb (GtkWidget* widget_in,
                                      GdkEvent* event_in,
                                      gpointer userData_in)
{
  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (event_in);
  ACE_UNUSED_ARG (userData_in);

  return TRUE; // do NOT propagate event
}
#endif // GTK_CHECK_VERSION (3,0,0)

void
acestream_gtk_window_destroy_cb (GtkWidget* widget_in,
                                 gpointer userData_in)
{
  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (userData_in);

  // *NOTE*: called in handleSessionMessage ()
  //gtk_main_quit ();
}

gboolean
acestream_gtk_window_delete_event_cb (GtkWidget* widget_in,
                                      GdkEvent* event_in,
                                      gpointer userData_in)
{
  // sanity check(s)
  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (event_in);
  Common_INotify* inotify_p = static_cast<Common_INotify*> (userData_in);
  ACE_ASSERT (inotify_p);

  inotify_p->notify ();

  return TRUE; // do NOT propagate event
}
