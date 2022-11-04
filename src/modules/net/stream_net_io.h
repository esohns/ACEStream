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

#ifndef STREAM_NET_IO_H
#define STREAM_NET_IO_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_headmoduletask_base.h"
#include "stream_task_base_synch.h"

extern const char libacestream_default_net_io_module_name_string[];

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
          typename TimerManagerType,
          typename AddressType,
          typename ConnectionManagerType,
          typename UserDataType>
class Stream_Module_Net_IOWriter_T;
class Stream_IOutboundDataNotify;

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
          typename TimerManagerType, // implements Common_ITimer
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
                                 StreamControlType,
                                 StreamNotificationType,
                                 UserDataType>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 Common_TimePolicy_t,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 StreamControlType,
                                 StreamNotificationType,
                                 UserDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Net_IOReader_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_Net_IOReader_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Stream_Module_Net_IOReader_T () {}

  // implement (part of) Stream_ITaskBase_T
  virtual void handleControlMessage (ControlMessageType&); // control message

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IOReader_T ())
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
                                       TimerManagerType,
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
          typename TimerManagerType, // implements Common_ITimer
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
                                      TimerManagerType,
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
                                            TimerManagerType,
                                            AddressType,
                                            ConnectionManagerType,
                                            UserDataType>;

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
                                      TimerManagerType,
                                      UserDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Net_IOWriter_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_Net_IOWriter_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  inline virtual ~Stream_Module_Net_IOWriter_T () {}

#if defined (__GNUG__) || defined (_MSC_VER)
  // *PORTABILITY*: for some reason, this base class member is not exposed
  //                (MSVC/gcc)
  using inherited::STATE_MACHINE_T::initialize;
#endif // __GNUG__ || _MSC_VER

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
  typedef ACE_Message_Queue<ACE_SYNCH_USE,
                            Common_TimePolicy_t> MESSAGEQUEUE_T;
  typedef ACE_Module<ACE_SYNCH_USE,
                     Common_TimePolicy_t> MODULE_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IOWriter_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IOWriter_T (const Stream_Module_Net_IOWriter_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Net_IOWriter_T& operator= (const Stream_Module_Net_IOWriter_T&))

  bool                        inbound_;
  bool                        manageSessionData_;
  Stream_IOutboundDataNotify* outboundNotificationHandle_;
};

// include template definition
#include "stream_net_io.inl"

#endif
