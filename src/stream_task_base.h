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

#ifndef STREAM_TASK_BASE_H
#define STREAM_TASK_BASE_H

#include "ace/Global_Macros.h"

#include "common_iget.h"
#include "common_iinitialize.h"
#include "common_task_base.h"

#include "stream_imodule.h"
#include "stream_isessionnotify.h"
#include "stream_itask.h"
#include "stream_messagequeue.h"

// forward declarations
class ACE_Message_Block;
template <ACE_SYNCH_DECL, class TIME_POLICY>
class ACE_Module;
class ACE_Time_Value;

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
          typename SessionEventType>
class Stream_TaskBase_T
 : public Common_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType>
 , public Common_IGet_T<ConfigurationType>
 , public Common_IInitialize_T<ConfigurationType>
 , public Stream_ITask_T<ControlMessageType,
                         DataMessageType,
                         SessionMessageType>
 , public Stream_IModuleHandler_T<ACE_SYNCH_USE,
                                  TimePolicyType>
{
 public:
  virtual ~Stream_TaskBase_T ();

  // implement Common_IGet_T
  virtual const ConfigurationType& get () const;

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  // *NOTE*: these are just default (essentially NOP) implementations
  inline virtual void handleControlMessage (ControlMessageType&) {};
  inline virtual void handleDataMessage (DataMessageType*&,
                                         bool&) {};
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass this message downstream ?
  virtual void handleProcessingError (const ACE_Message_Block* const); // message handle

  // implement Stream_IModuleHandler_T
  inline virtual bool postClone (ACE_Module<ACE_SYNCH_USE,
                                            TimePolicyType>*) { return true; };

  // implement Common_IDumpState
  inline virtual void dump_state () const {};

 protected:
  Stream_TaskBase_T ();

  // helper methods
  // standard message handling (to be used by both asynch/synch derivates)
  void handleMessage (ACE_Message_Block*, // message handle
                      bool&);             // return value: stop processing ?

  // default implementation to handle user messages
  virtual void handleUserMessage (ACE_Message_Block*, // control message
                                  bool&,              // return value: stop processing ?
                                  bool&);             // return value: pass message downstream ?
  virtual void notify (SessionEventType); // session event

  ConfigurationType*                        configuration_;
  bool                                      isInitialized_;
  bool                                      isLinked_;
  typename SessionMessageType::DATA_T*      sessionData_;

  // *TODO*: synchronous tasks don't need this
  Stream_MessageQueue_T<SessionMessageType> queue_;

 private:
  typedef Common_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType> inherited;
  typedef Stream_ITask_T<ControlMessageType,
                         DataMessageType,
                         SessionMessageType> inherited2;

  // convenient types
  typedef Stream_TaskBase_T<ACE_SYNCH_USE,
                            TimePolicyType,
                            ConfigurationType,
                            ControlMessageType,
                            DataMessageType,
                            SessionMessageType,
                            SessionIdType,
                            SessionEventType> OWN_TYPE_T;
  typedef Stream_ISessionNotify_T<SessionIdType,
                                  typename SessionMessageType::DATA_T::DATA_T,
                                  SessionEventType> INOTIFY_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBase_T (const Stream_TaskBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBase_T& operator= (const Stream_TaskBase_T&))

  // override/hide ACE_Task_Base members
  virtual int put (ACE_Message_Block*,
                   ACE_Time_Value*);
};

// include template definition
#include "stream_task_base.inl"

#endif
