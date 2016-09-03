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

#include "stream_dec_tools.h"

#include "ace/Log_Msg.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#ifdef __cplusplus
extern "C"
{
#include "libavutil/avutil.h"
}
#endif
#endif

#include "stream_macros.h"

void
Stream_Module_Decoder_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::initialize"));

}

std::string
Stream_Module_Decoder_Tools::compressionFormatToString (enum Stream_Decoder_CompressionFormatType format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::compressionFormatToString"));

  std::string result = ACE_TEXT_ALWAYS_CHAR ("Invalid");

  switch (format_in)
  {
    case STREAM_COMPRESSION_FORMAT_NONE:
      result = ACE_TEXT_ALWAYS_CHAR ("None"); break;
    case STREAM_COMPRESSION_FORMAT_GZIP:
      result = ACE_TEXT_ALWAYS_CHAR ("GZIP"); break;
    case STREAM_COMPRESSION_FORMAT_ZLIB:
      result = ACE_TEXT_ALWAYS_CHAR ("ZLIB"); break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown format (was: %d), aborting\n"),
                  format_in));
      break;
    }
  } // end SWITCH

  return result;
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
std::string
Stream_Module_Decoder_Tools::errorToString (int error_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::errorToString"));

  std::string result;

  int result_2 = -1;
  char buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));

  result_2 = av_strerror (error_in,
                          buffer,
                          sizeof (buffer));
  if (result_2 < 0)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to av_strerror(%d): \"%m\", continuing\n"),
                error_in));

  result = buffer;

  return result;
}

void
Stream_Module_Decoder_Tools::ALSA2SOX (const struct _snd_pcm_hw_params* format_in,
                                       struct sox_encodinginfo_t& encoding_out,
                                       struct sox_signalinfo_t& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::ALSA2SOX"));

  int result = -1;

  // initialize return value(s)
  ACE_OS::memset (&encoding_out, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&format_out, 0, sizeof (struct sox_signalinfo_t));

  enum _snd_pcm_format ALSA_format = SND_PCM_FORMAT_UNKNOWN;
  unsigned int channels = 0;
  unsigned int sample_rate = 0;
  int subunit_direction = 0;
  enum sox_encoding_t SOX_encoding = SOX_ENCODING_SIGN2;

  result = snd_pcm_hw_params_get_format (format_in,
                                         &ALSA_format);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_format(): \"%s\", returning\n"),
                ACE_TEXT (snd_strerror (result))));
    return;
  } // end IF
  switch (ALSA_format)
  {
    // PCM 'formats'
    case SND_PCM_FORMAT_S16_LE:
    case SND_PCM_FORMAT_S16_BE:
      break;
    case SND_PCM_FORMAT_U16_LE:
      SOX_encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_U16_BE:
      SOX_encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_S8:
      break;
    case SND_PCM_FORMAT_U8:
      SOX_encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_MU_LAW:
      SOX_encoding = SOX_ENCODING_ULAW;
      break;
    case SND_PCM_FORMAT_A_LAW:
      SOX_encoding = SOX_ENCODING_ALAW;
      break;
    case SND_PCM_FORMAT_S32_LE:
      break;
    case SND_PCM_FORMAT_S32_BE:
      break;
    case SND_PCM_FORMAT_U32_LE:
      SOX_encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_U32_BE:
      SOX_encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_S24_LE:
      break;
    case SND_PCM_FORMAT_S24_BE:
      break;
    case SND_PCM_FORMAT_U24_LE:
      SOX_encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_U24_BE:
      SOX_encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_FLOAT_LE:
      SOX_encoding = SOX_ENCODING_FLOAT;
      break;
    case SND_PCM_FORMAT_FLOAT_BE:
      SOX_encoding = SOX_ENCODING_FLOAT;
      break;
    case SND_PCM_FORMAT_FLOAT64_LE:
      SOX_encoding = SOX_ENCODING_FLOAT;
      break;
    case SND_PCM_FORMAT_FLOAT64_BE:
      SOX_encoding = SOX_ENCODING_FLOAT;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ALSA audio frame format (was: %d), returning\n"),
                  ALSA_format));
      return;
    }
  } // end SWITCH

  result = snd_pcm_hw_params_get_channels (format_in,
                                           &channels);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_channels(): \"%s\", returning\n"),
                ACE_TEXT (snd_strerror (result))));
    return;
  } // end IF
  result = snd_pcm_hw_params_get_rate (format_in,
                                       &sample_rate, &subunit_direction);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate(): \"%s\", returning\n"),
                ACE_TEXT (snd_strerror (result))));
    return;
  } // end IF

  encoding_out.encoding = SOX_encoding;
//      encoding_out.compression = 0.0;
  encoding_out.bits_per_sample = snd_pcm_format_width (ALSA_format);
  encoding_out.reverse_bytes = sox_option_default;
  encoding_out.reverse_nibbles = sox_option_default;
  encoding_out.reverse_bits = sox_option_default;
  encoding_out.opposite_endian = sox_false;

  format_out.rate = sample_rate;
  format_out.channels = channels;
  format_out.precision = snd_pcm_format_width (ALSA_format);
//      format_out.length = 0;
//      format_out.mult = NULL;
}

#endif
