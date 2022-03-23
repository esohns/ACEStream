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

#include "stream_dev_cam_source_vfw.h"

#include "stream_dev_defines.h"

const char libacestream_default_dev_cam_source_vfw_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_VIDEOFORWINDOW_DEFAULT_NAME_STRING);

LRESULT CALLBACK
acestream_vfw_error_cb (HWND window_in,
                        int errorId_in,
                        LPTSTR text_in)
{
  ACE_UNUSED_ARG (window_in);

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("vfw capture error (id: %d): \"%s\"\n"),
              errorId_in,
              ACE_TEXT (text_in)));

  return (LRESULT)TRUE; // continue
}

LRESULT CALLBACK
acestream_vfw_status_cb (HWND window_in,
                         int messageId_in,
                         LPTSTR text_in)
{
  ACE_UNUSED_ARG (window_in);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("vfw capture status (id: %d): \"%s\"\n"),
              messageId_in,
              ACE_TEXT (text_in)));

  return (LRESULT)TRUE; // continue
}

LRESULT CALLBACK
acestream_vfw_control_cb (HWND window_in,
                          int state_in)
{
  ACE_UNUSED_ARG (window_in);

  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("vfw capture control (state: %d)\n"),
  //            state_in));

  return (LRESULT)TRUE; // continue
}

LRESULT CALLBACK
acestream_vfw_video_cb (HWND window_in,
                        LPVIDEOHDR videoHeader_in)
{
  // sanity check(s)
  struct acestream_vfw_cbdata* cb_data_p =
    //(struct acestream_vfw_cbdata*)capGetUserData (window_in);
    (struct acestream_vfw_cbdata*)GetWindowLongPtr (window_in, GWLP_USERDATA);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->allocator);
  ACE_ASSERT (cb_data_p->queue);
  ACE_ASSERT (cb_data_p->sessionId);
  ACE_ASSERT (videoHeader_in);

  ACE_Message_Block* message_block_p =
    static_cast<ACE_Message_Block*> (cb_data_p->allocator->malloc (videoHeader_in->dwBytesUsed));
  ACE_ASSERT (message_block_p);

  int result =
    message_block_p->copy (reinterpret_cast<char*> (videoHeader_in->lpData),
                           static_cast<size_t> (videoHeader_in->dwBytesUsed));
  ACE_ASSERT (result == 0);

  result = cb_data_p->queue->enqueue (message_block_p,
                                      NULL);
  ACE_ASSERT (result != -1);

  return (LRESULT)TRUE; // continue
}
