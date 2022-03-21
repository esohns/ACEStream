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

#include "stream_dec_avi_encoder.h"

#include "stream_dec_defines.h"

const char libacestream_default_dec_avi_encoder_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_ENCODER_AVI_DEFAULT_NAME_STRING);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
int
stream_decoder_aviencoder_libav_write_cb (void* opaque_in,
                                          uint8_t* buffer_in,
                                          int buf_size_in)
{
//  STREAM_TRACE (ACE_TEXT ("::stream_decoder_aviencoder_libav_write_cb"));

  // sanity check(s)
  ACE_ASSERT (opaque_in);
  ACE_Message_Block* message_block_p =
      static_cast<ACE_Message_Block*> (opaque_in);
  ACE_ASSERT (message_block_p);
  ACE_UNUSED_ARG (buffer_in);

  // *NOTE*: the data has already been written at this point
  //         --> simply adjust the write pointer to update the message size
  message_block_p->wr_ptr (static_cast<size_t> (buf_size_in));

  return 0;
}
#endif // ACE_WIN32 || ACE_WIN64
