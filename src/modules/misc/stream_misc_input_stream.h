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

#ifndef STREAM_MISC_INPUT_STREAM_H
#define STREAM_MISC_INPUT_STREAM_H

#include "ace/Global_Macros.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_ilayout.h"
#include "stream_inotify.h"
#include "stream_streammodule_base.h"

#include "stream_misc_queue_source.h"

// global variables
extern const char libacestream_default_misc_input_stream_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ControlType,
          typename NotificationType,
          typename StatusType,       // state machine-
          typename StateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          typename TimerManagerType,       // implements Common_ITimer
          ///////////////////////////////
          typename HandlerConfigurationType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Miscellaneous_Input_Stream_T
 : public Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        libacestream_default_misc_input_stream_name_string,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType>
{
  typedef Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        libacestream_default_misc_input_stream_name_string,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> inherited;

 public:
  // convenient types
  typedef Stream_MessageQueue_T<ACE_SYNCH_USE,
                                TimePolicyType,
                                SessionMessageType> MESSAGE_QUEUE_T;
  typedef Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        libacestream_default_misc_input_stream_name_string,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> STREAM_BASE_T;

  Stream_Miscellaneous_Input_Stream_T ();
  virtual ~Stream_Miscellaneous_Input_Stream_T ();

  // override (part of) Stream_IStream_T
  virtual bool load (Stream_ILayout*, // return value: layout
                     bool&);          // return value: delete modules ?

  MESSAGE_QUEUE_T queue_; // input-

 protected:
  // convenient types
  typedef Stream_INotify_T<NotificationType> INOTIFY_T;
  typedef Stream_Module_QueueReader_T<ACE_SYNCH_USE,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      HandlerConfigurationType,
                                      ControlType,
                                      NotificationType,
                                      StateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      TimerManagerType,
                                      UserDataType> WRITER_T;
  typedef Stream_StreamModuleInputOnly_T<ACE_SYNCH_USE,                     // task synch type
                                         TimePolicyType,                    // time policy
                                         SessionDataType,                   // session data type
                                         enum Stream_SessionMessageType,    // session event type
                                         struct Stream_ModuleConfiguration, // module configuration type
                                         HandlerConfigurationType,          // module handler configuration type
                                         libacestream_default_misc_queue_module_name_string, // name
                                         INOTIFY_T,                         // stream notification interface type
                                         WRITER_T> SOURCE_MODULE_T;         // writer type

 private:
  // convenient types
  typedef Stream_Miscellaneous_Input_Stream_T<ACE_SYNCH_USE,
                                              TimePolicyType,
                                              ControlType,
                                              NotificationType,
                                              StatusType,
                                              StateType,
                                              ConfigurationType,
                                              StatisticContainerType,
                                              TimerManagerType,
                                              HandlerConfigurationType,
                                              SessionDataType,
                                              SessionDataContainerType,
                                              ControlMessageType,
                                              DataMessageType,
                                              SessionMessageType,
                                              UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Miscellaneous_Input_Stream_T (const Stream_Miscellaneous_Input_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Miscellaneous_Input_Stream_T& operator= (const Stream_Miscellaneous_Input_Stream_T&))
};

// include template definition
#include "stream_misc_input_stream.inl"

#endif
