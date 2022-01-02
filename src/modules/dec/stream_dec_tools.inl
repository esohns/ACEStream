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

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "stream_macros.h"

template <typename DistributionType>
void
Stream_Module_Decoder_Tools::noise (unsigned int sampleRate_in,
                                    unsigned int bytesPerSample_in,
                                    unsigned int channels_in,
                                    bool formatIsSigned_in,
                                    bool formatIsLittleEndian_in,
                                    uint8_t* buffer_in,
                                    unsigned int samplesToWrite_in,
                                    double amplitude_in,
                                    DistributionType& distribution_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::noise"));

  bool byte_swap_b =
    (formatIsLittleEndian_in ? (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
                             : (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN));
  uint8_t* data_p = buffer_in;
  typename DistributionType::result_type value;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    value =
      static_cast<typename DistributionType::result_type> (static_cast<double> (Common_Tools::getRandomNumber (distribution_inout)) * amplitude_in);
    for (unsigned int j = 0;
         j < channels_in;
         ++j, data_p += bytesPerSample_in)
      switch (bytesPerSample_in)
      {
        case 1:
        {
          if (formatIsSigned_in)
            *reinterpret_cast<int8_t*> (data_p) = static_cast<int8_t> (value);
          else
            *data_p = static_cast<uint8_t> (value);
          break;
        }
        case 2:
        {
          if (formatIsSigned_in)
          {
            if (byte_swap_b)
              *reinterpret_cast<int16_t*> (data_p) =
                Common_Tools::byteSwap (static_cast<int16_t> (value));
            else
              *reinterpret_cast<int16_t*> (data_p) =
                static_cast<int16_t> (value);
          } // end IF
          else
          {
            if (byte_swap_b)
              *reinterpret_cast<uint16_t*> (data_p) =
                Common_Tools::byteSwap (static_cast<uint16_t> (value));
            else
              *reinterpret_cast<uint16_t*> (data_p) =
                static_cast<uint16_t> (value);
          } // end ELSE
          break;
        }
        case 4:
        {
          if (std::is_integral<typename DistributionType::result_type>::value)
          {
            if (formatIsSigned_in)
            {
              if (byte_swap_b)
                *reinterpret_cast<int32_t*> (data_p) =
                  Common_Tools::byteSwap (static_cast<int32_t> (value));
              else
                *reinterpret_cast<int32_t*> (data_p) =
                  static_cast<int32_t> (value);
            } // end IF
            else
            {
              if (byte_swap_b)
                *reinterpret_cast<uint32_t*> (data_p) =
                  Common_Tools::byteSwap (static_cast<uint32_t> (value));
              else
                *reinterpret_cast<uint32_t*> (data_p) =
                  static_cast<uint32_t> (value);
            } // end ELSE
          } // end IF
          else
          { ACE_ASSERT (ACE_SIZEOF_FLOAT == 4);
            float value_d = static_cast<float> (value);
            if (byte_swap_b)
              *reinterpret_cast<int32_t*> (data_p) =
                Common_Tools::byteSwap (*reinterpret_cast<int32_t*> (&value_d));
            else
              *reinterpret_cast<int32_t*> (data_p) =
                *reinterpret_cast<int32_t*> (&value_d);
          } // end ELSE
          break;
        }
        case 8:
        {
          if (std::is_integral<typename DistributionType::result_type>::value)
          {
            if (formatIsSigned_in)
            {
              if (byte_swap_b)
                *reinterpret_cast<int64_t*> (data_p) =
                  Common_Tools::byteSwap (static_cast<int64_t> (value));
              else
                *reinterpret_cast<int64_t*> (data_p) =
                  static_cast<int64_t> (value);
            } // end IF
            else
            {
              if (byte_swap_b)
                *reinterpret_cast<uint64_t*> (data_p) =
                  Common_Tools::byteSwap (static_cast<uint64_t> (value));
              else
                *reinterpret_cast<uint64_t*> (data_p) =
                  static_cast<uint64_t> (value);
            } // end ELSE
          } // end IF
          else
          { ACE_ASSERT (ACE_SIZEOF_DOUBLE == 8);
            double value_d = static_cast<double> (value);
            if (byte_swap_b)
              *reinterpret_cast<int64_t*> (data_p) =
                Common_Tools::byteSwap (*reinterpret_cast<int64_t*> (&value_d));
            else
              *reinterpret_cast<int64_t*> (data_p) =
                *reinterpret_cast<int64_t*> (&value_d);
          } // end ELSE
          break;
        }
        case 16:
        { ACE_ASSERT (!std::is_integral<typename DistributionType::result_type>::value);
          ACE_ASSERT (ACE_SIZEOF_LONG_DOUBLE == 16);
          ACE_ASSERT (false); // *TODO*: int64_t will not work here
          long double value_d = static_cast<long double> (value);
          if (byte_swap_b)
            *reinterpret_cast<int64_t*> (data_p) =
              Common_Tools::byteSwap (*reinterpret_cast<int64_t*> (&value_d));
          else
            *reinterpret_cast<int64_t*> (data_p) =
              *reinterpret_cast<int64_t*> (&value_d);
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
}
