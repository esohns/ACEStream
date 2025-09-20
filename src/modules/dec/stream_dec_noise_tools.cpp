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

#include "stream_dec_noise_tools.h"

#include <cmath>

#include "ace/Log_Msg.h"

#include "common_math_tools.h"

//#define MAXIMUM_PHASE_D 2.0 * M_PI
static double acestream_noise_maximum_phase_d = 2.0 * M_PI;

void
Stream_Module_Decoder_Noise_Tools::pink_noise (unsigned int sampleRate_in,
                                               unsigned int bytesPerSample_in,
                                               unsigned int channels_in,
                                               bool formatIsSigned_in,
                                               bool formatIsLittleEndian_in,
                                               bool formatIsFloatingPoint_in,
                                               ACE_UINT8* buffer_in,
                                               unsigned int samplesToWrite_in,
                                               double amplitude_in,
                                               long double alpha_in,
                                               int numPoles_in,
                                               std::uniform_real_distribution<long double>& distribution_inout,
                                               long double multipliers_inout[],
                                               long double history_inout[])
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Noise_Tools::pink_noise"));

  bool byte_swap_b =
    (formatIsLittleEndian_in ? (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
                             : (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN));
  uint8_t* data_p = buffer_in;
  long double value;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    for (unsigned int j = 0;
         j < channels_in;
         ++j, data_p += bytesPerSample_in)
    {
      value = Common_Tools::getRandomNumber (distribution_inout) - 0.5l;
      for (int k = 0; k < numPoles_in; ++k)
        value -= multipliers_inout[k] * history_inout[k];
      ACE_OS::memmove (&history_inout[1], &history_inout[0], (numPoles_in - 1) * sizeof (long double));
      history_inout[0] = value;
      // value = std::max (std::min (value, 1.0l), -1.0l);
      value *= static_cast<long double> (amplitude_in);

      switch (bytesPerSample_in)
      {
        case 1:
        {
          if (formatIsSigned_in)
            *reinterpret_cast<int8_t*> (data_p) = static_cast<int8_t> (value * Common_Tools::max<int8_t> (1, true));
          else
            *data_p = static_cast<uint8_t> (std::abs (value) * Common_Tools::max<uint8_t> (1, false));
          break;
        }
        case 2:
        {
          if (formatIsSigned_in)
          {
            if (byte_swap_b)
              *reinterpret_cast<int16_t*> (data_p) = Common_Tools::byteSwap (static_cast<int16_t> (value * Common_Tools::max<int16_t> (2, true)));
            else
              *reinterpret_cast<int16_t*> (data_p) = static_cast<int16_t> (value * Common_Tools::max<int16_t> (2, true));
          } // end IF
          else
          {
            if (byte_swap_b)
              *reinterpret_cast<uint16_t*> (data_p) = Common_Tools::byteSwap (static_cast<uint16_t> (std::abs (value) * Common_Tools::max<uint16_t> (2, false)));
            else
              *reinterpret_cast<uint16_t*> (data_p) = static_cast<uint16_t> (std::abs (value) * Common_Tools::max<uint16_t> (2, false));
          } // end ELSE
          break;
        }
        case 4:
        {
          if (!formatIsFloatingPoint_in)
          {
            if (formatIsSigned_in)
            {
              if (byte_swap_b)
                *reinterpret_cast<int32_t*> (data_p) = Common_Tools::byteSwap (static_cast<int32_t> (value * Common_Tools::max<int32_t> (4, true)));
              else
                *reinterpret_cast<int32_t*> (data_p) = static_cast<int32_t> (value * Common_Tools::max<int32_t> (4, true));
            } // end IF
            else
            {
              if (byte_swap_b)
                *reinterpret_cast<uint32_t*> (data_p) = Common_Tools::byteSwap (static_cast<uint32_t> (std::abs (value) * Common_Tools::max<uint32_t> (4, false)));
              else
                *reinterpret_cast<uint32_t*> (data_p) = static_cast<uint32_t> (std::abs (value) * Common_Tools::max<uint32_t> (4, false));
            } // end ELSE
          } // end IF
          else
          { ACE_ASSERT (ACE_SIZEOF_FLOAT == 4);
            float value_f = static_cast<float> (value);
            if (byte_swap_b)
              *reinterpret_cast<float*> (data_p) = Common_Tools::byteSwap (value_f);
            else
              *reinterpret_cast<float*> (data_p) = value_f;
          } // end ELSE
          break;
        }
        case 8:
        {
          if (!formatIsFloatingPoint_in)
          {
            if (formatIsSigned_in)
            {
              if (byte_swap_b)
                *reinterpret_cast<int64_t*> (data_p) = Common_Tools::byteSwap (static_cast<int64_t> (value * Common_Tools::max<int64_t> (8, true)));
              else
                *reinterpret_cast<int64_t*> (data_p) = static_cast<int64_t> (value * Common_Tools::max<int64_t> (8, true));
            } // end IF
            else
            {
              if (byte_swap_b)
                *reinterpret_cast<uint64_t*> (data_p) = Common_Tools::byteSwap (static_cast<uint64_t> (std::abs (value) * Common_Tools::max<uint64_t> (8, false)));
              else
                *reinterpret_cast<uint64_t*> (data_p) = static_cast<uint64_t> (std::abs (value) * Common_Tools::max<uint64_t> (8, false));
            } // end ELSE
          } // end IF
          else
          { ACE_ASSERT (ACE_SIZEOF_DOUBLE == 8);
            double value_d = static_cast<double> (value);
            if (byte_swap_b)
              *reinterpret_cast<double*> (data_p) = Common_Tools::byteSwap (value_d);
            else
              *reinterpret_cast<double*> (data_p) = value_d;
          } // end ELSE
          break;
        }
        case 16:
        { ACE_ASSERT (formatIsFloatingPoint_in);
          ACE_ASSERT (ACE_SIZEOF_LONG_DOUBLE == 16);
          if (byte_swap_b)
            *reinterpret_cast<long double*> (data_p) = Common_Tools::byteSwap (value);
          else
            *reinterpret_cast<long double*> (data_p) = value;
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown value size (was: %u), returning\n"),
                      bytesPerSample_in));
          return;
        }
      } // end SWITCH
    } // end FOR
  } // end FOR
}

