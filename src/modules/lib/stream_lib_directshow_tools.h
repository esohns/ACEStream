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

#ifndef STREAM_LIB_DIRECTSHOW_TOOLS_H
#define STREAM_LIB_DIRECTSHOW_TOOLS_H

#include <map>
#include <string>

#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#include "minwindef.h"
#else
#include "windef.h"
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)
//#include <winnt.h>
#include "Ks.h"
#include "Guiddef.h"
#include "evr.h"
#include "strmif.h"
#include "mediaobj.h"

#include "ace/Global_Macros.h"

#include "common_image_common.h"

#include "stream_lib_common.h"
#include "stream_lib_directshow_common.h"

#if defined (FFMPEG_SUPPORT)
#include "stream_lib_ffmpeg_common.h"
#endif // FFMPEG_SUPPORT

class Stream_MediaFramework_DirectShow_Tools
{
  friend class Stream_MediaFramework_Tools;

 public:
  static bool initialize (bool = true); // initialize COM ?
  static void finalize (bool = true); // finalize COM ?

  // running object table
  static bool addToROT (IFilterGraph*, // filter graph handle
                        DWORD&);       // return value: ROT id
  static bool removeFromROT (DWORD); // ROT id

  // graph
  static void dump (const Stream_MediaFramework_DirectShow_Graph_t&); // graph layout
  static void dump (const Stream_MediaFramework_DirectShow_GraphConfiguration_t&); // graph configuration
  // *NOTE*: close an existing log file by supplying an empty file name
  static void debug (IGraphBuilder*,      // graph handle
                     const std::string&); // log file name

  static bool append (IGraphBuilder*,       // graph builder handle
                      IBaseFilter*,         // filter handle
                      const std::wstring&); // filter name
  static bool clear (IGraphBuilder*); // graph handle
  // *NOTE*: this uses IGraphBuilder::Connect())
  static bool connect (IGraphBuilder*, // graph handle
                       IBaseFilter*,   // filter 1
                       IBaseFilter*);  // filter 2
  // *NOTE*: this uses 'direct' pin connection (i.e. IPin::Connect()) where
  //         possible, and 'intelligent' pin connection (i.e.
  //         IGraphBuilder::Connect()) as fallback
  static bool connect (IGraphBuilder*,                                                // graph builder handle
                       const Stream_MediaFramework_DirectShow_GraphConfiguration_t&); // graph configuration
  // *NOTE*: this uses the 'intelligent' API (i.e. IGraphBuilder::Connect()) for
  //         all pins
  static bool graphBuilderConnect (IGraphBuilder*,                                   // graph builder handle
                                   const Stream_MediaFramework_DirectShow_Graph_t&); // graph layout
  static bool connectFirst (IGraphBuilder*,       // graph builder handle
                            const std::wstring&); // source filter name
  static bool connected (IGraphBuilder*,       // graph builder handle
                         const std::wstring&); // source filter name
  static bool disconnect (IBaseFilter*); // filter handle
  static bool disconnect (IGraphBuilder*); // graph builder handle
  static bool remove (IGraphBuilder*, // graph builder handle
                      IBaseFilter*);  // filter handle
  static void get (IGraphBuilder*,                             // graph handle
                   const std::wstring&,                        // source filter name
                   Stream_MediaFramework_DirectShow_Graph_t&); // return value: graph layout
  static bool has (IGraphBuilder*,       // graph handle
                   const std::wstring&); // filter name
  static bool has (const Stream_MediaFramework_DirectShow_GraphConfiguration_t&, // graph layout configuration
                   const std::wstring&);                                         // filter name

  // -------------------------------------

  // *NOTE*: currently, these work for capture graphs only
  static bool getBufferNegotiation (IGraphBuilder*,          // graph builder handle
                                    const std::wstring&,     // filter name
                                    IAMBufferNegotiation*&); // return value: capture filter output pin buffer allocator configuration handle
  static bool getVideoWindow (IGraphBuilder*,            // graph builder handle
                              const std::wstring&,       // video renderer filter name
                              IMFVideoDisplayControl*&); // return value: video renderer window configuration handle

  // *IMPORTANT NOTE*: caller must free() the return value !
  // *NOTE*: graph/filter must be connected
  static bool getOutputFormat (IGraphBuilder*,        // graph builder handle
                               const std::wstring&,   // filter name
                               struct _AMMediaType&); // return value: media type

  // pin
  static void dump (IPin*); // pin handle
  static std::string name (IPin*); // pin handle
  // *NOTE*: return value (if any) has an outstanding reference --> Release()
  static IBaseFilter* toFilter (IPin*); // pin handle
  static unsigned int countFormats (IPin*,                // pin handle
                                    REFGUID = GUID_NULL); // format type (GUID_NULL: all)
  // *NOTE*: this is also the most 'preferred' one
  // *NOTE*: "...A pin does not necessarily return any preferred media types.
  //         Moreover, the media types it returns might depend on the filter's
  //         connection status. For example, a filter's output pin might
  //         return a different set of media types depending on which media
  //         type was set for the filter's input pin. ..."
  static bool getFirstFormat (IPin*,                  // pin handle
                              REFGUID,                // subtype (GUID_NULL ? first format : first format of given subtype)
                              struct _AMMediaType*&); // return value: media type
  static bool hasUncompressedFormat (REFGUID,                // device category
                                     IPin*,                  // pin handle
                                     struct _AMMediaType*&); // return value: media type
  // *IMPORTANT NOTE*: caller must free() the return value !
  // *NOTE*: pin must be connected
  static struct _AMMediaType toFormat (IPin*); // pin handle

