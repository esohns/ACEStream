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

#ifndef STREAM_MISC_AGGREGATOR_H
#define STREAM_MISC_AGGREGATOR_H

#include <map>

#include "ace/Global_Macros.h"
#include "ace/Module.h"
#include "ace/Synch_Traits.h"
#include "ace/Task_T.h"
#include "ace/Stream.h"

#include "stream_common.h"
#include "stream_ilink.h"
#include "stream_task_base_synch.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionIdType,
          typename SessionDataType>
class Stream_Module_Aggregator_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 SessionIdType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_IModuleLinkCB
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

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Aggregator_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_Aggregator_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  inline virtual ~Stream_Module_Aggregator_T () {};

  // override some task-based members
  virtual int put (ACE_Message_Block*, // data chunk
                   ACE_Time_Value*);   // timeout value

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,   // configuration handle
                           Stream_IAllocator* = NULL); // allocator handle

  // implement (part of) Stream_ITaskBase_T
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  // convenient types
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;
  // *NOTE*: key: next module, value: previous module
  typedef std::map<MODULE_T*, MODULE_T*> MODULES_T;
  typedef typename MODULES_T::const_iterator MODULES_ITERATOR_T;
  typedef ACE_Task<ACE_SYNCH_USE,
                   TimePolicyType> TASK_T;
  typedef ACE_Stream<ACE_SYNCH_USE,
                     TimePolicyType> STREAM_T;
  typedef ACE_Stream_Iterator<ACE_SYNCH_USE,
                             TimePolicyType> STREAM_ITERATOR_T;

  ACE_SYNCH_MUTEX_T lock_;
  MODULES_T         modules_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_T (const Stream_Module_Aggregator_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Aggregator_T& operator= (const Stream_Module_Aggregator_T&))

  // implement Stream_IModuleLinkCB
  virtual void onLink ();
  virtual void onUnlink ();
};

// include template definition
#include "stream_misc_aggregator.inl"

#endif
