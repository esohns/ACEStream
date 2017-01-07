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

#ifndef STREAM_MISC_DIRECTSHOW_TARGET_H
#define STREAM_MISC_DIRECTSHOW_TARGET_H

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include <streams.h>
#include <strmif.h>

#include "common_iinitialize.h"

#include "stream_task_base_synch.h"

#include "stream_misc_directshow_asynch_source_filter.h"
#include "stream_misc_directshow_source_filter.h"

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
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
class Stream_Misc_DirectShow_Target_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_ControlType,
                                 Stream_SessionMessageType,
                                 Stream_UserData>
{
  typedef Stream_Misc_DirectShow_Asynch_Source_Filter_T<Common_TimePolicy_t,
                                                        SessionMessageType,
                                                        DataMessageType,
                                                        FilterConfigurationType,
                                                        PinConfigurationType,
                                                        MediaType> ASYNCH_FILTER_T;
  typedef Stream_Misc_DirectShow_Source_Filter_T<Common_TimePolicy_t,
                                                 SessionMessageType,
                                                 DataMessageType,
                                                 FilterConfigurationType,
                                                 PinConfigurationType,
                                                 MediaType> FILTER_T;
  friend class ASYNCH_FILTER_T;
  friend class FILTER_T;

 public:
  Stream_Misc_DirectShow_Target_T ();
  virtual ~Stream_Misc_DirectShow_Target_T ();

  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  bool           push_; // push/pull strategy

  IGraphBuilder* IGraphBuilder_;
  //IMemAllocator*     IMemAllocator_;
  //IMemInputPin*       IMemInputPin_; // 'push' handle

  IMediaControl* IMediaControl_;
  IMediaEventEx* IMediaEventEx_;
  DWORD          ROTID_;

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_ControlType,
                                 Stream_SessionMessageType,
                                 Stream_UserData> inherited;

  //ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Target_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Target_T (const Stream_Misc_DirectShow_Target_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Target_T& operator= (const Stream_Misc_DirectShow_Target_T&))

  // convenient types
  typedef Common_IInitialize_T<FilterConfigurationType> IINITIALIZE_FILTER_T;

  // helper methods
  bool initialize_DirectShow (REFGUID,                        // (source) filter CLSID
                              const FilterConfigurationType&, // (source) filter configuration
                              const struct _AMMediaType&,     // 'preferred' media type
                              const HWND,                     // (target) window handle {NULL: NullRenderer}
                              IGraphBuilder*&);               // return value: (capture) graph builder handle
};

// include template definition
#include "stream_misc_directshow_target.inl"

#endif