#if defined (LIBNOISE_SUPPORT)
void
Stream_Module_Decoder_Noise_Tools::perlin_noise (noise::module::Perlin& module_in,
                                                 unsigned int bytesPerSample_in,
                                                 unsigned int channels_in,
                                                 bool formatIsFloat_in,
                                                 bool formatIsSigned_in,
                                                 bool formatIsLittleEndian_in,
                                                 ACE_UINT8* buffer_in,
                                                 unsigned int samplesToWrite_in,
                                                 double amplitude_in,
                                                 double step_in,
                                                 double& x_inout,
                                                 double& y_inout,
                                                 double& z_inout)

{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Noise_Tools::perlin_noise"));

  // sanity check(s)
  ACE_ASSERT (bytesPerSample_in <= 16);
  ACE_ASSERT (amplitude_in >= 0.0 && amplitude_in <= 1.0);

  ACE_UINT64 maximum_value_i =
    Common_Tools::max<ACE_UINT64> (bytesPerSample_in,
                                   formatIsSigned_in);
  bool byte_swap_b =
    (formatIsLittleEndian_in ? (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
                             : (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN));
  long double value_d = 0.0;
  ACE_UINT8* data_p = buffer_in;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    value_d = module_in.GetValue (x_inout, y_inout, z_inout);
    value_d =
      (formatIsFloat_in ? value_d * amplitude_in
                        : (formatIsSigned_in ? value_d * static_cast<long double> (maximum_value_i) * amplitude_in
                                             : (value_d + 1.0) * (static_cast<long double> (maximum_value_i) / 2.0) * amplitude_in));
    for (unsigned int j = 0; j < channels_in; ++j, data_p += bytesPerSample_in)
      switch (bytesPerSample_in)
      {
        case 1:
        {
          *data_p =
            (formatIsSigned_in ? (ACE_UINT8) static_cast<ACE_INT8> (value_d)
                               : static_cast<ACE_UINT8> (value_d));
          break;
        }
        case 2:
        {
          *reinterpret_cast<ACE_UINT16*> (data_p) =
            (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT16)static_cast<ACE_INT16> (value_d)
                                                                     : static_cast<ACE_UINT16> (value_d))
                         : (formatIsSigned_in ? (ACE_UINT16)static_cast<ACE_INT16> (value_d)
                                              : static_cast<ACE_UINT16> (value_d)));
          break;
        }
        case 4:
        {
          if (formatIsFloat_in)
            *reinterpret_cast<float*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (static_cast<float> (value_d))
                           : static_cast<float> (value_d));
          else
            *reinterpret_cast<ACE_UINT32*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT32)static_cast<ACE_INT32> (value_d)
                                                                       : static_cast<ACE_UINT32> (value_d))
                           : (formatIsSigned_in ? (ACE_UINT32)static_cast<ACE_INT32> (value_d)
                                                : static_cast<ACE_UINT32> (value_d)));
          break;
        }
        case 8:
        {
          if (formatIsFloat_in)
            *reinterpret_cast<double*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (static_cast<double> (value_d))
                           : static_cast<double> (value_d));
          else
            *reinterpret_cast<ACE_UINT64*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT64)static_cast<ACE_INT64> (value_d)
                                                                       : static_cast<ACE_UINT64> (value_d))
                           : (formatIsSigned_in ? (ACE_UINT64)static_cast<ACE_INT64> (value_d)
                                                : static_cast<ACE_UINT64> (value_d)));
          break;
        }
        case 16:
        { ACE_ASSERT (formatIsFloat_in);
          ACE_ASSERT (ACE_SIZEOF_LONG_DOUBLE == 16);
          *reinterpret_cast<long double*> (data_p) =
            (byte_swap_b ? Common_Tools::byteSwap (value_d) : value_d);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown value size (was: %u), returning\n"),
                      bytesPerSample_in));
          return;
        }
      } // end SWITCH
    x_inout += step_in;
  } // end FOR
}
#endif // LIBNOISE_SUPPORT

