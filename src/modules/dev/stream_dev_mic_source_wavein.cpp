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

#include "stream_dev_mic_source_wavein.h"

#include "stream_dev_defines.h"

const char libacestream_default_dev_mic_source_wavein_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEIN_CAPTURE_DEFAULT_NAME_STRING);

void CALLBACK
stream_dev_wavein_data_cb (HWAVEIN   hwi,
                           UINT      uMsg,
                           DWORD_PTR dwInstance,
                           DWORD_PTR dwParam1,
                           DWORD_PTR dwParam2)
{
  //STREAM_TRACE (ACE_TEXT ("::stream_dev_wavein_data_cb"));

  // sanity check(s)
  struct stream_dev_wavein_cbdata* cb_data_p =
    reinterpret_cast<struct stream_dev_wavein_cbdata*> (dwInstance);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->task);
  ACE_ASSERT (cb_data_p->task->mod_);

  struct wavehdr_tag* wavehdr_p = NULL;
  int result = -1;

  switch (uMsg)
  {
    case WIM_CLOSE:
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: closed device...\n"),
      //            cb_data_p->task->mod_->name ()));
      return;
    }
    case WIM_DATA:
    {
      wavehdr_p = reinterpret_cast<struct wavehdr_tag*> (dwParam1);
      break;
    }
    case WIM_OPEN:
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("%s: opened device...\n"),
      //            cb_data_p->task->mod_->name ()));
      return;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: invalid/unknown message type (was: %u), returning\n"),
                  cb_data_p->task->mod_->name (),
                  uMsg));
      return;
    }
  } // end SWITCH
  ACE_ASSERT (wavehdr_p);
  ACE_ASSERT (wavehdr_p->dwUser < STREAM_DEV_WAVEIN_DEFAULT_DEVICE_BUFFERS);

  cb_data_p->buffers[wavehdr_p->dwUser]->wr_ptr (wavehdr_p->dwBytesRecorded);

  result = cb_data_p->task->putq (cb_data_p->buffers[wavehdr_p->dwUser],
                                  NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::putq(): \"%m\", returning\n"),
                cb_data_p->task->mod_->name ()));
    return;
  } // end IF

  --cb_data_p->inFlightBuffers;
}
