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
#include "d3d9.h"
#include "dxva2api.h"
#include "mfidl.h"
#include "mfreadwrite.h"
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
  static bool connectFirst (IGraphBuilder*); // graph handle
  static bool connected (IGraphBuilder*); // graph handle
  // *NOTE*: uses the 'intelligent' IGraphBuilder::Connect() API for all pins
  static bool graphConnect (IGraphBuilder*,                  // graph handle
                            const std::list<std::wstring>&); // graph
  static bool disconnect (IGraphBuilder*); // graph handle

  // -------------------------------------

  // *NOTE*: currently, these work for capture graphs only
  static bool getBufferNegotiation (IGraphBuilder*,          // graph handle
                                    IAMBufferNegotiation*&); // return value: capture filter output pin buffer allocator configuration handle
  // *IMPORTANT NOTE*: caller must deleteMediaType() the result !
  static bool getCaptureFormat (IGraphBuilder*,         // graph handle
                                struct _AMMediaType*&); // return value: media type
  static bool setCaptureFormat (IGraphBuilder*,              // graph handle
                                const struct _AMMediaType&); // media type
  static bool getOutputFormat (IGraphBuilder*,         // graph handle
                               struct _AMMediaType*&); // return value: media type

  //static bool getCaptureFormat (IMFSourceReader*, // source handle
  //                              IMFMediaType*&);  // return value: media type
  //static bool setCaptureFormat (IMFSourceReaderEx*,   // source handle
  //                              const IMFMediaType*); // media type
  //static bool getOutputFormat (IMFSourceReader*, // source handle
  //                             IMFMediaType*&);  // return value: media type
  static bool getCaptureFormat (IMFMediaSource*, // source handle
                                IMFMediaType*&);  // return value: media type
  static bool setCaptureFormat (IMFMediaSource*,      // source handle
                                const IMFMediaType*); // media type
  static bool setCaptureFormat (IMFTopology*,         // topology handle
                                const IMFMediaType*); // media type
  // *NOTE*: returns only the first available output type of the first output
  //         stream
  static bool getOutputFormat (IMFTransform*,   // MFT handle
                               IMFMediaType*&); // return value: media type
  static bool getOutputFormat (IMFTopology*,    // topology handle
                               IMFMediaType*&); // return value: media type

  //// *TODO*: using the Direct3D device manager (used by the EVR renderer) is
  ////         currently broken
  ////         --> pass NULL and use a different visualization module (e.g. the
  ////             Direct3D module)
  //static bool getSourceReader (IMFMediaSource*&,               // media device handle (in/out)
  //                             WCHAR*&,                        // return value: symbolic link
  //                             UINT32&,                        // return value: symbolic link size
  //                             const IDirect3DDeviceManager9*, // Direct3D device manager handle
  //                             const IMFSourceReaderCallback*, // callback handle
  //                             bool,                           // capture media type is YUV ?
  //                             IMFSourceReaderEx*&);           // return value: source reader handle
  static bool getMediaSource (const std::string&, // device ("FriendlyName")
                              IMFMediaSource*&,   // return value: media device handle
                              WCHAR*&,            // return value: symbolic link
                              UINT32&);           // return value: symbolic link size
  static bool getDirect3DDevice (const HWND,                      // target window handle
                                 const IMFMediaType*,             // media format handle
                                 IDirect3DDevice9Ex*&,            // return value: Direct3D device handle
                                 struct _D3DPRESENT_PARAMETERS_&, // return value: Direct3D presentation parameters
                                 IDirect3DDeviceManager9*&,       // return value: Direct3D device manager handle
                                 UINT&);                          // return value: reset token
  static bool initializeDirect3DManager (const IDirect3DDevice9Ex*, // Direct3D device handle
                                         IDirect3DDeviceManager9*&, // return value: Direct3D device manager handle
                                         UINT&);                    // return value: reset token

  // *NOTE*: loads the capture device filter and puts it into an empty (capture)
  //         graph
  static bool loadDeviceGraph (const std::string&,     // device ("FriendlyName")
                               IGraphBuilder*&,        // return value: (capture) graph handle
                               IAMBufferNegotiation*&, // return value: capture filter output pin buffer allocator configuration handle
                               IAMStreamConfig*&);     // return value: format configuration handle
  static bool loadDeviceTopology (const std::string&, // device ("FriendlyName")
                                  IMFMediaSource*&,   // input/return value: (capture) source handle
                                  IMFTopology*&);     // return value: topology handle
  // *NOTE*: disconnects the (capture) graph and removes all but the capture
  //         filter
  static bool resetDeviceGraph (IGraphBuilder*); // filter graph handle

  // -------------------------------------
  // *TODO*: remove these ASAP

  // *NOTE*: loads a filter graph (source side)
  static bool loadRendererGraph (const struct _AMMediaType&, // media type
                                 const HWND,                 // window handle [NULL: NullRenderer]
                                 IGraphBuilder*,             // graph handle
                                 std::list<std::wstring>&);  // return value: pipeline filter configuration
  static bool loadRendererTopology (const std::string&,                  // device ("FriendlyName")
                                    const IMFMediaType*,                 // grabber sink input media type handle
                                    const IMFSampleGrabberSinkCallback*, // grabber sink callback handle [NULL: do not use tee/grabber]
                                    const HWND,                          // window handle [NULL: do not use tee/EVR]
                                    IMFTopology*&);                      // input/return value: topology handle
  // *NOTE*: loads a filter graph (target side)
  static bool loadTargetRendererGraph (const HWND,                // window handle [NULL: NullRenderer]
                                       IGraphBuilder*&,           // return value: graph handle
                                       std::list<std::wstring>&); // return value: pipeline filter configuration
  //static bool loadTargetRendererTopology (const IMFMediaType*,                 // grabber sink input media type handle
  //                                        const IMFSampleGrabberSinkCallback*, // grabber sink callback handle [NULL: do not use tee/grabber]
  //                                        const HWND,                          // window handle [NULL: do not use tee/EVR]
  //                                        IMFTopology*&);                      // input/return value: topology handle

  // -------------------------------------

  // *NOTE*: close an existing log file by supplying an empty file name
  static void debug (IGraphBuilder*,      // graph handle
                     const std::string&); // log file name
  static void dump (IPin*); // pin handle
  static void dump (IMFSourceReader*); // source reader handle
  static void dump (IMFTransform*); // transform handle

  static bool isChromaLuminance (const IMFMediaType*); // media format handle
  static bool isRGB (const IMFMediaType*); // media format handle
  static bool isCompressed (const IMFMediaType*); // media format handle

  // *NOTE*: return value (if any) has an outstanding reference --> Release()
  static IPin* pin (IBaseFilter*,        // filter handle
                    enum _PinDirection); // direction
  // *NOTE*: return value (if any) has an outstanding reference --> Release()
  static IBaseFilter* pin2Filter (IPin*); // pin handle
  // *NOTE*: "...filters are given names when they participate in a filter
  //         graph..."
  static std::string name (IBaseFilter*); // filter handle

  static bool copyAttribute (IMFAttributes*,       // source
                             IMFAttributes*,       // destination
                             const struct _GUID&); // key
  static bool copyMediaType (const IMFMediaType*, // media type
                             IMFMediaType*&);     // return value: handle
  static bool copyMediaType (const struct _AMMediaType&, // media type
                             struct _AMMediaType*&);     // return value: handle
  static void deleteMediaType (struct _AMMediaType*&); // handle
  static void freeMediaType (struct _AMMediaType&);
  static std::string mediaSubTypeToString (const struct _GUID&); // GUID
  static std::string mediaTypeToString (const struct _AMMediaType&); // media type
  static std::string mediaTypeToString (const IMFMediaType*); // media type
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

  static bool setCaptureFormat (int,                        // device handle file descriptor
                                const struct v4l2_format&); // capture format
  static bool getCaptureFormat (int,                  // device handle file descriptor
                                struct v4l2_format&); // return value: format
  // *NOTE*: v4l uses time-per-frame (s) intervals, so the actual frame rate
  //         (fps) is the reciprocal of this value
  static bool getFrameRate (int,                 // device handle file descriptor
                            struct v4l2_fract&); // return value: frame rate (in time-per-frame (s))
  static bool setFrameRate (int,                       // file descriptor
                            const struct v4l2_fract&); // frame rate (in time-per-frame)

  static std::string formatToString (__u32); // format (fourcc)
