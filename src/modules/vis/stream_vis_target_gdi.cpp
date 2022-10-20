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

#include "stream_vis_target_gdi.h"

#include "stream_vis_defines.h"

// initialize globals
const char libacestream_default_vis_gdi_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_VIS_GDI_DEFAULT_NAME_STRING);

LRESULT CALLBACK
libacestream_gdi_window_proc_cb (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message) 
  {
    case WM_DESTROY:
      PostQuitMessage (0);
      break;
    //case WM_SIZE:
    //{
    //  //struct libacestream_gdi_window_proc_cb_data* data_p =
    //  //  (struct libacestream_gdi_window_proc_cb_data*)GetWindowLongPtr (hWnd, GWLP_USERDATA);
    //  //if (!data_p)
    //  //  return DefWindowProc (hWnd, message, wParam, lParam);
    //  //ACE_ASSERT (data_p->lock);

    //  //ACE_GUARD_RETURN (ACE_Thread_Mutex, aGuard, *data_p->lock, -1);

    //  //ACE_ASSERT (data_p->dc);
    //  //ReleaseDC (hWnd, *data_p->dc); *data_p->dc = NULL;
    //  //*data_p->dc = GetDC (hWnd);
    //  //ACE_ASSERT (*data_p->dc);
    //  //data_p->dc = &*data_p->dc;

    //  break;
    //}
    default:
      /* Call DefWindowProc() as default */
      return DefWindowProc (hWnd, message, wParam, lParam);
  }

  return 0;
}
