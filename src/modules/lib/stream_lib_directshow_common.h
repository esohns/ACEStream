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

#ifndef STREAM_LIB_DIRECTSHOW_COMMON_H
#define STREAM_LIB_DIRECTSHOW_COMMON_H

#include <deque>
#include <string>
#include <vector>

// *WARNING*: "...Note Header files ksproxy.h and dsound.h define similar but
//            incompatible versions of the IKsPropertySet interface.
//            Applications that require the KS proxy module should use the
//            version defined in ksproxy.h.The DirectSound version of
//            IKsPropertySet is described in the DirectSound reference pages in
//            the Microsoft Windows SDK documentation.
//            If an application must include both ksproxy.h and dsound.h,
//            whichever header file the compiler scans first is the one whose
//            definition of IKsPropertySet is used by the compiler. ..."
//#include "MMReg.h"
#include "WinNT.h"
#include "Guiddef.h"
#include "Ks.h"
#include "KsProxy.h"
#include "MMSystem.h"
#define INITGUID
#include "dsound.h"
#include "strmif.h"

#include "ace/OS.h"

#include "stream_lib_defines.h"

// forward declarations
class ACE_Message_Queue_Base;
class Stream_IAllocator;

typedef std::deque<struct _AMMediaType> Stream_MediaFramework_DirectShow_Formats_t;
typedef Stream_MediaFramework_DirectShow_Formats_t::iterator Stream_MediaFramework_DirectShow_FormatsIterator_t;

struct Stream_MediaFramework_DirectShow_FilterPinConfiguration
{
  Stream_MediaFramework_DirectShow_FilterPinConfiguration ()
   : allocatorProperties (NULL)
   , format ()
   , hasMediaSampleBuffers (false)
   , isTopToBottom (false)
   , queue (NULL)
  {}

  struct _AllocatorProperties* allocatorProperties;
  struct _AMMediaType          format; // (preferred) media type handle
  bool                         hasMediaSampleBuffers;
  // *NOTE*: some image formats have a bottom-to-top memory layout; in
  //         DirectShow, this is reflected by a positive biHeight; see also:
  //         https://msdn.microsoft.com/en-us/library/windows/desktop/dd407212(v=vs.85).aspx
  //         --> set this if the sample data is top-to-bottom
  bool                         isTopToBottom; // frame memory layout
  ACE_Message_Queue_Base*      queue;  // (inbound) buffer queue handle
};

struct Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Stream_MediaFramework_DirectShow_FilterConfiguration ()
   : allocator (NULL)
   , allocatorProperties (NULL)
   , pinConfiguration (NULL)
  {
    //ACE_OS::memset (&allocatorProperties,
    //                0,
    //                sizeof (struct _AllocatorProperties));
    //// *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
    ////         if this is -1/0 (why ?)
    ////allocatorProperties_.cbAlign = -1;  // <-- use default
    //allocatorProperties.cbAlign = 1;
    //allocatorProperties.cbBuffer = -1; // <-- use default
    //// *TODO*: IMemAllocator::SetProperties returns E_INVALIDARG (0x80070057)
    ////         if this is -1/0 (why ?)
    ////allocatorProperties.cbPrefix = -1; // <-- use default
    //allocatorProperties.cbPrefix = 0;
    //allocatorProperties.cBuffers =
    //  STREAM_LIB_DIRECTSHOW_FILTER_SOURCE_BUFFERS;
    ////allocatorProperties_.cBuffers = -1; // <-- use default
  }

  Stream_IAllocator*                                              allocator; // message-
  struct _AllocatorProperties*                                    allocatorProperties; // IMediaSample-
  struct Stream_MediaFramework_DirectShow_FilterPinConfiguration* pinConfiguration;
};

typedef std::vector<std::wstring> Stream_MediaFramework_DirectShow_Graph_t;
typedef Stream_MediaFramework_DirectShow_Graph_t::iterator Stream_MediaFramework_DirectShow_GraphIterator_t;
typedef Stream_MediaFramework_DirectShow_Graph_t::const_iterator Stream_MediaFramework_DirectShow_GraphConstIterator_t;
struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry
{
  Stream_MediaFramework_DirectShow_GraphConfigurationEntry ()
   : filterName ()
   , mediaType (NULL)
   , connectDirect (false)
  {}

  // *NOTE*: apparently, some filters (e.g. Video Resizer DSP DMO) need to
  //         connect to their downstream peer 'direct'ly
  bool                 connectDirect; // use IGraphBuilder::ConnectDirect() ? : IPin::Connect()
  std::wstring         filterName;
  struct _AMMediaType* mediaType; // ? media type to connect to the
                                  //   folowing input pin with
};
typedef std::list<struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry> Stream_MediaFramework_DirectShow_GraphConfiguration_t;
typedef Stream_MediaFramework_DirectShow_GraphConfiguration_t::iterator Stream_MediaFramework_DirectShow_GraphConfigurationIterator_t;
typedef Stream_MediaFramework_DirectShow_GraphConfiguration_t::const_iterator Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t;

union Stream_MediaFramework_DirectShow_AudioEffectOptions
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

#endif