void
Stream_Module_Decoder_Noise_Tools::sawtooth (unsigned int sampleRate_in,
                                             unsigned int bytesPerSample_in,
                                             unsigned int channels_in,
                                             bool formatIsFloat_in,
                                             bool formatIsSigned_in,
                                             bool formatIsLittleEndian_in,
                                             ACE_UINT8* buffer_in,
                                             unsigned int samplesToWrite_in,
                                             double amplitude_in,
                                             double frequency_in,
                                             double& phase_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Noise_Tools::sawtooth"));

  // sanity check(s)
  ACE_ASSERT (bytesPerSample_in <= 16);
  ACE_ASSERT (amplitude_in >= 0.0 && amplitude_in <= 1.0);

  double step_d =
    (acestream_noise_maximum_phase_d * frequency_in) / static_cast<double> (sampleRate_in);
  ACE_UINT64 maximum_value_i =
    Common_Tools::max<ACE_UINT64> (bytesPerSample_in,
                                   formatIsSigned_in);
  bool byte_swap_b =
    (formatIsLittleEndian_in ? (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
                             : (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN));
  long double value_d = 0.0;
  ACE_UINT8* data_p = buffer_in;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    value_d = (phase_inout / acestream_noise_maximum_phase_d);
    value_d = (2.0 * (value_d - std::floor (0.5 + value_d)));
    value_d = 
      (formatIsFloat_in ? value_d * amplitude_in
                        : (formatIsSigned_in ? value_d * static_cast<long double> (maximum_value_i) * amplitude_in
                                             : (value_d + 1.0) * (static_cast<long double> (maximum_value_i) / 2.0) * amplitude_in));
    for (unsigned int j = 0; j < channels_in; ++j, data_p += bytesPerSample_in)
      switch (bytesPerSample_in)
      {
        case 1:
        {
          *data_p =
            (formatIsSigned_in ? (ACE_UINT8) static_cast<ACE_INT8> (value_d)
                               : static_cast<ACE_UINT8> (value_d));
          break;
        }
        case 2:
        {
          *reinterpret_cast<ACE_UINT16*> (data_p) =
            (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT16)static_cast<ACE_INT16> (value_d)
                                                                     : static_cast<ACE_UINT16> (value_d))
                         : (formatIsSigned_in ? (ACE_UINT16)static_cast<ACE_INT16> (value_d)
                                              : static_cast<ACE_UINT16> (value_d)));
          break;
        }
        case 4:
        {
          if (formatIsFloat_in)
            *reinterpret_cast<float*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (static_cast<float> (value_d))
                           : static_cast<float> (value_d));
          else
            *reinterpret_cast<ACE_UINT32*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT32)static_cast<ACE_INT32> (value_d)
                                                                       : static_cast<ACE_UINT32> (value_d))
                           : (formatIsSigned_in ? (ACE_UINT32)static_cast<ACE_INT32> (value_d)
                                                : static_cast<ACE_UINT32> (value_d)));
          break;
        }
        case 8:
        {
          if (formatIsFloat_in)
            *reinterpret_cast<double*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (static_cast<double> (value_d))
                           : static_cast<double> (value_d));
          else
            *reinterpret_cast<ACE_UINT64*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT64)static_cast<ACE_INT64> (value_d)
                                                                       : static_cast<ACE_UINT64> (value_d))
                           : (formatIsSigned_in ? (ACE_UINT64)static_cast<ACE_INT64> (value_d)
                                                : static_cast<ACE_UINT64> (value_d)));
          break;
        }
        case 16:
        { ACE_ASSERT (formatIsFloat_in);
          ACE_ASSERT (ACE_SIZEOF_LONG_DOUBLE == 16);
          *reinterpret_cast<long double*> (data_p) =
            (byte_swap_b ? Common_Tools::byteSwap (value_d) : value_d);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown value size (was: %u), returning\n"),
                      bytesPerSample_in));
          return;
        }
      } // end SWITCH
    phase_inout += step_d;
    if (unlikely (phase_inout >= acestream_noise_maximum_phase_d))
      phase_inout -= acestream_noise_maximum_phase_d;
  } // end FOR
}

