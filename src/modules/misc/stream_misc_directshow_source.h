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

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_task_base_synch.h"

// forward declarations
class ACE_Message_Queue_Base;

template <typename SessionMessageType,
          typename MessageType,
          ///////////////////////////////
          typename ConfigurationType,
          ///////////////////////////////
          typename SessionDataType>
class Stream_Misc_DirectShow_Source_T
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType>
 , public Stream_IModuleHandler_T<ConfigurationType>
{
 public:
  Stream_Misc_DirectShow_Source_T ();
  virtual ~Stream_Misc_DirectShow_Source_T ();

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (MessageType*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  //virtual void handleSessionMessage (SessionMessageType*&, // session message handle
  //                                   bool&);               // return value: pass message downstream ?

                                                           // implement Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&);
  virtual const ConfigurationType& get () const;

 protected:
  ConfigurationType* configuration_;

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType> inherited;

  //ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_T (const Stream_Misc_DirectShow_Source_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Misc_DirectShow_Source_T& operator= (const Stream_Misc_DirectShow_Source_T&))

  IMemAllocator*     IMemAllocator_;
  IMemInputPin*      IMemInputPin_;
};

// include template implementation
#include "stream_misc_directshow_source.inl"

#endif
