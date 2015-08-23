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

#ifndef STREAM_MODULE_INPUT_STREAM_H
#define STREAM_MODULE_INPUT_STREAM_H

#include "ace/Atomic_Op.h"
#include "ace/Global_Macros.h"
#include "ace/Thread_Mutex.h"

#include "stream_base.h"
#include "stream_module_base.h"

#include "stream_module_queuereader.h"

template <typename TaskSynchType,
          typename TimePolicyType,
          ///////////////////////////////
          typename StatusType,
          typename StateType,
          ///////////////////////////////
          typename ConfigurationType,
          ///////////////////////////////
          typename StatisticContainerType,
          ///////////////////////////////
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          ///////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          typename SessionMessageType,
          typename ProtocolMessageType>
class Stream_Module_Input_Stream_T
 : public Stream_Base_T<TaskSynchType,
                        TimePolicyType,
                        /////////////////
                        StatusType,
                        StateType,
                        /////////////////
                        ConfigurationType,
                        /////////////////
                        StatisticContainerType,
                        /////////////////
                        ModuleConfigurationType,
                        HandlerConfigurationType,
                        /////////////////
                        SessionDataType,          // session data
                        SessionDataContainerType, // session data container (reference counted)
                        SessionMessageType,
                        ProtocolMessageType>
{
 public:
  Stream_Module_Input_Stream_T ();
  virtual ~Stream_Module_Input_Stream_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&); // configuration

  // *TODO*: re-consider this API
  void ping ();

  // implement Common_IStatistic_T
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<TaskSynchType,
                        TimePolicyType,
                        /////////////////
                        StatusType,
                        StateType,
                        /////////////////
                        ConfigurationType,
                        /////////////////
                        StatisticContainerType,
                        /////////////////
                        ModuleConfigurationType,
                        HandlerConfigurationType,
                        /////////////////
                        SessionDataType,          // session data
                        SessionDataContainerType, // session data container (reference counted)
                        SessionMessageType,
                        ProtocolMessageType> inherited;

  // convenient types
  typedef Stream_Module_Input_Stream_T<TaskSynchType,
                                       TimePolicyType,
                                       /////////////////
                                       StatusType,
                                       StateType,
                                       /////////////////
                                       ConfigurationType,
                                       /////////////////
                                       StatisticContainerType,
                                       /////////////////
                                       ModuleConfigurationType,
                                       HandlerConfigurationType,
                                       /////////////////
                                       SessionDataType,          // session data
                                       SessionDataContainerType, // session data container (reference counted)
                                       SessionMessageType,
                                       ProtocolMessageType> OWN_TYPE_T;
  typedef Stream_Module_QueueReader_T<SessionMessageType,
                                      ProtocolMessageType,
                                      ///
                                      ConfigurationType,
                                      ///
                                      StateType,
                                      ///
                                      SessionDataType,
                                      SessionDataContainerType,
                                      ///
                                      StatisticContainerType> QUEUEREADER_T;
  typedef Stream_StreamModuleInputOnly_T<TaskSynchType,                                // task synch type
                                         TimePolicyType,                               // time policy
                                         ModuleConfigurationType,                      // module configuration type
                                         HandlerConfigurationType,                     // module handler configuration type
                                         QUEUEREADER_T> QUEUEREADER_MODULE_T; // writer type

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Input_Stream_T (const Stream_Module_Input_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Input_Stream_T& operator= (const Stream_Module_Input_Stream_T&))

  // modules
  QUEUEREADER_MODULE_T queueReader_;

  static ACE_Atomic_Op<ACE_Thread_Mutex, unsigned long> currentSessionID;
};

// include template implementation
#include "stream_module_input_stream.inl"

#endif