void
Stream_Module_Decoder_Noise_Tools::sinus (unsigned int sampleRate_in,
                                          unsigned int bytesPerSample_in,
                                          unsigned int channels_in,
                                          bool formatIsFloat_in,
                                          bool formatIsSigned_in,
                                          bool formatIsLittleEndian_in,
                                          ACE_UINT8* buffer_in,
                                          unsigned int samplesToWrite_in,
                                          double amplitude_in,
                                          double frequency_in,
                                          double& phase_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Noise_Tools::sinus"));

  // sanity check(s)
  ACE_ASSERT (bytesPerSample_in <= 16);
  ACE_ASSERT (amplitude_in >= 0.0 && amplitude_in <= 1.0);

  double step_d =
    (acestream_noise_maximum_phase_d * frequency_in) / static_cast<double> (sampleRate_in);
  uint64_t maximum_value_i =
    Common_Tools::max<uint64_t> (bytesPerSample_in,
                                 formatIsSigned_in);
  bool byte_swap_b =
    (formatIsLittleEndian_in ? (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
                             : (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN));
  long double value_d = 0.0;
  uint8_t* data_p = buffer_in;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    value_d =
      (formatIsFloat_in ? std::sin (phase_inout) * amplitude_in
                        : (formatIsSigned_in ? std::sin (phase_inout) * static_cast<long double> (maximum_value_i) * amplitude_in
                                             : (std::sin (phase_inout) + 1.0) * (static_cast<long double> (maximum_value_i) / 2.0) * amplitude_in));
    for (unsigned int j = 0; j < channels_in; ++j, data_p += bytesPerSample_in)
      switch (bytesPerSample_in)
      {
        case 1:
        {
          *data_p =
            (formatIsSigned_in ? (uint8_t)static_cast<int8_t> (value_d)
                               : static_cast<uint8_t> (value_d));
          break;
        }
        case 2:
        {
          *reinterpret_cast<uint16_t*> (data_p) =
            (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (uint16_t)static_cast<int16_t> (value_d)
                                                                     : static_cast<uint16_t> (value_d))
                         : (formatIsSigned_in ? (uint16_t)static_cast<int16_t> (value_d)
                                              : static_cast<uint16_t> (value_d)));
          break;
        }
        case 4:
        {
          if (formatIsFloat_in)
            *reinterpret_cast<float*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (static_cast<float> (value_d))
                           : static_cast<float> (value_d));
          else
            *reinterpret_cast<uint32_t*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (uint32_t)static_cast<int32_t> (value_d)
                                                                       : static_cast<uint32_t> (value_d))
                           : (formatIsSigned_in ? (uint32_t)static_cast<int32_t> (value_d)
                                                : static_cast<uint32_t> (value_d)));
          break;
        }
        case 8:
        {
          if (formatIsFloat_in)
            *reinterpret_cast<double*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (static_cast<double> (value_d))
                           : static_cast<double> (value_d));
          else
            *reinterpret_cast<uint64_t*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (uint64_t)static_cast<int64_t> (value_d)
                                                                       : static_cast<uint64_t> (value_d))
                           : (formatIsSigned_in ? (uint64_t)static_cast<int64_t> (value_d)
                                                : static_cast<uint64_t> (value_d)));
          break;
        }
        case 16:
        { ACE_ASSERT (formatIsFloat_in);
          ACE_ASSERT (ACE_SIZEOF_LONG_DOUBLE == 16);
          *reinterpret_cast<long double*> (data_p) =
            (byte_swap_b ? Common_Tools::byteSwap (value_d) : value_d);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown value size (was: %u), returning\n"),
                      bytesPerSample_in));
          return;
        }
      } // end SWITCH
    phase_inout += step_d;
    if (unlikely (phase_inout >= acestream_noise_maximum_phase_d))
      phase_inout -= acestream_noise_maximum_phase_d;
  } // end FOR
}

