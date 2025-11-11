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

#include "stream_dev_cam_source_pipewire.h"

#include "spa/debug/types.h"
#include "spa/param/video/type-info.h"

#include "stream_dev_defines.h"

const char libacestream_default_dev_cam_source_pipewire_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEV_CAM_SOURCE_PIPEWIRE_DEFAULT_NAME_STRING);

void
acestream_dev_cam_pw_on_stream_param_changed_cb (void* userData_in,
                                                 uint32_t id_in,
                                                 const struct spa_pod* parameters_in)
{
  STREAM_TRACE (ACE_TEXT ("acestream_dev_cam_pw_on_stream_param_changed_cb"));

  // sanity check(s)
  struct Stream_Device_Pipewire_Capture_CBData* cb_data_p =
      static_cast<struct Stream_Device_Pipewire_Capture_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  /* NULL means to clear the format */
  if (parameters_in == NULL || id_in != SPA_PARAM_Format)
    return;
  int result = spa_format_parse (parameters_in,
                                 &cb_data_p->videoFormat.media_type,
                                 &cb_data_p->videoFormat.media_subtype);
  if (unlikely (result < 0))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to spa_format_parse(%@), returning\n"),
                parameters_in));
    return;
  } // end IF
  /* only accept video */
  if (unlikely (cb_data_p->videoFormat.media_type != SPA_MEDIA_TYPE_video))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid media type (was: %u), returning\n"),
                cb_data_p->videoFormat.media_type));
    return;
  } // end IF
  switch (cb_data_p->videoFormat.media_subtype)
  {
    case SPA_MEDIA_SUBTYPE_raw:
    {
      result = spa_format_video_raw_parse (parameters_in,
                                           &cb_data_p->videoFormat.info.raw);
      ACE_ASSERT (result >= 0);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("capturing raw format %d (%s), @ %dx%d, @ %d/%d fps\n"),
                  cb_data_p->videoFormat.info.raw.format, ACE_TEXT (spa_debug_type_find_name (spa_type_video_format, cb_data_p->videoFormat.info.raw.format)),
                  cb_data_p->videoFormat.info.raw.size.width, cb_data_p->videoFormat.info.raw.size.height,
                  cb_data_p->videoFormat.info.raw.framerate.num, cb_data_p->videoFormat.info.raw.framerate.denom));
      break;
    }
    case SPA_MEDIA_SUBTYPE_mjpg:
    {
      result = spa_format_video_mjpg_parse (parameters_in,
                                            &cb_data_p->videoFormat.info.mjpg);
      ACE_ASSERT (result >= 0);
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("capturing mjpeg format @ %dx%d, @ %d/%d fps\n"),
                  cb_data_p->videoFormat.info.mjpg.size.width, cb_data_p->videoFormat.info.mjpg.size.height,
                  cb_data_p->videoFormat.info.mjpg.framerate.num, cb_data_p->videoFormat.info.mjpg.framerate.denom));
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown media subtype (was: %u), returning\n"),
                  cb_data_p->videoFormat.media_subtype));
      return;
    }
  } // end SWITCH
}

void
acestream_dev_cam_pw_on_process_cb (void* userData_in)
{
  // STREAM_TRACE (ACE_TEXT ("acestream_dev_cam_pw_on_process_cb"));

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
  uint8_t* frame_p;
  ACE_Message_Block* message_block_p = NULL;
  int result;

  pw_buffer_p = pw_stream_dequeue_buffer (cb_data_p->stream);
  if (unlikely (!pw_buffer_p))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("failed to pw_stream_dequeue_buffer(): %m, returning\n")));
    return;
  } // end IF
  spa_buffer_p = pw_buffer_p->buffer;
  ACE_ASSERT (spa_buffer_p);
  frame_p = static_cast<uint8_t*> (spa_buffer_p->datas[0].data);
  if (unlikely (!frame_p))
  {
    ACE_DEBUG ((LM_WARNING,
                ACE_TEXT ("no frame data, returning\n")));
    goto continue_;
  } // end IF

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
  ACE_ASSERT (message_block_p->space () >= spa_buffer_p->datas[0].chunk->size);
  result = message_block_p->copy (reinterpret_cast<char*> (frame_p),
                                  spa_buffer_p->datas[0].chunk->size);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Block::copy(): \"%m\", returning\n")));
    message_block_p->release ();
    goto continue_;
  } // end IF
  ++cb_data_p->statistic->capturedFrames;

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

continue_:
  result = pw_stream_queue_buffer (cb_data_p->stream,
                                   pw_buffer_p);
  ACE_ASSERT (result == 0);
}
