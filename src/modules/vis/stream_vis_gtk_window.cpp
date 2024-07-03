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
  //Stream_IStreamControlBase* istream_control_base_p =
  //  static_cast<Stream_IStreamControlBase*> (userData_in);
  //ACE_ASSERT (istream_control_base_p);
  Common_INotify* inotify_p = static_cast<Common_INotify*> (userData_in);
  ACE_ASSERT (inotify_p);

  //istream_control_base_p->stop (false,  // wait for completion ?
  //                              true,   // recurse upstream ?
  //                              false); // high priority ?
  inotify_p->notify ();

  return TRUE; // do NOT propagate event
}
