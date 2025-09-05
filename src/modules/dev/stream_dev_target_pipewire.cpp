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

#include "stream_dev_target_pipewire.h"

#include "stream_dev_defines.h"

const char libacestream_default_dev_target_pipewire_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_TARGET_PIPEWIRE_DEFAULT_NAME_STRING);

void
acestream_dev_target_pw_on_stream_param_changed_cb (void* userData_in,
                                                    uint32_t id_in,
                                                    const struct spa_pod* parameters_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_dev_target_pw_on_stream_param_changed_cb"));

  // sanity check(s)
  struct Stream_Device_Pipewire_Playback_CBData* cb_data_p =
      static_cast<struct Stream_Device_Pipewire_Playback_CBData*> (userData_in);
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
              ACE_TEXT ("playing back %d channel(s) @ %uHz\n"),
              cb_data_p->format.info.raw.channels,
              cb_data_p->format.info.raw.rate));
}

void
acestream_dev_target_pw_on_process_cb (void* userData_in)
{
  // STREAM_TRACE (ACE_TEXT ("acestream_dev_target_pw_on_process_cb"));

  // sanity check(s)
  struct Stream_Device_Pipewire_Playback_CBData* cb_data_p =
      static_cast<struct Stream_Device_Pipewire_Playback_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->stream);

  struct pw_buffer* pw_buffer_p;
  struct spa_buffer* spa_buffer_p;
  uint8_t* samples_p;
  uint32_t index_i = 0;
  int result;
  uint32_t available_frames_i, available_frames_2, frames_to_copy_i, bytes_to_copy_i;

  pw_buffer_p = pw_stream_dequeue_buffer (cb_data_p->stream);
  if (unlikely (!pw_buffer_p))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to pw_stream_dequeue_buffer(): %m, returning\n")));
    return;
  } // end IF
  spa_buffer_p = pw_buffer_p->buffer;
  ACE_ASSERT (spa_buffer_p);
  samples_p = static_cast<uint8_t*> (spa_buffer_p->datas[0].data);
  if (unlikely (!samples_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("no space in buffer, returning\n")));
    goto continue_;
  } // end IF

  available_frames_i =
    spa_buffer_p->datas[0].maxsize / cb_data_p->frameSize;
  if (pw_buffer_p->requested)
    available_frames_i =
        std::min (static_cast<uint32_t> (pw_buffer_p->requested), available_frames_i);
  available_frames_2 = available_frames_i;
  while (available_frames_2)
  {
    if (!cb_data_p->buffer)
    {
      result = cb_data_p->queue->dequeue (cb_data_p->buffer,
                                          NULL);
      if (unlikely (result == -1))
      {
        int error = ACE_OS::last_error ();
        if (unlikely (error != ESHUTDOWN))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_Message_Queue_Base::dequeue(): \"%m\", returning\n")));
        available_frames_i -= available_frames_2;
        goto continue_;
      } // end IF
    } // end IF
    ACE_ASSERT (cb_data_p->buffer);

    frames_to_copy_i = cb_data_p->buffer->length () / cb_data_p->frameSize;
    frames_to_copy_i = std::min (frames_to_copy_i, available_frames_2);
    bytes_to_copy_i = cb_data_p->frameSize * frames_to_copy_i;

    ACE_OS::memcpy (&samples_p[index_i],
                    cb_data_p->buffer->rd_ptr (),
                    bytes_to_copy_i);
    index_i += bytes_to_copy_i;
    available_frames_2 -= frames_to_copy_i;
    cb_data_p->buffer->rd_ptr (bytes_to_copy_i);

    if (!cb_data_p->buffer->length ())
    {
      cb_data_p->buffer->release (); cb_data_p->buffer = NULL;
    } // end IF
  } // end WHILE

continue_:
  spa_buffer_p->datas[0].chunk->offset = 0;
  spa_buffer_p->datas[0].chunk->stride = cb_data_p->frameSize;
  spa_buffer_p->datas[0].chunk->size =
      available_frames_i * cb_data_p->frameSize;

  result = pw_stream_queue_buffer (cb_data_p->stream,
                                   pw_buffer_p);
  ACE_ASSERT (result == 0);
}
