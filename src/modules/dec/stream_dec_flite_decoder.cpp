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

#include "stream_dec_flite_decoder.h"

#include "stream_dec_defines.h"

const char libacestream_default_dec_flite_decoder_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_FLITE_DECODER_DEFAULT_NAME_STRING);

int
libacestream_flite_audio_stream_chunk_cb (const cst_wave* wave_in,
                                          int start_in,
                                          int size_in,
                                          int last_in,
                                          cst_audio_streaming_info* asi_in)
{
  //STREAM_TRACE (ACE_TEXT ("::libacestream_flite_audio_stream_chunk_cb"));

  // sanity check(s)
  if (!size_in)
    return 0; // nothing to do
  ACE_ASSERT (asi_in);
  struct libacestream_flite_audio_stream_chunk_cbdata* cbdata_p =
    static_cast<struct libacestream_flite_audio_stream_chunk_cbdata*> (asi_in->userdata);
  ACE_ASSERT (cbdata_p);
  ACE_ASSERT (cbdata_p->allocator);
  ACE_ASSERT (cbdata_p->task);

  int result = -1;
  ACE_Message_Block* message_block_p = NULL;

  // step1: allocate message block
  message_block_p =
    static_cast<ACE_Message_Block*> (cbdata_p->allocator->malloc (size_in * sizeof (short)));
  if (unlikely (!message_block_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_IAllocator::malloc(%d): \"%m\", aborting\n"),
                size_in));
    return -1;
  } // end IF

  // step2: copy data into message buffer
  result =
    message_block_p->copy (reinterpret_cast<char*> (&wave_in->samples[start_in]),
                           size_in * sizeof (short));
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Message_Block::copy(%d): \"%m\", aborting\n"),
                size_in));
    message_block_p->release ();
    return -1;
  } // end IF

  // step3: push data downstream
  result = cbdata_p->task->put_next (message_block_p, NULL);
  if (unlikely (result == -1))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to ACE_Task::put_next(): \"%m\", aborting\n")));
    message_block_p->release ();
    return -1;
  } // end IF

  return 0;
}
