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

#ifndef STREAM_LIB_COMMON_H
#define STREAM_LIB_COMMON_H

#include <list>
#include <string>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <guiddef.h>
#include <strmif.h>

#include "ace/OS.h"

#include "stream_lib_defines.h"
#endif // ACE_WIN32 || ACE_WIN64

// forward declarations
#if defined (ACE_WIN32) || defined (ACE_WIN64)
class ACE_Message_Queue_Base;
class Stream_IAllocator;
#endif // ACE_WIN32 || ACE_WIN64

// *TODO*: move these somewhere else
#if defined (ACE_WIN32) || defined (ACE_WIN64)
// {F9F62434-535B-4934-A695-BE8D10A4C699}
DEFINE_GUID (CLSID_ACEStream_MediaFramework_Source_Filter,
             0xf9f62434,
             0x535b,
             0x4934,
             0xa6, 0x95,
             0xbe, 0x8d, 0x10, 0xa4, 0xc6, 0x99);
// c553f2c0-1529-11d0-b4d1-00805f6cbbea
DEFINE_GUID (CLSID_ACEStream_MediaFramework_Asynch_Source_Filter,
             0xc553f2c0,
             0x1529,
             0x11d0,
             0xb4, 0xd1,
             0x00, 0x80, 0x5f, 0x6c, 0xbb, 0xea);

// {EFE6208A-0A2C-49fa-8A01-3768B559B6DA}
DEFINE_GUID (CLSID_ACEStream_MediaFramework_MF_MediaSource,
             0xefe6208a,
             0xa2c,
             0x49fa,
             0x8a, 0x1,
             0x37, 0x68, 0xb5, 0x59, 0xb6, 0xda);
#endif // ACE_WIN32 || ACE_WIN64

enum Stream_MediaFramework_Type
{
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  STREAM_MEDIAFRAMEWORK_DIRECTSHOW = 0,
  STREAM_MEDIAFRAMEWORK_MEDIAFOUNDATION,
#else
  STREAM_MEDIAFRAMEWORK_V4L = 0,
#endif // ACE_WIN32 || ACE_WIN64
  ////////////////////////////////////////
  STREAM_MEDIAFRAMEWORK_MAX,
  STREAM_MEDIAFRAMEWORK_INVALID
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
struct Stream_MediaFramework_DirectShow_FilterPinConfiguration
{
  Stream_MediaFramework_DirectShow_FilterPinConfiguration ()
   : format (NULL)
   , hasMediaSampleBuffers (false)
   , isTopToBottom (false)
   , queue (NULL)
  {}

  struct _AMMediaType*    format; // (preferred) media type handle
  bool                    hasMediaSampleBuffers;
  bool                    isTopToBottom; // frame memory layout
  ACE_Message_Queue_Base* queue;  // (inbound) buffer queue handle
};

struct Stream_MediaFramework_DirectShow_FilterConfiguration
{
  Stream_MediaFramework_DirectShow_FilterConfiguration ()
   : allocator (NULL)
   , allocatorProperties ()
  {
    ACE_OS::memset (&allocatorProperties,
                    0,
                    sizeof (struct _AllocatorProperties));
    // *TODO*: IMemAllocator::SetProperties returns VFW_E_BADALIGN (0x8004020e)
    //         if this is -1/0 (why ?)
    //allocatorProperties_.cbAlign = -1;  // <-- use default
    allocatorProperties.cbAlign = 1;
    allocatorProperties.cbBuffer = -1; // <-- use default
    // *TODO*: IMemAllocator::SetProperties returns E_INVALIDARG (0x80070057)
    //         if this is -1/0 (why ?)
    //allocatorProperties.cbPrefix = -1; // <-- use default
    allocatorProperties.cbPrefix = 0;
    allocatorProperties.cBuffers =
      MODULE_LIB_DIRECTSHOW_FILTER_SOURCE_BUFFERS;
    //allocatorProperties_.cBuffers = -1; // <-- use default
  }

  Stream_IAllocator*          allocator;
  struct _AllocatorProperties allocatorProperties;
};

typedef std::list<std::wstring> Stream_MediaFramework_DirectShow_Graph_t;
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
  struct _AMMediaType* mediaType; // media type to connect the
                                  // (head entry ? output- : input-) pin with
};
typedef std::list<struct Stream_MediaFramework_DirectShow_GraphConfigurationEntry> Stream_MediaFramework_DirectShow_GraphConfiguration_t;
typedef Stream_MediaFramework_DirectShow_GraphConfiguration_t::iterator Stream_MediaFramework_DirectShow_GraphConfigurationIterator_t;
typedef Stream_MediaFramework_DirectShow_GraphConfiguration_t::const_iterator Stream_MediaFramework_DirectShow_GraphConfigurationConstIterator_t;
#endif // ACE_WIN32 || ACE_WIN64

#endif
