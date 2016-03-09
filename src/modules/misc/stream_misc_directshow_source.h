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

#ifndef STREAM_MISC_DIRECTSHOW_SOURCE_H
#define STREAM_MISC_DIRECTSHOW_SOURCE_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "dshow.h"

#include "common_iinitialize.h"
#include "common_time_common.h"

#include "stream_common.h"
//#include "stream_messagequeue.h"
#include "stream_task_base_synch.h"

#include "stream_misc_directshow_asynch_source_filter.h"
#include "stream_misc_directshow_source_filter.h"

template <typename SessionMessageType,
          typename MessageType,
          ///////////////////////////////
          typename ConfigurationType,
          ///////////////////////////////
          typename SessionDataType,
          ///////////////////////////////
          typename FilterConfigurationType,
          typename PinConfigurationType,
          typename MediaType>
class Stream_Misc_DirectShow_Source_T
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType>
 , public Stream_IModuleHandler_T<ConfigurationType>
{
  //typedef Stream_Misc_DirectShow_Asynch_Source_Filter_T<Common_TimePolicy_t,
  //                                                      SessionMessageType,
  //                                                      MessageType,

  //                                                      PinConfigurationType,
  //                                                      MediaType> ASYNCH_FILTER_T;
  //typedef Stream_Misc_DirectShow_Source_Filter_T<Common_TimePolicy_t,
  //                                               SessionMessageType,
  //                                               MessageType,

  //                                               PinConfigurationType,
  //                                               MediaType> FILTER_T;
  //friend class ASYNCH_FILTER_T;
  //friend class FILTER_T;

 public:
  // convenience types
  typedef Common_IInitialize_T<FilterConfigurationType> IINITIALIZE_T;

  Stream_Misc_DirectShow_Source_T ();
  virtual ~Stream_Misc_DirectShow_Source_T ();

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (MessageType*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);
  virtual const ConfigurationType& get () const;

 protected:
  ConfigurationType*   configuration_;
  //struct _AMMediaType* mediaType_; // 'preferred' media type
  SessionDataType*     sessionData_;

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType> inherited;

  //ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_T (const Stream_Misc_DirectShow_Source_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_T& operator= (const Stream_Misc_DirectShow_Source_T&))

  // helper methods
  bool initialize_DirectShow (const struct _GUID&,            // (source) filter CLSID
                              const FilterConfigurationType&, // (source) filter configuration
                              const struct _AMMediaType&,     // 'preferred' media type
                              const HWND,                     // (target) window handle {NULL: NullRenderer}
                              IGraphBuilder*&);               // return value: (capture) graph builder handle
  void finalize_DirectShow ();

  bool                 isInitialized_;
  bool                 push_; // push/pull strategy

  IGraphBuilder*       IGraphBuilder_;
  //IMemAllocator*     IMemAllocator_;
  //IMemInputPin*       IMemInputPin_; // 'push' handle

  IMediaControl*       IMediaControl_;
  IMediaEventEx*       IMediaEventEx_;
  DWORD                ROTID_;
};

// include template definition
#include "stream_misc_directshow_source.inl"

#endif
