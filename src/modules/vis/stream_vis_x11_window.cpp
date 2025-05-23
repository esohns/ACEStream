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

#include "stream_vis_x11_window.h"

#include "common_ui_x11_tools.h"

#include "stream_vis_defines.h"

const char libacestream_default_vis_x11_window_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_X11_WINDOW_DEFAULT_NAME_STRING);

int
libacestream_vis_x11_error_handler_cb (Display* display_in,
                                       XErrorEvent* event_in)
{
//  STREAM_TRACE (ACE_TEXT ("libacestream_vis_x11_error_handler_cb"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("X11 error (display was: %@): \"%s\", returning\n"),
              display_in,
              ACE_TEXT (Common_UI_X11_Tools::toString (*display_in, event_in->error_code).c_str ())));

  return 0;
}

int
libacestream_vis_x11_io_error_handler_cb (Display* display_in)
{
//  STREAM_TRACE (ACE_TEXT ("libacestream_vis_x11_io_error_handler_cb"));

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("X11 I/O error (display was: %@): \"%s\", returning\n"),
              display_in,
              ACE_TEXT (Common_UI_X11_Tools::toString (*display_in, ACE_OS::last_error ()).c_str ())));

  return 0;
}
