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

//#include "ace/Synch.h"
#include "stream_dec_libav_img_decoder.h"

#include "stream_dec_defines.h"

const char libacestream_default_dec_libav_img_decoder_module_name_string[] =
  ACE_TEXT_ALWAYS_CHAR (STREAM_DEC_DECODER_LIBAV_IMG_DECODER_DEFAULT_NAME_STRING);

enum AVPixelFormat
stream_decoder_libav_img_getformat_cb (struct AVCodecContext* context_in,
                                       const enum AVPixelFormat* formats_in)
{
  STREAM_TRACE (ACE_TEXT ("::stream_decoder_libav_img_getformat_cb"));

  // sanity check(s)
  ACE_ASSERT (context_in);
  ACE_ASSERT (formats_in);
  ACE_ASSERT (context_in->opaque);

  enum AVPixelFormat* preferred_format_p =
    reinterpret_cast<enum AVPixelFormat*> (context_in->opaque);

  // try to find the preferred format first
  for (const enum AVPixelFormat* iterator = formats_in;
       *iterator != -1;
       ++iterator)
    if (*iterator == *preferred_format_p)
      return *iterator;
  ACE_DEBUG ((LM_WARNING,
              ACE_TEXT ("%s: preferred format (was: %s) not supported, falling back\n"),
              ACE_TEXT (avcodec_get_name (context_in->codec_id)),
              ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (*preferred_format_p).c_str ())));

  // *TODO*: set context_in->hw_frames_ctx here as well

  // accept first uncompressed format (if any) as a fallback
  for (const enum AVPixelFormat* iterator = formats_in;
       *iterator != -1;
       ++iterator)
    if (!Stream_Module_Decoder_Tools::isCompressedVideo (*iterator))
      return *iterator;

  ACE_DEBUG ((LM_ERROR,
              ACE_TEXT ("%s: does not support any uncompressed video format, aborting\n"),
              ACE_TEXT (avcodec_get_name (context_in->codec_id))));

  return AV_PIX_FMT_NONE;
}

//void
//stream_decoder_libav_nopfree_cb (void* opaque_in,
//                                 uint8_t* data_in)
//{
//  STREAM_TRACE (ACE_TEXT ("::stream_decoder_libav_nopfree_cb"));

//  ACE_UNUSED_ARG (opaque_in);
//  ACE_UNUSED_ARG (data_in);
//}
