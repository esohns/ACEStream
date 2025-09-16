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

#include "stream_dev_target_alsa.h"

#include "stream_dev_defines.h"

void
stream_dev_target_alsa_async_cb (snd_async_handler_t* handler_in)
{
  // never wait for the queue
  static ACE_Time_Value no_wait_2 = ACE_OS::gettimeofday ();

  // sanity check(s)
  ACE_ASSERT(handler_in);
  struct Stream_Device_ALSA_Playback_AsynchCBData* data_p =
    reinterpret_cast<struct Stream_Device_ALSA_Playback_AsynchCBData*> (snd_async_handler_get_callback_private (handler_in));
  ACE_ASSERT (data_p);
  snd_pcm_t* handle_p = snd_async_handler_get_pcm (handler_in);
  ACE_ASSERT (handle_p);

  snd_pcm_sframes_t available_frames, frames_written = 0;
  int result = -1;
  snd_pcm_uframes_t frames_to_write = 0;
  int error_i = -1;

  do
  {
    available_frames = snd_pcm_avail_update (handle_p);
    if (unlikely (available_frames < 0))
    { error_i = available_frames;
      // underrun ? --> recover
      if (likely ((error_i == -EPIPE) ||
                  (error_i == -ESTRPIPE)))
        goto recover;

      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_avail_update(): \"%s\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p)),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
    if (unlikely (available_frames == 0))
      return;

    if (!data_p->currentBuffer)
    {
      result = data_p->queue->dequeue (data_p->currentBuffer,
                                       &no_wait_2);
      if (unlikely (result == -1))
      {
        int error = ACE_OS::last_error ();
        if (likely (error == EAGAIN))
          return;
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to ACE_Message_Queue_Base::dequeue(): \"%m\", returning\n"),
                    ACE_TEXT (snd_pcm_name (handle_p))));
        goto error;
      } // end IF
    } // end IF
    ACE_ASSERT (data_p->currentBuffer);

    frames_to_write = data_p->currentBuffer->length () / data_p->frameSize;
    frames_to_write =
      (frames_to_write > static_cast<snd_pcm_uframes_t> (available_frames) ? available_frames
                                                                           : frames_to_write);
    frames_written = snd_pcm_writei (handle_p,
                                     data_p->currentBuffer->rd_ptr (),
                                     frames_to_write);
    if (unlikely (frames_written < 0))
    {
      // overrun ? --> recover
      if (likely (frames_written == -EPIPE))
        goto recover;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_writei(): \"%s\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p)),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
    data_p->currentBuffer->rd_ptr (frames_written * data_p->frameSize);

    if (!data_p->currentBuffer->length ())
    {
      data_p->currentBuffer->release (); data_p->currentBuffer = NULL;
    } // end IF

    continue;

recover:
//    ACE_DEBUG ((LM_WARNING,
//                ACE_TEXT ("%s: buffer underrun, recovering\n"),
//                ACE_TEXT (snd_pcm_name (handle_p))));

//        result = snd_pcm_prepare (handle_p);
    result = snd_pcm_recover (handle_p,
                              error_i,
#if defined (_DEBUG)
                              0);
#else
                              1);
#endif // _DEBUG
    if (unlikely (result < 0))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_recover(): \"%s\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p)),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
  } while (true);

error:
  if (data_p->currentBuffer)
  {
    data_p->currentBuffer->release (); data_p->currentBuffer = NULL;
  } // end IF
}

//////////////////////////////////////////

const char libacestream_default_dev_target_alsa_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_ALSA_DEFAULT_NAME_STRING);
