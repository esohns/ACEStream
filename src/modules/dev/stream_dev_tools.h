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

#ifndef STREAM_MODULE_DEV_TOOLS_H
#define STREAM_MODULE_DEV_TOOLS_H

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <list>
#endif
#include <map>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/Time_Value.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "dshow.h"
#else
#include "linux/videodev2.h"
#endif

#include "stream_dev_common.h"
#include "stream_dev_defines.h"
#include "stream_dev_exports.h"

// forward declarations
class ACE_Message_Block;
class Stream_IAllocator;

class Stream_Dev_Export Stream_Module_Device_Tools
{
 public:
  static void initialize ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *TODO*: move the generic (i.e. non-device-specific) DirectShow
  //         functionality somewhere else
  static bool clear (IGraphBuilder*); // graph handle
  // *NOTE*: see stream_dev_defines.h for supported filter names
  // *NOTE*: the current implementation uses 'direct' pin connection (i.e.
  //         IPin::Connect()) where possible, and 'intelligent' pin connection
  //         (i.e. IGraphBuilder::Connect()) as fallback.
  static bool connect (IGraphBuilder*,                  // graph handle
                       const std::list<std::wstring>&); // graph
  // *NOTE*: uses the 'intelligent' IGraphBuilder::Connect() API for all pins
  static bool graphConnect (IGraphBuilder*,                  // graph handle
                            const std::list<std::wstring>&); // graph
  static bool disconnect (IGraphBuilder*); // graph handle
  static bool load (const std::string&,  // device ("FriendlyName")
                    IGraphBuilder*&,     // return value: (capture) graph handle
                    IAMStreamConfig*&);  // return value: stream configuration handle
  static bool load (const HWND,                // window handle [NULL: NullRenderer]
                    IGraphBuilder*&,           // return value: graph handle
                    std::list<std::wstring>&); // return value: pipeline filter configuration
  static bool reset (IGraphBuilder*); // graph handle

  // *NOTE*: close an existing log file by supplying an empty file name
  static void debug (IGraphBuilder*,      // graph handle
                     const std::string&); // log file name
  static void dump (IPin*); // pin handle
  // *NOTE*: return value (if any) has an outstanding reference --> Release()
  static IBaseFilter* pin2Filter (IPin*); // pin handle
  // *NOTE*: "...filters are given names when they participate in a filter
  //         graph..."
  static std::string name (IBaseFilter*); // filter handle

  static bool getFormat (IGraphBuilder*,         // graph handle
                         struct _AMMediaType*&); // return value: media type
  static bool setFormat (IGraphBuilder*,              // graph handle
                         const struct _AMMediaType&); // media type
  static void deleteMediaType (struct _AMMediaType*&); // handle
  static void freeMediaType (struct _AMMediaType&);
  static std::string mediaSubTypeToString (const GUID&); // GUID
  static std::string mediaTypeToString (const struct _AMMediaType&); // media type
#else
  static bool canOverlay (int); // file descriptor
  static bool canStream (int); // file descriptor
  static void dump (int); // file descriptor
  static bool initializeCapture (int,         // file descriptor
                                 v4l2_memory, // I/O streaming method
                                 __u32&);     // #buffers (in/out)
  static bool initializeOverlay (int,                        // file descriptor
                                 const struct v4l2_window&); // (target) window
  // *IMPORTANT NOTE*: invoke this AFTER VIDIOC_S_FMT, and BEFORE
  //                   VIDIOC_STREAMON
  template <typename MessageType>
  static bool initializeBuffers (int,                        // file descriptor
                                 v4l2_memory,                // I/O streaming method
                                 __u32,                      // number of buffers
                                 /////////
                                 INDEX2BUFFER_MAP_T&,        // return value: buffer map
                                 /////////
                                 Stream_IAllocator* = NULL); // allocator
  template <typename MessageType>
  static void finalizeBuffers (int,                  // file descriptor
                               v4l2_memory,          // I/O streaming method
                               INDEX2BUFFER_MAP_T&); // buffer map
  static unsigned int queued (int,            // file descriptor
                              unsigned int,   // number of buffers
                              unsigned int&); // return value: #done

  static bool setFormat (int,    // file descriptor
                         __u32); // format (fourcc)
  static bool getFormat (int,                  // file descriptor
                         struct v4l2_format&); // return value: format
  static bool setResolution (int,           // file descriptor
                             unsigned int,  // width
                             unsigned int); // height
  static bool setInterval (int,                       // file descriptor
                           const struct v4l2_fract&); // frame interval

  static std::string formatToString (__u32); // format (fourcc)
#endif

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools (const Stream_Module_Device_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools& operator= (const Stream_Module_Device_Tools&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct less_guid
  {
    bool operator() (const GUID& lhs_in, const GUID& rhs_in) const
    {
      //ACE_ASSERT (lhs_in.Data2 == rhs_in.Data2);
      //ACE_ASSERT (lhs_in.Data3 == rhs_in.Data3);
      //ACE_ASSERT (*(long long*)lhs_in.Data4 == *(long long*)rhs_in.Data4);

      return (lhs_in.Data1 < rhs_in.Data1);
    }
  };
  typedef std::map<GUID, std::string, less_guid> GUID2STRING_MAP_T;
  typedef GUID2STRING_MAP_T::const_iterator GUID2STRING_MAP_ITERATOR_T;
  static GUID2STRING_MAP_T Stream_MediaMajorType2StringMap;
  static GUID2STRING_MAP_T Stream_MediaSubType2StringMap;
  static GUID2STRING_MAP_T Stream_FormatType2StringMap;

  static ACE_HANDLE logFileHandle;
#endif
};

// include template definitions
#include "stream_dev_tools.inl"

#endif
