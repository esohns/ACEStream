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

#ifndef STREAM_LIB_DIRECTSHOW_TARGET_H
#define STREAM_LIB_DIRECTSHOW_TARGET_H

#include <BaseTyps.h>
#include <OAIdl.h>
#include <control.h>
#include <guiddef.h>
#include <strmif.h>
#include <sdkddkver.h>
#if COMMON_OS_WIN32_TARGET_PLATFORM(0x0602) // _WIN32_WINNT_WIN8
#include <minwindef.h>
#else
#include <windef.h>
#endif // COMMON_OS_WIN32_TARGET_PLATFORM(0x0602)

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_iinitialize.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

// forward declarations
class Stream_IAllocator;

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType,
          ////////////////////////////////
          typename FilterConfigurationType, // DirectShow-
          typename PinConfigurationType,    // DirectShow-
          typename MediaType,
          typename FilterType>              // DirectShow-
class Stream_MediaFramework_DirectShow_Target_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public FilterType
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef FilterType inherited2;

 public:
  Stream_MediaFramework_DirectShow_Target_T (ISTREAM_T*); // stream handle
  virtual ~Stream_MediaFramework_DirectShow_Target_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // convenient types
  typedef FilterType FILTER_T;

  // helper methods
  bool loadGraph (REFGUID,                        // (source) filter CLSID
                  const FilterConfigurationType&, // (source) filter configuration
                  const struct _AMMediaType&,     // 'preferred' media type
                  HWND,                           // (target) window handle {NULL: NullRenderer}
                  IGraphBuilder*&);               // return value: graph builder handle

  // *IMPORTANT NOTE*: 'asynchronous' filters implement IAsyncReader (downstream
  //                   filters 'pull' media samples), 'synchronous' filters
  //                   implement IMemInputPin and 'push' media samples to
  //                   downstream filters
  bool           push_; // push/pull strategy

  IGraphBuilder* IGraphBuilder_;
  //IMemAllocator*     IMemAllocator_;
  //IMemInputPin*      IMemInputPin_; // 'push' handle

  IMediaControl* IMediaControl_;
  IMediaEventEx* IMediaEventEx_;
  DWORD          ROTID_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Target_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Target_T (const Stream_MediaFramework_DirectShow_Target_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_MediaFramework_DirectShow_Target_T& operator= (const Stream_MediaFramework_DirectShow_Target_T&))

  // convenient types
  typedef Common_IInitialize_T<FilterConfigurationType> IINITIALIZE_FILTER_T;
};

// include template definition
#include "stream_lib_directshow_target.inl"

#endif
