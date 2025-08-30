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

#include "stream_dev_mic_source_pipewire.h"

#include "stream_dev_defines.h"

const char libacestream_default_dev_mic_source_pipewire_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_MIC_SOURCE_PIPEWIRE_DEFAULT_NAME_STRING);

void
acestream_dev_mic_pw_on_stream_param_changed_cb (void* userData_in,
                                                 uint32_t id_in,
                                                 const struct spa_pod* parameters_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_dev_mic_pw_on_stream_param_changed_cb"));

  // sanity check(s)
  struct Stream_Device_Pipewire_Capture_CBData* cb_data_p =
      static_cast<struct Stream_Device_Pipewire_Capture_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  /* NULL means to clear the format */
  if (parameters_in == NULL || id_in != SPA_PARAM_Format)
    return;
  int result = spa_format_parse (parameters_in,
                                 &cb_data_p->format.media_type,
                                 &cb_data_p->format.media_subtype);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to spa_format_parse(%@), returning\n"),
                parameters_in));
    return;
  } // end IF
  /* only accept raw audio */
  if (unlikely (cb_data_p->format.media_type != SPA_MEDIA_TYPE_audio ||
                cb_data_p->format.media_subtype != SPA_MEDIA_SUBTYPE_raw))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid media (sub-)type (was: %u|%u), returning\n"),
                cb_data_p->format.media_type,
                cb_data_p->format.media_subtype));
    return;
  } // end IF
  result = spa_format_audio_raw_parse (parameters_in,
                                       &cb_data_p->format.info.raw);
  ACE_ASSERT (result >= 0);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("capturing %d channel(s) @ %uHz\n"),
              cb_data_p->format.info.raw.channels,
              cb_data_p->format.info.raw.rate));
}

void
acestream_dev_mic_pw_on_process_cb (void* userData_in)
{
  // STREAM_TRACE (ACE_TEXT ("acestream_dev_mic_pw_on_process_cb"));

  // sanity check(s)
  struct Stream_Device_Pipewire_Capture_CBData* cb_data_p =
      static_cast<struct Stream_Device_Pipewire_Capture_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->allocator);
  ACE_ASSERT (cb_data_p->allocatorConfiguration);
  ACE_ASSERT (cb_data_p->statistic);
  ACE_ASSERT (cb_data_p->stream);

  struct pw_buffer* pw_buffer_p;
  struct spa_buffer* spa_buffer_p;
  float* samples_p, max;
  uint32_t c, n, n_channels, n_samples, peak;
  ACE_Message_Block* message_block_p = NULL;
  uint32_t index_i = 0;
  int result;
  uint32_t available_samples_i, frames_to_copy_i;

  pw_buffer_p = pw_stream_dequeue_buffer (cb_data_p->stream);
  if (unlikely (!pw_buffer_p))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to pw_stream_dequeue_buffer(): %m, returning\n")));
    return;
  } // end IF
  spa_buffer_p = pw_buffer_p->buffer;
  ACE_ASSERT (spa_buffer_p);
  samples_p = static_cast<float*> (spa_buffer_p->datas[0].data);
  if (unlikely (!samples_p))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("no sample data, returning\n")));
    goto continue_;
  } // end IF

  n_channels = cb_data_p->format.info.raw.channels;
  n_samples = spa_buffer_p->datas[0].chunk->size / sizeof (float);

  /* move cursor up */
  fprintf (stdout, "%c[%dA", 0x1b, n_channels + 1);
  fprintf (stdout, "captured %d samples\n", n_samples / n_channels);
  for (c = 0; c < n_channels; c++)
  {
    max = 0.0f;
    for (n = c; n < n_samples; n += n_channels)
      max = fmaxf(max, fabsf(samples_p[n]));

    peak = (uint32_t)SPA_CLAMPF(max * 30, 0.f, 39.f);

    fprintf (stdout, "channel %d: |%*s%*s| peak:%f\n", c, peak+1, "*", 40 - peak, "", max);
  } // end FOR
  fflush (stdout);

  available_samples_i = n_samples;
  while (available_samples_i)
  {
    if (likely (cb_data_p->allocator))
    {
      try {
        message_block_p =
          static_cast<ACE_Message_Block*> (cb_data_p->allocator->malloc (cb_data_p->allocatorConfiguration->defaultBufferSize));
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Stream_IAllocator::malloc(%u), continuing\n"),
                    cb_data_p->allocatorConfiguration->defaultBufferSize));
        message_block_p = NULL;
      }
    } // end IF
    else
      ACE_NEW_NORETURN (message_block_p,
                        ACE_Message_Block (cb_data_p->allocatorConfiguration->defaultBufferSize));
    if (unlikely (!message_block_p))
    {
      ACE_DEBUG ((LM_CRITICAL,
                  ACE_TEXT ("failed to allocate memory, aborting\n")));
      goto continue_;
    } // end IF

    frames_to_copy_i = message_block_p->space () / cb_data_p->frameSize;
    frames_to_copy_i = std::min (frames_to_copy_i, available_samples_i);
    result = message_block_p->copy (reinterpret_cast<char*> (&samples_p[index_i]),
                                    cb_data_p->frameSize * frames_to_copy_i);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));
      message_block_p->release ();
      goto continue_;
    } // end IF
    index_i += cb_data_p->frameSize * frames_to_copy_i;
    available_samples_i -= frames_to_copy_i;
    cb_data_p->statistic->capturedFrames += frames_to_copy_i;

    result = cb_data_p->queue->enqueue_tail (message_block_p,
                                             NULL);
    if (unlikely (result == -1))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_Message_Queue_Base::enqueue_tail(): \"%m\", returning\n")));
      message_block_p->release ();
      goto continue_;
    } // end IF
    message_block_p = NULL;
  } // end WHILE

continue_:
  result = pw_stream_queue_buffer (cb_data_p->stream,
                                   pw_buffer_p);
  ACE_ASSERT (result == 0);
}
