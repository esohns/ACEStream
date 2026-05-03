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

#include "stream_vis_target_win32_base.h"

LRESULT CALLBACK
libacestream_vis_target_win32_base_window_proc_cb (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  //STREAM_TRACE (ACE_TEXT ("libacestream_vis_target_win32_base_window_proc_cb"));

  // sanity check(s)
  struct libacestream_vis_target_win32_base_window_proc_cb_data* cb_data_p =
    (struct libacestream_vis_target_win32_base_window_proc_cb_data*)GetWindowLongPtr (hWnd, GWLP_USERDATA);
  //ACE_ASSERT (cb_data_p); // available only after WM_CREATE (see below)

  BOOL bRet;

  switch (message) 
  {
    case WM_CREATE:
    {
      CREATESTRUCT* create_p = (CREATESTRUCT*)lParam;
      ACE_ASSERT (create_p);
      SetWindowLongPtr (hWnd, GWLP_USERDATA, (LONG_PTR)create_p->lpCreateParams);

      break;
    }
    case WM_CLOSE:
    { // *NOTE*: WM_CLOSE is normally NOT seen by GetMessage()...
      static bool is_first_b = true;
      if (is_first_b)
      { is_first_b = false;
        bRet = PostMessage (hWnd, message, wParam, lParam);
        ACE_ASSERT (bRet);
      } // end IF
      break;
    }
    case WM_DESTROY:
    {
      PostQuitMessage (0);
      break;
    }
    case WM_SIZE:
    {
      LONG width = LOWORD (lParam);
      LONG height = HIWORD (lParam);

      static struct tagSIZE previous_size_s = {0, 0};
      if (previous_size_s.cx == width &&
          previous_size_s.cy == height)
        return DefWindowProc (hWnd, message, wParam, lParam);
      previous_size_s = {width, height};

      bRet = PostMessage (hWnd, message, wParam, lParam);
      ACE_ASSERT (bRet);

      return DefWindowProc (hWnd, message, wParam, lParam);
    }
    case WM_SIZING:
    {
      struct tagRECT* rect_p = reinterpret_cast<struct tagRECT*> (lParam);
      ACE_ASSERT (rect_p);
      //if ((rect_p->right - rect_p->left) < 300)
      //{
      //  if (wParam == WMSZ_LEFT || wParam == WMSZ_TOPLEFT || wParam == WMSZ_BOTTOMLEFT)
      //    rect_p->left = rect_p->right - 300;
      //  else
      //    rect_p->right = rect_p->left + 300;
      //} // end IF
      //if ((rect_p->bottom - rect_p->top) < 200)
      //{
      //  if (wParam == WMSZ_TOP || wParam == WMSZ_TOPLEFT || wParam == WMSZ_TOPRIGHT)
      //    rect_p->top = rect_p->bottom - 200;
      //  else
      //    rect_p->bottom = rect_p->top + 200;
      //} // end IF

      //return TRUE;
      return DefWindowProc (hWnd, message, wParam, lParam);
    }
    default:
      return DefWindowProc (hWnd, message, wParam, lParam);
  }

  return 0;
}
