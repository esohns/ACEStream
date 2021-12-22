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

#ifndef STREAM_LIB_DIRECTSOUND_TOOLS_H
#define STREAM_LIB_DIRECTSOUND_TOOLS_H

#include <map>
#include <string>

#include "mmdeviceapi.h"

#include "ace/Global_Macros.h"

#include "stream_lib_directsound_common.h"

// forward declarations
enum _AUDCLNT_SHAREMODE;
struct IAudioEndpointVolume;
struct IAudioVolumeLevel;
struct IPart;

class Stream_MediaFramework_DirectSound_Tools
{
  friend class Stream_MediaFramework_DirectShow_Tools;

 public:
  static void initialize ();

  // devices
  static struct _GUID waveDeviceIdToDirectSoundGUID (ULONG,        // waveIn/Out device id
                                                     bool = true); // capture ? : playback
  static ULONG directSoundGUIDTowaveDeviceId (REFGUID); // device identifier

  // format
  static bool isFloat (const struct tWAVEFORMATEX&); // format
  static std::string toString (const struct tWAVEFORMATEX&, // format
                               bool = false);               // condensed version ?

  // waveOut
  static bool canRender (ULONG,                        // waveOut device id
                         const struct tWAVEFORMATEX&); // format
  // *IMPORTANT NOTE*: there seems to be no way to determine a 'preferred' format
  //                   --> iterate over all of them !
  static void getBestFormat (ULONG,                  // waveOut device id
                             struct tWAVEFORMATEX&); // return value: default format

  // WASAPI
  static bool canRender (REFGUID,                      // device identifier
                         enum _AUDCLNT_SHAREMODE,      // share mode
                         const struct tWAVEFORMATEX&); // format
  // *NOTE*: "...In shared mode, the audio engine always supports the mix format..."
  // *TODO*: what about exclusive mode ?
  // *IMPORTANT NOTE*: callers must 'CoTaskMemFree' any return values
  static struct tWAVEFORMATEX* getAudioEngineMixFormat (REFGUID); // device identifier
  static IAudioEndpointVolume* getVolumeControl (REFGUID); // device identifier
  static IAudioVolumeLevel* getMicrophoneBoostControl (REFGUID); // device identifier

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectSound_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectSound_Tools (const Stream_MediaFramework_DirectSound_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectSound_Tools& operator= (const Stream_MediaFramework_DirectSound_Tools&))

  // helper types
  typedef std::map<WORD, std::string> WORD_TO_STRING_MAP_T;
  typedef WORD_TO_STRING_MAP_T::const_iterator WORD_TO_STRING_MAP_ITERATOR_T;

  // helper methods
  static IMMDevice* getDevice (REFGUID); // device identifier [GUID_NULL ? default render device]
  // *IMPORTANT NOTE*: fire-and-forget the first argument
  static IAudioVolumeLevel* walkDeviceTreeFromPart (IPart*,              // part handle
                                                    const std::string&); // (volume-)control name
  static std::string toString_2 (const struct tWAVEFORMATEX&); // format

  static WORD_TO_STRING_MAP_T Stream_WaveFormatTypeToStringMap;
  static Stream_MediaFramework_GUIDToStringMap_t Stream_WaveFormatSubTypeToStringMap;
};

#endif
