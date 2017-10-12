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

#ifndef STREAM_MODULE_DEV_DIRECTSHOW_TOOLS_H
#define STREAM_MODULE_DEV_DIRECTSHOW_TOOLS_H

#include <map>
#include <string>

#include <cguid.h>
#include <d3d9.h>
#include <dxva2api.h>
#include <evr.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <strmif.h>
#include <mtype.h>

#include "ace/Global_Macros.h"

#include "stream_dec_common.h"

#include "stream_dev_common.h"
#include "stream_dev_exports.h"

class Stream_Dev_Export Stream_Module_Device_DirectShow_Tools
{
 public:
  static void initialize ();

  // running object table
  static bool addToROT (IFilterGraph*, // filter graph handle
                        DWORD&);       // return value: ROT id
  static bool removeFromROT (DWORD); // ROT id

  static bool clear (IGraphBuilder*); // graph handle
  // *NOTE*: this uses 'direct' pin connection (i.e. IPin::Connect()) where
  //         possible, and 'intelligent' pin connection (i.e.
  //         IGraphBuilder::Connect()) as fallback
  static bool connect (IGraphBuilder*,                                               // graph builder handle
                       const Stream_Module_Device_DirectShow_GraphConfiguration_t&); // graph configuration
  // *NOTE*: this uses the 'intelligent' API (i.e. IGraphBuilder::Connect()) for
  //         all pins
  static bool graphBuilderConnect (IGraphBuilder*,                                  // graph builder handle
                                   const Stream_Module_Device_DirectShow_Graph_t&); // graph layout
  static bool connectFirst (IGraphBuilder*,       // graph builder handle
                            const std::wstring&); // source filter name
  static bool connected (IGraphBuilder*,       // graph builder handle
                         const std::wstring&); // source filter name
  static bool disconnect (IBaseFilter*); // filter handle
  static bool disconnect (IGraphBuilder*); // graph builder handle
  static void get (IGraphBuilder*,                            // graph handle
                   const std::wstring&,                       // source filter name
                   Stream_Module_Device_DirectShow_Graph_t&); // return value: graph layout
  static bool has (IGraphBuilder*,       // graph handle
                   const std::wstring&); // filter name

  // -------------------------------------

  // *NOTE*: currently, these work for capture graphs only
  static bool getBufferNegotiation (IGraphBuilder*,          // graph builder handle
                                    const std::wstring&,     // filter name
                                    IAMBufferNegotiation*&); // return value: capture filter output pin buffer allocator configuration handle
  static bool getVideoWindow (IGraphBuilder*,            // graph builder handle
                              const std::wstring&,       // video renderer filter name
                              IMFVideoDisplayControl*&); // return value: video renderer window configuration handle

  // *IMPORTANT NOTE*: caller must deleteMediaType() the return value !
  static bool getCaptureFormat (IGraphBuilder*,         // graph builder handle
                                struct _AMMediaType*&); // return value: media type
  static bool setCaptureFormat (IGraphBuilder*,              // graph builder handle
                                REFGUID,                     // device category
                                const struct _AMMediaType&); // media type
  // *IMPORTANT NOTE*: caller must deleteMediaType() the return value !
  // *NOTE*: pin must be connected
  static bool getFormat (IPin*,                  // pin handle
                         struct _AMMediaType*&); // return value: media type
  // *NOTE*: graph/filter must be connected
  static bool getOutputFormat (IGraphBuilder*,         // graph builder handle
                               const std::wstring&,    // filter name
                               struct _AMMediaType*&); // return value: media type
  // *NOTE*: this is also the most 'preferred' one
  static bool getFirstFormat (IPin*,                  // pin handle
                              REFGUID,                // subtype (GUID_NULL ? first format : first format of given subtype)
                              struct _AMMediaType*&); // return value: media type
  static bool hasUncompressedFormat (REFGUID,                // device category
                                     IPin*,                  // pin handle
                                     struct _AMMediaType*&); // return value: media type

  static unsigned int countFormats (IPin*,                // pin handle
                                    REFGUID = GUID_NULL); // format type (GUID_NULL: all)
  static void listCaptureFormats (IBaseFilter*,         // filter handle
                                  REFGUID = GUID_NULL); // format type (GUID_NULL: all)

  // *NOTE*: loads the (capture device) filter and puts it into an empty graph
  static bool loadDeviceGraph (const std::string&,                        // device name ("FriendlyName")
                               REFGUID,                                   // device category
                               IGraphBuilder*&,                           // return value: (capture) graph handle
                               IAMBufferNegotiation*&,                    // return value: capture filter output pin buffer allocator configuration handle
                               IAMStreamConfig*&,                         // return value: format configuration handle
                               Stream_Module_Device_DirectShow_Graph_t&); // return value: graph layout
  static bool loadSourceGraph (IBaseFilter*,           // source filter
                               const std::wstring&,    // source filter name
                               IGraphBuilder*&,        // return value: (capture) graph handle
                               IAMBufferNegotiation*&, // return value: source filter output pin buffer allocator configuration handle
                               IAMStreamConfig*&);     // return value: format configuration handle
  // *NOTE*: disconnects the graph and removes all but a specific source filter
  static bool resetGraph (IGraphBuilder*, // filter graph handle
                          REFGUID);       // retain (device) category (GUID_NULL: retain first filter w/o input pins)

