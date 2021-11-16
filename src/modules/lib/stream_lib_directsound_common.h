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

#ifndef STREAM_LIB_DIRECTSOUND_COMMON_H
#define STREAM_LIB_DIRECTSOUND_COMMON_H

#include <map>
#include <utility>

#include "mmreg.h"
// *WARNING*: "...Note Header files ksproxy.h and dsound.h define similar but
//            incompatible versions of the IKsPropertySet interface.
//            Applications that require the KS proxy module should use the
//            version defined in ksproxy.h.The DirectSound version of
//            IKsPropertySet is described in the DirectSound reference pages in
//            the Microsoft Windows SDK documentation.
//            If an application must include both ksproxy.h and dsound.h,
//            whichever header file the compiler scans first is the one whose
//            definition of IKsPropertySet is used by the compiler. ..."
#include "dsound.h"

// forward declarations
enum _AM_AUDIO_RENDERER_STAT_PARAM;

union Stream_MediaFramework_DirectSound_AudioEffectOptions
{
  struct _DSCFXAec        AECOptions;
  struct _DSFXChorus      chorusOptions;
  struct _DSFXCompressor  compressorOptions;
  struct _DSFXDistortion  distortionOptions;
  struct _DSFXEcho        echoOptions;
  struct _DSFXParamEq     equalizerOptions;
  struct _DSFXFlanger     flangerOptions;
  struct _DSFXGargle      gargleOptions;
  struct _DSFXI3DL2Reverb reverbOptions;
  struct _DSFXWavesReverb wavesReverbOptions;
};

typedef std::pair<DWORD, DWORD> Stream_MediaFrameWork_DirectSound_StatisticValue_t;
typedef std::map<enum _AM_AUDIO_RENDERER_STAT_PARAM, Stream_MediaFrameWork_DirectSound_StatisticValue_t> Stream_MediaFrameWork_DirectSound_Statistics_t;
typedef Stream_MediaFrameWork_DirectSound_Statistics_t::const_iterator Stream_MediaFrameWork_DirectSound_StatisticsIterator_t;

#endif
