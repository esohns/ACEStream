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

#ifndef STREAM_LIB_ALSA_TOOLS_H
#define STREAM_LIB_ALSA_TOOLS_H

#define ALSA_PCM_NEW_HW_PARAMS_API
extern "C"
{
#include "alsa/asoundlib.h"
}

#include "ace/Global_Macros.h"

#include "stream_lib_alsa_common.h"

// forward declarations
#if defined (SOX_SUPPORT)
typedef double sox_rate_t;
struct sox_encodinginfo_t;
struct sox_signalinfo_t;
#endif // SOX_SUPPORT

class Stream_MediaFramework_ALSA_Tools
{
 public:
  static bool setFormat (struct _snd_pcm*,                                        // device handle
                         const struct Stream_MediaFramework_ALSA_Configuration&); // configuration
  static bool getFormat (struct _snd_pcm*,                                  // device handle
                         struct Stream_MediaFramework_ALSA_Configuration&); // return value: configuration

  static std::string getDeviceName (enum _snd_pcm_stream); // direction
  static std::string formatToString (const struct _snd_pcm*,            // device handle
                                     const struct _snd_pcm_hw_params*); // format

  static void dump (struct _snd_pcm*); // device handle

  static bool getCaptureVolumeLevels (const std::string&, // card name
                                      const std::string&, // selem name
                                      long&,              // return value: min level
                                      long&,              // return value: max level
                                      long&);             // return value: current level
  static bool setCaptureVolumeLevel (const std::string&, // card name
                                     const std::string&, // selem name
                                     long);              // level

#if defined (SOX_SUPPORT)
  static void ALSAToSoX (enum _snd_pcm_format,       // format
                         sox_rate_t,                 // sample rate
                         unsigned int,               // channels
                         struct sox_encodinginfo_t&, // return value: format
                         struct sox_signalinfo_t&);  // return value: format
#endif // SOX_SUPPORT

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_ALSA_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_ALSA_Tools (const Stream_MediaFramework_ALSA_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_ALSA_Tools& operator= (const Stream_MediaFramework_ALSA_Tools&))
};

#endif