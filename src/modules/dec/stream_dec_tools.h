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

#ifndef STREAM_MODULE_DEC_TOOLS_H
#define STREAM_MODULE_DEC_TOOLS_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include <string>

#include "sox.h"
#endif

#include "ace/Global_Macros.h"

#include "stream_dev_common.h"

#include "stream_dec_common.h"
#include "stream_dec_exports.h"

class Stream_Dec_Export Stream_Module_Decoder_Tools
{
 public:
  static void initialize ();

  static std::string compressionFormatToString (enum Stream_Decoder_CompressionFormatType);

  // *NOTE*: write a sinus waveform into the target buffer in the specified
  //         audio format
  // *WARNING*: make sure the data buffer contains enough space to hold the
  //            sample data
  static void sinus (double,       // frequency (Hz)
                     unsigned int, // sample rate (Hz)
                     unsigned int, // 'data' sample size (bytes)
                     unsigned int, // #channels
                     char*,        // target buffer
                     unsigned int, // #'data' samples to write
                     double&);     // (return value:) current phase

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  static std::string errorToString (int); // libav error

  static void ALSA2SOX (const Stream_Module_Device_ALSAConfiguration&, // format
                        struct sox_encodinginfo_t&,                    // return value: format
                        struct sox_signalinfo_t&);                     // return value: format
#endif

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools (const Stream_Module_Decoder_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Decoder_Tools& operator= (const Stream_Module_Decoder_Tools&))
};

#endif
