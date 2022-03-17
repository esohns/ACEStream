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

#include "stream_dev_mic_source_alsa.h"

#include "stream_dev_defines.h"

const char libacestream_default_dev_mic_source_alsa_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_ALSA_DEFAULT_NAME_STRING);

void
stream_dev_mic_source_alsa_async_callback (snd_async_handler_t* handler_in)
{
  // sanity check(s)
  ACE_ASSERT (handler_in);
  struct Stream_Device_ALSA_Capture_AsynchCBData* data_p =
    static_cast<struct Stream_Device_ALSA_Capture_AsynchCBData*> (snd_async_handler_get_callback_private (handler_in));
  ACE_ASSERT (data_p);
  ACE_ASSERT (data_p->allocatorConfiguration);
  ACE_ASSERT (data_p->queue);
  ACE_ASSERT (data_p->statistic);
  snd_pcm_t* handle_p = snd_async_handler_get_pcm (handler_in);
  ACE_ASSERT (handle_p);

  snd_pcm_sframes_t available_frames, frames_read = 0;
  int result = -1;
  ACE_Message_Block* message_block_p = NULL;
  snd_pcm_uframes_t frames_to_read;

  do
  {
    if (!message_block_p)
    {
      if (likely (data_p->allocator))
      {
        try {
          message_block_p =
            static_cast<ACE_Message_Block*> (data_p->allocator->malloc (data_p->allocatorConfiguration->defaultBufferSize));
        } catch (...) {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                      ACE_TEXT (snd_pcm_name (handle_p)),
                      data_p->allocatorConfiguration->defaultBufferSize));
          message_block_p = NULL;
        }
      } // end IF
      else
        ACE_NEW_NORETURN (message_block_p,
                          ACE_Message_Block (data_p->allocatorConfiguration->defaultBufferSize));
      if (unlikely (!message_block_p))
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate memory, aborting\n"),
                    ACE_TEXT (snd_pcm_name (handle_p))));
        goto error;
      } // end IF
    } // end IF

    available_frames = snd_pcm_avail_update (handle_p);
    if (unlikely (available_frames < 0))
    {
      // overrun ? --> recover
      if (available_frames == -EPIPE)
        goto recover;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_avail_update(): \"%s\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p)),
                  ACE_TEXT (snd_strerror (result))));
      goto error;
    } // end IF
    if (unlikely (available_frames == 0))
      break;

    frames_to_read = message_block_p->space () / data_p->frameSize;
    frames_to_read =
      (frames_to_read > static_cast<snd_pcm_uframes_t> (available_frames) ? available_frames
                                                                            : frames_to_read);
    frames_read = snd_pcm_readi (handle_p,
                                 message_block_p->wr_ptr (),
                                 frames_to_read);
    if (unlikely (frames_read < 0))
    {
      // overrun ? --> recover
      if (frames_read == -EPIPE)
        goto recover;
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to snd_pcm_readi(): \"%s\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p)),
                  ACE_TEXT (snd_strerror (frames_read))));
      goto error;
    } // end IF
    message_block_p->wr_ptr (static_cast<unsigned int> (frames_read) * data_p->frameSize);
    data_p->statistic->capturedFrames += frames_read;

    result = data_p->queue->enqueue_tail (message_block_p,
                                          NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to ACE_Message_Queue_Base::enqueue_tail(): \"%m\", returning\n"),
                  ACE_TEXT (snd_pcm_name (handle_p))));
      goto error;
    } // end IF
    message_block_p = NULL;

    continue;

recover:
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("%s: buffer overrun, recovering\n"),
                ACE_TEXT (snd_pcm_name (handle_p))));

    //        result = snd_pcm_prepare (handle_p);
    result = snd_pcm_recover (handle_p,
                              -EPIPE,
                              1); // silent
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
  if (message_block_p)
    message_block_p->release ();
}
