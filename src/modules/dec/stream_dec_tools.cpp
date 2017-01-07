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

#include <cmath>

#include <ace/Log_Msg.h>

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <combaseapi.h>
#include <strmif.h>
#else
#ifdef __cplusplus
extern "C"
{
#include <libavutil/avutil.h>
}
#endif
#endif

#include "common_tools.h"

#include "stream_macros.h"

void
Stream_Module_Decoder_Tools::initialize ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::initialize"));

}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
std::string
Stream_Module_Decoder_Tools::GUIDToString (REFGUID GUID_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::GUIDToString"));

  std::string result;

  OLECHAR GUID_string[CHARS_IN_GUID];
  ACE_OS::memset (GUID_string, 0, sizeof (GUID_string));
  int result_2 = StringFromGUID2 (GUID_in,
                                  GUID_string, CHARS_IN_GUID);
  ACE_ASSERT (result_2 == CHARS_IN_GUID);

#if defined (OLE2ANSI)
  result = GUID_string;
#else
  result = ACE_TEXT_ALWAYS_CHAR (ACE_TEXT_WCHAR_TO_TCHAR (GUID_string));
#endif

  return result;
}
struct _GUID
Stream_Module_Decoder_Tools::StringToGUID (const std::string& string_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::StringToGUID"));

  struct _GUID result = GUID_NULL;

  HRESULT result_2 = E_FAIL;
#if defined (OLE2ANSI)
  result_2 = CLSIDFromString (string_in.c_str (), &result);
#else
  result_2 =
    CLSIDFromString (ACE_TEXT_ALWAYS_WCHAR (string_in.c_str ()), &result);
#endif
  if (FAILED (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to CLSIDFromString(\"%s\"): \"%s\", aborting\n"),
                ACE_TEXT (string_in.c_str ()),
                ACE_TEXT (Common_Tools::error2String (result_2).c_str ())));
    return GUID_NULL;
  } // end IF

  return result;
}
#endif

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

void
Stream_Module_Decoder_Tools::sinus (double frequency_in,
                                    unsigned int sampleRate_in,
                                    unsigned int sampleSize_in, // 'data' sample
                                    unsigned int channels_in,
                                    char* buffer_in,
                                    unsigned int samplesToWrite_in, // #'data' samples
                                    double& phase_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::sinus"));

  static double maximum_phase = 2.0 * M_PI;
  double step =
    (maximum_phase * frequency_in) / static_cast<double> (sampleRate_in);
  unsigned int bytes_per_sample = sampleSize_in / channels_in;
  unsigned int maximum_value = (1 << ((bytes_per_sample * 8) - 1)) - 1;
  double phase = phase_inout;
  int value = 0;
  char* pointer_p = buffer_in;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    value = static_cast<int> (sin (phase) * maximum_value);
    for (unsigned int j = 0; j < channels_in; ++j)
    {
      for (unsigned int k = 0; k < bytes_per_sample; ++k)
      {
        if (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
          *(pointer_p + k) = (value >> (k * 8)) & 0xFF;
        else
          *(pointer_p + bytes_per_sample - 1 - k) = (value >> (k * 8)) & 0xFF;
      } // end FOR
      pointer_p += bytes_per_sample;
    } // end FOR
    phase += step;
    if (phase >= maximum_phase) phase -= maximum_phase;
  } // end FOR
  phase_inout = phase;
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
Stream_Module_Decoder_Tools::ALSA2SOX (const Stream_Module_Device_ALSAConfiguration& format_in,
                                       struct sox_encodinginfo_t& encoding_out,
                                       struct sox_signalinfo_t& format_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::ALSA2SOX"));

//  int result = -1;

  // initialize return value(s)
  ACE_OS::memset (&encoding_out, 0, sizeof (struct sox_encodinginfo_t));
  ACE_OS::memset (&format_out, 0, sizeof (struct sox_signalinfo_t));

  encoding_out.encoding = SOX_ENCODING_SIGN2;
//  enum _snd_pcm_format ALSA_format = SND_PCM_FORMAT_UNKNOWN;
//  unsigned int channels = 0;
//  unsigned int sample_rate = 0;
//  int subunit_direction = 0;

//  result = snd_pcm_hw_params_get_format (format_in,
//                                         &ALSA_format);
//  if (result < 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to snd_pcm_hw_params_get_format(): \"%s\", returning\n"),
//                ACE_TEXT (snd_strerror (result))));
//    return;
//  } // end IF
//  switch (ALSA_format)
  switch (format_in.format)
  {
    // PCM 'formats'
    case SND_PCM_FORMAT_S16_LE:
    case SND_PCM_FORMAT_S16_BE:
      break;
    case SND_PCM_FORMAT_U16_LE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_U16_BE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_S8:
      break;
    case SND_PCM_FORMAT_U8:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_MU_LAW:
      encoding_out.encoding = SOX_ENCODING_ULAW;
      break;
    case SND_PCM_FORMAT_A_LAW:
      encoding_out.encoding = SOX_ENCODING_ALAW;
      break;
    case SND_PCM_FORMAT_S32_LE:
      break;
    case SND_PCM_FORMAT_S32_BE:
      break;
    case SND_PCM_FORMAT_U32_LE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_U32_BE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_S24_LE:
      break;
    case SND_PCM_FORMAT_S24_BE:
      break;
    case SND_PCM_FORMAT_U24_LE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_U24_BE:
      encoding_out.encoding = SOX_ENCODING_UNSIGNED;
      break;
    case SND_PCM_FORMAT_FLOAT_LE:
      encoding_out.encoding = SOX_ENCODING_FLOAT;
      break;
    case SND_PCM_FORMAT_FLOAT_BE:
      encoding_out.encoding = SOX_ENCODING_FLOAT;
      break;
    case SND_PCM_FORMAT_FLOAT64_LE:
      encoding_out.encoding = SOX_ENCODING_FLOAT;
      break;
    case SND_PCM_FORMAT_FLOAT64_BE:
      encoding_out.encoding = SOX_ENCODING_FLOAT;
      break;
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown ALSA audio frame format (was: %d), returning\n"),
//                  ALSA_format));
                  format_in.format));
      return;
    }
  } // end SWITCH

//  result = snd_pcm_hw_params_get_channels (format_in,
//                                           &channels);
//  if (result < 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to snd_pcm_hw_params_get_channels(): \"%s\", returning\n"),
//                ACE_TEXT (snd_strerror (result))));
//    return;
//  } // end IF
//  result = snd_pcm_hw_params_get_rate (format_in,
//                                       &sample_rate, &subunit_direction);
//  if (result < 0)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to snd_pcm_hw_params_get_rate(): \"%s\", returning\n"),
//                ACE_TEXT (snd_strerror (result))));
//    return;
//  } // end IF

//      encoding_out.compression = 0.0;
//  encoding_out.bits_per_sample = snd_pcm_format_width (ALSA_format);
  encoding_out.bits_per_sample = snd_pcm_format_width (format_in.format);
  encoding_out.reverse_bytes = sox_option_default;
  encoding_out.reverse_nibbles = sox_option_default;
  encoding_out.reverse_bits = sox_option_default;
  encoding_out.opposite_endian = sox_false;

//  format_out.rate = sample_rate;
//  format_out.channels = channels;
//  format_out.precision = snd_pcm_format_width (ALSA_format);
    format_out.rate = format_in.rate;
    format_out.channels = format_in.channels;
    format_out.precision = snd_pcm_format_width (format_in.format);
//      format_out.length = 0;
//      format_out.mult = NULL;
}

#endif