void
Stream_Module_Decoder_Noise_Tools::square (unsigned int sampleRate_in,
                                           unsigned int bytesPerSample_in,
                                           unsigned int channels_in,
                                           bool formatIsFloat_in,
                                           bool formatIsSigned_in,
                                           bool formatIsLittleEndian_in,
                                           ACE_UINT8* buffer_in,
                                           unsigned int samplesToWrite_in,
                                           double amplitude_in,
                                           double frequency_in,
                                           double& phase_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Noise_Tools::square"));

  // sanity check(s)
  ACE_ASSERT (bytesPerSample_in <= 16);
  ACE_ASSERT (amplitude_in >= 0.0 && amplitude_in <= 1.0);

  double step_d =
    (acestream_noise_maximum_phase_d * frequency_in) / static_cast<double> (sampleRate_in);
  ACE_UINT64 maximum_value_i =
    Common_Tools::max<ACE_UINT64> (bytesPerSample_in,
                                   formatIsSigned_in);
  bool byte_swap_b =
    (formatIsLittleEndian_in ? (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
                             : (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN));
  long double value_d = 0.0;
  ACE_UINT8* data_p = buffer_in;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    value_d = std::sin (phase_inout);
    value_d =
      (formatIsFloat_in ? (std::signbit (value_d) ? -1.0 
                                                  : (value_d == 0.0) ? 0.0 : 1.0) * amplitude_in
                        : (formatIsSigned_in ? (std::signbit (value_d) ? -1.0 
                                                                       : (value_d == 0.0) ? 0.0 : 1.0) * static_cast<long double> (maximum_value_i) * amplitude_in
                                             : ((std::signbit (value_d) ? -1.0 
                                                                        : (value_d == 0.0) ? 0.0 : 1.0) + 1.0) * (static_cast<long double> (maximum_value_i) / 2.0) * amplitude_in));
    for (unsigned int j = 0; j < channels_in; ++j, data_p += bytesPerSample_in)
      switch (bytesPerSample_in)
      {
        case 1:
        {
          *data_p =
            (formatIsSigned_in ? (ACE_UINT8) static_cast<ACE_INT8> (value_d)
                               : static_cast<ACE_UINT8> (value_d));
          break;
        }
        case 2:
        {
          *reinterpret_cast<ACE_UINT16*> (data_p) =
            (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT16)static_cast<ACE_INT16> (value_d)
                                                                     : static_cast<ACE_UINT16> (value_d))
                         : (formatIsSigned_in ? (ACE_UINT16)static_cast<ACE_INT16> (value_d)
                                              : static_cast<ACE_UINT16> (value_d)));
          break;
        }
        case 4:
        {
          if (formatIsFloat_in)
            *reinterpret_cast<float*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (static_cast<float> (value_d))
                           : static_cast<float> (value_d));
          else
            *reinterpret_cast<ACE_UINT32*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT32)static_cast<ACE_INT32> (value_d)
                                                                       : static_cast<ACE_UINT32> (value_d))
                           : (formatIsSigned_in ? (ACE_UINT32)static_cast<ACE_INT32> (value_d)
                                                : static_cast<ACE_UINT32> (value_d)));
          break;
        }
        case 8:
        {
          if (formatIsFloat_in)
            *reinterpret_cast<double*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (static_cast<double> (value_d))
                           : static_cast<double> (value_d));
          else
            *reinterpret_cast<ACE_UINT64*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT64)static_cast<ACE_INT64> (value_d)
                                                                       : static_cast<ACE_UINT64> (value_d))
                           : (formatIsSigned_in ? (ACE_UINT64)static_cast<ACE_INT64> (value_d)
                                                : static_cast<ACE_UINT64> (value_d)));
          break;
        }
        case 16:
        { ACE_ASSERT (formatIsFloat_in);
          ACE_ASSERT (ACE_SIZEOF_LONG_DOUBLE == 16);
          *reinterpret_cast<long double*> (data_p) =
            (byte_swap_b ? Common_Tools::byteSwap (value_d) : value_d);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown value size (was: %u), returning\n"),
                      bytesPerSample_in));
          return;
        }
      } // end SWITCH
    phase_inout += step_d;
    if (unlikely (phase_inout >= acestream_noise_maximum_phase_d))
      phase_inout -= acestream_noise_maximum_phase_d;
  } // end FOR
}

