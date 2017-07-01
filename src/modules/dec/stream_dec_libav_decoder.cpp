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

#include "ace/Synch.h"
#include "stream_dec_libav_decoder.h"

void
Stream_Decoder_LibAVDecoder_LoggingCB (void* AVClassStruct_in,
                                       int level_in,
                                       const char* formatString_in,
                                       va_list arguments_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_LoggingCB"));

  ACE_UNUSED_ARG (AVClassStruct_in);

  char buffer[BUFSIZ];
  int print_prefix = 1;

  av_log_format_line (AVClassStruct_in,
                      level_in,
                      formatString_in,
                      arguments_in,
                      buffer,
                      sizeof (buffer),
                      &print_prefix);

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s"),
              buffer));
}

enum AVPixelFormat
Stream_Decoder_LibAVDecoder_GetFormat (struct AVCodecContext* context_in,
                                       const enum AVPixelFormat* formats_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_GetFormat"));

  // sanity check(s)
  ACE_ASSERT (context_in);
  ACE_ASSERT (context_in->opaque);
  ACE_ASSERT (formats_in);

  enum AVPixelFormat* preferred_format_p =
    reinterpret_cast<enum AVPixelFormat*> (context_in->opaque);

  // initialize return value(s)
  enum AVPixelFormat result = AV_PIX_FMT_NONE;

  // try to find the preferred format first
  for (const enum AVPixelFormat* iterator = formats_in;
       *iterator != -1;
       ++iterator)
    if (*iterator == *preferred_format_p)
      return *iterator;
  ACE_DEBUG ((LM_WARNING,
              ACE_TEXT ("codec does not support preferred video format (was: %d), falling back\n"),
              *preferred_format_p));

  // accept any uncompressed format as a fallback
  for (const enum AVPixelFormat* iterator = formats_in;
       *iterator != -1;
       ++iterator)
    if (!Stream_Module_Decoder_Tools::isCompressedVideo (*iterator))
      return *iterator;

  // *TODO*: set context_in->hw_frames_ctx here as well

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("codec does not support uncompressed video format, aborting\n")));

  return result;
}

void
Stream_Decoder_LibAVDecoder_NOPFree (void* opaque_in,
                                     uint8_t* data_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Decoder_LibAVDecoder_NOPFree"));

  ACE_UNUSED_ARG (opaque_in);
  ACE_UNUSED_ARG (data_in);
}
