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
//#include "ace/Synch_Traits.h"

#include "common_iget.h"
#include "common_iinitialize.h"
#include "common_task_base.h"

#include "stream_itask.h"
#include "stream_messagequeue.h"

// forward declarations
class ACE_Message_Block;
class ACE_Time_Value;

template <typename SynchStrategyType,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType>
class Stream_TaskBase_T
 : public Common_TaskBase_T<SynchStrategyType,
                            TimePolicyType>
 , public Common_IGet_T<ConfigurationType>
 , public Common_IInitialize_T<ConfigurationType>
 , public Stream_ITask_T<ControlMessageType,
                         DataMessageType,
                         SessionMessageType>
{
 public:
  virtual ~Stream_TaskBase_T ();

  // implement Common_IGet_T
  virtual const ConfigurationType& get () const;

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleControlMessage (ControlMessageType&); // control message handle
  // *NOTE*: these are just default (essentially NOP) implementations
  //virtual bool initialize (const void*); // configuration handle
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass this message downstream ?
  virtual void handleProcessingError (const ACE_Message_Block* const); // message handle

  // implement Common_IDumpState
  // *NOTE*: this is an implementation stub
  virtual void dump_state () const;

 protected:
  Stream_TaskBase_T ();

  // helper methods
  // standard message handling (to be used by both asynch/synch derivates)
  void handleMessage (ACE_Message_Block*, // message handle
                      bool&);             // return value: stop processing ?

  // default implementation to handle control messages
  virtual void handleControlMessage (ACE_Message_Block*, // control message
                                     bool&,              // return value: stop processing ?
                                     bool&);             // return value: pass message downstream ?

  ConfigurationType*  configuration_;
  //ACE_SYNCH_MUTEX     lock_;
  Stream_MessageQueue queue_;

 private:
  typedef Common_TaskBase_T<SynchStrategyType,
                            TimePolicyType> inherited;
  typedef Stream_ITask_T<ControlMessageType,
                         DataMessageType,
                         SessionMessageType> inherited2;

  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBase_T (const Stream_TaskBase_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_TaskBase_T& operator= (const Stream_TaskBase_T&))

  // override/hide ACE_Task_Base members
  virtual int put (ACE_Message_Block*,
                   ACE_Time_Value*);
};

// include template implementation
#include "stream_task_base.inl"

#endif
