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

#include "stream_vis_gtk_cairo_spectrum_analyzer.h"

#include "stream_vis_defines.h"

const char libacestream_default_vis_spectrum_analyzer_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GTK_SPECTRUM_ANALYZER_DEFAULT_NAME_STRING);

#if GTK_CHECK_VERSION(3,0,0)
gboolean
acestream_visualization_gtk_cairo_draw_cb (GtkWidget* widget_in,
                                           cairo_t* context_in,
                                           gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::acestream_visualization_gtk_cairo_draw_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_ASSERT (context_in);

  // sanity check(s)
  struct acestream_visualization_gtk_cairo_cbdata* cbdata_p =
    static_cast<struct acestream_visualization_gtk_cairo_cbdata*> (userData_in);
  ACE_ASSERT (cbdata_p);
  if (cbdata_p->context != context_in)
  {
    //cairo_destroy (cbdata_p->context);
    cbdata_p->context = context_in;
  } // end IF
  ACE_ASSERT (cbdata_p->dispatch);

  try {
    cbdata_p->dispatch->dispatch (userData_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), aborting\n")));
    return FALSE;
  }

  return TRUE; // do NOT propagate the event
}
#else
gboolean
acestream_visualization_gtk_cairo_expose_event_cb (GtkWidget* widget_in,
                                                   GdkEvent* event_in,
                                                   gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::acestream_visualization_gtk_cairo_expose_event_cb"));

  ACE_UNUSED_ARG (widget_in);
  ACE_UNUSED_ARG (event_in);

  // sanity check(s)
  struct acestream_visualization_gtk_cairo_cbdata* cbdata_p =
    static_cast<struct acestream_visualization_gtk_cairo_cbdata*> (userData_in);
  ACE_ASSERT (cbdata_p);
  ACE_ASSERT (cbdata_p->dispatch);

  try {
    cbdata_p->dispatch->dispatch (userData_in);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IDispatch::dispatch(), aborting\n")));
    return FALSE;
  }

  return TRUE; // do NOT propagate the event
} // acestream_visualization_gtk_cairo_expose_event_cb

//gboolean
//acestream_visualization_gtk_cairo_configure_event_cb (GtkWidget* widget_in,
//                                                      GdkEvent* event_in,
//                                                      gpointer userData_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::acestream_visualization_gtk_cairo_configure_event_cb"));

//  // sanity check(s)
//  ACE_ASSERT (event_in->configure.type == GDK_CONFIGURE);
//  struct acestream_visualization_gtk_cairo_cbdata* cbdata_p =
//    static_cast<struct acestream_visualization_gtk_cairo_cbdata*> (userData_in);
//  ACE_ASSERT (cbdata_p);
//  ACE_ASSERT (cbdata_p->resizeNotification);

//  GdkWindow* window_p = gtk_widget_get_window (widget_in);
//  if (!window_p)
//    return FALSE; // <-- not realized yet

//  try
//  {
//    cbdata_p->resizeNotification->setP (window_p);
//  } catch (...) {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("caught exception in Common_ISetP_T::setP(), continuing\n")));
//  }

//  return FALSE; // propagate event
//} // acestream_visualization_gtk_cairo_configure_event_cb
#endif // GTK_CHECK_VERSION(3,0,0)
void
acestream_visualization_gtk_cairo_size_allocate_cb (GtkWidget* widget_in,
                                                    GdkRectangle* allocation_in,
                                                    gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::acestream_visualization_gtk_cairo_size_allocate_cb"));

     // sanity check(s)
  struct acestream_visualization_gtk_cairo_cbdata* cbdata_p =
    static_cast<struct acestream_visualization_gtk_cairo_cbdata*> (userData_in);
  ACE_ASSERT (cbdata_p);
  ACE_ASSERT (cbdata_p->resizeNotification);

  GdkWindow* window_p = gtk_widget_get_window (widget_in);
  if (!window_p)
    return; // <-- not realized yet

  try
  {
    cbdata_p->resizeNotification->setP (window_p);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_ISetP_T::setP(), continuing\n")));
  }
} // acestream_visualization_gtk_cairo_size_allocate_cb

gboolean
acestream_visualization_gtk_cairo_idle_update_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::acestream_visualization_gtk_cairo_idle_update_cb"));

  // sanity check(s)
  struct acestream_visualization_gtk_cairo_cbdata* cbdata_p =
    static_cast<struct acestream_visualization_gtk_cairo_cbdata*> (userData_in);
  ACE_ASSERT (cbdata_p);
  ACE_ASSERT (cbdata_p->window);

  gdk_window_invalidate_rect (cbdata_p->window,
                              NULL,   // whole window
                              FALSE); // invalidate children ?

  return G_SOURCE_CONTINUE;
}