#endif

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools (const Stream_Module_Device_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_Tools& operator= (const Stream_Module_Device_Tools&))

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  struct less_guid
  {
    bool operator () (const struct _GUID& lhs_in,
                      const struct _GUID& rhs_in) const
    {
      //ACE_ASSERT (lhs_in.Data2 == rhs_in.Data2);
      //ACE_ASSERT (lhs_in.Data3 == rhs_in.Data3);
      //ACE_ASSERT (*(long long*)lhs_in.Data4 == *(long long*)rhs_in.Data4);

      return (lhs_in.Data1 < rhs_in.Data1);
    }
  };
  typedef std::map<struct _GUID, std::string, less_guid> GUID2STRING_MAP_T;
  typedef GUID2STRING_MAP_T::const_iterator GUID2STRING_MAP_ITERATOR_T;
  static GUID2STRING_MAP_T Stream_MediaMajorType2StringMap;
  static GUID2STRING_MAP_T Stream_MediaSubType2StringMap;
  static GUID2STRING_MAP_T Stream_FormatType2StringMap;

  static ACE_HANDLE logFileHandle;

  //// *NOTE*: (if the media type is not a 'native' format) "... The Source Reader
  ////         will automatically load the decoder. ..."
  //static bool setOutputFormat (IMFSourceReader*,     // source handle
  //                             const IMFMediaType*); // media type
#endif
};

// include template definitions
#include "stream_dev_tools.inl"

#endif
