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
libacestream_wave_in_data_cb (HWAVEIN   hwi,
                              UINT      uMsg,
                              DWORD_PTR dwInstance,
                              DWORD_PTR dwParam1,
                              DWORD_PTR dwParam2)
{
  // sanity check(s)
  struct libacestream_wave_in_cbdata* cb_data_p =
    reinterpret_cast<struct libacestream_wave_in_cbdata*> (dwInstance);
  ACE_ASSERT (cb_data_p);

  struct wavehdr_tag* wavehdr_p = NULL;
  switch (uMsg)
  {
    case WIM_CLOSE:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("closed capture device...\n"),
                  cb_data_p->task->mod_->name ()));
      return;
    }
    case WIM_DATA:
    {
      wavehdr_p = reinterpret_cast<struct wavehdr_tag*> (dwParam1);
      break;
    }
    case WIM_OPEN:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: opened capture device...\n"),
                  cb_data_p->task->mod_->name ()));
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
  ACE_ASSERT (wavehdr_p->dwUser < STREAM_DEV_MIC_WAVEIN_DEFAULT_DEVICE_BUFFERS);
  //cb_data_p->buffers[wavehdr_p->dwUser]->reset ();
  // generate sinus ?
  if (unlikely (cb_data_p->sinus))
    Stream_Module_Decoder_Tools::sinus (*cb_data_p->frequency,
                                        cb_data_p->sampleRate,
                                        cb_data_p->sampleSize,
                                        cb_data_p->channels,
                                        reinterpret_cast<uint8_t*> (cb_data_p->buffers[wavehdr_p->dwUser]->rd_ptr ()),
                                        (wavehdr_p->dwBytesRecorded / cb_data_p->sampleSize),
                                        cb_data_p->phase);
  cb_data_p->buffers[wavehdr_p->dwUser]->wr_ptr (wavehdr_p->dwBytesRecorded);
  ACE_ASSERT (cb_data_p->task);
  int result =
    cb_data_p->task->put_next (cb_data_p->buffers[wavehdr_p->dwUser], NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", continuing\n"),
                cb_data_p->task->mod_->name ()));
  } // end IF
}
