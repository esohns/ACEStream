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

gboolean
acestream_visualization_gtk_cairo_idle_update_cb (gpointer userData_in)
{
  STREAM_TRACE (ACE_TEXT ("::acestream_visualization_gtk_cairo_idle_update_cb"));

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
    return G_SOURCE_REMOVE;
  }

  return G_SOURCE_CONTINUE;
}