void
Stream_Module_Decoder_Noise_Tools::triangle (unsigned int sampleRate_in,
                                             unsigned int bytesPerSample_in,
                                             unsigned int channels_in,
                                             bool formatIsFloat_in,
                                             bool formatIsSigned_in,
                                             bool formatIsLittleEndian_in,
                                             ACE_UINT8* buffer_in,
                                             unsigned int samplesToWrite_in,
                                             double amplitude_in,
                                             double frequency_in,
                                             double& phase_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Noise_Tools::triangle"));

  // sanity check(s)
  ACE_ASSERT (bytesPerSample_in <= 16);
  ACE_ASSERT (amplitude_in >= 0.0 && amplitude_in <= 1.0);

  double step_d =
    (acestream_noise_maximum_phase_d * frequency_in) / static_cast<double> (sampleRate_in);
  ACE_UINT64 maximum_value_i =
    Common_Tools::max<ACE_UINT64> (bytesPerSample_in,
                                   formatIsSigned_in);
  bool byte_swap_b =
    (formatIsLittleEndian_in ? (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
                             : (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN));
  long double value_d = 0.0;
  ACE_UINT8* data_p = buffer_in;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    value_d = (phase_inout / acestream_noise_maximum_phase_d);
    value_d =
      (2.0 * std::abs (2.0 * (value_d - std::floor (0.5 + value_d)))) - 1.0;
    value_d = 
      (formatIsFloat_in ? value_d * amplitude_in
                        : (formatIsSigned_in ? value_d * static_cast<long double> (maximum_value_i) * amplitude_in
                                             : (value_d + 1.0) * (static_cast<long double> (maximum_value_i) / 2.0) * amplitude_in));
    for (unsigned int j = 0; j < channels_in; ++j, data_p += bytesPerSample_in)
      switch (bytesPerSample_in)
      {
        case 1:
        {
          *data_p =
            (formatIsSigned_in ? (ACE_UINT8) static_cast<ACE_INT8> (value_d)
                               : static_cast<ACE_UINT8> (value_d));
          break;
        }
        case 2:
        {
          *reinterpret_cast<ACE_UINT16*> (data_p) =
            (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT16)static_cast<ACE_INT16> (value_d)
                                                                     : static_cast<ACE_UINT16> (value_d))
                         : (formatIsSigned_in ? (ACE_UINT16)static_cast<ACE_INT16> (value_d)
                                              : static_cast<ACE_UINT16> (value_d)));
          break;
        }
        case 4:
        {
          if (formatIsFloat_in)
            *reinterpret_cast<float*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (static_cast<float> (value_d))
                           : static_cast<float> (value_d));
          else
            *reinterpret_cast<ACE_UINT32*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT32)static_cast<ACE_INT32> (value_d)
                                                                       : static_cast<ACE_UINT32> (value_d))
                           : (formatIsSigned_in ? (ACE_UINT32)static_cast<ACE_INT32> (value_d)
                                                : static_cast<ACE_UINT32> (value_d)));
          break;
        }
        case 8:
        {
          if (formatIsFloat_in)
            *reinterpret_cast<double*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (static_cast<double> (value_d))
                           : static_cast<double> (value_d));
          else
            *reinterpret_cast<ACE_UINT64*> (data_p) =
              (byte_swap_b ? Common_Tools::byteSwap (formatIsSigned_in ? (ACE_UINT64)static_cast<ACE_INT64> (value_d)
                                                                       : static_cast<ACE_UINT64> (value_d))
                           : (formatIsSigned_in ? (ACE_UINT64)static_cast<ACE_INT64> (value_d)
                                                : static_cast<ACE_UINT64> (value_d)));
          break;
        }
        case 16:
        { ACE_ASSERT (formatIsFloat_in);
          ACE_ASSERT (ACE_SIZEOF_LONG_DOUBLE == 16);
          *reinterpret_cast<long double*> (data_p) =
            (byte_swap_b ? Common_Tools::byteSwap (value_d) : value_d);
          break;
        }
        default:
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("invalid/unknown value size (was: %u), returning\n"),
                      bytesPerSample_in));
          return;
        }
      } // end SWITCH
    phase_inout += step_d;
    if (unlikely (phase_inout >= acestream_noise_maximum_phase_d))
      phase_inout -= acestream_noise_maximum_phase_d;
  } // end FOR
}
