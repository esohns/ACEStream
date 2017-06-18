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

#ifndef STREAM_MODULE_NET_IO_STREAM_H
#define STREAM_MODULE_NET_IO_STREAM_H

#include <string>

#include "ace/Global_Macros.h"

#include "stream_base.h"
#include "stream_module_base.h"

#include "stream_module_io.h"

static constexpr const char default_io_stream_name_string_[] =
    ACE_TEXT_ALWAYS_CHAR ("NetworkIOStream");

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          const char* StreamName, // *TODO*: use a variadic character array
          ////////////////////////////////
          typename ControlType,
          typename NotificationType,
          typename StatusType,               // state machine-
          typename StateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          ///////////////////////////////
          typename AllocatorConfigurationType,
          typename ModuleConfigurationType,
          typename HandlerConfigurationType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // (reference counted)
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename AddressType,
          typename ConnectionManagerType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_Net_IO_Stream_T
 : public Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        StreamName,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        AllocatorConfigurationType,
                        ModuleConfigurationType,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType>
{
  typedef Stream_Base_T<ACE_SYNCH_USE,
                        TimePolicyType,
                        StreamName,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        AllocatorConfigurationType,
                        ModuleConfigurationType,
                        HandlerConfigurationType,
                        SessionDataType,
                        SessionDataContainerType,
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> inherited;

 public:
  // convenient types
  typedef ACE_Module<ACE_SYNCH_USE,
                     TimePolicyType> MODULE_T;

  inline Stream_Module_Net_IO_Stream_T () : inherited () {};
  inline virtual ~Stream_Module_Net_IO_Stream_T () { inherited::shutdown (); };

  // implement (part of) Stream_IStream_T
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const typename inherited::CONFIGURATION_T&);

  // implement Common_IStatistic_T
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  inline virtual void report () const { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

  //inline void ping () { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

 protected:
  // convenient types
  typedef Stream_INotify_T<NotificationType> INOTIFY_T;
  typedef Stream_Module_Net_IOReader_T<ACE_SYNCH_USE,
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
                                       AddressType,
                                       ConnectionManagerType,
                                       UserDataType> READER_T;
  typedef Stream_Module_Net_IOWriter_T<ACE_SYNCH_USE,
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
                                       AddressType,
                                       ConnectionManagerType,
                                       UserDataType> WRITER_T;
  typedef Stream_StreamModule_T<ACE_SYNCH_USE,             // task synch type
                                TimePolicyType,            // time policy
                                Stream_SessionId_t,        // session id type
                                SessionDataType,           // session data type
                                Stream_SessionMessageType, // session event type
                                ModuleConfigurationType,   // module configuration type
                                HandlerConfigurationType,  // module handler configuration type
                                INOTIFY_T,                 // stream notification interface type
                                READER_T,                  // reader type
                                WRITER_T> IO_MODULE_T;     // writer type

 private:
  // convenient types
  typedef Stream_Module_Net_IO_Stream_T<ACE_SYNCH_USE,
                                        TimePolicyType,
                                        StreamName,
                                        ControlType,
                                        NotificationType,
                                        StatusType,
                                        StateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        AllocatorConfigurationType,
                                        ModuleConfigurationType,
                                        HandlerConfigurationType,
                                        SessionDataType,
                                        SessionDataContainerType,
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        AddressType,
                                        ConnectionManagerType,
                                        UserDataType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IO_Stream_T (const Stream_Module_Net_IO_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IO_Stream_T& operator= (const Stream_Module_Net_IO_Stream_T&))
};

// include template definition
#include "stream_module_io_stream.inl"

#endif
