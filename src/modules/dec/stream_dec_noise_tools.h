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

#ifndef STREAM_MODULE_DEC_NOISE_TOOLS_H
#define STREAM_MODULE_DEC_NOISE_TOOLS_H

#include <random>

#include "ace/Basic_Types.h"
#include "ace/Global_Macros.h"

#if defined (LIBNOISE_SUPPORT)
#include "noise/noise.h"
#endif // LIBNOISE_SUPPORT

class Stream_Module_Decoder_Noise_Tools
{
 public:
  // *NOTE*: write random noise into the target buffer in the specified
  //         audio format
  template <typename DistributionType>
  static void noise (unsigned int,       // sample rate (Hz)
                     unsigned int,       // #bytes/(mono-)sample
                     unsigned int,       // #channels
                     bool,               // format is signed ? : unsigned
                     bool,               // format is little endian ? : big endian
                     ACE_UINT8*,         // target buffer
                     unsigned int,       // #'data' samples to write
                     double,             // amplitude [0.0-1.0]
                     DistributionType&); // in/out: float/integer distribution handle

  static void pink_noise (unsigned int,    // sample rate (Hz)
                          unsigned int,    // #bytes/(mono-)sample
                          unsigned int,    // #channels
                          bool,            // format is signed ? : unsigned
                          bool,            // format is little endian ? : big endian
                          bool,            // format is floating point ? : integer
                          ACE_UINT8*,      // target buffer
                          unsigned int,    // #'data' samples to write
                          double,          // amplitude [0.0-1.0]
                          double,          // in: alpha
                          int,             // in: #poles
                          std::uniform_real_distribution<long double>&, // in/out: RNG
                          double[],        // in/out: multipliers
                          double[]);       // in/out: history

#if defined (LIBNOISE_SUPPORT)
  // *NOTE*: write perlin noise into the target buffer in the specified
  //         audio format
  static void perlin_noise (noise::module::Perlin&, // generator module handle
                            unsigned int,           // #bytes/(mono-)sample
                            unsigned int,           // #channels
                            bool,                   // format is floating point ? : integer
                            bool,                   // format is signed ? : unsigned
                            bool,                   // format is little endian ? : big endian
                            ACE_UINT8*,             // target buffer
                            unsigned int,           // #'data' samples to write
                            double,                 // amplitude [0.0-1.0]
                            double,                 // step
                            double&,                // in/out: coordinate x
                            double&,                // in/out: coordinate y
                            double&);               // in/out: coordinate z
#endif // LIBNOISE_SUPPORT

  // *NOTE*: write a sawtooth waveform into the target buffer in the specified
  //         audio format
  static void sawtooth (unsigned int, // sample rate (Hz)
                        unsigned int, // #bytes/(mono-)sample
                        unsigned int, // #channels
                        bool,         // format is floating point ? : integer
                        bool,         // format is signed ? : unsigned
                        bool,         // format is little endian ? : big endian
                        ACE_UINT8*,   // target buffer
                        unsigned int, // #'data' samples to write
                        double,       // amplitude [0.0-1.0]
                        double,       // frequency (Hz)
                        double&);     // in/out: current phase

  // *NOTE*: write a sine waveform into the target buffer in the specified
  //         audio format
  static void sinus (unsigned int, // sample rate (Hz)
                     unsigned int, // #bytes/(mono-)sample
                     unsigned int, // #channels
                     bool,         // format is floating point ? : integer
                     bool,         // format is signed ? : unsigned
                     bool,         // format is little endian ? : big endian
                     ACE_UINT8*,   // target buffer
                     unsigned int, // #'data' samples to write
                     double,       // amplitude [0.0-1.0]
                     double,       // frequency (Hz)
                     double&);     // in/out: current phase

  // *NOTE*: write a square waveform into the target buffer in the specified
  //         audio format
  static void square (unsigned int, // sample rate (Hz)
                      unsigned int, // #bytes/(mono-)sample
                      unsigned int, // #channels
                      bool,         // format is floating point ? : integer
                      bool,         // format is signed ? : unsigned
                      bool,         // format is little endian ? : big endian
                      ACE_UINT8*,   // target buffer
                      unsigned int, // #'data' samples to write
                      double,       // amplitude [0.0-1.0]
                      double,       // frequency (Hz)
                      double&);     // in/out: current phase

  // *NOTE*: write a square waveform into the target buffer in the specified
  //         audio format
  static void triangle (unsigned int, // sample rate (Hz)
                        unsigned int, // #bytes/(mono-)sample
                        unsigned int, // #channels
                        bool,         // format is floating point ? : integer
                        bool,         // format is signed ? : unsigned
                        bool,         // format is little endian ? : big endian
                        ACE_UINT8*,   // target buffer
                        unsigned int, // #'data' samples to write
                        double,       // amplitude [0.0-1.0]
                        double,       // frequency (Hz)
                        double&);     // in/out: current phase

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Noise_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Noise_Tools (const Stream_Module_Decoder_Noise_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Noise_Tools& operator= (const Stream_Module_Decoder_Noise_Tools&))
};

// include template definition
#include "stream_dec_noise_tools.inl"

#endif
