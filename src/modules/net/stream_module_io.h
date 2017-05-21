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

#ifndef STREAM_MODULE_NET_IO_H
#define STREAM_MODULE_NET_IO_H

#include <ace/Global_Macros.h>
#include <ace/Synch_Traits.h>

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"
#include "stream_task_base_synch.h"

// forward declarations
template <ACE_SYNCH_DECL,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename ConfigurationType,
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          typename SessionDataType,
          typename SessionDataContainerType,
          typename StatisticContainerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
class Stream_Module_Net_IOWriter_T;

template <ACE_SYNCH_DECL,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType,
          ////////////////////////////////
          typename AddressType,
          typename ConnectionManagerType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_Net_IOReader_T // --> input
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
{
 public:
  Stream_Module_Net_IOReader_T ();
  virtual ~Stream_Module_Net_IOReader_T ();

  // implement (part of) Stream_ITaskBase_T
  virtual void handleControlMessage (ControlMessageType&); // control message

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IOReader_T (const Stream_Module_Net_IOReader_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IOReader_T& operator= (const Stream_Module_Net_IOReader_T&))

  // convenient types
  typedef Stream_Module_Net_IOWriter_T<ACE_SYNCH_USE,
                                       ControlMessageType,
                                       DataMessageType,
                                       SessionMessageType,
                                       ConfigurationType,
                                       StreamControlType,
                                       StreamNotificationType,
                                       StreamStateType,
                                       SessionDataType,
                                       SessionDataContainerType,
                                       StatisticContainerType,
                                       AddressType,
                                       ConnectionManagerType,
                                       UserDataType> WRITER_T;
};

//////////////////////////////////////////

template <ACE_SYNCH_DECL,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename StreamControlType,
          typename StreamNotificationType,
          typename StreamStateType,
          ////////////////////////////////
          typename SessionDataType,          // session data
          typename SessionDataContainerType, // session message payload (reference counted)
          ////////////////////////////////
          typename StatisticContainerType,
          ////////////////////////////////
          typename AddressType,
          typename ConnectionManagerType,
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_Net_IOWriter_T // --> output
 : public Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      UserDataType>
{
  friend class Stream_Module_Net_IOReader_T<ACE_SYNCH_USE,
                                            ControlMessageType,
                                            DataMessageType,
                                            SessionMessageType,
                                            ConfigurationType,
                                            StreamControlType,
                                            StreamNotificationType,
                                            StreamStateType,
                                            SessionDataType,
                                            SessionDataContainerType,
                                            StatisticContainerType,
                                            AddressType,
                                            ConnectionManagerType,
                                            UserDataType>;

 public:
  // convenient types
  typedef Stream_IStream_T<ACE_SYNCH_USE,
                           Common_TimePolicy_t> ISTREAM_T;

  Stream_Module_Net_IOWriter_T (ISTREAM_T* = NULL, // stream handle
                                bool = true);      // generate session messages ?
  virtual ~Stream_Module_Net_IOWriter_T ();

#if defined (__GNUG__) || defined (_MSC_VER)
  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                    Common_TimePolicy_t,
                                    ControlMessageType,
                                    DataMessageType,
                                    SessionMessageType,
                                    ConfigurationType,
                                    StreamControlType,
                                    StreamNotificationType,
                                    StreamStateType,
                                    SessionDataType,
                                    SessionDataContainerType,
                                    StatisticContainerType,
                                    UserDataType>::initialize;
#endif

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IStatistic
  // *NOTE*: implements regular (timer-based) statistic collection
  virtual bool collect (StatisticContainerType&); // return value: (currently unused !)
  //virtual void report () const;

 private:
  // convenient types
  typedef Stream_HeadModuleTaskBase_T<ACE_SYNCH_USE,
                                      Common_TimePolicy_t,
                                      ControlMessageType,
                                      DataMessageType,
                                      SessionMessageType,
                                      ConfigurationType,
                                      StreamControlType,
                                      StreamNotificationType,
                                      StreamStateType,
                                      SessionDataType,
                                      SessionDataContainerType,
                                      StatisticContainerType,
                                      UserDataType> inherited;
  typedef ACE_Message_Queue<ACE_SYNCH_USE,
                            Common_TimePolicy_t> MESSAGEQUEUE_T;

  //  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IOWriter_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IOWriter_T (const Stream_Module_Net_IOWriter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IOWriter_T& operator= (const Stream_Module_Net_IOWriter_T&))

  typename ConnectionManagerType::CONNECTION_T* connection_;
  bool                                          inbound_;
  // *NOTE*: this lock prevents races during (ordered) shutdown
  // *TODO*: remove surplus STREAM_SESSION_END messages
  ACE_SYNCH_MUTEX_T                             lock_;
};

// include template definition
#include "stream_module_io.inl"

#endif