  // -------------------------------------
  // *TODO*: remove these ASAP

  // *NOTE*: loads a filter graph (source side)
  static bool loadAudioRendererGraph (const struct _AMMediaType&,                             // media type
                                      const int,                                              // output handle [0: null]
                                      IGraphBuilder*,                                         // graph handle
                                      REFGUID,                                                // DMO effect CLSID [GUID_NULL: no effect]
                                      const Stream_Decoder_DirectShow_AudioEffectOptions&,    // DMO effect options
                                      Stream_Module_Device_DirectShow_GraphConfiguration_t&); // return value: graph layout
  static bool loadVideoRendererGraph (REFGUID,                                                // (device) category (GUID_NULL: retain first filter w/o input pins)
                                      const struct _AMMediaType&,                             // (input) media type
                                      const HWND,                                             // window handle [NULL: NullRenderer]
                                      IGraphBuilder*,                                         // graph builder handle
                                      Stream_Module_Device_DirectShow_GraphConfiguration_t&); // return value: graph configuration
  // *NOTE*: loads a filter graph (target side). If the first parameter is NULL,
  //         the filter with the name of the second parameter is expected to be
  //         part of the graph (third parameter) already
  static bool loadTargetRendererGraph (IBaseFilter*,                                           // source filter handle
                                       const std::wstring&,                                    // source filter name
                                       const struct _AMMediaType&,                             // input media type
                                       const HWND,                                             // window handle [NULL: NullRenderer]
                                       IGraphBuilder*&,                                        // return value: graph handle
                                       IAMBufferNegotiation*&,                                 // return value: source filter output pin buffer allocator configuration handle
                                       Stream_Module_Device_DirectShow_GraphConfiguration_t&); // return value: graph layout

  // -------------------------------------

  // *NOTE*: close an existing log file by supplying an empty file name
  static void debug (IGraphBuilder*,      // graph handle
                     const std::string&); // log file name
  static void dump (const Stream_Module_Device_DirectShow_Graph_t&); // graph layout
  static void dump (const Stream_Module_Device_DirectShow_GraphConfiguration_t&); // graph configuration
  static void dump (IPin*); // pin handle
  static void dump (const struct _AMMediaType&); // media type

  static std::string name (IPin*); // pin handle
  // *NOTE*: return value (if any) has an outstanding reference --> Release()
  static IPin* pin (IBaseFilter*,        // filter handle
                    enum _PinDirection); // direction
  // *NOTE*: return value (if any) has an outstanding reference --> Release()
  static IBaseFilter* pinToFilter (IPin*); // pin handle
  // *NOTE*: "...filters are given names when they participate [!] in a filter
  //         graph..."
  static std::string name (IBaseFilter*); // filter handle

  static bool copyMediaType (const struct _AMMediaType&, // media type
                             struct _AMMediaType*&);     // return value: handle
  inline static bool AMMediaTypeToDMOMediaType (const struct _AMMediaType& mediaType_in,                                                                                                                                        // media type
                                                struct _DMOMediaType*& mediaType_out) { return Stream_Module_Device_DirectShow_Tools::copyMediaType (mediaType_in, reinterpret_cast<struct _AMMediaType*&> (mediaType_out)); }; // return value: media type handle
  static void deleteMediaType (struct _AMMediaType*&); // handle
  static inline void freeMediaType (struct _AMMediaType& mediaType_in) { FreeMediaType (mediaType_in); };
  static std::string mediaTypeToString (const struct _AMMediaType&, // media type
                                        bool = false);              // condensed version ?

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_DirectShow_Tools ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_DirectShow_Tools (const Stream_Module_Device_DirectShow_Tools&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Device_DirectShow_Tools& operator= (const Stream_Module_Device_DirectShow_Tools&))

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
  typedef std::map<struct _GUID, std::string, less_guid> GUID_TO_STRING_MAP_T;
  typedef GUID_TO_STRING_MAP_T::const_iterator GUID_TO_STRING_MAP_ITERATOR_T;
  static GUID_TO_STRING_MAP_T Stream_MediaMajorTypeToStringMap;
  typedef std::map<WORD, std::string> WORD_TO_STRING_MAP_T;
  typedef WORD_TO_STRING_MAP_T::const_iterator WORD_TO_STRING_MAP_ITERATOR_T;
  static WORD_TO_STRING_MAP_T Stream_WaveFormatTypeToStringMap;
  static GUID_TO_STRING_MAP_T Stream_WaveFormatSubTypeToStringMap;

  static ACE_HANDLE logFileHandle;

  static std::string mediaTypeToString2 (const struct _AMMediaType&); // media type
};

#endif
