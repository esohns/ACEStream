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

#ifndef STREAM_MODULE_TCPIO_STREAM_H
#define STREAM_MODULE_TCPIO_STREAM_H

#include "ace/Global_Macros.h"

#include "stream_base.h"
#include "stream_module_base.h"

#include "stream_module_tcpio.h"

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
          typename ProtocolMessageType,
          ///////////////////////////////
          typename ConnectionManagerType>
class Stream_Module_TCPIO_Stream_T
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
  Stream_Module_TCPIO_Stream_T ();
  virtual ~Stream_Module_TCPIO_Stream_T ();

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&); // configuration

  // implement Common_IStatistic_T
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;

  // *TODO*: re-consider this API
  void ping ();

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
  typedef Stream_Module_TCPIO_Stream_T<TaskSynchType,
                                       TimePolicyType,

                                       StatusType,
                                       StateType,

                                       ConfigurationType,

                                       StatisticContainerType,

                                       ModuleConfigurationType,
                                       HandlerConfigurationType,

                                       SessionDataType,          // session data
                                       SessionDataContainerType, // session data container (reference counted)
                                       SessionMessageType,
                                       ProtocolMessageType,

                                       ConnectionManagerType> OWN_TYPE_T;

  typedef Stream_Module_TCPWriter_T<SessionMessageType,
                                    ProtocolMessageType,
                                    /////
                                    HandlerConfigurationType,
                                    /////
                                    StateType,
                                    /////
                                    SessionDataType,
                                    SessionDataContainerType,
                                    /////
                                    StatisticContainerType,
                                    /////
                                    ConnectionManagerType> WRITER_T;
  typedef Stream_Module_TCPReader_T<SessionMessageType,
                                    ProtocolMessageType,
                                    /////
                                    ConfigurationType,
                                    /////
                                    HandlerConfigurationType,
                                    /////
                                    SessionDataType,
                                    SessionDataContainerType,
                                    /////
                                    ConnectionManagerType> READER_T;
  typedef Stream_StreamModule_T<TaskSynchType,             // task synch type
                                TimePolicyType,            // time policy
                                ModuleConfigurationType,   // module configuration type
                                HandlerConfigurationType,  // module handler configuration type
                                READER_T,                  // reader type
                                WRITER_T> TCPIO_MODULE_T;  // writer type

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_TCPIO_Stream_T (const Stream_Module_TCPIO_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_TCPIO_Stream_T& operator= (const Stream_Module_TCPIO_Stream_T&))

  // modules
  TCPIO_MODULE_T TCPIO_;
};

// include template implementation
#include "stream_module_tcpio_stream.inl"

#endif