  // filter
  // *NOTE*: "...filters are given names when they participate [!] in a filter
  //         graph..."
  static std::string name (IBaseFilter*); // filter handle
  // *NOTE*: return value (if any) has an outstanding reference --> Release()
  static IPin* pin (IBaseFilter*,        // filter handle
                    enum _PinDirection); // direction
  static IPin* capturePin (IBaseFilter*); // filter handle
  // *IMPORTANT NOTE*: caller must deleteMediaType() the return value !
  static struct _AMMediaType* defaultCaptureFormat (IBaseFilter*); // filter handle
  static IBaseFilter* next (IBaseFilter*); // filter handle

  static bool hasPropertyPages (IBaseFilter*); // filter handle
  // -------------------------------------

  static bool loadSourceGraph (IBaseFilter*,           // source filter
                               const std::wstring&,    // source filter name
                               IGraphBuilder*&,        // return value: (capture) graph handle
                               IAMBufferNegotiation*&, // return value: source filter output pin buffer allocator configuration handle
                               IAMStreamConfig*&);     // return value: format configuration handle
  // *NOTE*: disconnects the graph and removes all but a specific source filter
  static bool reset (IGraphBuilder*, // filter graph handle
                     REFGUID);       // retain (device) category (GUID_NULL: retain first filter w/o input pins)

  // -------------------------------------

  static bool match (const struct tagBITMAPINFOHEADER&,  // bitmap info
                     const struct tagBITMAPINFOHEADER&); // bitmap info

  static bool match (const struct tagVIDEOINFOHEADER&,  // video info
                     const struct tagVIDEOINFOHEADER&); // video info
  static bool match (const struct tagVIDEOINFOHEADER2&,  // video info
                     const struct tagVIDEOINFOHEADER2&); // video info

  // media type
  static void dump (const struct _AMMediaType&); // media type
  // *IMPORTANT NOTE*: callers must 'delete_' any return values
  static struct _AMMediaType* copy (const struct _AMMediaType&);
  static void delete_ (struct _AMMediaType*&); // media type
  static void free (struct _AMMediaType&); // media type
  static void free (Stream_MediaFramework_DirectShow_Formats_t&);
  static bool match (const struct _AMMediaType&,  // media type
                     const struct _AMMediaType&); // media type
  static void setFormat (REFGUID,               // media type
                         struct _AMMediaType&); // in/out: media type
  static void setResolution (const Common_Image_Resolution_t&, // resolution
                             struct _AMMediaType&); // in/out: media type
  static void setFramerate (const unsigned int&,   // framerate (i.e. fps)
                            struct _AMMediaType&); // in/out: media type
  static unsigned int toBitrate (const struct _AMMediaType&); // media type
  // *IMPORTANT NOTE*: callers must 'DeleteMediaType' any return values
  inline static DMO_MEDIA_TYPE* toDMOMediaType (const struct _AMMediaType& mediaType_in) { return reinterpret_cast<DMO_MEDIA_TYPE*> (Stream_MediaFramework_DirectShow_Tools::copy (mediaType_in)); }
  static unsigned int toFramerate (const struct _AMMediaType&); // media type
  static unsigned int toFramesize (const struct _AMMediaType&); // media type
  static Common_Image_Resolution_t toResolution (const struct _AMMediaType&); // media type
  static unsigned int toRowStride (const struct _AMMediaType&); // media type
  // *IMPORTANT NOTE*: callers must 'free' return values
  static struct _AMMediaType toRGB (const struct _AMMediaType&); // media type
  static std::string toString (const struct _AMMediaType&, // media type
                               bool = false);              // condensed version ?

#if defined (FFMPEG_SUPPORT)
  // *IMPORTANT NOTE*: callers must 'delete_' return values
  static struct _AMMediaType* to (const struct Stream_MediaFramework_FFMPEG_VideoMediaType&); // media type
  static enum AVPixelFormat mediaSubTypeToAVPixelFormat (REFGUID); // media foundation subtype
#endif // FFMPEG_SUPPORT

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Tools (const Stream_MediaFramework_DirectShow_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Tools& operator= (const Stream_MediaFramework_DirectShow_Tools&))

  static Stream_MediaFramework_GUIDToStringMap_t Stream_MediaMajorTypeToStringMap;
  typedef std::map<WORD, std::string> WORD_TO_STRING_MAP_T;
  typedef WORD_TO_STRING_MAP_T::const_iterator WORD_TO_STRING_MAP_ITERATOR_T;
  static WORD_TO_STRING_MAP_T Stream_WaveFormatTypeToStringMap;
  static Stream_MediaFramework_GUIDToStringMap_t Stream_WaveFormatSubTypeToStringMap;

  static ACE_HANDLE logFileHandle;

  static std::string toString_2 (const struct _AMMediaType&); // media type
};

#endif
