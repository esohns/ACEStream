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
                                    unsigned int samplesToWrite_in, // #'data' samples
                                    DistributionType& distribution_inout)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Decoder_Tools::noise"));

  // sanity check(s)
  ACE_ASSERT (ACE_SIZEOF_FLOAT == 4);
  ACE_ASSERT (ACE_SIZEOF_DOUBLE == 8);
  //ACE_ASSERT (ACE_SIZEOF_LONG_DOUBLE == 16); // *TODO*: 8 on Win32

  bool byte_swap_b =
    (formatIsLittleEndian_in ? (ACE_BYTE_ORDER != ACE_LITTLE_ENDIAN)
                             : (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN));
  uint8_t* data_p = buffer_in;
  typename DistributionType::result_type value;
  for (unsigned int i = 0; i < samplesToWrite_in; ++i)
  {
    value = Common_Tools::getRandomNumber (distribution_inout);
    for (unsigned int j = 0;
         j < channels_in;
         ++j, data_p += bytesPerSample_in)
      switch (bytesPerSample_in)
      {
        case 1:
        {
          *data_p =
            (formatIsSigned_in ? (uint8_t)static_cast<int8_t> (value)
                               : static_cast<uint8_t> (value));
          break;
        }
        case 2:
        {
          *reinterpret_cast<uint16_t*> (data_p) =
            (byte_swap_b ? (formatIsSigned_in ? (uint16_t)Common_Tools::byteSwap (static_cast<int16_t> (value))
                                              : Common_Tools::byteSwap (static_cast<uint16_t> (value)))
                         : (formatIsSigned_in ? (uint16_t)static_cast<int16_t> (value)
                                              : static_cast<uint16_t> (value)));
          break;
        }
        case 4:
        {
          if (std::is_integral<typename DistributionType::result_type>::value)
            *reinterpret_cast<uint32_t*> (data_p) =
              (byte_swap_b ? (formatIsSigned_in ? (uint32_t)Common_Tools::byteSwap (static_cast<int32_t> (value))
                                                : Common_Tools::byteSwap (static_cast<uint32_t> (value)))
                           : (formatIsSigned_in ? (uint32_t)static_cast<int32_t> (value)
                                                : static_cast<uint32_t> (value)));
          else
            *reinterpret_cast<float*> (data_p) =
              (byte_swap_b ? (float)Common_Tools::byteSwap (static_cast<uint32_t> (value))
                           : static_cast<float> (value));
          break;
        }
        case 8:
        {
          if (std::is_integral<typename DistributionType::result_type>::value)
            *reinterpret_cast<uint64_t*> (data_p) =
              (byte_swap_b ? (formatIsSigned_in ? (uint64_t)Common_Tools::byteSwap (static_cast<int64_t> (value))
                                                : Common_Tools::byteSwap (static_cast<uint64_t> (value)))
                           : (formatIsSigned_in ? (uint64_t)static_cast<int64_t> (value)
                                                : static_cast<uint64_t> (value)));
          else
            *reinterpret_cast<double*> (data_p) =
              (byte_swap_b ? (double)Common_Tools::byteSwap (static_cast<uint64_t> (value))
                           : static_cast<double> (value));
          break;
        }
        case 16:
        {
          if (std::is_integral<typename DistributionType::result_type>::value)
          {
            ACE_DEBUG ((LM_ERROR,
                        ACE_TEXT ("invalid/unknown value size (was: %u), returning\n"),
                        bytesPerSample_in));
            return;
          } // end IF
          *reinterpret_cast<long double*> (data_p) =
            (byte_swap_b ? (long double)Common_Tools::byteSwap (static_cast<uint64_t> (value)) // *TODO*: uint64_t will not work
                         : static_cast<long double> (value));
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
