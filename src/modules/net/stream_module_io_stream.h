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

#include <ace/Global_Macros.h>

#include "stream_base.h"
#include "stream_module_base.h"

#include "stream_module_io.h"

template <typename LockType,
          ////////////////////////////////
          ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ControlType,
          typename NotificationType,
          typename StatusType,
          typename StateType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StatisticContainerType,
          ///////////////////////////////
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
          typename ConnectionManagerType>
class Stream_Module_Net_IO_Stream_T
 : public Stream_Base_T<LockType,
                        ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        ModuleConfigurationType,
                        HandlerConfigurationType,
                        SessionDataType,          // session data
                        SessionDataContainerType, // session data container (reference counted)
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType>
{
 public:
  Stream_Module_Net_IO_Stream_T (const std::string&); // name
  virtual ~Stream_Module_Net_IO_Stream_T ();

  // implement (part of) Stream_IStreamControlBase
  virtual bool load (Stream_ModuleList_t&, // return value: module list
                     bool&);               // return value: delete modules ?

  // implement Common_IInitialize_T
  virtual bool initialize (const ConfigurationType&, // configuration
                           bool = true,              // setup pipeline ?
                           bool = true);             // reset session data ?

  // implement Common_IStatistic_T
  virtual bool collect (StatisticContainerType&); // return value: statistic data
  virtual void report () const;

  // *TODO*: re-consider this API
  void ping ();

 protected:
  typedef Stream_INotify_T<NotificationType> INOTIFY_T;
  typedef Stream_Module_Net_IOReader_T<ACE_SYNCH_USE,
                                       TimePolicyType,
                                       HandlerConfigurationType,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       AddressType,
                                       ConnectionManagerType> READER_T;
  typedef Stream_Module_Net_IOWriter_T<LockType,
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
                                       ConnectionManagerType> WRITER_T;
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

//  // modules
//  IO_MODULE_T IO_;

 private:
  typedef Stream_Base_T<LockType,
                        ACE_SYNCH_USE,
                        TimePolicyType,
                        ControlType,
                        NotificationType,
                        StatusType,
                        StateType,
                        ConfigurationType,
                        StatisticContainerType,
                        ModuleConfigurationType,
                        HandlerConfigurationType,
                        SessionDataType,          // session data
                        SessionDataContainerType, // session data container (reference counted)
                        ControlMessageType,
                        DataMessageType,
                        SessionMessageType> inherited;

  // convenient types
  typedef Stream_Module_Net_IO_Stream_T<LockType,
                                        ACE_SYNCH_USE,
                                        TimePolicyType,
                                        ControlType,
                                        NotificationType,
                                        StatusType,
                                        StateType,
                                        ConfigurationType,
                                        StatisticContainerType,
                                        ModuleConfigurationType,
                                        HandlerConfigurationType,
                                        SessionDataType,          // session data
                                        SessionDataContainerType, // session data container (reference counted)
                                        ControlMessageType,
                                        DataMessageType,
                                        SessionMessageType,
                                        AddressType,
                                        ConnectionManagerType> OWN_TYPE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IO_Stream_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IO_Stream_T (const Stream_Module_Net_IO_Stream_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IO_Stream_T& operator= (const Stream_Module_Net_IO_Stream_T&))
};

// include template definition
#include "stream_module_io_stream.inl"

#endif
