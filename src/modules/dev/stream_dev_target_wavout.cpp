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

#include "stream_dev_target_wavout.h"

#include "stream_dev_defines.h"

const char libacestream_default_dev_target_wavout_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_WAVEOUT_RENDER_DEFAULT_NAME_STRING);

void CALLBACK
stream_dev_waveout_data_cb (HWAVEOUT  hwo,
                            UINT      uMsg,
                            DWORD_PTR dwInstance,
                            DWORD_PTR dwParam1,
                            DWORD_PTR dwParam2)
{
  //STREAM_TRACE (ACE_TEXT ("::stream_dev_waveout_data_cb"));

  // sanity check(s)
  struct stream_dev_waveout_cbdata* cb_data_p =
    reinterpret_cast<struct stream_dev_waveout_cbdata*> (dwInstance);
  ACE_ASSERT (cb_data_p);

  switch (uMsg)
  {
    case WOM_CLOSE:
    case WOM_OPEN:
      break;
    case WOM_DONE:
    {
      // sanity check(s)
      ACE_ASSERT (cb_data_p->queue);
      struct wavehdr_tag* wave_hdr_p =
        reinterpret_cast<struct wavehdr_tag*> (dwParam1);
      ACE_ASSERT (wave_hdr_p);

      // step1: release buffer
      int result = cb_data_p->queue->enqueue (wave_hdr_p,
                                              NULL);
      if (unlikely (result == -1))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to ACE_Message_Queue_Ex::enqueue(): \"%m\", returning\n")));
        return;
      } // end IF

      // step2: update state
      --cb_data_p->inFlightBuffers;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid message type (was: %u), returning\n"),
                  uMsg));
      break;
    }
  } // end SWITCH
}
